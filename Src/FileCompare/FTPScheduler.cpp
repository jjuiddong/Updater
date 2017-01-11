
#include "stdafx.h"
#include "FTPScheduler.h"


unsigned WINAPI FTPSchedulerThread(cFTPScheduler *scheduler);


//--------------------------------------------------------------------------------------------------
// cProgressNotify
class cProgressNotify : public nsFTP::CFTPClient::CNotification
{
public:
	cProgressNotify(cFTPScheduler *p) :m_p(p) {}

	virtual void OnPreReceiveFile(const tstring& strSourceFile, const tstring& strTargetFile, long lFileSize)
	{
		cFTPScheduler::sState *s = new cFTPScheduler::sState;
		s->state = cFTPScheduler::sState::DOWNLOAD_BEGIN;
		s->fileName = wstr2str(strTargetFile);
		s->totalBytes = lFileSize;
		s->progressBytes = 0;

		m_fileSize = lFileSize;
		m_fileName = wstr2str(strTargetFile);

		AutoCSLock scs(m_p->m_csState);
		m_p->m_states.push_back(s);
	}

	virtual void OnBytesReceived(const nsFTP::TByteVector& vBuffer, long lReceivedBytes)
	{
		cFTPScheduler::sState *s = new cFTPScheduler::sState;
		s->state = cFTPScheduler::sState::DOWNLOAD;
		s->fileName = m_fileName;
		s->totalBytes = m_fileSize;
		s->progressBytes = lReceivedBytes;

		AutoCSLock scs(m_p->m_csState);
		m_p->m_states.push_back(s);
	}

	virtual void OnEndReceivingData(long lReceivedBytes) 
	{
	}

	virtual void OnPostReceiveFile(const tstring& strSourceFile, const tstring& strTargetFile, long lFileSize)
	{
		cFTPScheduler::sState *s = new cFTPScheduler::sState;
		s->state = cFTPScheduler::sState::DOWNLOAD_DONE;
		s->fileName = m_fileName;
		s->totalBytes = m_fileSize;
		s->progressBytes = m_fileSize;

		AutoCSLock scs(m_p->m_csState);
		m_p->m_states.push_back(s);
	}

	cFTPScheduler *m_p;
	string m_fileName;
	int m_fileSize;
};



//--------------------------------------------------------------------------------------------------
// cFTPScheduler
cFTPScheduler::cFTPScheduler()
	: m_state(STOP)
	, m_loop(false)
{
	m_observer = new cProgressNotify(this);
	m_client.AttachObserver(m_observer);
}

cFTPScheduler::~cFTPScheduler()
{
	Clear();
	SAFE_DELETE(m_observer);
}


// Initialize FTP Scheduler
bool cFTPScheduler::Init(const string &ftpAddress, const string &id, const string &passwd)
{
	if (STOP != m_state)
		Clear();

	m_ftpAddress = ftpAddress;
	m_id = id;
	m_passwd = passwd;
	m_client.SetResumeMode(false);

	return true;
}


// add Command
void cFTPScheduler::AddCommand(const vector<sCommand> &files)
{
	AutoCSLock cs(m_csTask);

	for each (auto file in files)
	{
		sTask *t = new sTask;
		t->type = file.cmd;
		t->remoteFileName = file.remoteFileName;
		t->localFileName = file.localFileName;
		t->fileSize = file.fileSize;
		m_taskes.push_back(t);
	}	
}


// Start Task
bool cFTPScheduler::Start()
{
	if (WORK == m_state)
		return false;

	m_loop = false;
	if (m_thread.joinable())
		m_thread.join();

	m_state = WORK;
	m_loop = true;
	m_thread = std::thread(FTPSchedulerThread, this);

	return true;
}


// Update Task State
// return value : 0 = stop threading
//						  1 = start 
int cFTPScheduler::Update(sState &state)
{
	if (STOP == m_state)
		return 0;

	AutoCSLock cs(m_csState);
	if (m_states.empty())
	{
		state.state = sState::NONE;
	}
	else
	{
		state = *m_states.front();
		rotatepopvector(m_states, 0);
	}

	return 1;
}


void cFTPScheduler::Clear()
{
	m_loop = false;
	if (m_thread.joinable())
		m_thread.join();

	{
		AutoCSLock cs(m_csTask);
		for each (auto p in m_taskes)
			delete p;
		m_taskes.clear();
	}

	{
		AutoCSLock cs(m_csState);
		for each (auto p in m_states)
			delete p;
		m_states.clear();
	}

	m_client.Logout();
}


unsigned WINAPI FTPSchedulerThread(cFTPScheduler *scheduler)
{

	// FTP Login
	scheduler->m_client.SetResumeMode(false);
	nsFTP::CLogonInfo info(str2wstr(scheduler->m_ftpAddress), 21,
		str2wstr(scheduler->m_id), str2wstr(scheduler->m_passwd));
	if (!scheduler->m_client.Login(info))
	{
		cFTPScheduler::sState *s = new cFTPScheduler::sState;
		s->state = cFTPScheduler::sState::ERR;
		s->data = cFTPScheduler::sState::LOGIN;

		AutoCSLock scs(scheduler->m_csState);
		scheduler->m_states.push_back(s);

		scheduler->m_state = cFTPScheduler::ERR_STOP;
		return 0;
	}
	
	// FTP Task work
	while (scheduler->m_client.IsConnected() && scheduler->m_loop)
	{
		{
			AutoCSLock cs(scheduler->m_csTask);
			if (scheduler->m_taskes.empty())
				break;

			cFTPScheduler::sTask *t = scheduler->m_taskes.front();
			switch (t->type)
			{
			case cFTPScheduler::eCommandType::DOWNLOAD:
			{
				if (scheduler->m_client.DownloadFile(str2wstr(t->remoteFileName),
					str2wstr(t->localFileName)))
				{
				}
				else
				{
					cFTPScheduler::sState *s = new cFTPScheduler::sState;
					s->state = cFTPScheduler::sState::ERR;
					s->data = cFTPScheduler::sState::DOWNLOAD;
					s->fileName = t->localFileName;

					AutoCSLock scs(scheduler->m_csState);
					scheduler->m_states.push_back(s);
				}
			}
			break;

			case cFTPScheduler::eCommandType::UPLOAD:
			{
				if (scheduler->m_client.UploadFile(str2wstr(t->localFileName),
					str2wstr(t->remoteFileName)))
				{
				}
				else
				{
					cFTPScheduler::sState *s = new cFTPScheduler::sState;
					s->state = cFTPScheduler::sState::ERR;
					s->data = cFTPScheduler::sState::UPLOAD;
					s->fileName = t->localFileName;

					AutoCSLock scs(scheduler->m_csState);
					scheduler->m_states.push_back(s);
				}
			}
			break;
			}

			common::removevector(scheduler->m_taskes, t);
			delete t;
		}
	}

	if (!scheduler->m_client.IsConnected())
	{
		scheduler->m_state = cFTPScheduler::ERR_STOP;
		return 0;
	}

	// Finish Download
	{
		cFTPScheduler::sState *s = new cFTPScheduler::sState;
		s->state = cFTPScheduler::sState::FINISH;
		AutoCSLock scs(scheduler->m_csState);
		scheduler->m_states.push_back(s);
	}

	scheduler->m_client.Logout();
	scheduler->m_state = cFTPScheduler::DONE;
	return 0;
}
