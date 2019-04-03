
#include "FTPScheduler.h"
#include "stdafx.h"
#include "../ZipLib/ZipFile.h"
#include "../ZipLib/streams/memstream.h"
#include "../ZipLib/methods/Bzip2Method.h"

unsigned FTPSchedulerThread(cFTPScheduler *scheduler);


//--------------------------------------------------------------------------------------------------
// cProgressNotify
class cProgressNotify : public nsFTP::CFTPClient::CNotification
{
public:
	cProgressNotify(cFTPScheduler *p) :m_p(p) {}

	virtual void OnPreReceiveFile(const tstring& strSourceFile, const tstring& strTargetFile
		, long lFileSize)
	{
		sMessage *s = new sMessage;
		s->type = sMessage::DOWNLOAD_BEGIN;
		s->fileName = wstr2str(strTargetFile);
		s->totalBytes = lFileSize;
		s->progressBytes = 0;

		m_progress = 0;
		m_fileName = wstr2str(strTargetFile);

		g_message.push(s);
	}

	virtual void OnBytesReceived(const nsFTP::TByteVector& vBuffer, long lReceivedBytes)
	{
		sMessage *s = new sMessage;
		s->type = sMessage::DOWNLOAD;
		s->fileName = m_fileName;
		s->totalBytes = m_fileSize;

		m_progress += lReceivedBytes;
		s->readBytes = lReceivedBytes;
		s->progressBytes = m_progress;

		g_message.push(s);
	}

	virtual void OnEndReceivingData(long lReceivedBytes) 
	{
	}

	virtual void OnPostReceiveFile(const tstring& strSourceFile, const tstring& strTargetFile, long lFileSize)
	{
		sMessage *s = new sMessage;
		s->type = sMessage::DOWNLOAD_DONE;
		s->fileName = m_fileName;
		s->totalBytes = m_fileSize;
		s->progressBytes = m_fileSize;

		g_message.push(s);
	}


	virtual void OnPreSendFile(const tstring& strSourceFile, const tstring& strTargetFile, long lFileSize) 
	{
		sMessage *s = new sMessage;
		s->type = sMessage::UPLOAD_BEGIN;
		s->fileName = wstr2str(strSourceFile);
		s->totalBytes = lFileSize;
		s->progressBytes = 0;

		m_progress = 0;
		m_fileSize = lFileSize;
		m_fileName = wstr2str(strSourceFile);

		g_message.push(s);
	}

	virtual void OnBytesSent(const nsFTP::TByteVector& vBuffer, long lSentBytes)
	{
		sMessage *s = new sMessage;
		s->type = sMessage::UPLOAD;
		s->fileName = m_fileName;
		s->totalBytes = m_fileSize;

		m_progress += lSentBytes;
		s->readBytes = lSentBytes;
		s->progressBytes = m_progress;

		g_message.push(s);
	}

	virtual void OnPostSendFile(const tstring& strSourceFile, const tstring& strTargetFile, long lFileSize) 
	{
		sMessage *s = new sMessage;
		s->type = sMessage::UPLOAD_DONE;
		s->fileName = m_fileName;
		s->totalBytes = m_fileSize;
		s->progressBytes = m_fileSize;

		g_message.push(s);
	}

	cFTPScheduler *m_p; // reference
	string m_fileName;
	long m_fileSize;
	long m_progress;
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


string Decryption(const string &str)
{
	const int MAGIC1 = 11;
	const int MAGIC2 = 3;
	string ret;
	for (int i = 0; i < (int)str.size(); ++i)
	{
		const char c = str[i];
		if (isalpha(c))
			ret += (char)((c - 'a' - MAGIC1 + ('z' - 'a')) % ('z' - 'a') + 'a');
		else if (isdigit(c))
			ret += (char)((c - '0' - MAGIC2 + ('9' - '0')) % ('9' - '0') + '0');
		else
			assert(!"Decryption Error");
	}
	return ret;
}


// Initialize FTP Scheduler
bool cFTPScheduler::Init(const string &ftpAddress, const string &id, const string &passwd, 
	const string &ftpDirectory, const string &sourceDirectory)
{
	if (STOP != m_state)
		Clear();

	m_ftpAddress = ftpAddress;
	m_id = id;
	m_passwd = passwd;
	m_ftpDirectory = ftpDirectory;
	m_sourceDirectory = sourceDirectory;
	m_client.SetResumeMode(false);

	if (ftpAddress == "jjuiddong.co.kr")
		m_passwd = Decryption(passwd);

	return true;
}


// add Command
void cFTPScheduler::AddCommand(const vector<sCommand> &files)
{
	for (auto &file : files)
	{
		sTask *t = new sTask;
		t->type = file.cmd;
		t->remoteFileName = file.remoteFileName;
		t->localFileName = (file.zipFileName.empty())? file.localFileName : file.zipFileName;
		t->fileSize = file.fileSize;
		m_taskes.push(t);
	}
}


// Check Upload Folder. If Not Exist, Make Folder
void cFTPScheduler::CheckFTPFolder()
{
	const string sourceDirectory = m_sourceDirectory + "/";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);

