
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
	, m_writeTotalBytes(0)
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
// 4. Copy or Remove Lastest Directory Files from Source Directory Files
// 5. Create Version File
// 6. Backup Lastest Directory Files with Zip
// 7. Zip Upload Files
// 8. Upload Files to FTP Server
// 9. Finish
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

	//------------------------------------------------------------------------------------------------------------------
	// Update Local Directory

	// Update Lastest Directory
	Log("Update Lastest Directory");

	CheckLocalFolder(diffs);
	for (auto &file : diffs)
	{
		const string fileName = file.second;

		switch (file.first)
		{
		case DifferentFileType::ADD:
		case DifferentFileType::MODIFY:
		{
			const string srcFileName = sourceFullDirectory + file.second;
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
	cVersionFile verFile = CreateVersionFile(sourceFullDirectory, diffs);


	//------------------------------------------------------------------------------------------------------------------
	// Prepare Zip File to Upload FTP
	vector<cFTPScheduler::sCommand> uploadFileList;
	m_zipFiles.clear();
	const long uploadTotalBytes = CreateUploadFiles(m_projInfo.sourceDirectory
		, m_projInfo.ftpDirectory, verFile, diffs, uploadFileList, m_zipFiles);

	// Add Version File
	uploadFileList.push_back(
		cFTPScheduler::sCommand(cFTPScheduler::eCommandType::UPLOAD
			, m_projInfo.ftpDirectory + "/version.ver"
			, GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver"));


	// Update Backup Directory with zip
	Log("Update Backup Directory");

	// Write Version file to Lastest Directory
	verFile.Write(GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver");
	// Write Backup Folder
	//const string backupFolderName = GetCurrentDateTime();
	//ZipLastestFiles(GetFullFileName(m_projInfo.backupDirectory) + "/" + backupFolderName + ".zip");

	Log("Zip Upload File");

	m_uploadFileList = uploadFileList;
	m_state = eState::ZIP_BEGIN;


	//------------------------------------------------------------------------------------------------------------------
	// Upload FTP Server
	//Log("Update FTP Server");

	// Upload Patch Files
	//m_ftpScheduler.Init(m_projInfo.ftpAddr, m_projInfo.ftpId, m_projInfo.ftpPasswd,
	//	m_projInfo.ftpDirectory, GetFullFileName(m_projInfo.sourceDirectory));
	
	m_isErrorOccur = false;
	m_progTotal.SetRange32(0, (int)uploadTotalBytes / 2); // 압축한 파일크기를 예상한 크기
	m_progTotal.SetPos(0);
	m_writeTotalBytes = 0;
	//m_uploadFileList = uploadFileList;
	//m_state = eState::UPLOAD;
	//m_ftpScheduler.AddCommand(uploadFileList);
	//m_ftpScheduler.Start();
}


// Create FPT Upload File List and Zip
// verFile : version file, update compress file size
// out1 : Command List
// out2 : ZipFile Name List
// return value : total Upload File Bytes
long CDiffDialog::CreateUploadFiles(
	const string &srcDirectory
	, const string &ftpDirectory
	, cVersionFile &verFile
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
				//ZipFile::AddFile(zipFileName, srcFullDirectory + file.second
				//	, GetFileName(file.second), LzmaMethod::Create());

				//uploadLocalFileName = zipFileName;
				//uploadLocalFileName = srcFullDirectory + file.second;
				//remoteFileName = ftpDirectory + "/" + fileName + ".zip";
				//const long compressSize = (long)FileSize(zipFileName);
				//uploadTotalBytes += compressSize;
				out2.push_back(zipFileName);

				// update Compressed Size
				//if (cVersionFile::sVersionInfo *p = verFile.GetVersionInfo(fileName))
				//	p->compressSize = compressSize;
			}
			else
			{
				//uploadLocalFileName = srcFullDirectory + file.second;
				//remoteFileName = ftpDirectory + "/" + fileName;
				//uploadTotalBytes += (long)FileSize(srcFullDirectory + file.second);
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
}


void CDiffDialog::MainLoop(const float deltaSeconds)
{
	cFTPScheduler::sState ftpState;
	ftpState.state = cFTPScheduler::sState::NONE;
	if (m_ftpScheduler.Update(ftpState))
	{
		if (cFTPScheduler::sState::NONE != ftpState.state)
			LogFTPState(ftpState);
	}

	switch (m_state)
	{
	case eState::WAIT:
		break;

	case eState::ZIP_BEGIN:
	{
		m_isZipLoop = false;
		if (m_zipThread.joinable())
			m_zipThread.join();

		int totalFileSize = 0;
		for (auto &file : m_uploadFileList)
			totalFileSize += file.fileSize;
		
		m_progZip.SetRange32(0, totalFileSize);
		m_progZip.SetPos(0);

		m_staticUploadPercentage.SetWindowTextW(
			common::formatw("0/%d", totalFileSize).c_str());

		m_zipThread = std::thread(ZipThreadFunction, this);

		m_state = eState::ZIP;
	}
	break;

	case eState::ZIP:
		break;

	case eState::UPLOAD:
		switch (ftpState.state)
		{
		case cFTPScheduler::sState::ERR:
			m_isErrorOccur = true;
			break;

		case cFTPScheduler::sState::FINISH:
			FinishUpload();
			break;

		case cFTPScheduler::sState::UPLOAD_BEGIN:
		{
			m_staticUploadFile.SetWindowTextW(
				formatw("%s", ftpState.fileName.c_str()).c_str());

			// 압축된 파일을 업로드 할 경우, 기존 파일크기보다 작아지니,
			// 전체 업로드 ProgressBar 범위를 재조정한다.
			int uploadTotalBytes = 0;
			for (auto &cmd : m_uploadFileList)
			{
				if (cmd.zipFileName == ftpState.fileName)
				{
					cmd.fileSize = ftpState.totalBytes; // update zip file size
					uploadTotalBytes += ftpState.totalBytes;
				}
				else
				{
					uploadTotalBytes += cmd.fileSize / 2; // 압축된 후 파일크기를 예상한 크기
				}
			}

			const int pos = m_progTotal.GetPos();
			m_progTotal.SetRange32(0, (int)uploadTotalBytes);
			m_progTotal.SetPos(pos);
		}
		break;

		case cFTPScheduler::sState::UPLOAD:
			m_progUpload.SetRange32(0, ftpState.totalBytes);
			m_progUpload.SetPos(ftpState.progressBytes);

			m_writeTotalBytes += ftpState.readBytes;
			m_progTotal.SetPos(m_writeTotalBytes);

			m_staticUploadPercentage.SetWindowTextW(
				formatw("%d/%d", ftpState.progressBytes, ftpState.totalBytes).c_str());
			break;
		}
		break;

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
		Log(format("Error = %d, filename = [ %s ]", state.data, state.fileName.c_str()));
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
		const long uploadTotalBytes = CreateUploadFiles(m_projInfo.lastestDirectory, m_projInfo.ftpDirectory,
			verFile, diff, uploadFileList, m_zipFiles);

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
		m_writeTotalBytes = 0;
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
	vector<cFTPScheduler::sCommand> uploadFileList = dlg->m_uploadFileList;

	while (dlg->m_isZipLoop)
	{
		

	}
}
