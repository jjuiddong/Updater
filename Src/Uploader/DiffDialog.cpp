
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
	, m_isLatestLoop(true)
	, m_zipProgressBytes(0)
	, m_zipTotalBytes(0)
	, m_backupProgressBytes(0)
	, m_backupTotalBytes(0)
	, m_latestProgressBytes(0)
	, m_latestTotalBytes(0)
	, m_backupProcess(0)
	, m_isLatestUpload(false)
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
	DDX_Control(pDX, IDC_PROGRESS_LASTEST, m_progLatest);
	DDX_Control(pDX, IDC_STATIC_ZIP_PERCENTAGE, m_staticZipPercentage);
	DDX_Control(pDX, IDC_STATIC_BACKUP_FILE, m_staticBackupFile);
	DDX_Control(pDX, IDC_STATIC_BACKUP_PERCENTAGE, m_staticBackupPercentage);
	DDX_Control(pDX, IDC_STATIC_LASTEST_FILE, m_staticLatestFile);
	DDX_Control(pDX, IDC_STATIC_LASTEST_PERCENTAGE, m_staticLatestPercentage);
	DDX_Check(pDX, IDC_CHECK_BACKUP, m_checkBackup);
}


BEGIN_MESSAGE_MAP(CDiffDialog, CDialogEx)
	ON_BN_CLICKED(IDOK, &CDiffDialog::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CDiffDialog::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_UPLOAD, &CDiffDialog::OnBnClickedButtonUpload)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_ALL_LASTEST_FILE_UPLOAD, &CDiffDialog::OnBnClickedButtonAllLatestFileUpload)
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
	ANCHOR_MAP_ENTRY(IDC_CHECK_BACKUP, ANF_LEFT | ANF_BOTTOM)
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

	// Compare Source Directory, Latest Directory
	vector<pair<DifferentFileType::Enum, string>> diff;
	CompareDirectory(sourceDirectory, m_projInfo.latestDirectory + "\\", diff);

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
// 1. Compare Source and Latest Directory Files
// 2. Check Diffrent File and Listing FileName
// 3. Create Version File
// 4. Download Version File from FTP Server
// 5. Compare FTP Server Version File and Update Version File
// 6. Decided which file to upload
// 7. Zip Upload Files
// 8. Upload Files to FTP Server
// 9. Copy or Remove Latest Directory Files from Source Directory Files
// 10. Copy Source Directory files to Backup Directory with Zip
// 11. Finish
void CDiffDialog::OnBnClickedButtonUpload()
{
	const string sourceDirectory = m_projInfo.sourceDirectory + "\\";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);

	// Compare Source Directory, Latest Directory
	vector<pair<DifferentFileType::Enum, string>> diffs;
	CompareDirectory(sourceDirectory, m_projInfo.latestDirectory + "\\", diffs);

	if (diffs.empty())
	{
		Log("Nothing to Upload");
		return;
	}

	// Create Version file Compare Source and Latest Directory
	m_verFile = CreateVersionFile(sourceFullDirectory, diffs);

	//---------------------------------------------------------------------
	// Prepare Zip File to Upload FTP
	cFileList uploadFiles;
	const long uploadTotalBytes = CreateUploadFiles(m_projInfo.sourceDirectory
		, diffs, uploadFiles);

	// Write Version file
	m_verFile.Write(ftppath::GetLocalFileName(m_projInfo.sourceDirectory
		, "version.ver"));

	// Add Version File
	uploadFiles.AddFile(
		cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD, "version.ver"));

	//---------------------------------------------------------------------
	Log("Zip Upload File");

	UpdateData(TRUE);
	m_backupType = (m_checkBackup) ? 1 : 0;

	m_isErrorOccur = false;
	m_diffFiles = diffs;
	m_uploadFiles = uploadFiles;
	m_isLatestUpload = false;
	m_sourceDirectoryPath = m_projInfo.sourceDirectory;
	m_ftpScheduler.Init(m_projInfo.ftpAddr, m_projInfo.ftpId, m_projInfo.ftpPasswd,
		m_projInfo.ftpDirectory, m_projInfo.sourceDirectory);

	m_state = eState::CHECK_VER;
	DownloadVersionFile();
}


