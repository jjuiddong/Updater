// DiffDialog.cpp : implementation file
//

#include "stdafx.h"
#include "Uploader.h"
#include "DiffDialog.h"
#include "afxdialogex.h"


// CDiffDialog dialog

IMPLEMENT_DYNAMIC(CDiffDialog, CDialogEx)

CDiffDialog::CDiffDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_DIFF, pParent)
{

}

CDiffDialog::~CDiffDialog()
{
}

void CDiffDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CDiffDialog, CDialogEx)
END_MESSAGE_MAP()


// CDiffDialog message handlers
