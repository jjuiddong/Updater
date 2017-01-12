//
// 2017-01-10, jjuiddong
// FTP Download, Upload Theading Scheduler
//
#pragma once


class cProgressNotify;

class cFTPScheduler
{
public:
	struct eCommandType
	{
		enum Enum { DOWNLOAD, UPLOAD, REMOVE, RENAME };
	};

	struct sCommand
	{
		eCommandType::Enum cmd;
		string remoteFileName;
		string localFileName;
		long fileSize;

		sCommand() {}
		sCommand(const eCommandType::Enum type0,
			const string &remoteFileName0, const string &localFileName0="", const long fileSize0=0)
			: cmd(type0), remoteFileName(remoteFileName0), localFileName(localFileName0), 
			fileSize(fileSize0)
		{
		}
	};

	struct sState
	{
		enum Enum { 
			NONE, 
			LOGIN, 
			DOWNLOAD_BEGIN,
			DOWNLOAD, 
			DOWNLOAD_DONE, 
			UPLOAD_BEGIN,
			UPLOAD, 
			UPLOAD_DONE, 
			ERR, 
			FINISH 
		};

		Enum state;
		int data; // especially ERR state data
		string fileName;
		int totalBytes;
		int readBytes;
		int progressBytes;
	};

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
	int Update(sState &state);
	void CheckFTPFolder();
	void MakeFTPFolder(const string &path, sFolderNode *node);
	void Clear();


public:
	enum STATE {STOP, ERR_STOP, WORK, DONE};

	STATE m_state;
	nsFTP::CFTPClient m_client;
	string m_ftpAddress;
	string m_id;
	string m_passwd;
	string m_ftpDirectory;
	string m_sourceDirectory;
	cProgressNotify *m_observer;

	vector<sTask*> m_taskes;		// FTP Scheduler Task List
	vector<sState*> m_states;		// FTP Scheduler State List to Display External Object

	bool m_loop;
	std::thread m_thread;
	CriticalSection m_csTask;
	CriticalSection m_csState;
};