	// Collect  Folders
	// change relative file path
	list<string> files;
	for (auto &task : m_taskes.m_q)
	{
		if (eCommandType::UPLOAD == task->type)
		{
			if (GetFileName(task->localFileName) == "version.ver")
				continue; // ignore version file

			const string fileName = DeleteCurrentPath(
				RelativePathTo(sourceFullDirectory, task->localFileName));
			files.push_back(fileName);
		}
	}

	vector<string> folders;
	sFolderNode *root = CreateFolderNode(files);
	MakeFTPFolder(m_ftpDirectory, root);
	DeleteFolderNode(root);
}


void cFTPScheduler::MakeFTPFolder(const string &path, sFolderNode *node)
{
	RET(!node);

	for (auto &child : node->children)
	{
		const string folderName = path + "/" + child.first;
		m_client.MakeDirectory(str2wstr(folderName));

		MakeFTPFolder(folderName, child.second);
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
//int cFTPScheduler::Update(sState &state)
//{
//	if (STOP == m_state)
//		return 0;
//
//	if (m_states.empty())
//	{
//		state.state = sState::NONE;
//	}
//	else
//	{
//		state = *m_states.front();
//		delete m_states.front();
//		rotatepopvector(m_states, 0);
//	}
//
//	return 1;
//}


void cFTPScheduler::Clear()
{
	m_loop = false;
	if (m_thread.joinable())
		m_thread.join();

	m_taskes.clear();
	m_client.Logout();
}


// Upload File
bool cFTPScheduler::Upload(const sTask &task)
{
	// Upload
	if (m_client.UploadFile(str2wstr(task.localFileName),
		str2wstr(task.remoteFileName)))
	{
		// nothing~
	}
	else
	{
		sMessage *s = new sMessage;
		s->type = sMessage::ERR;
		s->data = sMessage::UPLOAD;
		s->fileName = task.localFileName;

		g_message.push(s);
	}

	return true;
}


// Download File
bool cFTPScheduler::Download(const sTask &task)
{
	if (m_observer)
		m_observer->m_fileSize = task.fileSize;

	if (m_client.DownloadFile(str2wstr(task.remoteFileName),
		str2wstr(task.localFileName)))
	{
	}
	else
	{
		sMessage *s = new sMessage;
		s->type = sMessage::ERR;
		s->data = sMessage::DOWNLOAD;
		s->fileName = task.localFileName;

		g_message.push(s);
	}

	return true;
}


unsigned FTPSchedulerThread(cFTPScheduler *scheduler)
{
	// FTP Login
	scheduler->m_client.SetResumeMode(false);
	nsFTP::CLogonInfo info(str2wstr(scheduler->m_ftpAddress), 21,
		str2wstr(scheduler->m_id), str2wstr(scheduler->m_passwd));
	if (!scheduler->m_client.Login(info))
	{
		sMessage *s = new sMessage;
		s->type = sMessage::ERR;
		s->data = sMessage::LOGIN;

		g_message.push(s);

		scheduler->m_state = cFTPScheduler::ERR_STOP;
		return 0;
	}
	
	// If Need, Make FTP Folder
	scheduler->CheckFTPFolder();
	
	// FTP Task work
	while (scheduler->m_client.IsConnected() && scheduler->m_loop)
	{
		if (scheduler->m_taskes.empty())
			break;

		cFTPScheduler::sTask *task = NULL;
		if (!scheduler->m_taskes.front(task))
			break;

		switch (task->type)
		{
		case cFTPScheduler::eCommandType::DOWNLOAD:
			scheduler->Download(*task);
			break;

		case cFTPScheduler::eCommandType::UPLOAD:
			scheduler->Upload(*task);
			break;

		case cFTPScheduler::eCommandType::REMOVE:
			scheduler->m_client.Delete(str2wstr(task->remoteFileName));
			break;
		}

		scheduler->m_taskes.pop();
	}

	if (!scheduler->m_client.IsConnected())
	{
		scheduler->m_state = cFTPScheduler::ERR_STOP;
		return 0;
	}

	// Finish Task
	{
		sMessage *s = new sMessage;
		s->type = sMessage::FINISH;
		g_message.push(s);
	}

	scheduler->m_client.Logout();
	scheduler->m_state = cFTPScheduler::DONE;
	return 0;
}
