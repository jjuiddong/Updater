
#include "FTPScheduler.h"
#include "stdafx.h"
#include "../ZipLib/ZipFile.h"
#include "../ZipLib/streams/memstream.h"
#include "../ZipLib/methods/Bzip2Method.h"


//--------------------------------------------------------------------------------------------------
// cProgressNotify
class cProgressNotify : public nsFTP::CFTPClient::CNotification
{
public:
	cProgressNotify(cFTPScheduler *p) :m_p(p) {}

	virtual void OnPreReceiveFile(const tstring& strSourceFile, const tstring& strTargetFile
		, long lFileSize)
	{
		g_message.push(new sMessage(sMessage::DOWNLOAD_BEGIN
			, wstr2str(strTargetFile), lFileSize));

		m_progress = 0;
		m_fileName = wstr2str(strTargetFile);
	}

	virtual void OnBytesReceived(const nsFTP::TByteVector& vBuffer, long lReceivedBytes)
	{
		m_progress += lReceivedBytes;

		g_message.push(new sMessage(sMessage::DOWNLOAD
			, m_fileName, m_fileSize, lReceivedBytes, m_progress));
	}

	virtual void OnEndReceivingData(long lReceivedBytes) 
	{
	}

	virtual void OnPostReceiveFile(const tstring& strSourceFile, const tstring& strTargetFile, long lFileSize)
	{
		g_message.push(new sMessage(sMessage::DOWNLOAD_DONE
			, m_fileName, m_fileSize, 0, m_fileSize));
	}


	virtual void OnPreSendFile(const tstring& strSourceFile, const tstring& strTargetFile, long lFileSize) 
	{
		g_message.push(new sMessage(sMessage::UPLOAD_BEGIN
			, wstr2str(strSourceFile), lFileSize, 0, 0));

		m_progress = 0;
		m_fileSize = lFileSize;
		m_fileName = wstr2str(strSourceFile);
	}

	virtual void OnBytesSent(const nsFTP::TByteVector& vBuffer, long lSentBytes)
	{
		m_progress += lSentBytes;

		g_message.push(new sMessage(sMessage::UPLOAD
			, m_fileName, m_fileSize, lSentBytes, m_progress));
	}

	virtual void OnPostSendFile(const tstring& strSourceFile, const tstring& strTargetFile, long lFileSize) 
	{
		g_message.push(new sMessage(sMessage::UPLOAD_DONE
			, m_fileName, m_fileSize, 0, m_fileSize));
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
	const string sourceDirectory = m_sourceDirectory + "\\";
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


void cFTPScheduler::Clear()
{
	m_state = STOP;

	m_loop = false;
	if (m_thread.joinable())
		m_thread.join();

	m_taskes.clear();
	m_client.Logout();
}


// Upload File
bool cFTPScheduler::Upload(const sTask &task)
{
	// ftp path must \\ -> /
	string remoteFileName = task.remoteFileName;
	common::replaceAll(remoteFileName, "\\", "/");

	// Upload
	if (m_client.UploadFile(str2wstr(task.localFileName),
		str2wstr(remoteFileName)))
	{
		// nothing~
		return true;
	}
	else
	{
		g_message.push(new sMessage(sMessage::ERR, task.localFileName
			, 0, 0, 0, sMessage::UPLOAD, "FTP Upload Error"));
		return false;
	}

	return true;
}


// Download File
bool cFTPScheduler::Download(const sTask &task)
{
	if (m_observer)
		m_observer->m_fileSize = task.fileSize;

	// ftp path must \\ -> /
	string remoteFileName = task.remoteFileName;
	common::replaceAll(remoteFileName, "\\", "/");

	if (m_client.DownloadFile(str2wstr(remoteFileName),
		str2wstr(task.localFileName)))
	{
		return true;
	}
	else
	{
		g_message.push(new sMessage(sMessage::ERR, task.localFileName
			, 0, 0, 0, sMessage::DOWNLOAD, "FTP Download Error"));
		return false;
	}

	return true;
}


unsigned cFTPScheduler::FTPSchedulerThread(cFTPScheduler *ftp)
{
	// FTP Login
	ftp->m_client.SetResumeMode(false);
	nsFTP::CLogonInfo info(str2wstr(ftp->m_ftpAddress), 21,
		str2wstr(ftp->m_id), str2wstr(ftp->m_passwd));
	if (!ftp->m_client.Login(info))
	{
		g_message.push(new sMessage(sMessage::ERR, "", 0, 0, 0, sMessage::LOGIN
			, "FTP Login Error"));

		g_message.push(new sMessage(sMessage::FINISH, "", 0, 0, 0, sMessage::LOGIN
			, "Finish, Fail Work"));

		ftp->m_state = cFTPScheduler::ERR_STOP;
		return 0;
	}
	
	// If Need, Make FTP Folder
	ftp->CheckFTPFolder();
	
	// FTP Task work
	while (ftp->m_client.IsConnected() && ftp->m_loop)
	{
		if (ftp->m_taskes.empty())
			break;

		cFTPScheduler::sTask *task = NULL;
		if (!ftp->m_taskes.front(task))
			break;

		bool result = true;
		switch (task->type)
		{
		case cFTPScheduler::eCommandType::DOWNLOAD:
			result = ftp->Download(*task);
			break;

		case cFTPScheduler::eCommandType::UPLOAD:
			result = ftp->Upload(*task);
			break;

		case cFTPScheduler::eCommandType::REMOVE:
		{
			// ftp path must \\ -> /
			string remoteFileName = task->remoteFileName;
			common::replaceAll(remoteFileName, "\\", "/");
			ftp->m_client.Delete(str2wstr(remoteFileName));
		}
		break;
		}

		ftp->m_taskes.pop();

		if (!result)
			break; // error occured
	}

	ftp->m_taskes.clear();

	if (!ftp->m_client.IsConnected())
	{
		ftp->m_state = cFTPScheduler::ERR_STOP;
		return 0;
	}

	// Finish Task
	g_message.push(new sMessage(sMessage::FINISH, ""));

	ftp->m_client.Logout();
	ftp->m_state = cFTPScheduler::DONE;
	return 0;
}
