//
// 2017-01-10, jjuiddong
// FTP Download, Upload/Download Theading Scheduler
//
#pragma once


class cFileList;
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
		string fileName; // source file name (except directory path)
		bool isCompressed;
		long downloadFileSize;

		sCommand() {}
		sCommand(const eCommandType::Enum type
			, const string &fileName0
			, const bool isCompressed0 = false
			, const long downloadFileSize0 = 0)
			: cmd(type), fileName(fileName0), isCompressed(isCompressed0)
			, downloadFileSize(downloadFileSize0)
		{
		}
	};

	// thread command
	struct sTask
	{
		eCommandType::Enum type;
		string fileName; // use download/upload
		bool isCompressed; // use upload
		long downloadFileSize;
	};
	

	cFTPScheduler();
	virtual ~cFTPScheduler();

	bool Init(const string &ftpAddress, const string &id, const string &passwd, 
		const string &ftpDirectory="", const string &sourceDirectory="");
	void AddFileList(const cFileList &files);
	bool Start();
	void CheckFTPFolder();
	void MakeFTPFolder(const string &path, sFolderNode *node);
	bool Upload(const sTask &task);
	bool Download(const sTask &task);
	void ClearFileList();
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


namespace ftppath 
{
	string Decryption(const string &str);

	string GetRemoteFileName(const string &ftpDirectory, const string &fileName);
	string GetLocalFileName(const string &sourceDirectory, const string &fileName);
}
