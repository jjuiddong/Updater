
#include "stdafx.h"
#include "Uploader.h"
#include "UploaderDlg.h"
#include "afxdialogex.h"
#include "FileComparison.h"
#include "UploadManager.h"
#include "VersionFile.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CUploaderDlg dialog
CUploaderDlg::CUploaderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_UPLOADER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUploaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_TREE_SRCFILES, m_srcFileTree);
	DDX_Control(pDX, IDC_MFCEDITBROWSE_SRCDIR, m_browsSrcDir);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Control(pDX, IDC_COMBO_PROJECT, m_comboProject);
}

BEGIN_MESSAGE_MAP(CUploaderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CUploaderDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CUploaderDlg::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_READ, &CUploaderDlg::OnBnClickedButtonRead)
END_MESSAGE_MAP()

BEGIN_ANCHOR_MAP(CUploaderDlg)
	ANCHOR_MAP_ENTRY(IDC_TREE_SRCFILES, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_MFCEDITBROWSE_SRCDIR, ANF_LEFT | ANF_RIGHT | ANF_TOP)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_READ, ANF_RIGHT | ANF_TOP)
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

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	InitAnchors();

	m_srcFileTree.Update("./", list<string>());
	m_srcFileTree.ExpandAll();

	m_config.Read("uploader_config.json");
	
	for each (auto proj in m_config.m_projInfos)
		m_comboProject.InsertString(m_comboProject.GetCount(), str2wstr(proj->name).c_str());
	if (m_comboProject.GetCount() > 0)
		m_comboProject.SetCurSel(0);

	//m_browsSrcDir.SetWindowTextW(L"D:\\Project\\Updater\\Bin");
	//m_browsSrcDir.SetWindowTextW(L"D:\\Project\\Updater\\Src\\Uploader");
	m_browsSrcDir.SetWindowTextW(L"C:\\Users\\재정\\Desktop\\새 폴더");

	return TRUE;  // return TRUE  unless you set the focus to a control
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


void CUploaderDlg::OnBnClickedButtonRead()
{
	cUploaderConfig::sProjectInfo *projInfo = NULL;
	const int projId = m_comboProject.GetCurSel();
	if (projId < 0)
		return;
	if (projId < (int)m_config.m_projInfos.size())
		projInfo = m_config.m_projInfos[projId];
	if (!projInfo)
		return;

	CString path;
	m_browsSrcDir.GetWindowTextW(path);
	const string sourceDirectory = wstr2str((LPCTSTR)path) + "\\";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);
	m_srcFileTree.Update(sourceDirectory, {});
	m_srcFileTree.ExpandAll();	

	// Compare Source Directory, Lastest Directory
	vector<pair<DifferentFileType::Enum, string>> diff;
	CompareDirectory(sourceDirectory, projInfo->lastestDirectory+"/", diff);

	if (diff.empty())
	{
		m_listLog.InsertString(m_listLog.GetCount(), L"Nothing to Upload");
		return;
	}
	else
	{
		for each (auto item in diff)
			m_listLog.InsertString(m_listLog.GetCount(), str2wstr(item.second).c_str());
	}

	//------------------------------------------------------------------------------------------------------------------
	// Update Local Directory

	// Update Lastest Directory
	CheckLocalFolder(projInfo, diff);
	for each (auto file in diff)
	{
		const string srcFileName = file.second;
		const string fileName = DeleteCurrentPath(RelativePathTo(sourceFullDirectory, file.second));
		const string dstFileName = GetFullFileName(projInfo->lastestDirectory) + "/" + fileName;
		CopyFileA(srcFileName.c_str(), dstFileName.c_str(), FALSE);
	}

	// Write Version file to Lastest Directory
	cVersionFile verFile = CreateVersionFile(sourceFullDirectory, projInfo, diff);
	verFile.Write(GetFullFileName(projInfo->lastestDirectory) + "/version.ver");

	// Update Backup Directory
	const string backupFolderName = GetCurrentDateTime();
	const string dstFolderName = GetFullFileName(projInfo->backupDirectory) + "\\" + backupFolderName + "\\";
	const string srcFileCmd = sourceFullDirectory + "\\*";
	CreateDirectoryA(dstFolderName.c_str(), NULL);
	SHFILEOPSTRUCTA s = { 0 };
	s.hwnd = m_hWnd;
	s.wFunc = FO_COPY;
	s.fFlags = FOF_SILENT;
	s.pTo = dstFolderName.c_str();
	s.pFrom = srcFileCmd.c_str();
	SHFileOperationA(&s);
	
	// Write Backup VersionFile
	verFile.Write(dstFolderName + "/version.ver");


	//------------------------------------------------------------------------------------------------------------------
	// Upload FTP Server Different Files
	nsFTP::CFTPClient client;
	nsFTP::CLogonInfo info(str2wstr(projInfo->ftpAddr), 21, str2wstr(projInfo->ftpId), str2wstr(projInfo->ftpPasswd));
	if (client.Login(info))
	{
		CheckFTPFolder(client, projInfo, diff);
		for each (auto file in diff)
		{
			const string fileName = DeleteCurrentPath(RelativePathTo(sourceFullDirectory, file.second));
			const string remoteFileName = projInfo->ftpDirectory + "/" + fileName;

			if (file.first == 1)
			{ // delete

			}
			else
			{ // add, modify
				client.UploadFile(str2wstr(file.second), str2wstr(remoteFileName));
			}
		}

		// Upload VersionFile
		client.UploadFile(str2wstr(GetFullFileName(projInfo->lastestDirectory) + "/version.ver"), 
			str2wstr(projInfo->ftpDirectory + "/version.ver"));
	}

}


