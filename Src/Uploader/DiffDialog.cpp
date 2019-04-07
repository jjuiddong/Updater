
#include "stdafx.h"
#include "Uploader.h"
#include "DiffDialog.h"
#include "afxdialogex.h"
#include "UploaderDlg.h"
#include "../ZipLib/ZipFile.h"
#include "../ZipLib/streams/memstream.h"
#include "../ZipLib/methods/Bzip2Method.h"



CDiffDialog::CDiffDialog(CWnd* pParent, const cUploaderConfig::sProjectInfo &info)
	: CDialogEx(IDD_DIALOG_DIFF, pParent)
	, m_projInfo(info)
	, m_isMainLoop(true)
	, m_isErrorOccur(false)
	, m_uploadProgressBytes(0)
	, m_uploadTotalBytes(0)
	, m_state(eState::WAIT)
	, m_isZipLoop(true)
	, m_isBackupLoop(true)
	, m_isLastestLoop(true)
	, m_zipProgressBytes(0)
	, m_zipTotalBytes(0)
	, m_backupProgressBytes(0)
	, m_backupTotalBytes(0)
	, m_lastestProgressBytes(0)
	, m_lastestTotalBytes(0)
	, m_backupProcess(0)
	, m_isLastestUpload(false)
	, m_checkBackup(FALSE)
	, m_backupType(0)
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
	DDX_Control(pDX, IDC_PROGRESS_ZIP, m_progZip);
	DDX_Control(pDX, IDC_STATIC_ZIP_FILE, m_staticZipFile);
	DDX_Control(pDX, IDC_PROGRESS_BACKUP, m_progBackup);
	DDX_Control(pDX, IDC_PROGRESS_LASTEST, m_progLastest);
	DDX_Control(pDX, IDC_STATIC_ZIP_PERCENTAGE, m_staticZipPercentage);
	DDX_Control(pDX, IDC_STATIC_BACKUP_FILE, m_staticBackupFile);
	DDX_Control(pDX, IDC_STATIC_BACKUP_PERCENTAGE, m_staticBackupPercentage);
	DDX_Control(pDX, IDC_STATIC_LASTEST_FILE, m_staticLastestFile);
	DDX_Control(pDX, IDC_STATIC_LASTEST_PERCENTAGE, m_staticLastestPercentage);
	DDX_Check(pDX, IDC_CHECK_BACKUP, m_checkBackup);
}


BEGIN_MESSAGE_MAP(CDiffDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDiffDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDiffDialog::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_UPLOAD, &CDiffDialog::OnBnClickedButtonUpload)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_ALL_LASTEST_FILE_UPLOAD, &CDiffDialog::OnBnClickedButtonAllLastestFileUpload)
END_MESSAGE_MAP()

BEGIN_ANCHOR_MAP(CDiffDialog)
	ANCHOR_MAP_ENTRY(IDC_LIST_DIFF, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_UPLOAD, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_ALL_LASTEST_FILE_UPLOAD, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_LIST_LOG, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_PROGRESS_ZIP, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_PROGRESS_TOTAL, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_PROGRESS_UPLOAD, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_PROGRESS_BACKUP, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_PROGRESS_LASTEST, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_UPLOAD_FILE, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_UPLOAD_PERCENTAGE, ANF_RIGHT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_ZIP_FILE, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_ZIP_PERCENTAGE, ANF_RIGHT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_BACKUP_FILE, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_BACKUP_PERCENTAGE, ANF_RIGHT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_LASTEST_FILE, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_LASTEST_PERCENTAGE, ANF_RIGHT | ANF_RIGHT | ANF_BOTTOM)
END_ANCHOR_MAP()


// CDiffDialog message handlers
void CDiffDialog::OnBnClickedOk()
{
	//CDialogEx::OnOK();
}
void CDiffDialog::OnBnClickedCancel()
{
	m_isMainLoop = false;
	CDialogEx::OnCancel();
}


