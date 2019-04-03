
#include "stdafx.h"
#include "Uploader.h"
#include "UploaderDlg.h"
#include "afxdialogex.h"
#include "DiffDialog.h"
#include "ProjectEditor.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CUploaderDlg *g_UploaderDlg= NULL;
cSyncQueue<sMessage*, true> g_message; // FTP Scheduler State List to Display External Object


// CUploaderDlg dialog
CUploaderDlg::CUploaderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_UPLOADER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	g_UploaderDlg = this;
}

void CUploaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_SRCFILES, m_srcFileTree);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_SRCDIR, m_browsSrcDir);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Control(pDX, IDC_COMBO_PROJECT, m_comboProject);
	DDX_Control(pDX, IDC_TREE_PROJECT_INFO, m_treeProjectInfo);
	DDX_Control(pDX, IDC_TREE_LASTFILES, m_treeLastFiles);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_LASTDIR, m_browseLastDir);
}

BEGIN_MESSAGE_MAP(CUploaderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CUploaderDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CUploaderDlg::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_READ, &CUploaderDlg::OnBnClickedButtonRead)
	ON_CBN_SELCHANGE(IDC_COMBO_PROJECT, &CUploaderDlg::OnSelchangeComboProject)
	ON_EN_CHANGE(IDC_MFCEDITBROWSE_SRCDIR, &CUploaderDlg::OnChangeMfceditbrowseSrcdir)
	ON_BN_CLICKED(IDC_BUTTON_PROJECT_EDIT, &CUploaderDlg::OnBnClickedButtonProjectEdit)
	ON_BN_CLICKED(IDC_BUTTON_COMPARE_TO_UPLOAD, &CUploaderDlg::OnBnClickedButtonCompareToUpload)
END_MESSAGE_MAP()

BEGIN_ANCHOR_MAP(CUploaderDlg)
	ANCHOR_MAP_ENTRY(IDC_TREE_SRCFILES, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_TREE_LASTFILES, ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_TREE_PROJECT_INFO, ANF_LEFT | ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_MFCEDITBROWSE_SRCDIR, ANF_LEFT | ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_MFCEDITBROWSE_LASTDIR, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_READ, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_COMPARE_TO_UPLOAD, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_1, ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_LIST_LOG, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
END_ANCHOR_MAP()


// CUploaderDlg message handlers

BOOL CUploaderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}
	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	InitAnchors();

	RefreshProjectInfo();

	return TRUE;
}

void CUploaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CUploaderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

HCURSOR CUploaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CUploaderDlg::OnBnClickedOk()
{
	//CDialogEx::OnOK();
}


void CUploaderDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}


void CUploaderDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);
}


// Source File Directory Read
void CUploaderDlg::OnBnClickedButtonRead()
{
	CString dir;
	m_browsSrcDir.GetWindowTextW(dir);
	string srcDirectory = wstr2str((LPCTSTR)dir);
	m_srcFileTree.Update(srcDirectory + "/", list<string>());
	m_srcFileTree.ExpandAll();

	return;
}


// Project ComboBox Select Change Event
void CUploaderDlg::OnSelchangeComboProject()
{
	const int projId = m_comboProject.GetCurSel();
	if (projId < 0)
		return;
	if (projId >= (int)m_config.m_projInfos.size())
		return;

	cUploaderConfig::sProjectInfo &projInfo = m_config.m_projInfos[projId];

	UpdateProjectInfo();

	m_browsSrcDir.SetWindowTextW(
		str2wstr(GetFullFileName(projInfo.sourceDirectory)).c_str());
	m_browseLastDir.SetWindowTextW(
		str2wstr(GetFullFileName(projInfo.lastestDirectory)).c_str());

	m_srcFileTree.Update(projInfo.sourceDirectory + "/", list<string>());
	m_srcFileTree.ExpandAll();
	m_treeLastFiles.Update(projInfo.lastestDirectory + "/", list<string>());
	m_treeLastFiles.ExpandAll();
}


