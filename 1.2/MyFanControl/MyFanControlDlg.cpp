// MyFanControlDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "MyFanControl.h"
#include "MyFanControlDlg.h"
#include "afxdialogex.h"
static UINT WM_TASKBARCREATED = ::RegisterWindowMessage(_T("TaskbarCreated"));

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define WM_SHOWTASK (WM_USER +1)
#define IDR_SHOW 11
#define IDR_EXIT 12


__int64 CompareFileTime(FILETIME time1, FILETIME time2)
{
	__int64 a = __int64(time1.dwHighDateTime) << 32 | time1.dwLowDateTime;
	__int64 b = __int64(time2.dwHighDateTime) << 32 | time2.dwLowDateTime;

	return (b - a);
}
int GetCpuClock(int* CPU_usage)
{
	LARGE_INTEGER c1;
	QueryPerformanceCounter(&c1);
	LARGE_INTEGER c2;
	QueryPerformanceCounter(&c2);
	unsigned __int64 start = __rdtsc();
	///
	FILETIME preidleTime, prekernelTime, preuserTime;
	BOOL res = GetSystemTimes(&preidleTime, &prekernelTime, &preuserTime);
	///
	ULONGLONG startTickCount = GetTickCount64();
	while (GetTickCount64() - startTickCount < 100) {}
	LARGE_INTEGER c3;
	QueryPerformanceCounter(&c3);
	unsigned __int64 end = __rdtsc();
	LARGE_INTEGER freq;
	QueryPerformanceFrequency(&freq);
	//
	FILETIME idleTime, kernelTime, userTime;
	res = GetSystemTimes(&idleTime, &kernelTime, &userTime);
	__int64 idle = CompareFileTime(preidleTime, idleTime);
	__int64 kernel = CompareFileTime(prekernelTime, kernelTime);
	__int64 user = CompareFileTime(preuserTime, userTime);

	int cpu = int(((kernel - idle + user) * 100) / (kernel + user));
	if (CPU_usage)
		*CPU_usage = cpu;
	//
	unsigned __int64 e = (c3.QuadPart - c2.QuadPart) - (c2.QuadPart - c1.QuadPart);
	double elapsed = static_cast<double>(e) / static_cast<double>(freq.QuadPart);
	return int((static_cast<double>((end - start) / elapsed)) / 1000000);
}
////

// 用于应用程序"关于"菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

	// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CMyFanControlDlg 对话框



CMyFanControlDlg::CMyFanControlDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CMyFanControlDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
	m_bForceHideWindow = TRUE;//启动时强制隐藏窗口
#ifdef MY_DEBUG
	m_bForceHideWindow = FALSE;//启动时强制隐藏窗口
#endif
	m_hCoreThread = NULL;
	m_dwCoreThreadId = 0;
	m_nLastCoreUpdateTime = -1;
	m_bWindowVisible = FALSE;
	m_bAdvancedMode = FALSE; // 默认高级模式
	m_bTrayAdded = FALSE;
	m_nWindowSize[0] = 0;
	m_nWindowSize[1] = 0;

	// 初始化电源管理变量
	m_bResumeFromSleep = FALSE;
	m_nResumeOkCount = 0;

	// 初始化线程卡死处理变量
	m_bThreadKilledOnce = FALSE;
	m_dwFirstDeadTime = 0; 
	m_bWaitingForRestart = FALSE;

	m_nDutyEditCtlID[0][0] = IDC_EDIT_CPU0;
	m_nDutyEditCtlID[0][1] = IDC_EDIT_CPU1;
	m_nDutyEditCtlID[0][2] = IDC_EDIT_CPU2;
	m_nDutyEditCtlID[0][3] = IDC_EDIT_CPU3;
	m_nDutyEditCtlID[0][4] = IDC_EDIT_CPU4;
	m_nDutyEditCtlID[0][5] = IDC_EDIT_CPU5;
	m_nDutyEditCtlID[0][6] = IDC_EDIT_CPU6;
	m_nDutyEditCtlID[0][7] = IDC_EDIT_CPU7;
	m_nDutyEditCtlID[0][8] = IDC_EDIT_CPU8;
	m_nDutyEditCtlID[0][9] = IDC_EDIT_CPU9;
	m_nDutyEditCtlID[1][0] = IDC_EDIT_GPU0;
	m_nDutyEditCtlID[1][1] = IDC_EDIT_GPU1;
	m_nDutyEditCtlID[1][2] = IDC_EDIT_GPU2;
	m_nDutyEditCtlID[1][3] = IDC_EDIT_GPU3;
	m_nDutyEditCtlID[1][4] = IDC_EDIT_GPU4;
	m_nDutyEditCtlID[1][5] = IDC_EDIT_GPU5;
	m_nDutyEditCtlID[1][6] = IDC_EDIT_GPU6;
	m_nDutyEditCtlID[1][7] = IDC_EDIT_GPU7;
	m_nDutyEditCtlID[1][8] = IDC_EDIT_GPU8;
	m_nDutyEditCtlID[1][9] = IDC_EDIT_GPU9;

	// 温度档位控件ID
	m_nTempThresholdCtlID[0] = IDC_EDIT_TEMP0;
	m_nTempThresholdCtlID[1] = IDC_EDIT_TEMP1;
	m_nTempThresholdCtlID[2] = IDC_EDIT_TEMP2;
	m_nTempThresholdCtlID[3] = IDC_EDIT_TEMP3;
	m_nTempThresholdCtlID[4] = IDC_EDIT_TEMP4;
	m_nTempThresholdCtlID[5] = IDC_EDIT_TEMP5;
	m_nTempThresholdCtlID[6] = IDC_EDIT_TEMP6;
	m_nTempThresholdCtlID[7] = IDC_EDIT_TEMP7;
	m_nTempThresholdCtlID[8] = IDC_EDIT_TEMP8;
	m_nTempThresholdCtlID[9] = IDC_EDIT_TEMP9;
}

CMyFanControlDlg::~CMyFanControlDlg()
{
	DestroyWindow();
}

void CMyFanControlDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_STATUS, m_ctlStatus);
	DDX_Control(pDX, IDC_CHECK_TAKEOVER, m_ctlTakeOver);
	DDX_Control(pDX, IDC_CHECK_FORCE, m_ctlForcedCooling);
	DDX_Control(pDX, IDC_CHECK_LINEAR, m_ctlLinear);
	DDX_Control(pDX, IDC_CHECK_SOFTCONTROL, m_ctlSoftControl);
	DDX_Control(pDX, IDC_EDIT_INTERVAL, m_ctlInterval);
	DDX_Control(pDX, IDC_EDIT_TREANSITION, m_ctlTransition);
	DDX_Control(pDX, IDC_EDIT_FORCE_TEMP, m_ctlForceTemp);
	DDX_Control(pDX, IDC_CHECK_AUTORUN, m_ctlAutorun);
	DDX_Control(pDX, IDC_EDIT_GPU_FREQUENCY, m_ctlFrequency);
	DDX_Control(pDX, IDC_CHECK_LOCK_GPU_FREQUANCY, m_ctlLockGpuFrequancy);
	DDX_Control(pDX, IDC_CHECK_LOCK_MEM_OVERCLOCK, m_ctlLockMemOverclock);
	DDX_Control(pDX, IDC_EDIT_MEM_OFFSET, m_ctlMemOffset);
}

BEGIN_MESSAGE_MAP(CMyFanControlDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_TIMER()
	ON_BN_CLICKED(IDC_BUTTON_SAVE, &CMyFanControlDlg::OnBnClickedButtonSave)
	ON_BN_CLICKED(IDC_BUTTON_RESET, &CMyFanControlDlg::OnBnClickedButtonReset)
	ON_BN_CLICKED(IDC_BUTTON_LOAD, &CMyFanControlDlg::OnBnClickedButtonLoad)
	ON_MESSAGE(WM_SHOWTASK, &CMyFanControlDlg::OnShowTask)
	ON_BN_CLICKED(IDC_CHECK_TAKEOVER, &CMyFanControlDlg::OnBnClickedCheckTakeover)
	ON_BN_CLICKED(IDC_CHECK_FORCE, &CMyFanControlDlg::OnBnClickedCheckForce)
	ON_BN_CLICKED(IDC_CHECK_LINEAR, &CMyFanControlDlg::OnBnClickedCheckLinear)
	ON_BN_CLICKED(IDC_CHECK_SOFTCONTROL, &CMyFanControlDlg::OnBnClickedCheckSoftControl)
	ON_BN_CLICKED(IDC_BUTTON_ADVANCED, &CMyFanControlDlg::OnBnClickedButtonAdvanced)
	ON_BN_CLICKED(IDC_CHECK_AUTORUN, &CMyFanControlDlg::OnBnClickedCheckAutorun)
	ON_BN_CLICKED(IDC_CHECK_LOCK_GPU_FREQUANCY, &CMyFanControlDlg::OnBnClickedCheckLockGpuFrequancy)
	ON_BN_CLICKED(IDC_CHECK_LOCK_MEM_OVERCLOCK, &CMyFanControlDlg::OnBnClickedCheckLockMemOverclock)
	ON_REGISTERED_MESSAGE(WM_TASKBARCREATED, &CMyFanControlDlg::OnTaskbarCreated)
	ON_MESSAGE(WM_POWERBROADCAST, &CMyFanControlDlg::OnPowerBroadcast)
	ON_MESSAGE(WM_GPU_ERROR, &CMyFanControlDlg::OnGpuError)
END_MESSAGE_MAP()


// 电源事件处理
LRESULT CMyFanControlDlg::OnPowerBroadcast(WPARAM wParam, LPARAM lParam)
{
	if (wParam == PBT_APMRESUMEAUTOMATIC || wParam == PBT_APMRESUMESUSPEND)
	{
		TRACE("系统从休眠/睡眠恢复\n");
		m_bResumeFromSleep = TRUE;
		m_nResumeOkCount = 0;

		// 同步到核心的GPU信息对象
		m_core.m_GpuInfo.m_bResumeFromSleep = TRUE;
	}
	else if (wParam == PBT_APMSUSPEND)
	{
		TRACE("系统进入休眠/睡眠\n");
	}
	return TRUE;
}

// GPU错误通知处理
LRESULT CMyFanControlDlg::OnGpuError(WPARAM wParam, LPARAM lParam)
{
	LPCTSTR szMsg = (LPCTSTR)lParam;
	if (szMsg != NULL)
	{
		// 托盘气泡提示
		NOTIFYICONDATA nid = { 0 };
		nid.cbSize = sizeof(NOTIFYICONDATA);
		nid.hWnd = this->m_hWnd;
		nid.uID = IDR_MAINFRAME;
		nid.uFlags = NIF_INFO;
		nid.dwInfoFlags = NIIF_WARNING;
		nid.uTimeout = 5000;  // 5秒显示时间

		// 复制消息到气泡
		_tcsncpy_s(nid.szInfoTitle, 64, _T("GPU控制错误"), 63);
		_tcsncpy_s(nid.szInfo, 256, szMsg, 255);

		Shell_NotifyIcon(NIM_MODIFY, &nid);

		// 同时输出到调试日志
		TRACE("GPU Error: %s\n", szMsg);

		// 释放发送方分配的内存
		delete[] szMsg;
	}
	return 0;
}

LRESULT CMyFanControlDlg::OnTaskbarCreated(WPARAM wParam, LPARAM lParam)
{
	m_bTrayAdded = FALSE;
	SetTray("蓝天风扇监控");
	return 0;
}


BOOL CMyFanControlDlg::OnInitDialog()
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

	SetIcon(m_hIcon, TRUE);
	SetIcon(m_hIcon, FALSE);

	CRect rect;
	this->GetWindowRect(rect);
	m_nWindowSize[0] = rect.Width();
	m_nWindowSize[1] = rect.Height();
	SetAdvancedMode(TRUE);

	SetTray("蓝天风扇监控");

	// 设置核心的父对话框指针
	m_core.SetParentDialog(this);

	if (m_hCoreThread == NULL)
	{
		m_hCoreThread = CreateThread(NULL, NULL, CoreThread, this, CREATE_SUSPENDED, &m_dwCoreThreadId);
		if (m_hCoreThread)
		{
			SetThreadPriority(m_hCoreThread, THREAD_PRIORITY_TIME_CRITICAL);
			ResumeThread(m_hCoreThread);
		}
	}
	SetTimer(0, 100, NULL);
	m_ctlAutorun.SetCheck(SetAutorunReg(FALSE) || SetAutorunTask(FALSE));

	return TRUE;
}

void CMyFanControlDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

void CMyFanControlDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this);
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

HCURSOR CMyFanControlDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

DWORD CMyFanControlDlg::CoreThread(LPVOID lParam)
{
	CMyFanControlDlg* pDlg = (CMyFanControlDlg*)lParam;
	pDlg->m_core.Run();
	return 0;
}

void CMyFanControlDlg::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if (m_bForceHideWindow)
	{
		lpwndpos->flags &= ~SWP_SHOWWINDOW;
	}
	CDialogEx::OnWindowPosChanging(lpwndpos);
}

void CMyFanControlDlg::OnOK()
{
	if (!m_core.m_nExit)
		m_core.m_nExit = 1;

	if (m_core.m_nExit == 1)
	{
		int count = 0;
		while (m_core.m_nExit == 1 && count++ < 100)
		{
			Sleep(100);
		}
	}
	if (m_core.m_nExit)
	{
		KillTimer(0);
		SetTray(NULL);
		CDialogEx::OnOK();
	}
}

void CMyFanControlDlg::OnCancel()
{
	if (m_bWindowVisible)
	{
		this->ShowWindow(SW_HIDE);
	}
	else
	{
		this->ShowWindow(SW_SHOW);
		SetForegroundWindow();
	}
}

void CMyFanControlDlg::OnTimer(UINT_PTR nIDEvent)
{
	static int nCheckThreadCount = 0;
	CDialogEx::OnTimer(nIDEvent);

	if (m_core.m_nExit == 2)
	{
		OnOK();
		return;
	}

	// 处理休眠恢复后的计数器
	if (m_bResumeFromSleep)
	{
		if (m_core.m_nLastUpdateTime != m_nLastCoreUpdateTime)
		{
			m_nResumeOkCount++;
			if (m_nResumeOkCount >= 3)
			{
				TRACE("休眠恢复后已成功更新3次，清除恢复标志\n");
				m_bResumeFromSleep = FALSE;
				m_nResumeOkCount = 0;
				m_core.m_GpuInfo.m_bResumeFromSleep = FALSE;
			}
		}
	}

	nCheckThreadCount++;
	if (nCheckThreadCount > 150 && !m_bWaitingForRestart)
	{
		// 工作线程卡死处理
		TRACE("检测到工作线程可能卡死，尝试退出...\n");

		// 先尝试退出旧线程
		if (!m_core.m_nExit)
		{
			m_core.m_nExit = 1;
		}

		// 等待最多3秒
		DWORD dwWait = WaitForSingleObject(m_hCoreThread, 3000);
		if (dwWait == WAIT_TIMEOUT)
		{
			TRACE("检测到工作线程卡死，程序将退出\n");
			m_core.m_nExit = 2;
			KillTimer(0);
			MessageBox(_T("检测到工作线程卡死，程序将关闭。\r\n请检查硬件兼容性或尝试重启系统。"), _T("严重错误"), MB_OK | MB_ICONERROR);
			OnOK();
			return;
		}
		CloseHandle(m_hCoreThread);
		m_hCoreThread = NULL;

		// 检查是否一分钟内再次卡死
		ULONGLONG dwNow = GetTickCount64();
		if (m_bThreadKilledOnce && (dwNow - m_dwFirstDeadTime < 60000))
		{
			MessageBox(_T("工作线程屡次卡死，程序将关闭。\r\n请检查硬件兼容性或尝试重启系统。"), _T("严重错误"), MB_OK | MB_ICONERROR);
			m_core.m_nExit = 2;
			OnOK();
			return;
		}

		// 记录第一次卡死时间
		if (!m_bThreadKilledOnce)
		{
			m_bThreadKilledOnce = TRUE;
			m_dwFirstDeadTime = dwNow;
			m_bWaitingForRestart = TRUE;

			// 提示（PostMessage实现）
			PostMessage(WM_USER + 200);  // 将在下次消息循环中处理
		}
		else
		{
			// 创建新线程
			TRACE("重新创建工作线程...\n");
			m_core.m_nExit = 0;
			m_core.m_nInit = 0;  // 需要重新初始化
			m_hCoreThread = CreateThread(NULL, NULL, CoreThread, this, 0, &m_dwCoreThreadId);
			if (m_hCoreThread)
			{
				SetThreadPriority(m_hCoreThread, THREAD_PRIORITY_TIME_CRITICAL);
			}
			m_bWaitingForRestart = FALSE;
			nCheckThreadCount = 0;
			MessageBox(_T("检测到工作线程卡死，已自动重启。\r\n如果问题持续，建议检查硬件兼容性。"), _T("工作线程重启"), MB_OK | MB_ICONINFORMATION);
		}
	}

	// 处理线程重启提示消息
	if (m_bWaitingForRestart)
	{
		MSG msg;
		if (PeekMessage(&msg, m_hWnd, WM_USER + 200, WM_USER + 200, PM_REMOVE))
		{
			// 创建新线程
			TRACE("重新创建工作线程（用户确认后）...\n");
			m_core.m_nExit = 0;
			m_core.m_nInit = 0;
			m_hCoreThread = CreateThread(NULL, NULL, CoreThread, this, 0, &m_dwCoreThreadId);
			if (m_hCoreThread)
			{
				SetThreadPriority(m_hCoreThread, THREAD_PRIORITY_TIME_CRITICAL);
			}
			m_bWaitingForRestart = FALSE;
			nCheckThreadCount = 0;
		}
	}

	if (m_core.m_nInit != 1)
		return;

	static BOOL LastVisible = FALSE;
	m_bWindowVisible = IsWindowVisible();
	if (m_bWindowVisible && !LastVisible)
	{
		m_core.m_bUpdateRPM = TRUE;
		UpdateGui(TRUE);
	}
	else if (!m_bWindowVisible && LastVisible)
	{
		m_core.m_bUpdateRPM = FALSE;
	}
	LastVisible = m_bWindowVisible;

	if (m_nLastCoreUpdateTime != m_core.m_nLastUpdateTime)
	{
		if (m_bWindowVisible)
			UpdateGui(FALSE);
		nCheckThreadCount = 0;
		m_nLastCoreUpdateTime = m_core.m_nLastUpdateTime;
	}
}

