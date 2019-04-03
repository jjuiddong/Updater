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
	void FinishUpload();
	long CreateUploadFiles(const string &srcDirectory, const string &ftpDirectory
		, const vector<pair<DifferentFileType::Enum, string>> &diffFiles
		, OUT vector<cFTPScheduler::sCommand> &out1, OUT vector<string> &out2);
	bool ZipLastestFiles(const string &dstFileName);
	void LogFTPState(const sMessage &state);
	void Log(const string &msg);
	long GetUploadFileSize();
	static void ZipThreadFunction(CDiffDialog *dlg);


public:
	struct eState { enum Enum { WAIT, ZIP, UPLOAD, FINISH}; };

	eState::Enum m_state;
	bool m_loop;
	bool m_isErrorOccur;
	long m_uploadBytes;
	cUploaderConfig::sProjectInfo m_projInfo;
	cVersionFile m_verFile;
	cFTPScheduler m_ftpScheduler;
	vector<string> m_zipFiles;
	vector<cFTPScheduler::sCommand> m_uploadFileList;
	bool m_isZipLoop;
	long m_zipProgressBytes;
	std::thread m_zipThread;


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
	CStatic m_staticZipFile;
	CStatic m_staticZipPercentage;
	CStatic m_staticUploadFile;
	CStatic m_staticUploadPercentage;
	afx_msg void OnBnClickedButtonAllLastestFileUpload();
};