BOOL CDiffDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitAnchors();

	m_listDiff.InsertColumn(0, L"State", 0, 60);
	m_listDiff.InsertColumn(1, L"File Name", 0, 500);

	const string sourceDirectory = m_projInfo.sourceDirectory + "\\";

	// Compare Source Directory, Lastest Directory
	vector<pair<DifferentFileType::Enum, string>> diff;
	CompareDirectory(sourceDirectory, m_projInfo.lastestDirectory + "\\", diff);

	CString stateStr[] = { L"Add", L"Remove", L"Modify" };
	for (auto &file : diff)
	{
		m_listDiff.InsertItem(0, stateStr[file.first]);
		m_listDiff.SetItem(0, 1, LVIF_TEXT, str2wstr(sourceDirectory + file.second).c_str(),
			0, 0, 0, 0, 0);
	}

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
// Process ->
// 1. Compare Source and Lastest Directory Files
// 2. Check Diffrent File and Listing FileName
// 3. Create Version File
// 4. Zip Upload Files
// 5. Upload Files to FTP Server
// 6. Copy or Remove Lastest Directory Files from Source Directory Files
// 7. Copy Source Directory files to Backup Directory with Zip
// 8. Finish
void CDiffDialog::OnBnClickedButtonUpload()
{
	const string sourceDirectory = m_projInfo.sourceDirectory + "\\";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);

	// Compare Source Directory, Lastest Directory
	vector<pair<DifferentFileType::Enum, string>> diffs;
	CompareDirectory(sourceDirectory, m_projInfo.lastestDirectory + "\\", diffs);

	if (diffs.empty())
	{
		Log("Nothing to Upload");
		return;
	}

	// Write Version file to Lastest Directory
	m_verFile = CreateVersionFile(sourceFullDirectory, diffs);

	//---------------------------------------------------------------------
	// Prepare Zip File to Upload FTP
	vector<cFTPScheduler::sCommand> uploadFileList;
	const long uploadTotalBytes = CreateUploadFiles(m_projInfo.sourceDirectory
		, m_projInfo.ftpDirectory, diffs, uploadFileList);

	// Write Temporal Version file where Uploader.exe directory
	m_verFile.Write("./version.ver");
	const long versionFileSize = (long)common::FileSize("./version.ver");

	// Add Version File
	uploadFileList.push_back(
		cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
			, "version.ver", m_projInfo.ftpDirectory + "/version.ver", "version.ver"
			, "", versionFileSize, versionFileSize));

	//---------------------------------------------------------------------
	Log("Zip Upload File");

	UpdateData(TRUE);
	m_backupType = (m_checkBackup) ? 1 : 0;

	m_isErrorOccur = false;
	m_diffFiles = diffs;
	m_uploadFileList = uploadFileList;
	m_isLastestUpload = false;
	m_ftpScheduler.Init(m_projInfo.ftpAddr, m_projInfo.ftpId, m_projInfo.ftpPasswd,
		m_projInfo.ftpDirectory, GetFullFileName(m_projInfo.sourceDirectory));

	m_isZipLoop = false;
	if (m_zipThread.joinable())
		m_zipThread.join();

	m_state = eState::ZIP;
	m_isZipLoop = true;
	m_zipThread = std::thread(ZipThreadFunction, this);
}


// 가장 최근 파일 전체를 업로드한다.
// 버전관리에 문제가 있을 때, 파일 전체를 덮어씌워서 문제를 해결한다.
// Upload Lastest Directory Files
// Process ->
// 1. Collect Lastest Directory Files
// 2. Zip Upload Files
// 3. Upload Files to FTP Server
// 4. Finish
void CDiffDialog::OnBnClickedButtonAllLastestFileUpload()
{
	if (m_state == eState::UPLOAD)
		return;

	if (IDYES != ::AfxMessageBox(L"Upload All Lastest File?", MB_YESNO))
		return;

	const string lastestFullDirectory = GetFullFileName(m_projInfo.lastestDirectory) + "\\";

	list<string> files;
	CollectFiles2({}, lastestFullDirectory, lastestFullDirectory, files);

	vector<pair<DifferentFileType::Enum, string>> diffs;
	for each (auto file in files)
	{
		if ("version.ver" != file) // except version file, insert in CreateUploadFiles() function
			diffs.push_back({ DifferentFileType::ADD, file });
	}

	vector<cFTPScheduler::sCommand> uploadFileList;
	cVersionFile verFile;
	if (verFile.Read(GetFullFileName(m_projInfo.lastestDirectory) + "\\version.ver"))
	{
		const long uploadTotalBytes = CreateUploadFiles(m_projInfo.lastestDirectory
			, m_projInfo.ftpDirectory, diffs, uploadFileList);

		// Add Version File
		uploadFileList.push_back(
			cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
				, "version.ver"
				, m_projInfo.ftpDirectory + "\\version.ver"
				, GetFullFileName(m_projInfo.lastestDirectory) + "\\version.ver"));

		//---------------------------------------------------------------------
		Log("Zip Upload File");

		m_isErrorOccur = false;
		m_diffFiles = diffs;
		m_uploadFileList = uploadFileList;
		m_isLastestUpload = true;
		m_backupType = 0;
		m_ftpScheduler.Init(m_projInfo.ftpAddr, m_projInfo.ftpId, m_projInfo.ftpPasswd,
			m_projInfo.ftpDirectory, GetFullFileName(m_projInfo.lastestDirectory));

		m_isZipLoop = false;
		if (m_zipThread.joinable())
			m_zipThread.join();

		m_state = eState::ZIP;
		m_isZipLoop = true;
		m_zipThread = std::thread(ZipThreadFunction, this);
	}
	else
	{
		::AfxMessageBox(L"Error!! Upload Lastest File, Not Exist version.ver");
	}
}


