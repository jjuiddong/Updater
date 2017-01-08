#pragma once
#include "afxcmn.h"
#include "UploaderConfig.h"
#include "afxwin.h"


// CDiffDialog dialog
class CDiffDialog : public CDialogEx
{
public:
	CDiffDialog(CWnd* pParent, const cUploaderConfig::sProjectInfo &info);   // standard constructor
	virtual ~CDiffDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DIFF };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	void CheckFTPFolder(nsFTP::CFTPClient &client, vector<pair<DifferentFileType::Enum, string>> &diffFiles);
	void CheckLocalFolder(vector<pair<DifferentFileType::Enum, string>> &diffFiles);
	void MakeFTPFolder(nsFTP::CFTPClient &client, const string &path, sFolderNode *node);
	void MakeLocalFolder(const string &path, sFolderNode *node);
	cVersionFile CreateVersionFile(const string &srcDirectoryPath, vector<pair<DifferentFileType::Enum, string>> &diffFiles);


public:
	cUploaderConfig::sProjectInfo m_projInfo;


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
};
