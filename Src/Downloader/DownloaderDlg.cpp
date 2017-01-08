
#include "stdafx.h"
#include "Downloader.h"
#include "DownloaderDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDownloaderDlg::CDownloaderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DOWNLOADER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDownloaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
}

BEGIN_MESSAGE_MAP(CDownloaderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CDownloaderDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDownloaderDlg::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_WM_TIMER()
END_MESSAGE_MAP()

BOOL CDownloaderDlg::OnInitDialog()
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

	dbg::RemoveLog();

	if (!m_config.Read("downloader_config.json"))
	{
		AfxMessageBox(L"Read Configuration Error!!");
		OnCancel();
		return FALSE;
	}

	SetTimer(0, 1000, NULL);

	return TRUE;
}

void CDownloaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CDownloaderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);
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


HCURSOR CDownloaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDownloaderDlg::OnBnClickedOk()
{
	//CDialogEx::OnOK();
}

void CDownloaderDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}



void CDownloaderDlg::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
}


void CDownloaderDlg::OnTimer(UINT_PTR nIDEvent)
{
	if (nIDEvent == 0)
	{
		KillTimer(nIDEvent);
		m_listLog.InsertString(m_listLog.GetCount(), L"Connect Server ... \n");
		SetTimer(1, 0, NULL);
	}
	else if (nIDEvent == 1)
	{
		KillTimer(nIDEvent);
		DownloadProcess();
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CDownloaderDlg::DownloadProcess()
{
	nsFTP::CFTPClient client;
	nsFTP::CLogonInfo info(str2wstr(m_config.m_ftpAddr), 21, 
		str2wstr(m_config.m_ftpId), str2wstr(m_config.m_ftpPasswd));
	if (!client.Login(info))
	{
		::AfxMessageBox(L"FTP Login Error ");
		return;
	}

	client.SetResumeMode(false);

	const string localFullDirectoryName = GetFullFileName(m_config.m_localDirectory);

	// Download VersionFile
	if (!client.DownloadFile(str2wstr(m_config.m_ftpDirectory + "/version.ver")
		, str2wstr(localFullDirectoryName + "/temp_version.ver")))
	{
		// Error Download VersionFile, then All File Download
		// todo: All File Download
		dbg::Log("Err Download VersionFile \n");
		m_listLog.InsertString(m_listLog.GetCount(), L"Err Download VersionFile \n");
		return;
	}

	// Read VersionFile
	cVersionFile localVer;
	localVer.Read(GetFullFileName(m_config.m_localDirectory) + "/version.ver");
	cVersionFile remoteVer;
	remoteVer.Read(GetFullFileName(m_config.m_localDirectory) + "/temp_version.ver");

	// Remove temparal file
	{
		const string rmFile = localFullDirectoryName + "/temp_version.ver";
		DeleteFileA(rmFile.c_str());
	}

	// Compare VersionFile
	vector<cVersionFile::sCompareInfo> compResult;
	if (localVer.Compare(remoteVer, compResult) <= 0)
	{
		//AfxMessageBox(L"Last Version!!");
		m_listLog.InsertString(m_listLog.GetCount(), L"Lastest Version!!");
		return; // finish, need not download
	}

	
	// If Need Download file, Create Folder and then Download files
	list<string> updateFiles;
	for each (auto comp in compResult)
	{
		if (cVersionFile::sCompareInfo::UPDATE == comp.state)
			updateFiles.push_back(comp.fileName);
	}
	sFolderNode *root = common::CreateFolderNode(updateFiles);
	MakeLocalFolder(localFullDirectoryName, root);
	common::DeleteFolderNode(root);


	// Download Files from FTP
	int downloadFileCount = 0;
	bool isErrorOccur = false;
	for each (auto comp in compResult)
	{
		switch (comp.state)
		{
		case cVersionFile::sCompareInfo::NOT_UPDATE:
			break;
		
		case cVersionFile::sCompareInfo::UPDATE:
		{
			++downloadFileCount;
			if (client.DownloadFile(str2wstr(m_config.m_ftpDirectory + "/" + comp.fileName)
				, str2wstr(localFullDirectoryName + "/" + comp.fileName)))
			{
				dbg::Log("Download File... %s\n", comp.fileName.c_str());
				m_listLog.InsertString(m_listLog.GetCount(), 
					formatw("Download File... %s\n", comp.fileName.c_str()).c_str());
			}
			else
			{
				isErrorOccur = true;
				dbg::Log("Download Error Occur %s\n", comp.fileName.c_str());
				m_listLog.InsertString(m_listLog.GetCount(),
					formatw("Download Error Occur %s\n", comp.fileName.c_str()).c_str());
			}
		}
		break;

		case cVersionFile::sCompareInfo::REMOVE:
		{
			const string rmFile = localFullDirectoryName + "/" + comp.fileName;
			DeleteFileA(rmFile.c_str());

			dbg::Log("Remove File... %s\n", comp.fileName.c_str());
			m_listLog.InsertString(m_listLog.GetCount(),
				formatw("Remove %s\n", comp.fileName.c_str()).c_str());
		}
		break;
		}
	}

	// Download Finish
	if (!isErrorOccur)
	{
		// Update VersionFile
		remoteVer.Write(localFullDirectoryName + "/version.ver");
		m_listLog.InsertString(m_listLog.GetCount(), L"Download Complete!!");
	}	
}


void CDownloaderDlg::MakeLocalFolder(const string &path, common::sFolderNode *node)
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