// 가장 최근 파일 전체를 업로드한다.
// 버전관리에 문제가 있을 때, 파일 전체를 덮어씌워서 문제를 해결한다.
// Upload Latest Directory Files
// Process ->
// 1. Collect Latest Directory Files
// 2. Download Version File from FTP Server
// 3. Compare FTP Server Version File and Update Version File
// 4. Zip Upload Files
// 5. Upload Files to FTP Server
// 6. Finish
void CDiffDialog::OnBnClickedButtonAllLatestFileUpload()
{
	if (m_state == eState::UPLOAD)
		return;

	if (IDYES != ::AfxMessageBox(L"Upload All Latest File?", MB_YESNO))
		return;

	const string latestFullDirectory = GetFullFileName(m_projInfo.latestDirectory) + "\\";

	list<string> files;
	CollectFiles2({}, latestFullDirectory, latestFullDirectory, files);

	vector<pair<DifferentFileType::Enum, string>> diffs;
	for each (auto file in files)
	{
		if ("version.ver" != file) // except version file, insert in CreateUploadFiles() function
			diffs.push_back({ DifferentFileType::ADD, file });
	}

	cFileList uploadFiles;
	cVersionFile verFile;
	if (!verFile.Read(GetFullFileName(m_projInfo.latestDirectory) + "\\version.ver"))
	{
		::AfxMessageBox(L"Error!! Upload Latest File, Not Exist version.ver");
		return;
	}

	const long uploadTotalBytes = CreateUploadFiles(m_projInfo.latestDirectory
		, diffs, uploadFiles);

	// Add Version File
	uploadFiles.AddFile( cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
			, "version.ver"));

	//---------------------------------------------------------------------
	Log("Zip Upload File");

	m_isErrorOccur = false;
	m_diffFiles = diffs;
	m_uploadFiles = uploadFiles;
	m_isLatestUpload = true;
	m_sourceDirectoryPath = m_projInfo.latestDirectory;
	m_backupType = 0;
	m_ftpScheduler.Init(m_projInfo.ftpAddr, m_projInfo.ftpId, m_projInfo.ftpPasswd,
		m_projInfo.ftpDirectory, m_projInfo.latestDirectory);

	m_state = eState::CHECK_VER;
	DownloadVersionFile();
}


// Create FTP Upload File List
// out : Upload FileList
// return value : total Upload File Bytes
long CDiffDialog::CreateUploadFiles(
	const string &srcDirectory
	, const vector<pair<DifferentFileType::Enum, string>> &diffFiles
	, OUT cFileList &out
)
{
	long uploadTotalBytes = 0;

	for (auto &file : diffFiles)
	{
		const string fileName = file.second;

		if (DifferentFileType::REMOVE == file.first)
		{ // delete
			out.AddFile(cFTPScheduler::sCommand(cFTPScheduler::eCommandType::REMOVE
				, fileName, true));
		}
		else
		{ // add, modify

			const long fileSize = (long)FileSize(
				ftppath::GetLocalFileName(srcDirectory, fileName));
			uploadTotalBytes += fileSize;

			out.AddFile( cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
					, fileName, fileSize > 0));
		}
	}

	return uploadTotalBytes;
}


