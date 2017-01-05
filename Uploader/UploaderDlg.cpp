
#include "stdafx.h"
#include "Uploader.h"
#include "UploaderDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CUploaderDlg dialog
CUploaderDlg::CUploaderDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_UPLOADER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CUploaderDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CUploaderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CUploaderDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CUploaderDlg::OnBnClickedCancel)
	ON_BN_CLICKED(IDC_BUTTON_UPLOAD, &CUploaderDlg::OnBnClickedButtonUpload)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD, &CUploaderDlg::OnBnClickedButtonDownload)
END_MESSAGE_MAP()


// CUploaderDlg message handlers

BOOL CUploaderDlg::OnInitDialog()
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

	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CUploaderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CUploaderDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting
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

HCURSOR CUploaderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CUploaderDlg::OnBnClickedOk()
{
	//CDialogEx::OnOK();
}


void CUploaderDlg::OnBnClickedCancel()
{
	CDialogEx::OnCancel();
}


void CUploaderDlg::OnBnClickedButtonUpload()
{
	nsFTP::CFTPClient client;

	nsFTP::CLogonInfo info(
		L"jjuiddong.co.kr"
		,21
		,L"jjuiddong"
		,L"ddong800"
		);
	if (client.Login(info))
	{
		const int files = client.UploadFile(
			L"up files.txt"
			, L"/www/rok/up files.txt"
		);

		if (files > 0)
		{
			int a = 0;
		}
		else
		{
			int a = 0;
		}
	}

}


void CUploaderDlg::OnBnClickedButtonDownload()
{
	nsFTP::CFTPClient client;

	nsFTP::CLogonInfo info(
		L"jjuiddong.co.kr"
		, 21
		, L"jjuiddong"
		, L"ddong800"
	);
	if (client.Login(info))
	{
		const int files = client.DownloadFile(
			L"/www/rok/up files.txt"
			,L"download files.txt"
		);

		if (files > 0)
		{
			int a = 0;
		}
		else
		{
			int a = 0;
		}
	}

}
