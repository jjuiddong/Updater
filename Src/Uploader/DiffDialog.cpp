
#include "stdafx.h"
#include "Uploader.h"
#include "DiffDialog.h"
#include "afxdialogex.h"
#include "UploaderDlg.h"


CDiffDialog::CDiffDialog(CWnd* pParent, const cUploaderConfig::sProjectInfo &info)
	: CDialogEx(IDD_DIALOG_DIFF, pParent)
	, m_projInfo(info)
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
END_ANCHOR_MAP()


// CDiffDialog message handlers
void CDiffDialog::OnBnClickedOk()
{
	//CDialogEx::OnOK();
}
void CDiffDialog::OnBnClickedCancel()
{
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
		m_listLog.InsertString(m_listLog.GetCount(), L"Nothing to Upload");
		return;
	}
	else
	{
		//for each (auto item in diff)
		//	m_listLog.InsertString(m_listLog.GetCount(), str2wstr(item.second).c_str());
	}

	//------------------------------------------------------------------------------------------------------------------
	// Update Local Directory

	// Update Lastest Directory
	m_listLog.InsertString(m_listLog.GetCount(), L"Update Lastest Directory");

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
	m_listLog.InsertString(m_listLog.GetCount(), L"Update Backup Directory");

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
	m_listLog.InsertString(m_listLog.GetCount(), L"Update FTP Server");

	nsFTP::CFTPClient client;
	client.SetResumeMode(false);
	nsFTP::CLogonInfo info(str2wstr(m_projInfo.ftpAddr), 21, str2wstr(m_projInfo.ftpId), str2wstr(m_projInfo.ftpPasswd));
	if (client.Login(info))
	{
		CheckFTPFolder(client, diff);
		for each (auto file in diff)
		{
			const string fileName = DeleteCurrentPath(RelativePathTo(sourceFullDirectory, file.second));
			const string remoteFileName = m_projInfo.ftpDirectory + "/" + fileName;

			if (DifferentFileType::REMOVE == file.first)
			{ // delete
				client.Delete(str2wstr(remoteFileName));
				m_listLog.InsertString(m_listLog.GetCount(),
					formatw("Delete FTP Remote file %s", remoteFileName.c_str()).c_str());
			}
			else
			{ // add, modify
				client.UploadFile(str2wstr(file.second), str2wstr(remoteFileName));

				m_listLog.InsertString(m_listLog.GetCount(),
					formatw("Upload FTP file %s", remoteFileName.c_str()).c_str());
			}
		}

		// Upload VersionFile
		client.UploadFile(str2wstr(GetFullFileName(m_projInfo.lastestDirectory) + "/version.ver"),
			str2wstr(m_projInfo.ftpDirectory + "/version.ver"));
		m_listLog.InsertString(m_listLog.GetCount(), L"Upload VersionFile");
	}

	// Update Project Information, especially Lastest Version Information
	g_UploaderDlg->UpdateProjectInfo();
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

