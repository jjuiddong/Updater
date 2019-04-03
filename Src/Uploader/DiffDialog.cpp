
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
	, m_loop(true)
	, m_isErrorOccur(false)
	, m_uploadBytes(0)
	, m_state(eState::WAIT)
	, m_isZipLoop(true)
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
	DDX_Control(pDX, IDC_STATIC_ZIP_PERCENTAGE, m_staticZipPercentage);
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
	ANCHOR_MAP_ENTRY(IDC_STATIC_UPLOAD_FILE, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_UPLOAD_PERCENTAGE, ANF_RIGHT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_ZIP_FILE, ANF_LEFT | ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_STATIC_ZIP_PERCENTAGE, ANF_RIGHT | ANF_RIGHT | ANF_BOTTOM)
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

	const string sourceDirectory = m_projInfo.sourceDirectory + "/";

	// Compare Source Directory, Lastest Directory
	vector<pair<DifferentFileType::Enum, string>> diff;
	CompareDirectory(sourceDirectory, m_projInfo.lastestDirectory + "/", diff);

	CString stateStr[] = { L"Add", L"Remove", L"Modify" };
	for (auto &file : diff)
	{
		m_listDiff.InsertItem(0, stateStr[file.first]);
		m_listDiff.SetItem(0, 1, LVIF_TEXT, str2wstr(sourceDirectory + file.second).c_str(),
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
// Process ->
// 1. Compare Source and Lastest Directory Files
// 2. Check Diffrent File and Listing FileName
// 3. Create Version File
// 4. Zip Upload Files
// 5. Upload Files to FTP Server
// 6. Copy or Remove Lastest Directory Files from Source Directory Files
// 7. Backup Lastest Directory Files with Zip
// 8. Finish
void CDiffDialog::OnBnClickedButtonUpload()
{
	const string sourceDirectory = m_projInfo.sourceDirectory + "/";
	const string sourceFullDirectory = GetFullFileName(sourceDirectory);

	// Compare Source Directory, Lastest Directory
	vector<pair<DifferentFileType::Enum, string>> diffs;
	CompareDirectory(sourceDirectory, m_projInfo.lastestDirectory + "/", diffs);

	if (diffs.empty())
	{
		Log("Nothing to Upload");
		return;
	}

	// Write Version file to Lastest Directory
	m_verFile = CreateVersionFile(sourceFullDirectory, diffs);

	//------------------------------------------------------------------------------------------------------------------
	// Prepare Zip File to Upload FTP
	vector<cFTPScheduler::sCommand> uploadFileList;
	m_zipFiles.clear();
	const long uploadTotalBytes = CreateUploadFiles(m_projInfo.sourceDirectory
		, m_projInfo.ftpDirectory, diffs, uploadFileList, m_zipFiles);

	// Add Version File
	uploadFileList.push_back(
		cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
			, m_projInfo.ftpDirectory + "/version.ver", "version.ver"));

			//, GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver"));

	// Update Backup Directory with zip
	//Log("Update Backup Directory");

	// Write Version file to Lastest Directory
	//verFile.Write(GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver");
	// Write Backup Folder
	//const string backupFolderName = GetCurrentDateTime();
	//ZipLastestFiles(GetFullFileName(m_projInfo.backupDirectory) + "/" + backupFolderName + ".zip");

	//------------------------------------------------------------------------------------------------------------------
	Log("Zip Upload File");
	m_uploadFileList = uploadFileList;
	m_isZipLoop = false;
	if (m_zipThread.joinable())
		m_zipThread.join();
	m_state = eState::ZIP;
	m_isZipLoop = true;
	m_zipThread = std::thread(ZipThreadFunction, this);

	//------------------------------------------------------------------------------------------------------------------
	// Upload FTP Server
	//Log("Update FTP Server");

	// Upload Patch Files
	//m_ftpScheduler.Init(m_projInfo.ftpAddr, m_projInfo.ftpId, m_projInfo.ftpPasswd,
	//	m_projInfo.ftpDirectory, GetFullFileName(m_projInfo.sourceDirectory));
	
	m_isErrorOccur = false;
	m_progTotal.SetRange32(0, (int)uploadTotalBytes / 2); // 압축한 파일크기를 예상한 크기
	m_progTotal.SetPos(0);
	m_uploadBytes = 0;
	//m_uploadFileList = uploadFileList;
	//m_state = eState::UPLOAD;
	//m_ftpScheduler.AddCommand(uploadFileList);
	//m_ftpScheduler.Start();


	//------------------------------------------------------------------------------------------------------------------
	// Update Local Directory

	// Update Lastest Directory
	//Log("Update Lastest Directory");

	//CheckLocalFolder(diffs);
	//for (auto &file : diffs)
	//{
	//	const string fileName = file.second;

	//	switch (file.first)
	//	{
	//	case DifferentFileType::ADD:
	//	case DifferentFileType::MODIFY:
	//	{
	//		const string srcFileName = sourceFullDirectory + file.second;
	//		const string dstFileName = GetFullFileName(m_projInfo.lastestDirectory) + "/" + fileName;
	//		CopyFileA(srcFileName.c_str(), dstFileName.c_str(), FALSE);
	//	}
	//	break;

	//	case DifferentFileType::REMOVE:
	//	{
	//		const string dstFileName = GetFullFileName(m_projInfo.lastestDirectory) + "/" + fileName;
	//		DeleteFileA(dstFileName.c_str());
	//	}
	//	break;
	//	}
	//}

	// Write Version file to Lastest Directory
	//verFile.Write(GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver");
}


// Create FPT Upload File List and Zip
// out1 : Command List
// out2 : ZipFile Name List
// return value : total Upload File Bytes
long CDiffDialog::CreateUploadFiles(
	const string &srcDirectory
	, const string &ftpDirectory
	, const vector<pair<DifferentFileType::Enum, string>> &diffFiles
	, OUT vector<cFTPScheduler::sCommand> &out1
	, OUT vector<string> &out2
)
{
	const string srcFullDirectory = CheckDirectoryPath(GetFullFileName(srcDirectory) + "/");

	long uploadTotalBytes = 0;

	for (auto &file : diffFiles)
	{
		const string fileName = file.second;

		if (DifferentFileType::REMOVE == file.first)
		{ // delete
			const string remoteFileName = ftpDirectory + "/" + fileName;

			out1.push_back(
				cFTPScheduler::sCommand(cFTPScheduler::eCommandType::REMOVE
					, remoteFileName));

			out1.push_back(
				cFTPScheduler::sCommand(cFTPScheduler::eCommandType::REMOVE
					, remoteFileName + ".zip"));
		}
		else
		{ // add, modify
			string remoteFileName = ftpDirectory + "/" + fileName;
			string uploadLocalFileName = srcFullDirectory + file.second;
			string zipFileName;

			const long fileSize = (long)FileSize(srcFullDirectory + file.second);
			uploadTotalBytes += fileSize;

			// if file size 0, ignore zip process
			if (FileSize(srcFullDirectory + file.second) > 0)
			{
				zipFileName = srcFullDirectory + file.second + ".zip";
				remoteFileName += ".zip";
				out2.push_back(zipFileName);
			}

			out1.push_back(
				cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
					, remoteFileName, uploadLocalFileName, zipFileName, fileSize));
		}
	}

	return uploadTotalBytes;
}


