
#include "stdafx.h"
#include "Uploader.h"
#include "DiffDialog.h"
#include "afxdialogex.h"
#include "UploaderDlg.h"


CDiffDialog::CDiffDialog(CWnd* pParent, const cUploaderConfig::sProjectInfo &info)
	: CDialogEx(IDD_DIALOG_DIFF, pParent)
	, m_projInfo(info)
	, m_loop(true)
	, m_isErrorOccur(false)
	, m_writeTotalBytes(0)
	, m_state(eState::WAIT)
{
}

CDiffDialog::~CDiffDialog()
{
}

void CDiffDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_DIFF, m_listDiff);
	DDX_Control(pDX, IDC_LIST_LOG, m_listLog);
	DDX_Control(pDX, IDC_PROGRESS_TOTAL, m_progTotal);
	DDX_Control(pDX, IDC_PROGRESS_UPLOAD, m_progUpload);
	DDX_Control(pDX, IDC_STATIC_UPLOAD_FILE, m_staticUploadFile);
	DDX_Control(pDX, IDC_STATIC_UPLOAD_PERCENTAGE, m_staticUploadPercentage);
}


BEGIN_MESSAGE_MAP(CDiffDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDiffDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDiffDialog::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_UPLOAD, &CDiffDialog::OnBnClickedButtonUpload)
	ON_WM_SIZE()
END_MESSAGE_MAP()

BEGIN_ANCHOR_MAP(CDiffDialog)
	ANCHOR_MAP_ENTRY(IDC_LIST_DIFF, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_UPLOAD, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_LIST_LOG, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_PROGRESS_TOTAL, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_PROGRESS_UPLOAD, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_UPLOAD_FILE, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_UPLOAD_PERCENTAGE, ANF_RIGHT | ANF_RIGHT | ANF_BOTTOM)
END_ANCHOR_MAP()


// CDiffDialog message handlers
void CDiffDialog::OnBnClickedOk()
{
	//CDialogEx::OnOK();
}
void CDiffDialog::OnBnClickedCancel()
{
	m_loop = false;
	CDialogEx::OnCancel();
}


BOOL CDiffDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitAnchors();

	m_listDiff.InsertColumn(0, L"State", 0, 60);
	m_listDiff.InsertColumn(1, L"File Name", 0, 500);

	const string sourceDirectory = m_projInfo.sourceDirectory + "\\";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);

	// Compare Source Directory, Lastest Directory
	vector<pair<DifferentFileType::Enum, string>> diff;
	CompareDirectory(sourceDirectory, m_projInfo.lastestDirectory + "/", diff);

	CString stateStr[] = { L"Add", L"Remove", L"Modify" };
	for each (auto file in diff)
	{
		m_listDiff.InsertItem(0, stateStr[file.first]);
		m_listDiff.SetItem(0, 1, LVIF_TEXT, str2wstr(file.second).c_str(),
			0, 0, 0, 0, 0);
	}

	m_ftpScheduler.Init(m_projInfo.ftpAddr, m_projInfo.ftpId, m_projInfo.ftpPasswd, 
		m_projInfo.ftpDirectory, GetFullFileName(m_projInfo.sourceDirectory));

	return TRUE;
}


void CDiffDialog::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);
}


// Upload Different Files
void CDiffDialog::OnBnClickedButtonUpload()
{
	const string sourceDirectory = m_projInfo.sourceDirectory + "\\";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);

	// Compare Source Directory, Lastest Directory
	vector<pair<DifferentFileType::Enum, string>> diff;
	CompareDirectory(sourceDirectory, m_projInfo.lastestDirectory + "/", diff);

	if (diff.empty())
	{
		Log("Nothing to Upload");
		return;
	}

	//------------------------------------------------------------------------------------------------------------------
	// Update Local Directory

	// Update Lastest Directory
	Log("Update Lastest Directory");

	CheckLocalFolder(diff);
	for each (auto file in diff)
	{
		const string fileName = DeleteCurrentPath(RelativePathTo(sourceFullDirectory, file.second));

		switch (file.first)
		{
		case DifferentFileType::ADD:
		case DifferentFileType::MODIFY:
		{
			const string srcFileName = file.second;
			const string dstFileName = GetFullFileName(m_projInfo.lastestDirectory) + "/" + fileName;
			CopyFileA(srcFileName.c_str(), dstFileName.c_str(), FALSE);
		}
		break;

		case DifferentFileType::REMOVE:
		{
			const string dstFileName = GetFullFileName(m_projInfo.lastestDirectory) + "/" + fileName;
			DeleteFileA(dstFileName.c_str());
		}
		break;
		}
	}

	// Write Version file to Lastest Directory
	cVersionFile verFile = CreateVersionFile(sourceFullDirectory, diff);
	verFile.Write(GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver");

	// Update Backup Directory
	Log("Update Backup Directory");

	const string backupFolderName = GetCurrentDateTime();
	const string dstFolderName = GetFullFileName(m_projInfo.backupDirectory) + "\\" + backupFolderName + "\\";
	const string srcFileCmd = sourceFullDirectory + "\\*";

	CreateDirectoryA(dstFolderName.c_str(), NULL);
	if (FileOperationFunc(FO_COPY, dstFolderName, srcFileCmd) != 0)
	{
		dbg::Log("FileCopy Error = %d \n", GetLastError());
	}

	// Write Backup VersionFile
	verFile.Write(dstFolderName + "/version.ver");



	//------------------------------------------------------------------------------------------------------------------
	// Upload FTP Server Different Files
	Log("Update FTP Server");

	long uploadTotalBytes = 0;
	vector<cFTPScheduler::sCommand> dnFileList;

	for each (auto file in diff)
	{
		const string fileName = DeleteCurrentPath(RelativePathTo(sourceFullDirectory, file.second));
		const string remoteFileName = m_projInfo.ftpDirectory + "/" + fileName;
		uploadTotalBytes += (long)FileSize(file.second);

		if (DifferentFileType::REMOVE == file.first)
		{ // delete
			dnFileList.push_back(
				cFTPScheduler::sCommand(cFTPScheduler::eCommandType::REMOVE
					, remoteFileName));
		}
		else
		{ // add, modify
			dnFileList.push_back(
				cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
					, remoteFileName, file.second));
		}
	}

	dnFileList.push_back(
		cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
			, m_projInfo.ftpDirectory + "/version.ver"
			, GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver"));
	
	// Download Patch Files
	m_isErrorOccur = false;
	m_progTotal.SetRange32(0, (int)uploadTotalBytes);
	m_progTotal.SetPos(0);
	m_writeTotalBytes = 0;
	m_state = eState::UPLOAD;
	m_ftpScheduler.AddCommand(dnFileList);
	m_ftpScheduler.Start();
}


