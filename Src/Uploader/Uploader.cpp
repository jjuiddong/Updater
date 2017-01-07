
#include "stdafx.h"
#include "Uploader.h"
#include "UploaderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUploaderApp
BEGIN_MESSAGE_MAP(CUploaderApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

CUploaderApp::CUploaderApp()
{
}

CUploaderApp theApp;

BOOL CUploaderApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();
	AfxEnableControlContainer();
	CShellManager *pShellManager = new CShellManager;	
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));

	CUploaderDlg dlg;
	m_pMainWnd = &dlg;
	dlg.DoModal();

	if (pShellManager != NULL)
		delete pShellManager;

#ifndef _AFXDLL
	ControlBarCleanUp();
#endif

	return FALSE;
}