void CMyFanControlDlg::UpdateGui(BOOL bFull)
{
	int row = m_ctlStatus.GetItemCount();
	int col = m_ctlStatus.GetHeaderCtrl()->GetItemCount();
	if (row != 7 || col != 3)
	{
		if (!bFull)
		{
			UpdateGui(TRUE);
			return;
		}
		CRect rect;
		m_ctlStatus.GetWindowRect(rect);
		int width = rect.Width();
		m_ctlStatus.DeleteAllItems();
		while (m_ctlStatus.DeleteColumn(0));
		int i = 0;
		m_ctlStatus.InsertColumn(i++, "", LVCFMT_CENTER, int(width * 0.32));
		m_ctlStatus.InsertColumn(i++, "CPU", LVCFMT_CENTER, int(width * 0.33));
		m_ctlStatus.InsertColumn(i++, "GPU", LVCFMT_CENTER, int(width * 0.33));
		i = 0;
		m_ctlStatus.InsertItem(i++, "当前温度");
		m_ctlStatus.InsertItem(i++, "设定挡位");
		m_ctlStatus.InsertItem(i++, "当前转速%");
		m_ctlStatus.InsertItem(i++, "转速RPM");
		m_ctlStatus.InsertItem(i++, "目标转速%");
		m_ctlStatus.InsertItem(i++, "运行频率");
		m_ctlStatus.InsertItem(i++, "使用率%");
	}
	char str[256];
	for (int i = 0; i < 2; i++)
	{
		sprintf_s(str, 256, "%d", m_core.m_nCurTemp[i]);
		m_ctlStatus.SetItemText(0, i + 1, str);
		sprintf_s(str, 256, "%d", m_core.m_nSetDutyLevel[i]);
		m_ctlStatus.SetItemText(1, i + 1, str);
		sprintf_s(str, 256, "%d", m_core.m_nCurDuty[i]);
		m_ctlStatus.SetItemText(2, i + 1, str);
		sprintf_s(str, 256, "%d", m_core.m_nSoftTargetDuty[i]);
		m_ctlStatus.SetItemText(4, i + 1, str);
		if (m_core.m_nCurRPM[i] >= 0)
		{
			sprintf_s(str, 256, "%d", m_core.m_nCurRPM[i]);
			m_ctlStatus.SetItemText(3, i + 1, str);
		}
		else
		{
			m_ctlStatus.SetItemText(3, i + 1, "-");
		}
		if (i == 1)
		{
			sprintf_s(str, 256, "%d/%d", m_core.m_GpuInfo.m_nGraphicsClock, m_core.m_GpuInfo.m_nMemoryClock);
			m_ctlStatus.SetItemText(5, i + 1, str);
			sprintf_s(str, 256, "%d", m_core.m_GpuInfo.m_nUsage);
			m_ctlStatus.SetItemText(6, i + 1, str);
		}
	}
	int fc = m_ctlForcedCooling.GetCheck();
	if (fc ^ m_core.m_bForcedCooling)
	{
		m_ctlForcedCooling.SetCheck(m_core.m_bForcedCooling);
	}
	if (!bFull)
		return;
	int to = m_ctlTakeOver.GetCheck();
	if (to ^ m_core.m_config.TakeOver)
		m_ctlTakeOver.SetCheck(m_core.m_config.TakeOver);
	int lc = m_ctlLinear.GetCheck();
	if (lc ^ m_core.m_config.Linear)
		m_ctlLinear.SetCheck(m_core.m_config.Linear);
	int sc = m_ctlSoftControl.GetCheck();
	if (sc ^ m_core.m_config.SoftControl)
		m_ctlSoftControl.SetCheck(m_core.m_config.SoftControl);
	int lf = m_ctlLockGpuFrequancy.GetCheck();
	if (lf ^ m_core.m_config.LockGPUFrequency)
		m_ctlLockGpuFrequancy.SetCheck(m_core.m_config.LockGPUFrequency);
	int lmo = m_ctlLockMemOverclock.GetCheck();
	if (lmo ^ m_core.m_config.LockMemOverclock)
		m_ctlLockMemOverclock.SetCheck(m_core.m_config.LockMemOverclock);
	sprintf_s(str, 256, "%d", m_core.m_config.UpdateInterval);
	m_ctlInterval.SetWindowTextA(str);
	sprintf_s(str, 256, "%d", m_core.m_config.TransitionTemp);
	m_ctlTransition.SetWindowTextA(str);
	sprintf_s(str, 256, "%d", m_core.m_config.ForceTemp);
	m_ctlForceTemp.SetWindowTextA(str);
	sprintf_s(str, 256, "%d", m_core.m_config.GPUFrequency);
	m_ctlFrequency.SetWindowTextA(str);
	sprintf_s(str, 256, "%d", m_core.m_config.MemOverclockOffset);
	m_ctlMemOffset.SetWindowTextA(str);
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 10; j++)
		{
			sprintf_s(str, 256, "%d", m_core.m_config.DutyList[i][j]);
			GetDlgItem(m_nDutyEditCtlID[i][j])->SetWindowTextA(str);
		}
	for (int j = 0; j < 10; j++)
	{
		sprintf_s(str, 256, "%d", m_core.m_config.TempThresholds[j]);
		GetDlgItem(m_nTempThresholdCtlID[j])->SetWindowTextA(str);
	}
}

