//
// 2017-01-11, jjuiddong
// - Upload Different Source & Lastest Files
//		- ZipFile Upload
//
#pragma once
#include "afxcmn.h"
#include "UploaderConfig.h"
#include "afxwin.h"
#include "../FileCompare/FTPScheduler.h"


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
	void CheckFTPFolder(nsFTP::CFTPClient &client, vector<pair<DifferentFileType::Enum, string>> &diffFiles);
	void CheckLocalFolder(vector<pair<DifferentFileType::Enum, string>> &diffFiles);
	void MakeFTPFolder(nsFTP::CFTPClient &client, const string &path, sFolderNode *node);
	void MakeLocalFolder(const string &path, sFolderNode *node);
	cVersionFile CreateVersionFile(const string &srcDirectoryPath, vector<pair<DifferentFileType::Enum, string>> &diffFiles);
	void MainLoop(const float deltaSeconds);
	void FinishUpload();
	void LogFTPState(const cFTPScheduler::sState &state);
	void Log(const string &msg);


public:
	struct eState { enum Enum { WAIT, UPLOAD, FINISH,}; };

	eState::Enum m_state;
	bool m_loop;
	bool m_isErrorOccur;
	long m_writeTotalBytes;
	cUploaderConfig::sProjectInfo m_projInfo;
	cFTPScheduler m_ftpScheduler;
	vector<string> m_zipFiles;


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
	CProgressCtrl m_progTotal;
	CProgressCtrl m_progUpload;
	CStatic m_staticUploadFile;
	CStatic m_staticUploadPercentage;
};
