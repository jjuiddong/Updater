#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "UploaderConfig.h"


// CUploaderDlg dialog
class CUploaderDlg : public CDialogEx
{
public:
	CUploaderDlg(CWnd* pParent = NULL);	// standard constructor
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPLOADER_DIALOG };
#endif


protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void CheckFTPFolder(nsFTP::CFTPClient &client, cUploaderConfig::sProjectInfo *projInfo, 
		vector<std::pair<int, string>> &diffFiles);
	void CheckLocalFolder(cUploaderConfig::sProjectInfo *projInfo, vector<std::pair<int, string>> &diffFiles);
	void MakeFTPFolder(nsFTP::CFTPClient &client, const string &path, CFileTreeCtrl::sTreeNode *node);
	void MakeLocalFolder(const string &path, CFileTreeCtrl::sTreeNode *node);


// Implementation
protected:
	HICON m_hIcon;
	CFileTreeCtrl m_srcFileTree;
	cUploaderConfig m_config;


	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	DECLARE_ANCHOR_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonUpload();
	afx_msg void OnBnClickedButtonDownload();
	CMFCEditBrowseCtrl m_browsSrcDir;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonRead();
	CListBox m_listLog;
	CComboBox m_comboProject;
};
