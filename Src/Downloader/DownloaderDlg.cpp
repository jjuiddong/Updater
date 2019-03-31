
#include "stdafx.h"
#include "Downloader.h"
#include "DownloaderDlg.h"
#include "afxdialogex.h"
#include "../ZipLib/ZipFile.h"
#include "../ZipLib/streams/memstream.h"
#include "../ZipLib/methods/Bzip2Method.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CDownloaderDlg::CDownloaderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DOWNLOADER_DIALOG, pParent)
	, m_loop(true)
	, m_state(eState::CHECK_VERSION)
	, m_isErrorOccur(false)
	, m_readTotalBytes(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CDownloaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Control(pDX, IDC_PROGRESS_FTP, m_progFTP);
	DDX_Control(pDX, IDC_STATIC_PROGRESS, m_staticProgress);
	DDX_Control(pDX, IDC_STATIC_PERCENTAGE, m_staticPercentage);
	DDX_Control(pDX, IDC_PROGRESS_TOTAL, m_progTotal);
}

BEGIN_MESSAGE_MAP(CDownloaderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CDownloaderDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDownloaderDlg::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_HELP, &CDownloaderDlg::OnBnClickedButtonHelp)
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

	if (!m_config.Read(g_configFileName))
	{
		AfxMessageBox(L"Read Configuration Error!!");
		SetWindowText(L"DownLoader - < Error!! Read Config File >");
		return FALSE;
	}

	if (!m_ftpScheduler.Init(m_config.m_ftpAddr, m_config.m_ftpId, m_config.m_ftpPasswd))
	{
		AfxMessageBox(L"FTP Scheduler Error!!");
		SetWindowText(L"DownLoader - < Error!! FTP Connection >");
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
	m_loop = false;
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
		Log("Connect Server ... \n");
		SetTimer(1, 0, NULL);
	}
	else if (nIDEvent == 1)
	{
		KillTimer(nIDEvent);
		DownloadVersionFile();
	}

	CDialogEx::OnTimer(nIDEvent);
}


void CDownloaderDlg::DownloadVersionFile()
{
	const string localFullDirectoryName = GetFullFileName(m_config.m_localDirectory);

	vector<cFTPScheduler::sCommand> dnFileList;
	dnFileList.push_back(
		cFTPScheduler::sCommand(
			cFTPScheduler::eCommandType::DOWNLOAD
			, m_config.m_ftpDirectory + "/version.ver"
			, localFullDirectoryName + "/temp_version.ver"));
	m_ftpScheduler.AddCommand(dnFileList);
	m_ftpScheduler.Start();
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


void CDownloaderDlg::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	int oldT = timeGetTime();
	while (m_loop)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		const int curT = timeGetTime();
		if (curT - oldT > 30)
		{
			const float deltaSeconds = (curT - oldT) * 0.001f;
			oldT = curT;
			MainLoop(deltaSeconds);
		}
	}
}