// Check Upload Folder. If Not Exist, Make Folder
void CUploaderDlg::CheckFTPFolder(
	nsFTP::CFTPClient &client, 
	cUploaderConfig::sProjectInfo *projInfo, 
	vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	RET(!projInfo);
	
	// Collect  Folders
	CString path;
	m_browsSrcDir.GetWindowTextW(path);
	const string sourceDirectory = wstr2str((LPCTSTR)path) + "\\";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);
	list<string> files;
	for each (auto &file in diffFiles)
	{
		if (DifferentFileType::REMOVE != file.first) // add, modify
		{
			const string fileName = DeleteCurrentPath(RelativePathTo(sourceFullDirectory, file.second));
			files.push_back(fileName);
		}
	}
	
	vector<string> folders;
	CFileTreeCtrl::sTreeNode *root = m_srcFileTree.CreateFolderNode(files);
	MakeFTPFolder(client, projInfo->ftpDirectory, root);
	m_srcFileTree.DeleteTreeNode(root);
}


void CUploaderDlg::MakeFTPFolder(nsFTP::CFTPClient &client, const string &path, CFileTreeCtrl::sTreeNode *node)
{
	RET(!node);

	for each (auto child in node->children)
	{
		const string folderName = path + "/" + child.first;
		client.MakeDirectory(str2wstr(folderName));

		MakeFTPFolder(client, folderName, child.second);
	}
}


// Check Local Folder to Copy Lastest Directory
void CUploaderDlg::CheckLocalFolder(
	cUploaderConfig::sProjectInfo *projInfo,
	vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	RET(!projInfo);

	// Collect  Folders
	CString path;
	m_browsSrcDir.GetWindowTextW(path);
	const string sourceDirectory = wstr2str((LPCTSTR)path) + "\\";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);
	list<string> files;
	for each (auto &file in diffFiles)
	{
		if (DifferentFileType::REMOVE != file.first)
		{
			const string fileName = DeleteCurrentPath(RelativePathTo(sourceFullDirectory, file.second));
			files.push_back(fileName);
		}
	}

	vector<string> folders;
	CFileTreeCtrl::sTreeNode *root = m_srcFileTree.CreateFolderNode(files);
	MakeLocalFolder(GetFullFileName(projInfo->lastestDirectory), root);
	m_srcFileTree.DeleteTreeNode(root);
}


void CUploaderDlg::MakeLocalFolder(const string &path, CFileTreeCtrl::sTreeNode *node)
{
	RET(!node);

	for each (auto child in node->children)
	{
		const string folderName = path + "/" + child.first;
		if (!IsFileExist(folderName))
			CreateDirectoryA(folderName.c_str(), NULL);

		MakeLocalFolder(folderName, child.second);
	}
}


// Read Lastest VersionFile and then Merge DifferentFiles
cVersionFile CUploaderDlg::CreateVersionFile(
	const string &srcDirectoryPath,
	cUploaderConfig::sProjectInfo *projInfo, 
	vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	cVersionFile verFile;
	RETV(!projInfo, verFile);

	verFile.Read(GetFullFileName(projInfo->lastestDirectory) + "/version.ver");
	verFile.Update(srcDirectoryPath, diffFiles);
	return verFile;
}