// Check Upload Folder. If Not Exist, Make Folder
void CDiffDialog::CheckFTPFolder(nsFTP::CFTPClient &client, vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	const string sourceDirectory = m_projInfo.sourceDirectory + "\\";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);

	// Collect  Folders
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
	sFolderNode *root = CreateFolderNode(files);
	MakeFTPFolder(client, m_projInfo.ftpDirectory, root);
	DeleteFolderNode(root);
}


void CDiffDialog::MakeFTPFolder(nsFTP::CFTPClient &client, const string &path, sFolderNode *node)
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
void CDiffDialog::CheckLocalFolder(vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	const string sourceDirectory = m_projInfo.sourceDirectory + "\\";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);

	// Collect  Folders
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
	sFolderNode *root = CreateFolderNode(files);
	MakeLocalFolder(GetFullFileName(m_projInfo.lastestDirectory), root);
	DeleteFolderNode(root);
}


void CDiffDialog::MakeLocalFolder(const string &path, sFolderNode *node)
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
cVersionFile CDiffDialog::CreateVersionFile(
	const string &srcDirectoryPath,
	vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	cVersionFile verFile;

	// Read Lastest VersionFile
	if (verFile.Read(GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver"))
		verFile.m_version++; // version update

	verFile.Update(srcDirectoryPath, diffFiles);
	return verFile;
}


void CDiffDialog::Run()
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


void CDiffDialog::MainLoop(const float deltaSeconds)
{
	cFTPScheduler::sState state;
	if (m_ftpScheduler.Update(state))
	{
		if (cFTPScheduler::sState::NONE != state.state)
			LogFTPState(state);
	}

	switch (m_state)
	{
	case eState::WAIT:
		break;

	case eState::UPLOAD:
		switch (state.state)
		{
		case cFTPScheduler::sState::ERR:
			m_isErrorOccur = true;
			break;

		case cFTPScheduler::sState::FINISH:
			FinishUpload();
			break;

		case cFTPScheduler::sState::UPLOAD_BEGIN:
		{
			//const string localFullDirectoryName = GetFullFileName(m_config.m_localDirectory);
			m_staticUploadFile.SetWindowTextW(
				formatw("%s", state.fileName.c_str()).c_str());
		}
		break;

		case cFTPScheduler::sState::UPLOAD:
			m_progUpload.SetRange32(0, state.totalBytes);
			m_progUpload.SetPos(state.progressBytes);

			m_writeTotalBytes += state.readBytes;
			m_progTotal.SetPos(m_writeTotalBytes);

			m_staticUploadPercentage.SetWindowTextW(
				formatw("%d/%d", state.progressBytes, state.totalBytes).c_str());
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


void CDiffDialog::FinishUpload()
{
	if (m_isErrorOccur)
	{
		::AfxMessageBox(L"Error Upload File\n");
		Log("Download Error!!");
	}
	else
	{
		Log("Upload Complete!!");
	}

	// Update Project Information, especially Lastest Version Information
	g_UploaderDlg->UpdateProjectInfo();

	m_state = eState::FINISH;
}


void CDiffDialog::LogFTPState(const cFTPScheduler::sState &state)
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
		//Log(format("Upload... %s", state.fileName.c_str()));
		break;
	case cFTPScheduler::sState::UPLOAD:
		//Log(format("Upload... %s", state.fileName.c_str()));
		break;
	case cFTPScheduler::sState::UPLOAD_DONE:
		//Log(format("Upload Done %s", state.fileName.c_str()));
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


void CDiffDialog::Log(const string &msg)
{
	m_listLog.InsertString(m_listLog.GetCount(), str2wstr(msg).c_str());
	m_listLog.SetCurSel(m_listLog.GetCount() - 1);
	m_listLog.ShowCaret();
}