void CDownloaderDlg::MainLoop(const float deltaSeconds)
{
	cFTPScheduler::sState state;
	if (m_ftpScheduler.Update(state))
	{
		if (cFTPScheduler::sState::NONE != state.state)
			LogFTPState(state);
	}

	switch (m_state)
	{
	case eState::CHECK_VERSION:
		if (cFTPScheduler::sState::ERR == state.state)
		{
			string msg;
			switch (state.data)
			{
			case cFTPScheduler::sState::LOGIN: 
				msg = "Error FTP Connection\n"; 
				Log("Error FTP Connection\n");
				Log("- Check ConfigFile FTP Address, id, pass\n");
				break;

			case cFTPScheduler::sState::DOWNLOAD:
				msg = "Error Download Version File\n";
				Log("Error Download Version File");
				Log("- FTP Connection Success but Error Occured");
				Log("- Check ConfigFile FTP Directory");
				Log("- Check ConfigFile Download Local Directory");
				break;

			default: msg = "Error Download Version File\n"; 
				break;
			}

			Log(msg);
			::AfxMessageBox(str2wstr(msg).c_str());
			m_isErrorOccur = true;
		}
		else if (cFTPScheduler::sState::FINISH == state.state)
		{
			if (!m_isErrorOccur)
				CheckVersionFile();
		}
		break;

	case eState::DOWNLOAD:
		switch (state.state)
		{
		case cFTPScheduler::sState::ERR:
			m_isErrorOccur = true;
			break;

		case cFTPScheduler::sState::FINISH:
		{
			FinishDownloadFile();

			int lower, upper;
			m_progTotal.GetRange(lower, upper);
			m_progTotal.SetPos(upper);

			if (!m_isErrorOccur && common::IsFileExist(m_config.m_exeFileName))
			{
				::ShellExecuteA(NULL, "open", m_config.m_exeFileName.c_str(), NULL
					, m_config.m_localDirectory.c_str(), SW_SHOW);
			}
		}
		break;

		case cFTPScheduler::sState::DOWNLOAD_BEGIN:
		{
			const string localFullDirectoryName = GetFullFileName(m_config.m_localDirectory);
			m_staticProgress.SetWindowTextW(
				formatw("%s", 
					DeleteCurrentPath(RelativePathTo(localFullDirectoryName, state.fileName)).c_str()).c_str());
		}
		break;

		case cFTPScheduler::sState::DOWNLOAD:
		{
			m_progFTP.SetRange32(0, state.totalBytes);
			m_progFTP.SetPos(state.progressBytes);
			
			m_readTotalBytes += state.readBytes;
			m_progTotal.SetPos(m_readTotalBytes);

			m_staticPercentage.SetWindowTextW(
				formatw("%d/%d bytes", state.progressBytes, state.totalBytes).c_str());
		}
		break;

		case cFTPScheduler::sState::DOWNLOAD_DONE:
		{
			if (common::GetFileExt(state.fileName) == ".zip")
			{
				string sourceFileName = RemoveFileExt(state.fileName);
				string inFileName = GetFileName(sourceFileName);
				try {
					ZipFile::ExtractFile(state.fileName, inFileName, sourceFileName);
				}
				catch (...){
					Log(common::format("Error Occur!!, Error Unzip File = [ %s ]\n"
						, sourceFileName.c_str()));
				}

				remove(state.fileName.c_str()); // remove zip file
			}
		}
		break;

		}
		break;

	case eState::FINISH:
		break;

	default:
		assert(0);
		break;
	}
}


void CDownloaderDlg::CheckVersionFile()
{
	const string localFullDirectoryName = GetFullFileName(m_config.m_localDirectory);

	// Read VersionFile
	cVersionFile localVer;
	localVer.Read(GetFullFileName(m_config.m_localDirectory) + "/version.ver");
	cVersionFile remoteVer;
	remoteVer.Read(GetFullFileName(m_config.m_localDirectory) + "/temp_version.ver");

	// Compare VersionFile
	vector<cVersionFile::sCompareInfo> compResult;
	if (localVer.Compare(remoteVer, compResult) <= 0)
	{
		Log("Lastest Version!!");

		// Remove temporal file
		const string rmFile = localFullDirectoryName + "/temp_version.ver";
		DeleteFileA(rmFile.c_str());

		if (common::IsFileExist(m_config.m_exeFileName))
		{
			::ShellExecuteA(NULL, "open", m_config.m_exeFileName.c_str(), NULL
				, m_config.m_localDirectory.c_str(), SW_SHOW);
		}

		return; // finish, need not download
	}
	
	// If Need Download, Create Folder and then Download
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
	long downloadTotalBytes = 0;
	vector<cFTPScheduler::sCommand> dnFileList;
	for each (auto comp in compResult)
	{
		switch (comp.state)
		{
		case cVersionFile::sCompareInfo::UPDATE:
		{
			string remoteFileName;
			string localFileName;
			if (comp.fileSize > 0) // Zip File
			{
				remoteFileName = m_config.m_ftpDirectory + "/" + comp.fileName + ".zip";
				localFileName = localFullDirectoryName + "/" + comp.fileName + ".zip";
			}
			else
			{ // No Zip File
				remoteFileName = m_config.m_ftpDirectory + "/" + comp.fileName;
				localFileName = localFullDirectoryName + "/" + comp.fileName;
			}

			dnFileList.push_back(
				cFTPScheduler::sCommand(cFTPScheduler::eCommandType::DOWNLOAD
					, remoteFileName, localFileName, comp.compressSize));

			downloadTotalBytes += comp.compressSize;
		}
		break;

		case cVersionFile::sCompareInfo::REMOVE:
		{
			const string rmFile = localFullDirectoryName + "/" + comp.fileName;
			DeleteFileA(rmFile.c_str());

			Log(format("Remove %s\n", comp.fileName.c_str()));
		}
		break;
		}
	}

	// Download Patch Files
	m_isErrorOccur = false;
	m_progTotal.SetRange32(0, (int)downloadTotalBytes);
	m_progTotal.SetPos(0);
	m_readTotalBytes = 0;
	m_state = eState::DOWNLOAD;
	m_ftpScheduler.AddCommand(dnFileList);
	m_ftpScheduler.Start();
}


