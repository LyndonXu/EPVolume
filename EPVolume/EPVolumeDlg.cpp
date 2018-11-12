
// EPVolumeDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "EPVolume.h"
#include "EPVolumeDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define MSG_SPEAKER		(WM_USER + 200)
#define MSG_PHONE		(WM_USER + 201)
#define WM_SHOWTASK		(WM_USER + 202)



// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CEPVolumeDlg 对话框



CEPVolumeDlg::CEPVolumeDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_EPVOLUME_DIALOG, pParent)
	, m_pVolumeCtrl(NULL)
	, m_s32ChosenColor(0)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEPVolumeDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_Dev, m_csComboDev);
	DDX_Control(pDX, IDC_MFC_COLOR_BTN, m_csMFCColorBtn);
}

BEGIN_MESSAGE_MAP(CEPVolumeDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_HSCROLL()
	ON_BN_CLICKED(IDOK, &CEPVolumeDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDC_CHECK_Speaker, &CEPVolumeDlg::OnBnClickedCheckSpeaker)
	ON_BN_CLICKED(IDC_CHECK_Phone, &CEPVolumeDlg::OnBnClickedCheckPhone)
	ON_MESSAGE(MSG_SPEAKER, MsgSpeaker)
	ON_MESSAGE(MSG_PHONE, MsgPhone)
	ON_MESSAGE(WM_SHOWTASK, OnShowTask)
	ON_WM_DESTROY()
	ON_BN_CLICKED(IDC_BTN_Reg, &CEPVolumeDlg::OnBnClickedBtnReg)
	ON_CBN_SELCHANGE(IDC_COMBO_Dev, &CEPVolumeDlg::OnCbnSelchangeComboDev)
	ON_BN_CLICKED(IDC_MFC_COLOR_BTN, &CEPVolumeDlg::OnBnClickedMfcColorBtn)
	ON_NOTIFY_EX_RANGE(TTN_NEEDTEXT, 0, 0xFFFF, &CEPVolumeDlg::OnToolTipNotify)
END_MESSAGE_MAP()


INT32 EnumAudioDeviceCB(EmAudioDeviceType emType, const WCHAR *pDevName, const WCHAR *pDevID, void *pContext)
{
	TRACE(L"\nemType:%d, device name: %s, ID: %s\n", emType, pDevName, pDevID);
	if (emType == _Audio_Device_Normal)
	{
		CEPVolumeDlg *pDlg = (CEPVolumeDlg *)pContext;
		pDlg->m_csComboDev.AddString(pDevName);
		pDlg->m_csMapAudioOutDev.insert(make_pair(pDevName, pDevID));
	}
	return 0;
}


// CEPVolumeDlg 消息处理程序

BOOL CEPVolumeDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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
		pSysMenu->EnableMenuItem(SC_CLOSE, MF_GRAYED);
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	((CSliderCtrl *)GetDlgItem(IDC_SLIDER_Speaker))->SetRange(0, 100);
	((CSliderCtrl *)GetDlgItem(IDC_SLIDER_Phone))->SetRange(0, 100);

	m_pVolumeCtrl = new CEPVolumeCtrl;
	if (m_pVolumeCtrl == NULL)
	{
		return FALSE;
	}

	m_pVolumeCtrl->EnumAudioDevice(_DEV_Audio_Type_OUT, EnumAudioDeviceCB, this);

	m_csComboDev.SetCurSel(0);

	m_pVolumeCtrl->StartVolumeCtrl(_DEV_Audio_Type_IN, L"High Definition Audio",
		GetSafeHwnd(), MSG_PHONE);
	m_pVolumeCtrl->StartVolumeCtrl(_DEV_Audio_Type_OUT, L"High Definition Audio",
		GetSafeHwnd(), MSG_SPEAKER);

	INT nVolume = 0;
	m_pVolumeCtrl->GetVolume(_DEV_Audio_Type_IN, &nVolume);
	((CSliderCtrl *)GetDlgItem(IDC_SLIDER_Phone))->SetPos(nVolume);

	m_pVolumeCtrl->GetVolume(_DEV_Audio_Type_OUT, &nVolume);
	((CSliderCtrl *)GetDlgItem(IDC_SLIDER_Speaker))->SetPos(nVolume);

	BOOL bMute;

	m_pVolumeCtrl->GetMute(_DEV_Audio_Type_IN, &bMute);
	((CButton *)GetDlgItem(IDC_CHECK_Phone))->SetCheck(bMute ? BST_CHECKED : BST_UNCHECKED);

	m_pVolumeCtrl->GetMute(_DEV_Audio_Type_OUT, &bMute);
	((CButton *)GetDlgItem(IDC_SLIDER_Speaker))->SetCheck(bMute ? BST_CHECKED : BST_UNCHECKED);

#if 0
	NotifyInit();
	ShowWindow(SW_MINIMIZE);
	PostMessage(WM_SYSCOMMAND, SC_MINIMIZE, 0);
#endif

	m_s32ChosenColor = RGB(255, 0, 0);

	//m_csMFCColorBtn.EnableAutomaticButton(_T("目前"), m_s32ChosenColor);
	m_csMFCColorBtn.SetColor(m_s32ChosenColor);
	m_csMFCColorBtn.EnableOtherButton(_T("其它颜色"));
	//m_csMFCColorBtn.SetColor((COLORREF)-1);
	//m_csMFCColorBtn.SetColumnsNumber(5);

	{
		//EnableToolTips();

		m_csToolTips.Create(this);
		m_csToolTips.Activate(TRUE);

		m_csToolTips.AddTool(GetDlgItem(IDC_SLIDER_Speaker), LPSTR_TEXTCALLBACK); //注释：红色表示tip内容可以动态改变！！
		m_csToolTips.AddTool(GetDlgItem(IDC_SLIDER_Phone), LPSTR_TEXTCALLBACK); //注释：红色表示tip内容可以动态改变！！


		//RECT stRect = { 0, -32, 32, 0 };
		//m_csToolTips.AddTool(this, L"Tips", &stRect, IDD_DLG_Show);
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CEPVolumeDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else if (nID == SC_MAXIMIZE) //最大化
	{
		this->ShowWindow(SW_SHOW);
	}
	else if (nID == SC_MINIMIZE) //最小化，把他隐藏起来
	{
		this->ShowWindow(SW_HIDE);
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEPVolumeDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CEPVolumeDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CEPVolumeDlg::OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	if (pScrollBar != NULL && nSBCode < SB_ENDSCROLL)
	{
		UINT nId = pScrollBar->GetDlgCtrlID();
		INT nVolume = ((CSliderCtrl*)pScrollBar)->GetPos();
		if (nId == IDC_SLIDER_Speaker)
		{
			m_pVolumeCtrl->SetVolume(_DEV_Audio_Type_OUT, nVolume);
			m_csToolTips.Update();
		}
		else if (nId == IDC_SLIDER_Phone)
		{
			m_pVolumeCtrl->SetVolume(_DEV_Audio_Type_IN, nVolume);
			m_csToolTips.Update();
		}
	}
	CDialogEx::OnHScroll(nSBCode, nPos, pScrollBar);
}


void CEPVolumeDlg::OnBnClickedOk()
{
	// TODO: 在此添加控件通知处理程序代码
	CDialogEx::OnOK();
}


void CEPVolumeDlg::OnBnClickedCheckSpeaker()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bMute = ((CButton *)GetDlgItem(IDC_CHECK_Speaker))->GetCheck() == BST_CHECKED;
	m_pVolumeCtrl->SetMute(_DEV_Audio_Type_OUT, bMute);

}


void CEPVolumeDlg::OnBnClickedCheckPhone()
{
	// TODO: 在此添加控件通知处理程序代码
	BOOL bMute = ((CButton *)GetDlgItem(IDC_CHECK_Phone))->GetCheck() == BST_CHECKED;
	m_pVolumeCtrl->SetMute(_DEV_Audio_Type_IN, bMute);
}


LRESULT CEPVolumeDlg::MsgSpeaker(WPARAM wParam, LPARAM lParam)
{
	if (wParam == _Volume_Ctrl_Type_Mute)
	{
		((CButton *)GetDlgItem(IDC_CHECK_Speaker))->SetCheck(lParam ? BST_CHECKED : BST_UNCHECKED);
	}
	else if (wParam == _Volume_Ctrl_Type_Volume)
	{
		((CSliderCtrl *)GetDlgItem(IDC_SLIDER_Speaker))->SetPos(lParam);
	}

	return 0;
}
LRESULT CEPVolumeDlg::MsgPhone(WPARAM wParam, LPARAM lParam)
{
	if (wParam == _Volume_Ctrl_Type_Mute)
	{
		((CButton *)GetDlgItem(IDC_CHECK_Phone))->SetCheck(lParam ? BST_CHECKED : BST_UNCHECKED);
	}
	else if (wParam == _Volume_Ctrl_Type_Volume)
	{
		((CSliderCtrl *)GetDlgItem(IDC_SLIDER_Phone))->SetPos(lParam);
	}

	return 0;
}



void CEPVolumeDlg::OnDestroy()
{
	CDialogEx::OnDestroy();

	// TODO: 在此处添加消息处理程序代码
	if (m_pVolumeCtrl != NULL)
	{
		delete m_pVolumeCtrl;
	}

	NotifyDestory();

}

int CEPVolumeDlg::NotifyInit()
{
	m_NID.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	m_NID.hWnd = GetSafeHwnd();
	m_NID.uID = IDR_MAINFRAME;
	m_NID.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	m_NID.uCallbackMessage = WM_SHOWTASK;             // 自定义的消息名称
	m_NID.hIcon = m_hIcon;
	wcscpy_s(m_NID.szTip,128, L"服务器程序");                // 信息提示条为"服务器程序"，VS2008 UNICODE编码用wcscpy_s()函数

	Shell_NotifyIcon(NIM_ADD, &m_NID);                // 在托盘区添加图标

	return 0;
}
int CEPVolumeDlg::NotifyDestory()
{
	Shell_NotifyIcon(NIM_DELETE, &m_NID);
	return 0;
}


LRESULT CEPVolumeDlg::OnShowTask(WPARAM wParam, LPARAM lParam)
{
	if (wParam != IDR_MAINFRAME)
	{
		return 1;
	}
	switch (lParam)
	{
		case WM_RBUTTONUP:                                        // 右键起来时弹出菜单
		{
			CPoint Point;
			::GetCursorPos(&Point);                    // 得到鼠标位置
			CMenu menu;
			menu.CreatePopupMenu();                    // 声明一个弹出式菜单
			menu.AppendMenu(MF_STRING, WM_DESTROY, L"关闭");
			menu.TrackPopupMenu(TPM_LEFTALIGN, Point.x, Point.y, this);
			HMENU hmenu = menu.Detach();
			menu.DestroyMenu();
			break;
		}
		case WM_LBUTTONDBLCLK:                                 // 双击左键的处理
		{
			ShowWindow(SW_SHOWNORMAL);         // 显示主窗口
			static bool boFirst = true;
			if (!boFirst)
			{
				break;
			}
			boFirst = false;
			int s32Width = GetSystemMetrics(SM_CXSCREEN);
			int s32Height = GetSystemMetrics(SM_CYSCREEN);

			CRect csRect;
			GetWindowRect(&csRect);

			s32Height = s32Height;

			csRect.MoveToXY((s32Width - csRect.Width()) / 2,
				(s32Height - csRect.Height()) / 2);

			MoveWindow(&csRect);

			break;
		}
	}
	return 0;

}

void CEPVolumeDlg::OnBnClickedBtnReg()
{
	// TODO: 在此添加控件通知处理程序代码 "EPVolume"
	HKEY   hKey, hExeKey;
	TCHAR tcFileName[MAX_PATH] = { 0 };
	TCHAR tcRealFileName[MAX_PATH] = { 0 };
	LPCTSTR lpRun = _T("Software\\Microsoft\\Windows\\CurrentVersion\\Run");

	//得到程序自身的全路径   
	DWORD dwRet = GetModuleFileName(NULL, (LPWCH)tcFileName, MAX_PATH);
	TCHAR *pStr = wcsrchr(tcFileName, L'\\');
	if (pStr != NULL)
	{
		memcpy(tcRealFileName, pStr + 1,
			sizeof(TCHAR) * (wcsrchr(pStr, L'.') - pStr - 1));
	}
	bool boFind = false;
	do 
	{
		long lRet = RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_READ, &hKey);
		if (lRet != ERROR_SUCCESS)
		{
			break;
		}
		lRet = RegOpenKeyEx(hKey, tcRealFileName, 0, KEY_READ, &hExeKey);
		if (lRet != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			break;
		}

		TCHAR tcValue[MAX_PATH];
		LONG lLen = MAX_PATH;
		lRet = RegQueryValue(hExeKey, NULL, tcValue, &lLen);
		if (lRet != ERROR_SUCCESS)
		{
			RegCloseKey(hExeKey);
			RegCloseKey(hKey);
			break;
		}


		RegCloseKey(hExeKey);
		RegCloseKey(hKey);
		boFind = true;
	} while (0);

	if (boFind)
	{
		return;
	}

	long lRet = RegOpenKeyEx(HKEY_CURRENT_USER, lpRun, 0, KEY_WRITE, &hKey);
	if (lRet != ERROR_SUCCESS)
	{
		return;
	}
	lRet = RegSetValueEx(hKey, tcRealFileName, 0, REG_SZ, (const BYTE*)(LPCSTR)tcFileName, MAX_PATH);

	RegCloseKey(hKey);
	return;
}