// Check Local Folder to Copy Latest Directory
void CDiffDialog::CheckLocalFolder(const vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
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
	MakeLocalFolder(GetFullFileName(m_projInfo.latestDirectory), root);
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


// Read Latest VersionFile and then Merge DifferentFiles
cVersionFile CDiffDialog::CreateVersionFile(
	const string &srcDirectoryPath,
	vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	cVersionFile verFile;

	// Read Latest VersionFile
	if (verFile.Read(GetFullFileName(m_projInfo.latestDirectory) + "\\version.ver"))
		verFile.m_version++; // version update

	verFile.Update(srcDirectoryPath, diffFiles);
	return verFile;
}


void CDiffDialog::DownloadVersionFile()
{
	// FTP에서 다운로드 받을 version.ver 파일은 임시 디렉토리에 
	// 저장하고, 로컬 디렉토리에 저장된 version 파일과 비교한다.
	char tempPath[MAX_PATH];
	GetCurrentDirectoryA(ARRAYSIZE(tempPath), tempPath);

	// create temp folder
	string temporalDownloadDirectoryPath = tempPath;
	temporalDownloadDirectoryPath += "/temp";
	CreateDirectoryA(temporalDownloadDirectoryPath.c_str(), NULL);

	//CreateDirectoryA
	m_temporalDownloadDirectoryPath = temporalDownloadDirectoryPath;

	if (!m_ftpScheduler.Init(m_projInfo.ftpAddr
		, m_projInfo.ftpId
		, m_projInfo.ftpPasswd
		, m_projInfo.ftpDirectory
		, temporalDownloadDirectoryPath))
	{
		AfxMessageBox(L"FTP Scheduler Error!!");
		SetWindowText(L"DownLoader - < Error!! FTP Connection >");
		return;
	}

	cFileList dnFiles;
	dnFiles.AddFile(
		cFTPScheduler::sCommand(cFTPScheduler::eCommandType::DOWNLOAD, "version.ver"));

	m_ftpScheduler.ClearFileList();
	m_ftpScheduler.AddFileList(dnFiles);
	m_ftpScheduler.Start();
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
		if ((m_state == eState::WAIT) || (m_state == eState::FINISH))
			Sleep(1);

		const float deltaSeconds = (curT - oldT) * 0.001f;
		oldT = curT;
		MainLoop(deltaSeconds);
	}

	// remove temporal zip files
	m_uploadFiles.RemoveTemporalZipFile(m_sourceDirectoryPath);
	m_uploadFiles.Clear();
	// remove version file
	remove(ftppath::GetLocalFileName(
		m_projInfo.sourceDirectory, "version.ver").c_str());

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
	case eState::CHECK_VER: CheckVersionStateProcess(message); break;
	case eState::ZIP: ZipStateProcess(message); break;
	case eState::UPLOAD: UploadStateProcess(message); break;
	case eState::BACKUP: BackupStateProcess(message); break;
	case eState::FINISH:
	{
		// Trick.. 
		int lower, upper;
		m_progTotal.GetRange(lower, upper);
		m_progTotal.SetPos(upper);
		m_uploadFiles.RemoveTemporalZipFile(m_sourceDirectoryPath);
		// remove version file
		remove(ftppath::GetLocalFileName(
			m_projInfo.sourceDirectory, "version.ver").c_str());
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
			m_uploadFiles.RemoveTemporalZipFile(m_sourceDirectoryPath);

			// Latest 파일만 업로드할 경우, 백업을 남기지 않는다
			if (m_isLatestUpload)
			{
				m_state = eState::FINISH;
				m_progBackup.SetRange32(0, 1);
				m_progBackup.SetPos(1);
				m_progLatest.SetRange32(0, 1);
				m_progLatest.SetPos(1);
				break;
			}

			// copy backup, latest folder
			m_isLatestLoop = false;
			if (m_latestThread.joinable())
				m_latestThread.join();
			m_isBackupLoop = false;
			if (m_backupThread.joinable())
				m_backupThread.join();

			// compute backup & latest copy file size
			m_state = eState::BACKUP;

			// 최신 파일 폴더로 업데이트될 파일들을 검사한다.
			{
				const long latestTotalBytes = 
					m_uploadFiles.GetTotalSourceFileSize(m_sourceDirectoryPath);

				m_latestProgressBytes = 0;
				m_latestTotalBytes = latestTotalBytes;
				m_progLatest.SetRange32(0, latestTotalBytes);
				m_progLatest.SetPos(0);
				m_isLatestLoop = true;
				m_latestThread = std::thread(CDiffDialog::LatestThreadFunction, this);
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


// FTP서버의 version파일과 local의 파일을 비교해서, 최신버전으로
// 업데이트한다.
void CDiffDialog::CheckVersionStateProcess(const sMessage &message)
{
	if (sMessage::ERR == message.type)
	{
		string msg;
		switch (message.data)
		{
		case sMessage::LOGIN:
			msg = "Error FTP Connection\n";
			Log("Error FTP Connection\n");
			Log("- Check ConfigFile FTP Address, id, pass\n");
			m_isErrorOccur = true;
			::AfxMessageBox(str2wstr(msg).c_str());
			break;

		case sMessage::DOWNLOAD:
			// no version file
			break;

		default:
			msg = "Error Download Version File\n";
			Log("Error Download Version File");
			Log(common::format("- error code = %d", message.data));
			m_isErrorOccur = true;
			::AfxMessageBox(str2wstr(msg).c_str());
			break;
		}
	}
	else if (sMessage::FINISH == message.type)
	{
		if (!m_isErrorOccur)
			CheckVersionFile();
	}
}


// FTP에서 임시로 받은 version.ver 파일과 현재 로컬에 저장된
// version.ver과 비교해서, 업데이트될 버전을 검사한다.
void CDiffDialog::CheckVersionFile()
{
	vector<cVersionFile::sCompareInfo> compResult;

	cVersionFile remoteVer;
	if (!remoteVer.Read(ftppath::GetLocalFileName(m_temporalDownloadDirectoryPath
		, "version.ver")))
		goto next; // not exist version file, next step

	if (m_verFile.Compare(remoteVer, compResult) <= 0)
		goto next; // no update file, next step;

	// 버전 파일을 검사해서, 최신 버전정보로 업데이트한다.
	// Uploader가 두 개이상의 컴퓨터에서 파일을 업로드할 경우
	// 서로 다른 버전파일이 문제를 일으킬수 있기 때문에
	// FTP서버에 있는 파일을 최신파일로 설정하기 위한 작업이다.
	//
	// 1. 버전 값이 높은것으로 대체한다.
	// 2. 지워진 파일은 무시한다. (다시 업로드할수 있기 때문에)
	// 3. 새로 추가된 파일은 무시한다. (새로 추가되었기 때문에 버전값은 1이다)

	for (auto &comp : compResult)
	{
		if (cVersionFile::sCompareInfo::UPDATE != comp.state)
			continue;
		if (auto *ver = m_verFile.GetVersionInfo(comp.fileName))
			if (ver->version >= 0) // 지워질 파일은 version을 바꾸지 않는다.
				ver->version = comp.maxVersion;
	}

next:
	// 다음단계인 압축단계로 넘어간다.

	// update ftp source directory path
	m_ftpScheduler.Init(m_projInfo.ftpAddr, m_projInfo.ftpId, m_projInfo.ftpPasswd,
		m_projInfo.ftpDirectory, m_sourceDirectoryPath);

	// terminate and resume zip thread
	m_isZipLoop = false;
	if (m_zipThread.joinable())
		m_zipThread.join();

	m_state = eState::ZIP;
	m_isZipLoop = true;
	m_zipThread = std::thread(ZipThreadFunction, this);
	return;
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
		for (auto &file : m_uploadFiles.m_files)
		{
			for (auto &ver : m_verFile.m_files)
			{
				if (ver.fileName == file.fileName)
				{
					if (file.isCompressed)
					{
						const long fileSize = (long)common::FileSize(
							ftppath::GetLocalFileName(m_sourceDirectoryPath
								, file.fileName) + ".zip");
						ver.compressSize = fileSize;
					}
					break;
				}
			}
		}
		m_verFile.Write(ftppath::GetLocalFileName(m_projInfo.sourceDirectory
			, "version.ver")); // update file

		//----------------------------------------------------------------------------
		// Upload FTP Server
		Log("Upload FTP Server");

		// Upload Patch Files
		m_isErrorOccur = false;
		m_progTotal.SetRange32(0, (int)m_uploadFiles.GetUploadFileSize(m_sourceDirectoryPath));
		m_progTotal.SetPos(0);
		m_uploadProgressBytes = 0;
		m_uploadTotalBytes = m_uploadFiles.GetUploadFileSize(m_sourceDirectoryPath);
		m_state = eState::UPLOAD;
		m_ftpScheduler.ClearFileList();
		m_ftpScheduler.AddFileList(m_uploadFiles);
		m_ftpScheduler.Start();
	}
	break;

	default: assert(!"Error!! ZipStateProcess, undefined message type"); break;
	}
}


// backup and latest folder update
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

	case sMessage::LATEST_PROCESS_BEGIN:
		Log("Update Latest Directory");
		break;

	case sMessage::LATEST:
		m_latestProgressBytes += message.progressBytes;
		m_progLatest.SetPos(m_latestProgressBytes);
		m_staticLatestFile.SetWindowTextW(
			formatw("%s", message.fileName.c_str()).c_str());
		m_staticLatestPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_latestProgressBytes, m_latestTotalBytes).c_str());
		break;

	case sMessage::LATEST_PROCESS_DONE:
		m_latestProgressBytes += message.progressBytes;
		m_progLatest.SetPos(m_latestProgressBytes);
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

	// Update Project Information, especially Latest Version Information
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
	case sMessage::LATEST_PROCESS_BEGIN:
	case sMessage::LATEST:
	case sMessage::LATEST_PROCESS_DONE:
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

	m_isLatestLoop = false;
	if (m_latestThread.joinable())
		m_latestThread.join();

	m_isBackupLoop = false;
	if (m_backupThread.joinable())
		m_backupThread.join();
}


