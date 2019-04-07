#include "stdafx.h"
#include "Uploader.h"
#include "ProjectEditor.h"
#include "afxdialogex.h"
#include "UploaderDlg.h"


CProjectEditor::CProjectEditor(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_PROJECT, pParent)
	, m_selectProjectId(-1)
{

}

CProjectEditor::~CProjectEditor()
{
}

void CProjectEditor::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_MFCPROPERTYGRID_PROJECT, m_propProject);
	DDX_Control(pDX, IDC_LIST_PROJECT, m_listProject);
}


BEGIN_MESSAGE_MAP(CProjectEditor, CDialogEx)
	ON_BN_CLICKED(IDOK, &CProjectEditor::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CProjectEditor::OnBnClickedCancel)
	ON_WM_SIZE()
	ON_BN_CLICKED(IDC_BUTTON_ADD, &CProjectEditor::OnBnClickedButtonAdd)
	ON_BN_CLICKED(IDC_BUTTON_DELETE, &CProjectEditor::OnBnClickedButtonDelete)
	ON_LBN_SELCHANGE(IDC_LIST_PROJECT, &CProjectEditor::OnSelchangeListProject)
	ON_BN_CLICKED(IDC_BUTTON_REFRESH, &CProjectEditor::OnBnClickedButtonRefresh)
	ON_BN_CLICKED(IDNO, &CProjectEditor::OnBnClickedNo)
	ON_BN_CLICKED(IDC_BUTTON_APP, &CProjectEditor::OnBnClickedButtonApp)
END_MESSAGE_MAP()


BEGIN_ANCHOR_MAP(CProjectEditor)
	ANCHOR_MAP_ENTRY(IDC_MFCPROPERTYGRID_PROJECT, ANF_LEFT | ANF_RIGHT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_LIST_PROJECT, ANF_LEFT | ANF_TOP | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDOK, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDNO, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_APP, ANF_RIGHT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_ADD, ANF_LEFT | ANF_BOTTOM)
	ANCHOR_MAP_ENTRY(IDC_BUTTON_DELETE, ANF_LEFT | ANF_BOTTOM)
END_ANCHOR_MAP()

// CProjectEditor message handlers
void CProjectEditor::OnBnClickedOk()
{
	OnBnClickedButtonApp(); // Save Information
	CDialogEx::OnOK();
}

void CProjectEditor::OnBnClickedNo()
{
	OnBnClickedCancel();
}

void CProjectEditor::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}


BOOL CProjectEditor::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	InitAnchors();

	// to Resize PropertyControl Header Width
	CRect rect;
	GetClientRect(rect);
	m_propProject.GetWindowRect(&rect);
	m_propProject.PostMessage(WM_SIZE, 0, MAKELONG(rect.Width(), rect.Height()));

	ReadConfigFile();

	return TRUE;
}


void CProjectEditor::OnSize(UINT nType, int cx, int cy)
{
	CDialogEx::OnSize(nType, cx, cy);
	CRect rcWnd;
	GetWindowRect(&rcWnd);
	HandleAnchors(&rcWnd);
}


void CProjectEditor::OnBnClickedButtonAdd()
{
	cUploaderConfig::sProjectInfo info;
	info.name = "Project Name";
	info.ftpAddr = "192.168.0.1";
	info.ftpId = "anonymous";
	info.ftpPasswd = "****";
	info.ftpDirectory = "www/rok";
	info.lastestDirectory = "lastest";
	info.backupDirectory = "backup";
	info.sourceDirectory = "source";
	info.exeFileName = "Execute FileName";
	m_projInfos.push_back(info);

	RefreshProjectInfos(m_projInfos);
}


void CProjectEditor::OnBnClickedButtonDelete()
{
	const int projId = m_listProject.GetCurSel();
	if (m_projInfos.size() <= (unsigned int)projId)
		return;

	if (IDYES == ::AfxMessageBox(
		formatw("Delete [ %s ] \n", m_projInfos[projId].name.c_str()).c_str(), MB_YESNO))
	{
		rotatepopvector(m_projInfos, projId);
		RefreshProjectInfos(m_projInfos);
	}	
}


bool CProjectEditor::ReadConfigFile()
{
	cUploaderConfig config;
	if (!config.Read(g_uploaderConfigFileName))
		return false;

	m_projInfos = config.m_projInfos;
	RefreshProjectInfos(config.m_projInfos);

	return true;
}


