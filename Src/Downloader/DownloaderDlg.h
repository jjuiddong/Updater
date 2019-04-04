//
// Download Dialog
//
#pragma once


class CDownloaderDlg : public CDialogEx
{
public:
	CDownloaderDlg(CWnd* pParent = NULL);	// standard constructor
	enum { IDD = IDD_DOWNLOADER_DIALOG };
	void Run();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void MakeLocalFolder(const string &path, common::sFolderNode *node);
	void MainLoop(const float deltaSeconds);
	void DownloadVersionFile();
	void CheckVersionFile();
	void FinishDownloadFile();
	void Log(const string &msg);
	void LogMessage(const sMessage &state);


protected:
	struct eState {
		enum Enum { CHECK_VERSION, DOWNLOAD, FINISH };
	};

	eState::Enum m_state;
	HICON m_hIcon;
	bool m_isMainLoop;
	bool m_isErrorOccur;
	int m_readTotalBytes;
	cDownloaderConfig m_config;
	cFTPScheduler m_ftpScheduler;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	CListBox m_listLog;
	CProgressCtrl m_progFTP;
	CStatic m_staticProgress;
	CStatic m_staticPercentage;
	CProgressCtrl m_progTotal;
	afx_msg void OnBnClickedButtonHelp();
};