// compress thread function
void CDiffDialog::ZipThreadFunction(CDiffDialog *dlg)
{
	cFileList &uploadFiles = dlg->m_uploadFiles;
	const string &sourceDirectory = dlg->m_sourceDirectoryPath;

	const int totalZipBytes = uploadFiles.GetTotalUploadSourceFileSize(sourceDirectory);

	g_message.push(new sMessage(sMessage::ZIP_PROCESS_BEGIN, "", totalZipBytes));

	for (uint i=0; dlg->m_isZipLoop && (i < uploadFiles.m_files.size()); ++i)
	{
		auto &file = uploadFiles.m_files[i];
		if (file.cmd != cFTPScheduler::eCommandType::UPLOAD)
			continue;

		if (file.fileName == "version.ver")
			continue; // ignore version file

		const string localFileName = ftppath::GetLocalFileName(sourceDirectory
			, file.fileName);

		const string zipFileName = ftppath::GetLocalFileName(sourceDirectory
			, file.fileName) + ".zip";

		const long fileSize = (long)common::FileSize(localFileName);

		g_message.push(new sMessage(sMessage::ZIP_BEGIN, file.fileName, fileSize));

		try {
			ZipFile::AddFile(zipFileName, localFileName, GetFileName(file.fileName)
				, DeflateMethod::Create());
		}
		catch (...)
		{
			// error occured
			g_message.push(new sMessage(sMessage::ERR, localFileName
				, 0, 0, 0, sMessage::ZIP
				, "Error Compress file"));

			g_message.push(new sMessage(sMessage::FINISH, "", 0, 0, 0, sMessage::ZIP
				, "Finish Error Compress"));
			break;
		}

		g_message.push(new sMessage(sMessage::ZIP_DONE, localFileName
			, fileSize, fileSize, fileSize));
	}

	g_message.push(new sMessage(sMessage::ZIP_PROCESS_DONE, ""
		, totalZipBytes, totalZipBytes, 0));
}