// Create FTP Upload File List and Zip
// out1 : Command List
// out2 : ZipFile Name List
// return value : total Upload File Bytes
long CDiffDialog::CreateUploadFiles(
	const string &srcDirectory
	, const string &ftpDirectory
	, const vector<pair<DifferentFileType::Enum, string>> &diffFiles
	, OUT vector<cFTPScheduler::sCommand> &out1
)
{
	const string srcFullDirectory = CheckDirectoryPath(GetFullFileName(srcDirectory) + "\\");

	long uploadTotalBytes = 0;

	for (auto &file : diffFiles)
	{
		const string fileName = file.second;

		if (DifferentFileType::REMOVE == file.first)
		{ // delete
			const string remoteFileName = ftpDirectory + "\\" + fileName;

			out1.push_back(
				cFTPScheduler::sCommand(cFTPScheduler::eCommandType::REMOVE
					, fileName, remoteFileName));

			out1.push_back(
				cFTPScheduler::sCommand(cFTPScheduler::eCommandType::REMOVE
					, fileName, remoteFileName + ".zip"));
		}
		else
		{ // add, modify
			string remoteFileName = ftpDirectory + "\\" + fileName;
			string uploadLocalFileName = srcFullDirectory + file.second;
			string zipFileName;

			const long fileSize = (long)FileSize(srcFullDirectory + file.second);
			uploadTotalBytes += fileSize;

			// if file size 0, ignore zip process
			if (FileSize(srcFullDirectory + file.second) > 0)
			{
				zipFileName = srcFullDirectory + file.second + ".zip";
				remoteFileName += ".zip";
			}

			out1.push_back(
				cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
					, fileName, remoteFileName, uploadLocalFileName, zipFileName, fileSize, fileSize));
		}
	}

	return uploadTotalBytes;
}