// Check Local Folder to Copy Lastest Directory
void CDiffDialog::CheckLocalFolder(const vector<pair<DifferentFileType::Enum, string>> &diffFiles)
{
	const string sourceDirectory = m_projInfo.sourceDirectory + "/";
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

	m_isZipLoop = false;
	if (m_zipThread.joinable())
		m_zipThread.join();

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
		if (sMessage::NONE != message.type)
			LogFTPState(message);
	}

	switch (m_state)
	{
	case eState::WAIT: break;
	case eState::ZIP: ZipStateProcess(message); break;
	case eState::UPLOAD: UploadStateProcess(message); break;

	case eState::FINISH:
	{
		// Trick.. 
		int lower, upper;
		m_progTotal.GetRange(lower, upper);
		m_progTotal.SetPos(upper);

		// remove temporal zip files
		for each (auto file in m_zipFiles)
			remove(file.c_str());
		m_zipFiles.clear();
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
	case sMessage::FINISH: FinishUpload(); break;

	case sMessage::UPLOAD_BEGIN:
	{
		m_staticUploadFile.SetWindowTextW(
			formatw("%s", message.fileName.c_str()).c_str());
	}
	break;

	case sMessage::UPLOAD:
		//m_progUpload.SetRange32(0, message.totalBytes);
		m_progUpload.SetPos(message.progressBytes);
		m_uploadBytes += message.readBytes;
		m_progTotal.SetPos(m_uploadBytes);
		m_staticUploadPercentage.SetWindowTextW(
			formatw("%d/%d", message.progressBytes, message.totalBytes).c_str());
		break;

	case sMessage::UPLOAD_DONE:
		//m_progUpload.SetRange32(0, message.totalBytes);
		m_progUpload.SetPos(message.progressBytes);
		m_uploadBytes += message.readBytes;
		m_progTotal.SetPos(m_uploadBytes);
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
	case sMessage::ZIP_PROCESS_BEGIN:
	{
		m_progZip.SetRange32(0, message.totalBytes);
		m_progZip.SetPos(0);

		m_zipProgressBytes = 0;
		m_staticUploadPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_zipProgressBytes, message.totalBytes).c_str());
	}
	break;

	case sMessage::ZIP_BEGIN:
		m_staticUploadPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_zipProgressBytes, message.totalBytes).c_str());
		break;

	case sMessage::ZIP:
		m_zipProgressBytes += message.progressBytes;
		m_progZip.SetPos(m_zipProgressBytes);
		m_staticUploadPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_zipProgressBytes, message.totalBytes).c_str());
		break;

	case sMessage::ZIP_DONE:
		m_zipProgressBytes += message.progressBytes;
		m_progZip.SetPos(m_zipProgressBytes);
		m_staticUploadPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_zipProgressBytes, message.totalBytes).c_str());
		break;

	case sMessage::ZIP_PROCESS_DONE:
	{
		m_progZip.SetRange32(0, message.totalBytes);
		m_progZip.SetPos(message.progressBytes);

		m_staticUploadPercentage.SetWindowTextW(
			common::formatw("%d/%d", m_zipProgressBytes, message.totalBytes).c_str());

		//------------------------------------------------------------------------------------------------------------------
		// Upload FTP Server
		Log("Update FTP Server");

		// Upload Patch Files
		m_isErrorOccur = false;
		m_progTotal.SetRange32(0, (int)GetUploadFileSize());
		m_progTotal.SetPos(0);
		m_uploadBytes = 0;
		m_state = eState::UPLOAD;

		m_ftpScheduler.Init(m_projInfo.ftpAddr, m_projInfo.ftpId, m_projInfo.ftpPasswd,
			m_projInfo.ftpDirectory, GetFullFileName(m_projInfo.sourceDirectory));
		m_ftpScheduler.AddCommand(m_uploadFileList);
		m_ftpScheduler.Start();
	}
	break;

	default: assert(!"ZipStateProcess, undefined message type"); break;
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


