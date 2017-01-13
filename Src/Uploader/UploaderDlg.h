#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "afxeditbrowsectrl.h"
#include "afxshelltreectrl.h"



// CUploaderDlg dialog
class CUploaderDlg : public CDialogEx
{
public:
	CUploaderDlg(CWnd* pParent = NULL);	// standard constructor
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_UPLOADER_DIALOG };
#endif

	void UpdateProjectInfo();
	void RefreshProjectInfo();


protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;
	CFileTreeCtrl m_srcFileTree;
	CFileTreeCtrl m_treeLastFiles;
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
	CMFCEditBrowseCtrl m_browsSrcDir;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonRead();
	CListBox m_listLog;
	CComboBox m_comboProject;
	CTreeCtrl m_treeProjectInfo;
	afx_msg void OnSelchangeComboProject();
	CMFCEditBrowseCtrl m_browseLastDir;
	afx_msg void OnChangeMfceditbrowseSrcdir();
	afx_msg void OnBnClickedButtonProjectEdit();
	afx_msg void OnBnClickedButtonCompareToUpload();
};
