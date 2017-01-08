#pragma once

#include "DownloaderConfig.h"
#include "afxwin.h"



class CDownloaderDlg : public CDialogEx
{
public:
	CDownloaderDlg(CWnd* pParent = NULL);	// standard constructor

#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DOWNLOADER_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	void DownloadProcess();
	void MakeLocalFolder(const string &path, common::sFolderNode *node);


protected:
	HICON m_hIcon;
	cDownloaderConfig m_config;


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
};