BOOL CMyFanControlDlg::CheckAndSave()
{
	//检查设置
	char str[256];
	m_ctlInterval.GetWindowTextA(str, 256);
	int nInterval = atoi(str);
	if (nInterval < 1 || nInterval > 5)
	{
		AfxMessageBox(_T("更新间隔必须为1-5"));
		return FALSE;
	}
	//
	m_ctlTransition.GetWindowTextA(str, 256);
	int nTransition = atoi(str);
	if (nTransition < 0 || nTransition > 10)
	{
		AfxMessageBox(_T("过渡温度必须为0-10"));
		return FALSE;
	}
	//
	m_ctlForceTemp.GetWindowTextA(str, 256);
	int nForceTemp = atoi(str);
	if (nForceTemp < 0 || nForceTemp > 90)
	{
		AfxMessageBox(_T("强制冷却温度必须为0-90"));
		return FALSE;
	}
	//
	m_ctlFrequency.GetWindowTextA(str, 256);
	int nFrequency = atoi(str);
	if (!CheckInputFrequency(nFrequency))
		return FALSE;

	if (nFrequency == 0)
		nFrequency = m_core.m_GpuInfo.m_nStandardFrequency;

	// 显存偏移
	m_ctlMemOffset.GetWindowTextA(str, 256);
	int nMemOffset = atoi(str);

	// 获取显存偏移锁定状态
	BOOL bLockMemOverclock = m_ctlLockMemOverclock.GetCheck();

	if (bLockMemOverclock)
	{
		if (nMemOffset < m_core.m_GpuInfo.m_nMemoryRangeMin || nMemOffset > m_core.m_GpuInfo.m_nMemoryRangeMax)
		{
			char str2[256];
			sprintf_s(str2, 256, "显存频率偏移必须为%d-%d", m_core.m_GpuInfo.m_nMemoryRangeMin, m_core.m_GpuInfo.m_nMemoryRangeMax);
			AfxMessageBox(str2);
			return FALSE;
		}
		// 安全警告
		if (nMemOffset > 0)
		{
			char str2[256];
			sprintf_s(str2, 256, "显存频率偏移为%dMHz，超频可能降低系统稳定性。\n是否确认要设置？", nMemOffset);
			int rv = MessageBox(str2, _T("确认设置"), MB_YESNO);
			if (IDNO == rv)
				return FALSE;
		}
	}
	//
	int nDutyList[2][10];
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			GetDlgItem(m_nDutyEditCtlID[i][j])->GetWindowTextA(str, 256);
			nDutyList[i][j] = atoi(str);
			if (nDutyList[i][j] < 0 || nDutyList[i][j]>100)
			{
				char str2[256];
				sprintf_s(str2, 256, "%s风扇转速设定错误，必须为0-100", i ? "GPU" : "CPU");
				AfxMessageBox(str2);
				return FALSE;
			}
		}
	}
	// 温度档位
	int nTempThresholds[10];
	for (int j = 0; j < 10; j++)
	{
		GetDlgItem(m_nTempThresholdCtlID[j])->GetWindowTextA(str, 256);
		nTempThresholds[j] = atoi(str);
		if (nTempThresholds[j] < 30 || nTempThresholds[j] > 100)
		{
			char str2[256];
			sprintf_s(str2, 256, "温度档位%d设定错误，必须为30-100", j + 1);
			AfxMessageBox(str2);
			return FALSE;
		}
	}
	// 检查温度档位是否递减
	for (int j = 0; j < 9; j++)
	{
		if (nTempThresholds[j] <= nTempThresholds[j + 1])
		{
			AfxMessageBox(_T("温度档位从上往下必须严格递减"));
			return FALSE;
		}
	}
	//应用设置
	m_core.m_config.UpdateInterval = nInterval;
	m_core.m_config.TransitionTemp = nTransition;
	m_core.m_config.ForceTemp = nForceTemp;
	m_core.m_config.GPUFrequency = nFrequency;
	m_core.m_config.LockMemOverclock = bLockMemOverclock;
	m_core.m_config.MemOverclockOffset = nMemOffset;
	for (int i = 0; i < 2; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			m_core.m_config.DutyList[i][j] = nDutyList[i][j];
		}
	}
	for (int j = 0; j < 10; j++)
	{
		m_core.m_config.TempThresholds[j] = nTempThresholds[j];
	}
	//保存
	m_core.m_config.SaveConfig();
	return TRUE;
}

void CMyFanControlDlg::OnBnClickedButtonSave()
{
	// TODO:  在此添加控件通知处理程序代码
	if (CheckAndSave())
		UpdateGui(TRUE);
}


void CMyFanControlDlg::OnBnClickedButtonReset()
{
	// TODO:  在此添加控件通知处理程序代码
	m_core.m_config.LoadDefault();
	UpdateGui(TRUE);
}


void CMyFanControlDlg::OnBnClickedButtonLoad()
{
	// TODO:  在此添加控件通知处理程序代码
	m_core.m_config.LoadConfig();
	UpdateGui(TRUE);
}

void CMyFanControlDlg::SetTray(PCSTR string)//设置托盘图标
{
	NOTIFYICONDATA nid;
	nid.cbSize = (DWORD)sizeof(NOTIFYICONDATA);
	nid.hWnd = this->m_hWnd;
	nid.uID = IDR_MAINFRAME;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
	nid.uCallbackMessage = WM_SHOWTASK;//自定义的消息名称  
	nid.hIcon = LoadIcon(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_MAINFRAME));
	if (string)
	{
		strcpy_s(nid.szTip, 128, string);//信息提示内容  
		if (!m_bTrayAdded)  // 改为成员变量
		{
			Shell_NotifyIcon(NIM_ADD, &nid);//在托盘区添加图标
			m_bTrayAdded = TRUE;  // 改为成员变量
		}
		else
		{
			Shell_NotifyIcon(NIM_MODIFY, &nid);//修改托盘区图标
		}
	}
	else
	{
		Shell_NotifyIcon(NIM_DELETE, &nid);
		m_bTrayAdded = FALSE;
	}
}


