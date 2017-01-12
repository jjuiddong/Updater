#pragma once
#include "afxpropertygridctrl.h"
#include "afxwin.h"

class CProjectEditor : public CDialogEx
{
public:
	CProjectEditor(CWnd* pParent = NULL);   // standard constructor
	virtual ~CProjectEditor();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_DIALOG_PROJECT };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	bool ReadConfigFile();
	void UpdateProjectInfo(const cUploaderConfig::sProjectInfo &proj);
	void RefreshProjectInfos(const vector<cUploaderConfig::sProjectInfo> &projInfos);
	void StoreCurrentProjectInfo();


	int m_selectProjectId;
	vector<cUploaderConfig::sProjectInfo> m_projInfos;


	DECLARE_MESSAGE_MAP()
	DECLARE_ANCHOR_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	CMFCPropertyGridCtrl m_propProject;
	CListBox m_listProject;
	afx_msg void OnSize(UINT nType, int cx, int cy);
	afx_msg void OnBnClickedButtonAdd();
	afx_msg void OnBnClickedButtonDelete();
	afx_msg void OnSelchangeListProject();
	afx_msg void OnBnClickedButtonRefresh();
	afx_msg void OnBnClickedNo();
	afx_msg void OnBnClickedButtonApp();
};
