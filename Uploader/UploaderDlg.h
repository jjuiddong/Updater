#pragma once


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


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnBnClickedButtonUpload();
	afx_msg void OnBnClickedButtonDownload();
};