// Check Local Folder to Copy Lastest Directory
void CDiffDialog::CheckLocalFolder(const vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	const string sourceDirectory = m_projInfo.sourceDirectory + "\\";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);

	// Collect  Folders
	list<string> files;
	for (auto &file : diffFiles)
	{
		if (DifferentFileType::REMOVE != file.first)
		{
			const string fileName = file.second;
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
		const string folderName = path + "\\" + child.first;
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
	if (verFile.Read(GetFullFileName(m_projInfo.lastestDirectory) + "\\version.ver"))
		verFile.m_version++; // version update

	verFile.Update(srcDirectoryPath, diffFiles);
	return verFile;
}


void CDiffDialog::Run()
{
	MSG msg;
	ZeroMemory(&msg, sizeof(MSG));

	int oldT = timeGetTime();
	while (m_isMainLoop)
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

	// remove temporal zip files
	for (auto &file : m_uploadFileList)
		if (!file.zipFileName.empty())
			remove(file.zipFileName.c_str());
	m_uploadFileList.clear();

	TerminateThread();

	m_ftpScheduler.Clear();
}


void CDiffDialog::MainLoop(const float deltaSeconds)
{
	sMessage message;
	message.type = sMessage::NONE;
	sMessage *front = NULL;
	if (g_message.front(front))
	{
		message = *front;
		g_message.pop();
		LogMessage(message);
	}

	switch (m_state)
	{
	case eState::WAIT: break;
	case eState::ZIP: ZipStateProcess(message); break;
	case eState::UPLOAD: UploadStateProcess(message); break;
	case eState::BACKUP: BackupStateProcess(message); break;
	case eState::FINISH:
	{
		// Trick.. 
		int lower, upper;
		m_progTotal.GetRange(lower, upper);
		m_progTotal.SetPos(upper);

		// remove temporal zip files
		for (auto &file : m_uploadFileList)
			if (!file.zipFileName.empty())
				remove(file.zipFileName.c_str());

		m_state = eState::WAIT;
	}
	break;

	default: assert(!"CDiffDialog::MainLoop undefined state"); break;
	}
}


void CDiffDialog::UploadStateProcess(const sMessage &message)
{
	switch (message.type)
	{
	case sMessage::NONE: break;
	case sMessage::ERR: m_isErrorOccur = true; break;
	case sMessage::FINISH:
	{
		if (m_isErrorOccur)
		{
			::AfxMessageBox(L"Error Upload File\n");
			Log("Upload Error!!");
			m_state = eState::FINISH;
		}
		else
		{
			int lower, upper;
			m_progTotal.GetRange(lower, upper);
			m_progTotal.SetPos(upper);

			// remove temporal zip files
			for (auto &file : m_uploadFileList)
				if (!file.zipFileName.empty())
					remove(file.zipFileName.c_str());

			// Lastest 파일만 업로드할 경우, 백업을 남기지 않는다
			if (m_isLastestUpload)
			{
				m_state = eState::FINISH;
				m_progBackup.SetRange32(0, 1);
				m_progBackup.SetPos(1);
				m_progLastest.SetRange32(0, 1);
				m_progLastest.SetPos(1);
				break;
			}

			// copy backup, lastest folder
			m_isLastestLoop = false;
			if (m_lastestThread.joinable())
				m_lastestThread.join();
			m_isBackupLoop = false;
			if (m_backupThread.joinable())
				m_backupThread.join();

			// compute backup & lastest copy file size
			m_state = eState::BACKUP;

			{
				long lastestTotalBytes = 0;
				for (auto &file : m_uploadFileList)
					if (file.cmd == cFTPScheduler::eCommandType::UPLOAD)
						lastestTotalBytes += file.srcFileSize;

				m_lastestProgressBytes = 0;
				m_lastestTotalBytes = lastestTotalBytes;
				m_progLastest.SetRange32(0, lastestTotalBytes);
				m_progLastest.SetPos(0);
				m_isLastestLoop = true;
				m_lastestThread = std::thread(CDiffDialog::LastestThreadFunction, this);
			}

			if (m_backupType == 1) // backup zip file?
			{
				long backupTotalBytes = 0;
				for (auto &file : m_verFile.m_files)
					backupTotalBytes += file.fileSize;

				m_backupProcess = 0;
				m_backupProgressBytes = 0;
				m_backupTotalBytes = backupTotalBytes;
				m_progBackup.SetRange32(0, backupTotalBytes);
				m_progBackup.SetPos(0);
				m_isBackupLoop = true;
				m_backupThread = std::thread(CDiffDialog::BackupThreadFunction, this);
			}
			else
			{
				m_progBackup.SetRange32(0, 1);
				m_progBackup.SetPos(1);
			}
		}
	}
	break;

	case sMessage::UPLOAD_BEGIN:
		m_staticUploadFile.SetWindowTextW(
			formatw("%s", message.fileName.c_str()).c_str());
		break;

	case sMessage::UPLOAD:
		m_progUpload.SetPos(message.progressBytes);
		m_uploadProgressBytes += message.readBytes;
		m_progTotal.SetPos(m_uploadProgressBytes);
		m_staticUploadPercentage.SetWindowTextW(
			formatw("%d/%d", message.progressBytes, message.totalBytes).c_str());
		break;

	case sMessage::UPLOAD_DONE:
		m_progUpload.SetPos(message.progressBytes);
		m_uploadProgressBytes += message.readBytes;
		m_progTotal.SetPos(m_uploadProgressBytes);
		m_staticUploadPercentage.SetWindowTextW(
			formatw("%d/%d", message.progressBytes, message.totalBytes).c_str());
		break;

	default: assert(!"DiffDialog::UploadStateProcess, undefined message type"); break;
	}
}


void CDiffDialog::ZipStateProcess(const sMessage &message)
{
	switch (message.type)
	{
	case sMessage::NONE: break;
	case sMessage::ERR: m_isErrorOccur = true; break;
	case sMessage::FINISH:
	if (m_isErrorOccur)
	{
		::AfxMessageBox(L"Error Compress File\n");
		Log("Compress File Error!!");
		m_state = eState::FINISH;
	}
	break;

	case sMessage::ZIP_PROCESS_BEGIN:
	{
		m_progZip.SetRange32(0, message.totalBytes);
		m_progZip.SetPos(0);

		m_zipProgressBytes = 0;
		m_zipTotalBytes = message.totalBytes;
		m_staticZipPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_zipProgressBytes, message.totalBytes).c_str());
	}
	break;

	case sMessage::ZIP_BEGIN:
		m_staticZipFile.SetWindowTextW(
			formatw("%s", message.fileName.c_str()).c_str());
		m_staticZipPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_zipProgressBytes, m_zipTotalBytes).c_str());
		break;

	case sMessage::ZIP:
		m_zipProgressBytes += message.progressBytes;
		m_progZip.SetPos(m_zipProgressBytes);
		m_staticZipPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_zipProgressBytes, m_zipTotalBytes).c_str());
		break;

	case sMessage::ZIP_DONE:
		m_zipProgressBytes += message.progressBytes;
		m_progZip.SetPos(m_zipProgressBytes);
		m_staticZipPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_zipProgressBytes, m_zipTotalBytes).c_str());
		break;

	case sMessage::ZIP_PROCESS_DONE:
	{
		m_progZip.SetRange32(0, message.totalBytes);
		m_progZip.SetPos(message.totalBytes);

		m_staticZipPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_zipProgressBytes, message.totalBytes).c_str());

		// update zip file size
		for (auto &file : m_uploadFileList)
		{
			for (auto &ver : m_verFile.m_files)
			{
				if (file.srcFileName == ver.fileName)
				{
					ver.compressSize = file.fileSize;
					break;
				}
			}
		}
		m_verFile.Write(".\\version.ver"); // update file

		//----------------------------------------------------------------------------
		// Upload FTP Server
		Log("Upload FTP Server");

		// Upload Patch Files
		m_isErrorOccur = false;
		m_progTotal.SetRange32(0, (int)GetUploadFileSize());
		m_progTotal.SetPos(0);
		m_uploadProgressBytes = 0;
		m_uploadTotalBytes = GetUploadFileSize();
		m_state = eState::UPLOAD;
		m_ftpScheduler.AddCommand(m_uploadFileList);
		m_ftpScheduler.Start();
	}
	break;

	default: assert(!"Error!! ZipStateProcess, undefined message type"); break;
	}
}