// copy source file to latest folder
void CDiffDialog::LatestThreadFunction(CDiffDialog *dlg)
{
	g_message.push(new sMessage(sMessage::LATEST_PROCESS_BEGIN, ""));

	dlg->CheckLocalFolder(dlg->m_diffFiles);

	for (u_int i=0; dlg->m_isLatestLoop && (i < dlg->m_uploadFiles.m_files.size()); ++i)
	{
		auto &file = dlg->m_uploadFiles.m_files[i];

		const string localFileName = ftppath::GetLocalFileName(
			dlg->m_sourceDirectoryPath, file.fileName);

		const long fileSize = (long)common::FileSize(localFileName);

		switch (file.cmd)
		{
		case cFTPScheduler::eCommandType::UPLOAD:
		{
			const string srcFileName = localFileName;
			const string dstFileName = ftppath::GetLocalFileName(
				dlg->m_projInfo.latestDirectory, file.fileName);
			CopyFileA(srcFileName.c_str(), dstFileName.c_str(), FALSE);

			g_message.push(new sMessage(sMessage::LATEST, srcFileName, fileSize
				, fileSize, fileSize));
		}
		break;

		case cFTPScheduler::eCommandType::REMOVE:
		{
			const string dstFileName = ftppath::GetLocalFileName(
				dlg->m_projInfo.latestDirectory, file.fileName);
			DeleteFileA(dstFileName.c_str());
		}
		break;
		}
	}

	g_message.push(new sMessage(sMessage::LATEST_PROCESS_DONE, ""));
}


// copy source file to backup folder with zip
void CDiffDialog::BackupThreadFunction(CDiffDialog *dlg)
{
	g_message.push(new sMessage(sMessage::BACKUP_PROCESS_BEGIN, ""));

	const string srcDirectory = dlg->m_sourceDirectoryPath + "\\";
	const string srcFullDirectory = GetFullFileName(srcDirectory);
	const string backupFolderName = GetCurrentDateTime();
	const string dstFileName = ftppath::GetLocalFileName(
			dlg->m_projInfo.backupDirectory, backupFolderName + ".zip");

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
					, DeflateMethod::Create());

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