void CProjectEditor::UpdateProjectInfo(const cUploaderConfig::sProjectInfo &proj)
{
	if (m_propProject.GetPropertyCount() <= 0)
	{
		CMFCPropertyGridProperty *pGroup = new CMFCPropertyGridProperty(_T("Project Information"));
		m_propProject.AddProperty(pGroup);
		pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Project Name"), 
			COleVariant(L"Project Name", VT_BSTR), _T("Project Name")));
		pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("FTP Address"), 
			COleVariant(L"192.168.0.1", VT_BSTR), _T("FTP Address")));
		pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("FTP ID"),
			COleVariant(L"anonymous", VT_BSTR), _T("FTP ID")));
		pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("FTP PassWord"),
			COleVariant(L"****", VT_BSTR), _T("FTP PassWord")));
		pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("FTP Directory"),
			COleVariant(L"/www/rok", VT_BSTR), _T("FTP Directory")));
		TCHAR dir[MAX_PATH];
		GetCurrentDirectory(sizeof(dir), dir);
		pGroup->AddSubItem(new CMFCPropertyGridFileProperty(_T("Lastest Directory"), dir));
		pGroup->AddSubItem(new CMFCPropertyGridFileProperty(_T("Backup Directory"), dir));
		pGroup->AddSubItem(new CMFCPropertyGridFileProperty(_T("Source Directory"), dir));
		pGroup->AddSubItem(new CMFCPropertyGridProperty(_T("Execute FileName"),
			COleVariant(L"excute.exe", VT_BSTR), _T("Execute FileName")));		
	}

	CMFCPropertyGridProperty * pGroup = m_propProject.GetProperty(0);
	pGroup->GetSubItem(0)->SetValue(COleVariant(str2wstr(proj.name).c_str(), VT_BSTR));
	pGroup->GetSubItem(1)->SetValue(COleVariant(str2wstr(proj.ftpAddr).c_str(), VT_BSTR));
	pGroup->GetSubItem(2)->SetValue(COleVariant(str2wstr(proj.ftpId).c_str(), VT_BSTR));
	pGroup->GetSubItem(3)->SetValue(COleVariant(str2wstr(proj.ftpPasswd).c_str(), VT_BSTR));
	pGroup->GetSubItem(4)->SetValue(COleVariant(str2wstr(proj.ftpDirectory).c_str(), VT_BSTR));
	pGroup->GetSubItem(5)->SetValue(COleVariant(str2wstr(proj.lastestDirectory).c_str(), VT_BSTR));
	pGroup->GetSubItem(6)->SetValue(COleVariant(str2wstr(proj.backupDirectory).c_str(), VT_BSTR));
	pGroup->GetSubItem(7)->SetValue(COleVariant(str2wstr(proj.sourceDirectory).c_str(), VT_BSTR));
	pGroup->GetSubItem(8)->SetValue(COleVariant(str2wstr(proj.exeFileName).c_str(), VT_BSTR));
}


void CProjectEditor::OnSelchangeListProject()
{
	if (m_selectProjectId >= 0)
		StoreCurrentProjectInfo();

	const int projId = m_listProject.GetCurSel();
	m_selectProjectId = projId;
	if (projId < 0)
		return;

	if ((int)m_projInfos.size() <= projId)
		return;

	UpdateProjectInfo(m_projInfos[projId]);
}


void CProjectEditor::OnBnClickedButtonRefresh()
{
	ReadConfigFile();
}


void CProjectEditor::RefreshProjectInfos(const vector<cUploaderConfig::sProjectInfo> &projInfos)
{
	m_selectProjectId = -1;

	while (m_listProject.GetCount()>0)
		m_listProject.DeleteString(0);

	for each (auto proj in projInfos)
		m_listProject.AddString(str2wstr(proj.name).c_str());

	if (!projInfos.empty())
	{
		m_selectProjectId = 0;
		m_listProject.SetCurSel(0);
		UpdateProjectInfo(projInfos[0]);
	}
}


void CProjectEditor::StoreCurrentProjectInfo()
{
	if ((int)m_projInfos.size() <= m_selectProjectId)
		return;
	if (m_selectProjectId < 0)
		return;

	CMFCPropertyGridProperty * pGroup = m_propProject.GetProperty(0);
	if (!pGroup)
		return;
	
	cUploaderConfig::sProjectInfo &info = m_projInfos[m_selectProjectId];
	info.name = wstr2str((LPCTSTR)(CString)pGroup->GetSubItem(0)->GetValue());
	info.ftpAddr = wstr2str((LPCTSTR)(CString)pGroup->GetSubItem(1)->GetValue());
	info.ftpId = wstr2str((LPCTSTR)(CString)pGroup->GetSubItem(2)->GetValue());
	info.ftpPasswd = wstr2str((LPCTSTR)(CString)pGroup->GetSubItem(3)->GetValue());
	info.ftpDirectory = wstr2str((LPCTSTR)(CString)pGroup->GetSubItem(4)->GetValue());
	info.lastestDirectory = wstr2str((LPCTSTR)(CString)pGroup->GetSubItem(5)->GetValue());
	info.backupDirectory = wstr2str((LPCTSTR)(CString)pGroup->GetSubItem(6)->GetValue());
	info.sourceDirectory = wstr2str((LPCTSTR)(CString)pGroup->GetSubItem(7)->GetValue());
	info.exeFileName = wstr2str((LPCTSTR)(CString)pGroup->GetSubItem(8)->GetValue());
}


// Apply Button Clicked
void CProjectEditor::OnBnClickedButtonApp()
{
	StoreCurrentProjectInfo();

	const int projId = m_listProject.GetCurSel();

	cUploaderConfig config;
	config.m_projInfos = m_projInfos;
	config.Write(g_uploaderConfigFileName);
	ReadConfigFile();

	if ((projId > 0) && ((int)m_projInfos.size() > projId))
	{
		m_selectProjectId = projId;
		m_listProject.SetCurSel(projId);
		UpdateProjectInfo(m_projInfos[projId]);
	}

	// Update Project Information
	g_UploaderDlg->RefreshProjectInfo();
}