void CDiffDialog::LogFTPState(const sMessage &state)
{
	switch (state.type)
	{
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
		break;
	case sMessage::ERR:
		Log(format("Error = %d, filename = [ %s ]", state.data, state.fileName.c_str()));
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


void CDiffDialog::OnBnClickedButtonAllLastestFileUpload()
{
	if (m_state == eState::UPLOAD)
		return;

	if (IDYES != ::AfxMessageBox(L"Upload All File?", MB_YESNO))
		return;

	const string lastestFullDirectory = GetFullFileName(m_projInfo.lastestDirectory) + "/";

	list<string> files;
	CollectFiles2({}, lastestFullDirectory, lastestFullDirectory, files);

	vector<pair<DifferentFileType::Enum, string>> diff;
	for each (auto file in files)
	{
		if ("version.ver" != file) // except version file, insert in CreateUploadFiles() function
			diff.push_back({ DifferentFileType::ADD, file });
	}

	vector<cFTPScheduler::sCommand> uploadFileList;
	m_zipFiles.clear();
	cVersionFile verFile;
	if (verFile.Read(GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver"))
	{
		const long uploadTotalBytes = CreateUploadFiles(m_projInfo.lastestDirectory
			, m_projInfo.ftpDirectory, diff, uploadFileList, m_zipFiles);

		// Add Version File
		uploadFileList.push_back(
			cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
				, m_projInfo.ftpDirectory + "/version.ver"
				, GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver"));

		// Upload Patch Files
		m_ftpScheduler.Init(m_projInfo.ftpAddr, m_projInfo.ftpId, m_projInfo.ftpPasswd,
			m_projInfo.ftpDirectory, GetFullFileName(m_projInfo.lastestDirectory));

		m_isErrorOccur = false;
		m_progTotal.SetRange32(0, (int)uploadTotalBytes);
		m_progTotal.SetPos(0);
		m_uploadBytes = 0;
		m_uploadFileList = uploadFileList;
		m_state = eState::UPLOAD;
		m_ftpScheduler.AddCommand(uploadFileList);
		m_ftpScheduler.Start();
	}
	else
	{
		::AfxMessageBox(L"Error!! Upload Lastest File, Not Exist version.ver");
	}
}


// Zip File In Lastest Directory
bool CDiffDialog::ZipLastestFiles(const string &dstFileName)
{
	const string lastestDirectory = m_projInfo.lastestDirectory + "/";
	const string lastestFullDirectory = GetFullFileName(lastestDirectory);

	list<string> files;
	CollectFiles2({}, lastestFullDirectory, lastestFullDirectory, files);
	if (files.empty())
		return false;

	for (auto &file : files)
	{
		if (FileSize(lastestFullDirectory + file) > 0)
			ZipFile::AddFile(dstFileName, lastestFullDirectory+file, file
				, LzmaMethod::Create());
	}

	return true;
}


// compress thread function
void CDiffDialog::ZipThreadFunction(CDiffDialog *dlg)
{
	vector<cFTPScheduler::sCommand> &uploadFileList = dlg->m_uploadFileList;
	int totalZipBytes = 0;
	for (auto &file : uploadFileList)
		totalZipBytes += file.fileSize;

	g_message.push(new sMessage(sMessage::ZIP_PROCESS_BEGIN, "", totalZipBytes));

	for (uint i=0; dlg->m_isZipLoop && (i < uploadFileList.size()); ++i)
	{
		auto &file = uploadFileList[i];
		if (common::GetFileName(file.localFileName) == "version.ver")
			continue; // ignore version file

		g_message.push(new sMessage(sMessage::ZIP_BEGIN, file.localFileName
			, file.fileSize));

		ZipFile::AddFile(file.zipFileName, file.localFileName
			, GetFileName(file.localFileName), LzmaMethod::Create());

		g_message.push(new sMessage(sMessage::ZIP_DONE, file.localFileName
			, file.fileSize, file.fileSize, file.fileSize));

		// update file size
		file.fileSize = (long)common::FileSize(file.zipFileName);
	}

	g_message.push(new sMessage(sMessage::ZIP_PROCESS_DONE, ""
		, totalZipBytes, totalZipBytes, 0));
}