LRESULT CMyFanControlDlg::OnShowTask(WPARAM wParam, LPARAM lParam)
{//wParam接收的是图标的ID，而lParam接收的是鼠标的行为 
	if (wParam != IDR_MAINFRAME)
		return 1;
	switch (lParam)
	{
	case WM_LBUTTONUP://左键单击显示主界面
	{

	}break;
	case WM_RBUTTONUP://右击弹出菜单
	{
		LPPOINT lpoint = new tagPOINT;
		::GetCursorPos(lpoint);//得到鼠标位置
		CMenu menu;
		menu.CreatePopupMenu();
		if (m_bWindowVisible)
			menu.AppendMenu(MFT_STRING, IDR_SHOW, _T("隐藏"));
		else
			menu.AppendMenu(MFT_STRING, IDR_SHOW, _T("显示"));
		menu.AppendMenu(MFT_SEPARATOR);
		menu.AppendMenu(MFT_STRING, IDR_EXIT, _T("退出"));
		SetForegroundWindow();//不加此行在菜单外点击菜单不销毁
		int xx = TrackPopupMenu(menu, TPM_RETURNCMD, lpoint->x, lpoint->y, NULL, this->m_hWnd, NULL);//显示菜单并获取选项ID
		if (xx == IDR_SHOW)
		{
			OnCancel();

		}
		else if (xx == IDR_EXIT)
		{
			OnOK();
		}
		HMENU hmenu = menu.Detach();
		menu.DestroyMenu();
		delete lpoint;
	}break;
	case WM_LBUTTONDBLCLK:
	{
		this->ShowWindow(SW_SHOW);
		SetForegroundWindow();
	}break;
	case WM_MOUSEMOVE:
	{
		static int LastUpdate = -1;
		if (LastUpdate != m_core.m_nLastUpdateTime)
		{
			char str[128];
			sprintf_s(str, 128, "CPU：%d℃，%d%%\nGPU：%d℃，%d%%", m_core.m_nCurTemp[0], m_core.m_nCurDuty[0], m_core.m_nCurTemp[1], m_core.m_nCurDuty[1]);
			SetTray(str);
			LastUpdate = m_core.m_nLastUpdateTime;
		}

	}break;
	}
	return 0;
}

void CMyFanControlDlg::OnBnClickedCheckTakeover()
{
	// TODO:  在此添加控件通知处理程序代码
	int val = m_ctlTakeOver.GetCheck();
	m_core.m_config.TakeOver = val;
}


void CMyFanControlDlg::OnBnClickedCheckForce()
{
	// TODO:  在此添加控件通知处理程序代码
	int val = m_ctlForcedCooling.GetCheck();
	m_core.m_bForcedCooling = val;
}


void CMyFanControlDlg::OnBnClickedCheckLinear()
{
	// TODO:  在此添加控件通知处理程序代码
	int val = m_ctlLinear.GetCheck();
	m_core.m_config.Linear = val;
}

void CMyFanControlDlg::OnBnClickedCheckSoftControl()
{
	// TODO:  在此添加控件通知处理程序代码
	int val = m_ctlSoftControl.GetCheck();
	m_core.m_config.SoftControl = val;

	// 初始化软性控制当前值
	if (val)
	{
		for (int i = 0; i < 2; i++)
		{
			m_core.m_nSoftCurrentDuty[i] = m_core.m_nCurDuty[i];
			m_core.m_nSoftTargetDuty[i] = m_core.m_nSetDuty[i];
		}
	}
}

void CMyFanControlDlg::SetAdvancedMode(BOOL bAdvanced)
{
	CRect rect;
	this->GetWindowRect(rect);
	if (bAdvanced)
	{
		MoveWindow(rect.left, rect.top, m_nWindowSize[0], m_nWindowSize[1], TRUE);
		GetDlgItem(IDC_BUTTON_ADVANCED)->SetWindowTextA("简单模式");
	}
	else
	{
		MoveWindow(rect.left, rect.top, m_nWindowSize[0] * 335 / 582, m_nWindowSize[1] * 283 / 485, FALSE);
		GetDlgItem(IDC_BUTTON_ADVANCED)->SetWindowTextA("高级模式");
	}
	m_bAdvancedMode = !m_bAdvancedMode;
}

void CMyFanControlDlg::OnBnClickedButtonAdvanced()
{
	// TODO:  在此添加控件通知处理程序代码
	SetAdvancedMode(!m_bAdvancedMode);
}


void CMyFanControlDlg::OnBnClickedCheckAutorun()
{
	int val = m_ctlAutorun.GetCheck();
	int set_rv;
	if (val)
	{
		int rv = MessageBox(_T("请选择开机自动启动方式：\r\n\r\n按\"是\"：注册表启动项自启动\r\n按\"否\"：任务计划自启动（管理员权限）\r\n按\"取消\"：放弃操作"), _T("开机自动启动"), MB_YESNOCANCEL);

		if (IDYES == rv)
		{
			set_rv = SetAutorunReg(TRUE, TRUE);
			set_rv = SetAutorunReg(FALSE);
		}
		else if (IDNO == rv)
		{
			set_rv = SetAutorunTask(TRUE, TRUE);
			set_rv = SetAutorunTask(FALSE);
		}
		else
		{
			set_rv = FALSE;
		}
		m_ctlAutorun.SetCheck(set_rv);
	}
	else
	{
		if (SetAutorunReg(FALSE))
			set_rv = SetAutorunReg(TRUE, FALSE);
		if (SetAutorunTask(FALSE))
			set_rv = SetAutorunTask(TRUE, FALSE);
		m_ctlAutorun.SetCheck(SetAutorunReg(FALSE) || SetAutorunTask(FALSE));
	}
	return;


}