// Download Finish
void CDownloaderDlg::FinishDownloadFile()
{
	const string localFullDirectoryName = GetFullFileName(m_config.m_localDirectory);

	if (m_isErrorOccur)
	{
		::AfxMessageBox(L"Error Download File\n");
		Log("Download Error!!");
	}
	else
	{
		// Update VersionFile
		cVersionFile remoteVer;
		remoteVer.Read(localFullDirectoryName  + "/temp_version.ver");
		remoteVer.Write(localFullDirectoryName + "/version.ver");
		Log("Download Complete!!");	
	}

	// Remove temporal file
	const string rmFile = localFullDirectoryName + "/temp_version.ver";
	DeleteFileA(rmFile.c_str());

	m_state = eState::FINISH;
}


void CDownloaderDlg::Log(const string &msg)
{
	m_listLog.InsertString(m_listLog.GetCount(),str2wstr(msg).c_str());
	m_listLog.SetCurSel(m_listLog.GetCount() - 1);
	m_listLog.ShowCaret();
}


void CDownloaderDlg::LogFTPState(const cFTPScheduler::sState &state)
{
	switch (state.state)
	{
	case cFTPScheduler::sState::DOWNLOAD_BEGIN:
		//Log(format("Download... %s, 0/%d", state.fileName.c_str(), state.totalBytes));
		break;
	case cFTPScheduler::sState::DOWNLOAD:
		//Log(format("Download... %s, %d/%d", state.fileName.c_str(), state.progressBytes, state.totalBytes));
		break;
	case cFTPScheduler::sState::DOWNLOAD_DONE:
		//Log(format("Download Done %s, %d/%d", state.fileName.c_str(), state.progressBytes, state.totalBytes));
		break;
	case cFTPScheduler::sState::UPLOAD_BEGIN:
		Log(format("Upload... %s", state.fileName.c_str()));
		break;
	case cFTPScheduler::sState::UPLOAD:
		Log(format("Upload... %s", state.fileName.c_str()));
		break;
	case cFTPScheduler::sState::UPLOAD_DONE:
		Log(format("Upload Done %s", state.fileName.c_str()));
		break;
	case cFTPScheduler::sState::ERR:
		Log(format("Error = %d", state.data));
		break;
	case cFTPScheduler::sState::FINISH:
		Log(format("Finish Scheduler "));
		break;
	default:
		assert(0);
		break;
	}
}


// Help Button
// config 파일 편집 설명
void CDownloaderDlg::OnBnClickedButtonHelp()
{
	const char *msg = "* Help\n\
Downloader Use Config File\n\
\n\
Config File Setting\n\
	- Json Format\n\
\n\
Config File Sample \n\
\n\
{\n\
	\"project name\": \"Name\",\n\
	\"ftp address\" : \"FTP Site.com\",\n\
	\"ftp id\" : \"FTP User ID\",\n\
	\"ftp pass\" : \"FTP User Password\",\n\
	\"ftp dir\" : \"FTP Directory ex)www/data/project1\",\n\
	\"local dir\" : \"Download Directory ex)C:/Project1\",\n\
	\"exe file name\" : \"Execute FileName\"\n\
}\n\
-----------------------------------------------------------\n\
\n\
"
;
	const wstring str = common::formatw("%s ConfigFileName = [ %s ]"
		, msg, g_configFileName.c_str());
	::AfxMessageBox(str.c_str());
}