// backup and lastest folder update
void CDiffDialog::BackupStateProcess(const sMessage &message)
{
	switch (message.type)
	{
	case sMessage::NONE: break;
	case sMessage::ERR: m_isErrorOccur = true; break;
	case sMessage::FINISH:
		if (m_isErrorOccur)
		{
			::AfxMessageBox(L"Error Backup File\n");
			Log("Backup Error!!");
			m_state = eState::FINISH;
		}
		break;

	case sMessage::LASTEST_PROCESS_BEGIN:
		Log("Update Lastest Directory");
		break;

	case sMessage::LASTEST:
		m_lastestProgressBytes += message.progressBytes;
		m_progLastest.SetPos(m_lastestProgressBytes);
		m_staticLastestFile.SetWindowTextW(
			formatw("%s", message.fileName.c_str()).c_str());
		m_staticLastestPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_lastestProgressBytes, m_lastestTotalBytes).c_str());
		break;

	case sMessage::LASTEST_PROCESS_DONE:
		m_lastestProgressBytes += message.progressBytes;
		m_progLastest.SetPos(m_lastestProgressBytes);
		++m_backupProcess;
		if ((0 == m_backupType) || (m_backupProcess >= 2))
			FinishUpload();
		break;

	case sMessage::BACKUP_PROCESS_BEGIN:
		Log("Update Backup Directory");
		break;

	case sMessage::BACKUP:
		m_backupProgressBytes += message.progressBytes;
		m_progBackup.SetPos(m_backupProgressBytes);
		m_staticBackupFile.SetWindowTextW(
			formatw("%s", message.fileName.c_str()).c_str());
		m_staticBackupPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_backupProgressBytes, m_backupTotalBytes).c_str());
		break;

	case sMessage::BACKUP_PROCESS_DONE:
		m_backupProgressBytes += message.progressBytes;
		m_progBackup.SetPos(m_backupProgressBytes);
		++m_backupProcess;
		if (m_backupProcess >= 2)
			FinishUpload();
		break;

	default: assert(!"Error!! BackupStateProcess, undefined message type"); break;
	}
}


