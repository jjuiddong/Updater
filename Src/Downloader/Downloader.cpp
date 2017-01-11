
#include "stdafx.h"
#include "Downloader.h"
#include "DownloaderDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CDownloaderApp
BEGIN_MESSAGE_MAP(CDownloaderApp, CWinApp)
	ON_COMMAND(ID_HELP, &CWinApp::OnHelp)
END_MESSAGE_MAP()

// CDownloaderApp construction
CDownloaderApp::CDownloaderApp()
{
	m_dwRestartManagerSupportFlags = AFX_RESTART_MANAGER_SUPPORT_RESTART;
}

CDownloaderApp theApp;

BOOL CDownloaderApp::InitInstance()
{
	INITCOMMONCONTROLSEX InitCtrls;
	InitCtrls.dwSize = sizeof(InitCtrls);
	InitCtrls.dwICC = ICC_WIN95_CLASSES;
	InitCommonControlsEx(&InitCtrls);
	CWinApp::InitInstance();
	AfxEnableControlContainer();
	CShellManager *pShellManager = new CShellManager;
	CMFCVisualManager::SetDefaultManager(RUNTIME_CLASS(CMFCVisualManagerWindows));
	SetRegistryKey(_T("Local AppWizard-Generated Applications"));
	CDownloaderDlg *dlg = new CDownloaderDlg();
	dlg->Create(CDownloaderDlg::IDD);
	dlg->ShowWindow(SW_SHOW);
	dlg->Run();

	if (pShellManager != NULL)
		delete pShellManager;
	delete dlg;
#ifndef _AFXDLL
	ControlBarCleanUp();
#endif
	return FALSE;
}

