
#include "stdafx.h"
#include "FTPScheduler.h"
#include "FTPProgress.h"
#include "FileList.h"
#include "../ZipLib/ZipFile.h"
#include "../ZipLib/streams/memstream.h"
#include "../ZipLib/methods/Bzip2Method.h"


string ftppath::Decryption(const string &str)
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
		m_passwd = ftppath::Decryption(passwd);

	return true;
}


// add Command
void cFTPScheduler::AddFileList(const cFileList &files)
{
	for (auto &file : files.m_files)
	{
		sTask *t = new sTask;
		t->type = file.cmd;
		t->fileName = file.fileName;
		t->isCompressed = file.isCompressed;
		t->downloadFileSize = file.downloadFileSize;
		m_taskes.push(t);
	}
}


void cFTPScheduler::ClearFileList()
{
	m_taskes.clear();
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
			if (task->fileName == "version.ver")
				continue; // ignore version file
			files.push_back(task->fileName);
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
	string remoteFileName = ftppath::GetRemoteFileName(m_ftpDirectory, task.fileName);
	string localFileName = ftppath::GetLocalFileName(m_sourceDirectory, task.fileName);
	if (task.isCompressed)
	{
		remoteFileName += ".zip";
		localFileName += ".zip";
	}

	// Upload
	if (m_client.UploadFile(str2wstr(localFileName), str2wstr(remoteFileName)
		, false, nsFTP::CRepresentation(nsFTP::CType::Image()), true))
	{
		// nothing~
		return true;
	}
	else
	{
		g_message.push(new sMessage(sMessage::ERR, task.fileName
			, 0, 0, 0, sMessage::UPLOAD, "FTP Upload Error"));
		return false;
	}

	return true;
}


// Download File
bool cFTPScheduler::Download(const sTask &task)
{
	if (m_observer)
	{
		m_observer->m_fileSize = task.downloadFileSize;
		m_observer->m_sourceFileName = task.fileName;
	}

	string remoteFileName = ftppath::GetRemoteFileName(m_ftpDirectory, task.fileName);
	string localFileName = ftppath::GetLocalFileName(m_sourceDirectory, task.fileName);
	if (task.isCompressed)
	{
		remoteFileName += ".zip";
		localFileName += ".zip";
	}

	if (m_client.DownloadFile(str2wstr(remoteFileName), str2wstr(localFileName)
		, nsFTP::CRepresentation(nsFTP::CType::Image()), true ))
	{
		return true;
	}
	else
	{
		g_message.push(new sMessage(sMessage::ERR, task.fileName
			, 0, 0, 0, sMessage::DOWNLOAD, "FTP Download Error"));
		return false;
	}

	return true;
}


// FTP Thread Function
unsigned cFTPScheduler::FTPSchedulerThread(cFTPScheduler *ftp)
{
	// FTP Login
	dbg::Logc(1, "FTP Login ...%s, %s\n"
		, ftp->m_ftpAddress.c_str()
		, ftp->m_id.c_str());

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

		dbg::Logc(1, "FTP Login Fail\n");
		return 0;
	}

	dbg::Logc(1, "FTP Login Success\n");

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
			string remoteFileName = ftppath::GetRemoteFileName(
				ftp->m_ftpDirectory, task->fileName);
			if (task->isCompressed)
				remoteFileName += ".zip";
			ftp->m_client.Delete(str2wstr(remoteFileName));
		}
		break;
		}

		ftp->m_taskes.pop();

		//if (!result)
		//	break; // error occured
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



// return FTP FileName
// ftp path must be \\ -> /
string ftppath::GetRemoteFileName(const string &ftpDirectory, const string &fileName)
{
	string remoteFileName = ftpDirectory + "\\" + fileName;
	common::replaceAll(remoteFileName, "\\", "/");
	return remoteFileName;
}


// return Local Directory FileName
string ftppath::GetLocalFileName(const string &sourceDirectory, const string &fileName)
{
	const string srcFullDirectory = CheckDirectoryPath(GetFullFileName(sourceDirectory) 
		+ "\\");
	const string localFileName = srcFullDirectory + fileName;
	return localFileName;
}
