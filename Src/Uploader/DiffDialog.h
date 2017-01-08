#pragma once


// CDiffDialog dialog

class CDiffDialog : public CDialogEx
{
	DECLARE_DYNAMIC(CDiffDialog)

public:
	CDiffDialog(CWnd* pParent = NULL);   // standard constructor
	virtual ~CDiffDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_DIFF };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