void CEPVolumeDlg::OnCbnSelchangeComboDev()
{
	// TODO: 在此添加控件通知处理程序代码

	int s32CurSel = m_csComboDev.GetCurSel();
	CString csStr;
	m_csComboDev.GetLBText(s32CurSel, csStr);

	TRACE(L"%s\n", csStr.GetString());

	m_pVolumeCtrl->SetDefaultDeivce(_DEV_Audio_Type_OUT, csStr.GetString());
}


//void CEPVolumeDlg::OnBnClickedBtnColor()
//{
//	// TODO: 在此添加控件通知处理程序代码
//	CColorDialog csDlg(RGB(255, 0, 0));
//	csDlg.DoModal();
//	COLORREF stColor = csDlg.GetColor();
//	stColor = stColor;
//}


void CEPVolumeDlg::OnBnClickedMfcColorBtn()
{
	// TODO: 在此添加控件通知处理程序代码
	m_s32ChosenColor = m_csMFCColorBtn.GetColor(); 

	m_s32ChosenColor = m_s32ChosenColor;
}

BOOL CEPVolumeDlg::OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult)
{

	TOOLTIPTEXT *pTTT = (TOOLTIPTEXT *)pNMHDR;

	UINT  uID = pNMHDR->idFrom;
	if (pTTT->uFlags & TTF_IDISHWND)
	{
		uID = ::GetDlgCtrlID((HWND)uID);
	}

	if (uID == 0)
	{
		return FALSE;
	}
	else if (uID == IDC_SLIDER_Speaker || uID == IDC_SLIDER_Phone)
	{
		CString csStr;
		csStr.Format(_T("%d"), ((CSliderCtrl *)GetDlgItem(uID))->GetPos());
		wcscpy_s(pTTT->lpszText, 80, csStr.GetString());
	}
	//else if (uID == IDC_BTN_Set)
	//{
	//	pTTT->lpszText = L"test";
	//	uID = uID;
	//}
	return   TRUE;
}


BOOL CEPVolumeDlg::PreTranslateMessage(MSG* pMsg)
{
	// TODO: 在此添加专用代码和/或调用基类
	if (m_csToolTips.GetSafeHwnd() != NULL)
	{
		m_csToolTips.RelayEvent(pMsg);
	}

	return CDialogEx::PreTranslateMessage(pMsg);
}
