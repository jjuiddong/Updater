//
// 2017-01-10, jjuiddong
// FTP Download, Upload/Download Theading Scheduler
//
#pragma once


class cProgressNotify;

class cFTPScheduler
{
public:
	struct eCommandType {
		enum Enum { DOWNLOAD, UPLOAD, REMOVE, RENAME };
	};

	struct sCommand 
	{
		eCommandType::Enum cmd;
		string srcFileName; // source file name (except directory path)
		string remoteFileName;
		string localFileName;
		string zipFileName;
		long fileSize; // file size (after compressed)
		long srcFileSize; // source file size (befor compressed)

		sCommand() {}
		sCommand(const eCommandType::Enum type0
			, const string &srcFileName0
			, const string &remoteFileName0
			, const string &localFileName0=""
			, const string &zipFileName0 = ""
			, const long fileSize0=0
			, const long srcFileSize0 = 0)
			: cmd(type0), srcFileName(srcFileName0), remoteFileName(remoteFileName0)
				, localFileName(localFileName0),zipFileName(zipFileName0)
				, fileSize(fileSize0), srcFileSize(srcFileSize0)
		{
		}
	};

	// thread command
	struct sTask
	{
		eCommandType::Enum type;
		string remoteFileName;
		string localFileName;
		long fileSize;
	};
	

	cFTPScheduler();
	virtual ~cFTPScheduler();

	bool Init(const string &ftpAddress, const string &id, const string &passwd, 
		const string &ftpDirectory="", const string &sourceDirectory="");
	void AddCommand(const vector<sCommand> &files);
	bool Start();
	void CheckFTPFolder();
	void MakeFTPFolder(const string &path, sFolderNode *node);
	bool Upload(const sTask &task);
	bool Download(const sTask &task);
	void Clear();
	static unsigned FTPSchedulerThread(cFTPScheduler *scheduler);


public:
	enum STATE {STOP, ERR_STOP, WORK, DONE};

	STATE m_state;
	nsFTP::CFTPClient m_client;
	string m_ftpAddress;
	string m_id;
	string m_passwd;
	string m_ftpDirectory; // remote ftp destination directory
	string m_sourceDirectory; // local source directory
	cProgressNotify *m_observer;
	cSyncQueue<sTask*, true> m_taskes; // FTP Scheduler Task List

	bool m_loop;
	std::thread m_thread;
};