// Change String Event in Source File Directory Browser
void CUploaderDlg::OnChangeMfceditbrowseSrcdir()
{
	CString path;
	m_browsSrcDir.GetWindowTextW(path);
	const string sourceDirectory = wstr2str((LPCTSTR)path) + "/";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);
	m_srcFileTree.Update(sourceDirectory, {});
	m_srcFileTree.ExpandAll();
}


// Update Current Project Information
void CUploaderDlg::UpdateProjectInfo()
{
	const int projId = m_comboProject.GetCurSel();
	if (projId < 0)
		return;
	if (projId >= (int)m_config.m_projInfos.size())
		return;
	cUploaderConfig::sProjectInfo &projInfo = m_config.m_projInfos[projId];

	m_treeProjectInfo.DeleteAllItems();
	m_treeProjectInfo.InsertItem(formatw("Project Name = %s", projInfo.name.c_str()).c_str());
	m_treeProjectInfo.InsertItem(formatw("FTP Address = %s", projInfo.ftpAddr.c_str()).c_str());
	m_treeProjectInfo.InsertItem(formatw("FTP ID = %s", projInfo.ftpId.c_str()).c_str());
	m_treeProjectInfo.InsertItem(formatw("FTP Passwd = %s", projInfo.ftpPasswd.c_str()).c_str());
	m_treeProjectInfo.InsertItem(formatw("FTP Directory = %s", projInfo.ftpDirectory.c_str()).c_str());
	m_treeProjectInfo.InsertItem(formatw("Lastest Directory = %s", projInfo.lastestDirectory.c_str()).c_str());
	m_treeProjectInfo.InsertItem(formatw("Backup Directory = %s", projInfo.backupDirectory.c_str()).c_str());
	m_treeProjectInfo.InsertItem(formatw("Source Directory = %s", projInfo.sourceDirectory.c_str()).c_str());
	m_treeProjectInfo.InsertItem(formatw("Exe FileName = %s", projInfo.exeFileName.c_str()).c_str());

	// Read Lastest Version
	cVersionFile verFile;
	if (verFile.Read(GetFullFileName(projInfo.lastestDirectory + "/version.ver")))
		m_treeProjectInfo.InsertItem(formatw("Lastest Version = %d", verFile.m_version).c_str());
	else
		m_treeProjectInfo.InsertItem(formatw("Lastest Version = xxx").c_str());
}


// Update Project ConfigFile, Reload Project Information
void CUploaderDlg::RefreshProjectInfo()
{
	m_config.Read(g_uploaderConfigFileName);

	while (m_comboProject.GetCount() > 0)
		m_comboProject.DeleteString(0);

	for each (auto proj in m_config.m_projInfos)
		m_comboProject.InsertString(m_comboProject.GetCount(), str2wstr(proj.name).c_str());

	if (m_comboProject.GetCount() > 0)
	{
		m_comboProject.SetCurSel(0);
		OnSelchangeComboProject(); // update project information
	}
	else
	{
		TCHAR dir[MAX_PATH];
		GetCurrentDirectory(sizeof(dir), dir);
		m_browsSrcDir.SetWindowTextW(dir);
	}
}


void CUploaderDlg::OnBnClickedButtonProjectEdit()
{
	CProjectEditor dlg;
	dlg.DoModal();	
}


void CUploaderDlg::OnBnClickedButtonCompareToUpload()
{
	const int projId = m_comboProject.GetCurSel();
	if (projId < 0)
		return;
	if (projId >= (int)m_config.m_projInfos.size())
		return;

	cUploaderConfig::sProjectInfo &projInfo = m_config.m_projInfos[projId];

	CString path;
	m_browsSrcDir.GetWindowTextW(path);
	const string sourceDirectory = wstr2str((LPCTSTR)path) + "/";
	projInfo.sourceDirectory = sourceDirectory;

	// Show Diff Dialog
	CDiffDialog *dlg = new CDiffDialog(this, projInfo);
	dlg->Create(CDiffDialog::IDD);
	dlg->ShowWindow(SW_SHOW);
	dlg->Run();
	delete dlg;
}