long CDiffDialog::GetUploadFileSize()
{
	long fileSize = 0;
	for (auto &file : m_uploadFileList)
		fileSize += file.fileSize;
	return fileSize;
}


void CDiffDialog::FinishUpload()
{
	if (m_isErrorOccur)
	{
		::AfxMessageBox(L"Error Upload File\n");
		Log("Upload Error!!");
	}
	else
	{
		Log("Upload Complete!!");
	}

	// Update Project Information, especially Lastest Version Information
	g_UploaderDlg->UpdateProjectInfo();	

	m_state = eState::FINISH;
}


void CDiffDialog::LogMessage(const sMessage &state)
{
	switch (state.type)
	{
	case sMessage::NONE:
		break;
	case sMessage::DOWNLOAD_BEGIN:
		//Log(format("Download... %s, 0/%d", state.fileName.c_str(), state.totalBytes));
		break;
	case sMessage::DOWNLOAD:
		//Log(format("Download... %s, %d/%d", state.fileName.c_str(), state.progressBytes, state.totalBytes));
		break;
	case sMessage::DOWNLOAD_DONE:
		//Log(format("Download Done %s, %d/%d", state.fileName.c_str(), state.progressBytes, state.totalBytes));
		break;
	case sMessage::UPLOAD_BEGIN:
		//Log(format("Upload... %s", state.fileName.c_str()));
		break;
	case sMessage::UPLOAD:
		//Log(format("Upload... %s", state.fileName.c_str()));
		break;
	case sMessage::UPLOAD_DONE:
		//Log(format("Upload Done %s", state.fileName.c_str()));
		break;
	case sMessage::ZIP_PROCESS_BEGIN:
	case sMessage::ZIP_BEGIN:
	case sMessage::ZIP:
	case sMessage::ZIP_DONE:
	case sMessage::ZIP_PROCESS_DONE:
	case sMessage::BACKUP_PROCESS_BEGIN:
	case sMessage::BACKUP:
	case sMessage::BACKUP_PROCESS_DONE:
	case sMessage::LASTEST_PROCESS_BEGIN:
	case sMessage::LASTEST:
	case sMessage::LASTEST_PROCESS_DONE:
		break;

	case sMessage::ERR:
		Log(format("Error = %d, filename = [ %s ], desc = \" %s \""
			, state.data, state.fileName.c_str(), state.desc.c_str()));
		break;
	case sMessage::FINISH:
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


void CDiffDialog::TerminateThread()
{
	m_isZipLoop = false;
	if (m_zipThread.joinable())
		m_zipThread.join();

	m_isLastestLoop = false;
	if (m_lastestThread.joinable())
		m_lastestThread.join();

	m_isBackupLoop = false;
	if (m_backupThread.joinable())
		m_backupThread.join();
}


// compress thread function
void CDiffDialog::ZipThreadFunction(CDiffDialog *dlg)
{
	vector<cFTPScheduler::sCommand> &uploadFileList = dlg->m_uploadFileList;
	int totalZipBytes = 0;
	for (auto &file : uploadFileList)
		if (file.cmd == cFTPScheduler::eCommandType::UPLOAD)
			totalZipBytes += file.fileSize;

	g_message.push(new sMessage(sMessage::ZIP_PROCESS_BEGIN, "", totalZipBytes));

	for (uint i=0; dlg->m_isZipLoop && (i < uploadFileList.size()); ++i)
	{
		auto &file = uploadFileList[i];
		if (file.cmd != cFTPScheduler::eCommandType::UPLOAD)
			continue;

		if (common::GetFileName(file.localFileName) == "version.ver")
			continue; // ignore version file

		g_message.push(new sMessage(sMessage::ZIP_BEGIN, file.localFileName
			, file.fileSize));

		try {
			ZipFile::AddFile(file.zipFileName, file.localFileName
				, GetFileName(file.localFileName), LzmaMethod::Create());
		}
		catch (...)
		{
			// error occured
			g_message.push(new sMessage(sMessage::ERR, file.localFileName
				, 0, 0, 0, sMessage::ZIP
				, "Error Compress file"));

			g_message.push(new sMessage(sMessage::FINISH, "", 0, 0, 0, sMessage::ZIP
				, "Finish Error Compress"));
			break;
		}

		g_message.push(new sMessage(sMessage::ZIP_DONE, file.localFileName
			, file.fileSize, file.fileSize, file.fileSize));

		// update file size
		file.fileSize = (long)common::FileSize(file.zipFileName);
	}

	g_message.push(new sMessage(sMessage::ZIP_PROCESS_DONE, ""
		, totalZipBytes, totalZipBytes, 0));
}


// copy source file to lastest folder
void CDiffDialog::LastestThreadFunction(CDiffDialog *dlg)
{
	g_message.push(new sMessage(sMessage::LASTEST_PROCESS_BEGIN, ""));

	dlg->CheckLocalFolder(dlg->m_diffFiles);

	for (u_int i=0; dlg->m_isLastestLoop && (i < dlg->m_uploadFileList.size()); ++i)
	{
		auto &file = dlg->m_uploadFileList[i];
		const string fileName = file.srcFileName;

		switch (file.cmd)
		{
		case cFTPScheduler::eCommandType::UPLOAD:
		{
			const string srcFileName = file.localFileName;
			const string dstFileName = GetFullFileName(dlg->m_projInfo.lastestDirectory) + "\\" + fileName;
			CopyFileA(srcFileName.c_str(), dstFileName.c_str(), FALSE);

			g_message.push(new sMessage(sMessage::LASTEST, srcFileName, file.srcFileSize
				, file.srcFileSize, file.srcFileSize));
		}
		break;

		case cFTPScheduler::eCommandType::REMOVE:
		{
			const string dstFileName = GetFullFileName(dlg->m_projInfo.lastestDirectory) + "\\" + fileName;
			DeleteFileA(dstFileName.c_str());
		}
		break;
		}
	}

	g_message.push(new sMessage(sMessage::LASTEST_PROCESS_DONE, ""));
}


// copy source file to backup folder with zip
void CDiffDialog::BackupThreadFunction(CDiffDialog *dlg)
{
	g_message.push(new sMessage(sMessage::BACKUP_PROCESS_BEGIN, ""));

	const string srcDirectory = dlg->m_projInfo.sourceDirectory + "\\";
	const string srcFullDirectory = GetFullFileName(srcDirectory);
	const string backupFolderName = GetCurrentDateTime();
	const string dstFileName = GetFullFileName(dlg->m_projInfo.backupDirectory) + "\\"
		+ backupFolderName + ".zip";

	list<string> files;
	CollectFiles2({}, srcFullDirectory, srcFullDirectory, files);
	if (!files.empty())
	{
		files.push_back("version.ver"); // add version file

		auto it = files.begin();
		while (dlg->m_isBackupLoop && (files.end() != it))
		{
			auto &file = *it++;
			string fileName = file;
			string fullFileName = srcFullDirectory + file;
			if (file == "version.ver")
				fullFileName = "version.ver";

			const long fileSize = (long)FileSize(fullFileName);
			if (fileSize <= 0)
				continue; // empty file, ignore

			try {
				ZipFile::AddFile(dstFileName, fullFileName, fileName
					, LzmaMethod::Create());

				g_message.push(new sMessage(sMessage::BACKUP, fileName, fileSize
					, fileSize, fileSize));
			}
			catch (...)
			{
				// error occured
				g_message.push(new sMessage(sMessage::ERR, fileName
					, 0, 0, 0, sMessage::BACKUP
					, "Error Backup Compress file"));

				g_message.push(new sMessage(sMessage::FINISH, "", 0, 0, 0, sMessage::BACKUP
					, "Finish Error Backup Compress"));
				break;
			}
		}
	}

	if (!dlg->m_isBackupLoop) // cancel backup?
		remove(dstFileName.c_str());

	g_message.push(new sMessage(sMessage::BACKUP_PROCESS_DONE, ""));
}