BOOL CMyFanControlDlg::SetAutorunReg(BOOL bWrite, BOOL bAutorun)
{
	HKEY hKey;
	if (RegOpenKey(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run"), &hKey) != ERROR_SUCCESS)
	{
		AfxMessageBox(_T("无法打开注册表"));
		return FALSE;
	}
	PCSTR strProduct = "LanTianFanMonitor";

	if (bWrite)
	{
		if (bAutorun)
		{
			CString strPath = GetExePath() + _T("\\MyFanControl.exe");
			unsigned long nSize = 0;


			nSize = strPath.GetLength();
			if (RegSetValueEx(hKey, strProduct, 0, REG_SZ,
				(unsigned char*)strPath.GetBuffer(strPath.GetLength()), nSize) != ERROR_SUCCESS)
			{
				AfxMessageBox(_T("无法写入注册表启动项，需要用管理员权限运行"));
				RegCloseKey(hKey);
				return FALSE;
			}
		}
		else
		{
			if (RegDeleteValue(hKey, strProduct) != ERROR_SUCCESS)
			{
				AfxMessageBox(_T("无法删除注册表启动项，需要用管理员权限运行"));
				RegCloseKey(hKey);
				return FALSE;
			}
		}

		RegCloseKey(hKey);
	}
	else
	{
		//检查注册表项
		BOOL bRet = FALSE;
		unsigned long lSize = sizeof(bRet);
		if (RegQueryValueEx(hKey, strProduct, NULL, NULL, NULL, &lSize) != ERROR_SUCCESS)
		{
			RegCloseKey(hKey);
			return FALSE;
		}
		RegCloseKey(hKey);
		return lSize > 0 ? TRUE : FALSE;
	}

	return TRUE;
}

BOOL CMyFanControlDlg::SetAutorunTask(BOOL bWrite, BOOL bAutorun)
{
	CString strTaskName = _T("蓝天风扇监控");
	CString strPath = GetExePath() + _T("\\MyFanControl.exe");
	CString strcmd;
	CString strXmlPath = GetExePath() + _T("\\task.xml");
	if (bWrite)
	{
		if (bAutorun)
		{
			BOOL rv = CMyFanControlDlg::CreateTaskXml(strXmlPath, strPath);
			if (!rv)
			{
				AfxMessageBox(_T("无法创建任务计划程序xml文件"));
				return FALSE;
			}
			strcmd.Format(_T("SCHTASKS /Create /F /XML %s /TN %s"), (LPCTSTR)strXmlPath, (LPCTSTR)strTaskName);

		}
		else
		{
			strcmd = _T("SCHTASKS /Delete /F /TN ") + strTaskName;
		}
	}
	else
	{
		strcmd = _T("SCHTASKS /Query /TN ") + strTaskName;
	}
	CString rs = ExecuteCmd(strcmd);
	if (bWrite && bAutorun)
		remove(strXmlPath);
	if (rs == _T("[执行失败]") || rs.Find(_T("拒绝访问")) >= 0)
	{
		CString str;
		str.Format(_T("无法%s任务计划程序，需要用管理员权限运行"), bWrite ? (bAutorun ? _T("创建") : _T("删除")) : _T("读取"));
		AfxMessageBox(str);
		return FALSE;
	}
	PCTSTR strFind = bWrite ? (bAutorun ? _T("成功创建") : _T("成功删除")) : (LPCTSTR)strTaskName;
	if (rs.Find(strFind) >= 0)
		return TRUE;
	return FALSE;
}

CString CMyFanControlDlg::ExecuteCmd(CString str)
{
	SECURITY_ATTRIBUTES sa;
	HANDLE hRead, hWrite;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.lpSecurityDescriptor = NULL;
	sa.bInheritHandle = TRUE;
	if (!CreatePipe(&hRead, &hWrite, &sa, 0))
	{
#ifdef MY_DEBUG
		AfxMessageBox(_T("无法创建管道"));
#endif
		return _T("[执行失败]");
	}
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi = { 0 };  // 初始化为0
	si.hStdError = hWrite;
	si.hStdOutput = hWrite;
	si.wShowWindow = SW_HIDE;
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	char cmdline[1024];
	strcpy_s(cmdline, 1024, CT2A(str));
	if (!CreateProcess(NULL, cmdline, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &pi))
	{
#ifdef MY_DEBUG
		AfxMessageBox(_T("无法创建命令行进程"));
#endif
		CloseHandle(hWrite);
		CloseHandle(hRead);
		return _T("[执行失败]");
	}
	CloseHandle(hWrite);

	char buffer[4096] = "";
	memset(buffer, 0, 4096);
	CStringA outputA;
	DWORD byteRead;
	int i = 0;
	while (true)
	{
		Sleep(100);
		if (ReadFile(hRead, buffer, 4095, &byteRead, NULL) == NULL)
			break;
		if (byteRead)
			outputA += buffer;
		if (i++ >= 50)
			break;
	}
	CloseHandle(hRead);

	// 关闭进程句柄，防止泄漏
	CloseHandle(pi.hThread);
	CloseHandle(pi.hProcess);

	return CString(outputA);
}

BOOL CMyFanControlDlg::CreateTaskXml(PCSTR strXmlPath, PCSTR strTargetPath)
{
	//运行权限
	//最高权限<RunLevel>HighestAvailable</RunLevel>\r\n
	//普通权限<RunLevel>LeastPrivilege</RunLevel>\r\n
	//用户组
	//SYSTEM<UserId>S-1-5-18</UserId>\r\n
	//USERS<GroupId>S-1-5-32-545</GroupId>\r\n
	PCSTR XmlStr = "\
<?xml version=\"1.0\" encoding=\"UTF-16\"?>\r\n\
<Task version=\"1.2\" xmlns=\"http://schemas.microsoft.com/windows/2004/02/mit/task\">\r\n\
  <RegistrationInfo>\r\n\
    <Date>2018-11-16T16:16:29</Date>\r\n\
    <Author>HQ</Author>\r\n\
    <URI>\\蓝天风扇监控</URI>\r\n\
  </RegistrationInfo>\r\n\
  <Triggers>\r\n\
    <LogonTrigger>\r\n\
      <Enabled>true</Enabled>\r\n\
    </LogonTrigger>\r\n\
  </Triggers>\r\n\
  <Principals>\r\n\
    <Principal id=\"Author\">\r\n\
      <GroupId>S-1-5-32-545</GroupId>\r\n\
      <RunLevel>HighestAvailable</RunLevel>\r\n\
    </Principal>\r\n\
  </Principals>\r\n\
  <Settings>\r\n\
    <MultipleInstancesPolicy>IgnoreNew</MultipleInstancesPolicy>\r\n\
    <DisallowStartIfOnBatteries>false</DisallowStartIfOnBatteries>\r\n\
    <StopIfGoingOnBatteries>false</StopIfGoingOnBatteries>\r\n\
    <AllowHardTerminate>false</AllowHardTerminate>\r\n\
    <StartWhenAvailable>false</StartWhenAvailable>\r\n\
    <RunOnlyIfNetworkAvailable>false</RunOnlyIfNetworkAvailable>\r\n\
    <IdleSettings>\r\n\
      <StopOnIdleEnd>false</StopOnIdleEnd>\r\n\
      <RestartOnIdle>false</RestartOnIdle>\r\n\
    </IdleSettings>\r\n\
    <AllowStartOnDemand>true</AllowStartOnDemand>\r\n\
    <Enabled>true</Enabled>\r\n\
    <Hidden>false</Hidden>\r\n\
    <RunOnlyIfIdle>false</RunOnlyIfIdle>\r\n\
    <WakeToRun>false</WakeToRun>\r\n\
    <ExecutionTimeLimit>PT0S</ExecutionTimeLimit>\r\n\
    <Priority>7</Priority>\r\n\
  </Settings>\r\n\
  <Actions Context=\"Author\">\r\n\
    <Exec>\r\n\
      <Command>%s</Command>\r\n\
    </Exec>\r\n\
  </Actions>\r\n\
</Task>\r\n";
	char str[10240];
	sprintf_s(str, 10240, XmlStr, strTargetPath);
	FILE* fp;
	fp = fopen(strXmlPath, "wt");
	if (!fp)
		return FALSE;
	fwrite(str, strlen(str), 1, fp);

	fclose(fp);
	return TRUE;
}

void CMyFanControlDlg::OnBnClickedCheckLockGpuFrequancy()
{
	// TODO:  在此添加控件通知处理程序代码
	int val = m_ctlLockGpuFrequancy.GetCheck();
	if (val)
	{
		char str[256];
		m_ctlFrequency.GetWindowTextA(str, 256);
		int nFrequency = atoi(str);
		if (!CheckInputFrequency(nFrequency))
			m_ctlLockGpuFrequancy.SetCheck(FALSE);
	}

	m_core.m_config.LockGPUFrequency = m_ctlLockGpuFrequancy.GetCheck();
}

BOOL CMyFanControlDlg::CheckInputFrequency(int nFrequency)
{
	char str[256];
	if (nFrequency < 0 || nFrequency > m_core.m_GpuInfo.m_nMaxFrequency)
	{
		char str2[256];
		sprintf_s(str2, 256, "GPU频率限制必须为0-%d，默认频率为%d，0为默认频率", m_core.m_GpuInfo.m_nMaxFrequency, m_core.m_GpuInfo.m_nStandardFrequency);
		AfxMessageBox(str2);
		return FALSE;
	}
	else
	{
		if (nFrequency > m_core.m_GpuInfo.m_nStandardFrequency)
		{
			char str2[256];
			sprintf_s(str2, 256, "GPU默认频率为%d，超频会降低系统稳定性，并会增加发热量。\n注意：由于功率限制，可能无法达到设定的频率。\n是否确认要超频？", m_core.m_GpuInfo.m_nStandardFrequency);
			int rv = MessageBox(str2, _T("确认要超频？"), MB_YESNO);

			if (IDYES == rv)
			{
			}
			else if (IDNO == rv)
			{
				return FALSE;
			}
		}
		else if (nFrequency == 0)
		{
			nFrequency = m_core.m_GpuInfo.m_nStandardFrequency;
			m_core.m_config.GPUFrequency = m_core.m_GpuInfo.m_nStandardFrequency;
			sprintf_s(str, 256, "%d", m_core.m_config.GPUFrequency);
			m_ctlFrequency.SetWindowTextA(str);
		}
	}


	return TRUE;
}

void CMyFanControlDlg::OnBnClickedCheckLockMemOverclock()
{
	int val = m_ctlLockMemOverclock.GetCheck();
	if (val)
	{
		// 检查显存偏移值是否有效
		char str[256];
		m_ctlMemOffset.GetWindowTextA(str, 256);
		int nMemOffset = atoi(str);

		if (nMemOffset < m_core.m_GpuInfo.m_nMemoryRangeMin || nMemOffset > m_core.m_GpuInfo.m_nMemoryRangeMax)
		{
			char str2[256];
			sprintf_s(str2, 256, "显存频率偏移必须为%d-%d，默认值为0", m_core.m_GpuInfo.m_nMemoryRangeMin, m_core.m_GpuInfo.m_nMemoryRangeMax);
			AfxMessageBox(str2);
			m_ctlLockMemOverclock.SetCheck(FALSE);
			return;
		}

		// 安全警告
		if (nMemOffset > 0)
		{
			int rv = MessageBox(_T("修改显存频率可能导致系统不稳定或损坏显卡，是否确认？"), _T("安全警告"), MB_YESNO | MB_ICONWARNING);
			if (rv == IDNO)
			{
				m_ctlLockMemOverclock.SetCheck(FALSE);
				return;
			}
		}
	}

	m_core.m_config.LockMemOverclock = m_ctlLockMemOverclock.GetCheck();
}