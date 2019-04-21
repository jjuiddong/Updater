//
// 2017-01-11, jjuiddong
// - Upload Different Source & Lastest Files
//		- ZipFile Upload
//
#pragma once


// CDiffDialog dialog
class CDiffDialog : public CDialogEx
{
public:
	CDiffDialog(CWnd* pParent, const cUploaderConfig::sProjectInfo &info);   // standard constructor
	virtual ~CDiffDialog();
	enum { IDD = IDD_DIALOG_DIFF };

	void Run();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void CheckLocalFolder(const vector<pair<DifferentFileType::Enum, string>> &diffFiles);
	void MakeLocalFolder(const string &path, sFolderNode *node);
	cVersionFile CreateVersionFile(const string &srcDirectoryPath
		, vector<pair<DifferentFileType::Enum, string>> &diffFiles);
	void MainLoop(const float deltaSeconds);
	void ZipStateProcess(const sMessage &message);
	void UploadStateProcess(const sMessage &message);
	void BackupStateProcess(const sMessage &message);
	void FinishUpload();
	long CreateUploadFiles(const string &srcDirectory
		, const vector<pair<DifferentFileType::Enum, string>> &diffFiles
		, OUT cFileList &out);
	void LogMessage(const sMessage &state);
	void Log(const string &msg);
	void TerminateThread();

	static void ZipThreadFunction(CDiffDialog *dlg);
	static void LastestThreadFunction(CDiffDialog *dlg);
	static void BackupThreadFunction(CDiffDialog *dlg);


public:
	struct eState { 
		enum Enum { WAIT, ZIP, UPLOAD, BACKUP, FINISH}; 
	};

	eState::Enum m_state;
	bool m_isMainLoop;
	bool m_isErrorOccur;
	bool m_isLastestUpload; // lastest폴더 파일만 업로드할 경우 true, (ignore backup process)
	string m_sourceDirectoryPath; // source file directory path (source / lastest directory path)
	cUploaderConfig::sProjectInfo m_projInfo;
	cFTPScheduler m_ftpScheduler;
	cVersionFile m_verFile;
	cFileList m_uploadFiles;
	vector<pair<DifferentFileType::Enum, string>> m_diffFiles;
	
	bool m_isZipLoop;
	long m_zipProgressBytes;
	long m_zipTotalBytes;
	std::thread m_zipThread;
	long m_uploadProgressBytes;
	long m_uploadTotalBytes;
	bool m_isBackupLoop;
	long m_backupProgressBytes;
	long m_backupTotalBytes;
	std::thread m_backupThread;
	bool m_isLastestLoop;
	long m_lastestProgressBytes;
	long m_lastestTotalBytes;
	std::thread m_lastestThread;

	bool m_backupType; // 0:copy lastest folder, 1: copy lastest folder & zip backup file
	int m_backupProcess; // check process, copy lastest folder, zip backup file

	DECLARE_MESSAGE_MAP()
	DECLARE_ANCHOR_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonUpload();
	CListCtrl m_listDiff;
	virtual BOOL OnInitDialog();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	CListBox m_listLog;
	CProgressCtrl m_progZip;
	CProgressCtrl m_progTotal;
	CProgressCtrl m_progUpload;
	CProgressCtrl m_progBackup;
	CProgressCtrl m_progLastest;
	CStatic m_staticZipFile;
	CStatic m_staticZipPercentage;
	CStatic m_staticUploadFile;
	CStatic m_staticUploadPercentage;
	CStatic m_staticBackupFile;
	CStatic m_staticBackupPercentage;
	CStatic m_staticLastestFile;
	CStatic m_staticLastestPercentage;
	BOOL m_checkBackup;
	afx_msg void OnBnClickedButtonAllLastestFileUpload();
};
