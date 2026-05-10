#include "stdafx.h"
#include "Core.h"
#include "MyFanControlDlg.h"


int TEMP_LIST[10] = { 90, 85, 80, 75, 70, 65, 60, 55, 50, 45 };


int GetTime(tm* pt, int offset)
{
	tm t = { 0 };
	time_t tt;
	if (!pt)
		pt = &t;
	time(&tt);
	tt += offset;
	localtime_s(pt, &tt);
	return (pt->tm_hour * 10000 + pt->tm_min * 100 + pt->tm_sec);
}
int GetTimeInterval(int a, int b, int* p)
{
	//时间差，输入两个6位数时间，如开盘时间91500，得到a-b，并转化为6位数时间，指针p接受以秒计的时间差
	int a1 = a / 10000;
	int a2 = (a % 10000) / 100;
	int a3 = a % 100;

	int b1 = b / 10000;
	int b2 = (b % 10000) / 100;
	int b3 = b % 100;

	int c = (a1 - b1) * 3600 + (a2 - b2) * 60 + a3 - b3;
	if (p)
		*p = c;
	int sgn = (c >= 0) ? 1 : -1;
	c = abs(c);

	int d = (c / 3600) * 10000 + (c % 3600) / 60 * 100 + c % 60;
	return d * sgn;
}

CString GetExePath()
{
	TCHAR pathbuf[MAX_PATH] = { 0 };
	DWORD pathlen = ::GetModuleFileName(NULL, pathbuf, MAX_PATH);
	if (pathlen == 0 || pathlen >= MAX_PATH)
		return _T("");

	// 从后向前查找最后一个反斜杠
	int i = (int)pathlen - 1;
	while (i >= 0 && pathbuf[i] != _T('\\'))
		i--;

	if (i >= 0)
		pathbuf[i + 1] = _T('\0');   // 截断为目录路径
	else
		pathbuf[0] = _T('\0');       // 未找到反斜杠

	return CString(pathbuf);
}


CGPUInfo::CGPUInfo()
{
	TRACE0("开始加载NVGPU_DLL.dll。\n");
	m_hGPUdll = NULL;
	m_nMemOverclockOffset = 0;

	// 初始化状态缓存
	m_nLastSetCoreOC = 0;
	m_nLastSetMemOC = 0;
	m_nLastLockedClock = 0;
	m_bMemOCSet = FALSE;
	m_bResumeFromSleep = FALSE;
	m_hWndNotify = NULL;

	CString dllpth = GetExePath() + _T("\\NVGPU_DLL.dll");
	m_hGPUdll = LoadLibrary(dllpth);
	if (m_hGPUdll == NULL)
	{
		TRACE0("无法加载" + dllpth + "\n");
		return;
	}

	m_pfnInitGPU_API = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "InitGPU_API");
	m_pfnSet_GPU_Number = (In_1_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Set_GPU_Number");
	m_pfnGet_GPU_Base_Clock = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Base_Clock");
	m_pfnGet_GPU_Boost_Clock = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Boost_Clock");
	m_pfnCheck_GPU_VRAM_Clock = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Check_GPU_VRAM_Clock");
	m_pfnGet_GPU_Graphics_Clock = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Graphics_Clock");
	m_pfnGet_GPU_Memory_Clock = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Memory_Clock");
	m_pfnGet_Memory_OC_max = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_Memory_OC_max");
	m_pfnGet_GPU_Util = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Util");
	m_pfnGet_GPU_name = (In_0_Out_s_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_name");
	m_pfnGet_GPU_TotalNumber = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_TotalNumber");
	m_pfnGet_GPU_Overclock_range = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Overclock_range");
	m_pfnGet_Memory_range = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_Memory_range");
	m_pfnGet_GPU_Overclock_rangeMax = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Overclock_rangeMax");
	m_pfnGet_GPU_Overclock_rangeMin = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_GPU_Overclock_rangeMin");
	m_pfnGet_Memory_range_max = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_Memory_range_max");
	m_pfnGet_Memory_range_min = (In_0_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_Memory_range_min");
	m_pfnGet_NVDeviceID = (In_1_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Get_NVDeviceID");
	m_pfnLock_Frequency = (In_2_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Lock_Frequency");
	m_pfnLock_Frequency_MEM = (In_2_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Lock_Frequency_MEM");
	m_pfnSet_CoreOC = (In_2_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Set_CoreOC");
	m_pfnSet_MEMOC = (In_2_Out_n_Func*)::GetProcAddress(m_hGPUdll, "Set_MEMOC");
	m_pfnCloseGPU_API = (In_0_Out_0_Func*)::GetProcAddress(m_hGPUdll, "CloseGPU_API");

	//
	if (m_pfnInitGPU_API())
	{
		TRACE0("InitGPU_API初始化失败。\n");
		FreeLibrary(m_hGPUdll);
		m_hGPUdll = NULL;
		return;
	}
	m_pfnSet_GPU_Number(0);
	m_nBaseClock = m_pfnGet_GPU_Base_Clock();
	m_nBoostClock = m_pfnGet_GPU_Boost_Clock();
	m_sName = m_pfnGet_GPU_name();
	m_nDeviceID = m_pfnGet_NVDeviceID(0);
	m_nGraphicsRangeMax = m_pfnGet_GPU_Overclock_rangeMax();
	m_nGraphicsRangeMin = m_pfnGet_GPU_Overclock_rangeMin();

	// 标准频率和最大频率
	m_nStandardFrequency = m_nBoostClock - m_nGraphicsRangeMin;
	m_nMaxFrequency = m_nStandardFrequency + m_nGraphicsRangeMax;

	m_nMemoryRangeMax = 2000;
	m_nMemoryRangeMin = -2000;

	// 校验显存范围合理性
	if (m_nMemoryRangeMax < m_nMemoryRangeMin || m_nMemoryRangeMax == 0)
	{
		TRACE("Invalid Memory OC range from NVAPI (min=%d, max=%d), falling back to ±500\n",
			m_nMemoryRangeMin, m_nMemoryRangeMax);
		m_nMemoryRangeMax = 500;
		m_nMemoryRangeMin = -500;
	}

	m_nLockClock = -1;

	Update();

	TRACE0("成功加载NVGPU_DLL.dll。\n");
}

CGPUInfo::~CGPUInfo()
{
	if (m_hGPUdll != NULL)
	{
		LockFrequency();//还原GPU频率设置
		SetMemOverclockOffset(0);//还原显存偏移
		m_pfnCloseGPU_API();
		FreeLibrary(m_hGPUdll);
		m_hGPUdll = NULL;
	}
}

void CGPUInfo::SetNotifyWindow(HWND hWnd)
{
	m_hWndNotify = hWnd;
}

void CGPUInfo::ReportError(LPCTSTR szErrorMsg)
{
	if (m_hWndNotify && szErrorMsg)
	{
		// 分配堆内存存储错误消息（接收方负责释放）
		size_t len = _tcslen(szErrorMsg) + 1;
		TCHAR* pMsg = new TCHAR[len];
		_tcscpy_s(pMsg, len, szErrorMsg);

		if (!::PostMessage(m_hWndNotify, WM_GPU_ERROR, 0, (LPARAM)pMsg))
		{
			// 发送失败则释放内存
			delete[] pMsg;
		}
	}
	else
	{
		// 没有通知窗口，输出到调试日志
		TRACE("GPU Error: %s\n", szErrorMsg);
	}
}

BOOL CGPUInfo::Update()
{
	if (!m_hGPUdll)
		return FALSE;
	if (!m_pfnCheck_GPU_VRAM_Clock())
		return FALSE;
	m_nGraphicsClock = m_pfnGet_GPU_Graphics_Clock();
	m_nMemoryClock = m_pfnGet_GPU_Memory_Clock();
	m_nUsage = m_pfnGet_GPU_Util();
	return TRUE;
}

BOOL CGPUInfo::LockFrequency(int frequency)
{
	if (!m_hGPUdll)
		return FALSE;
	if (frequency < 0 || frequency > m_nMaxFrequency)
		return FALSE;
	if (frequency == 0)
		frequency = m_nStandardFrequency;

	int GpuOverclock = 0;
	int GpuClock = 0;

	if (frequency > 0 && frequency < m_nStandardFrequency)
	{
		//降频
		GpuClock = frequency;
	}
	else if (frequency > m_nStandardFrequency)
	{
		//超频
		GpuOverclock = frequency - m_nStandardFrequency;
	}

	// 检查状态缓存，如果和上次设置完全一致则跳过
	if (m_nLockClock == frequency &&
		m_nLastSetCoreOC == GpuOverclock &&
		m_nLastLockedClock == GpuClock)
	{
		return TRUE;
	}

	m_nLockClock = frequency;

	// 设置核心超频偏移
	int rv1 = (m_pfnSet_CoreOC(0, GpuOverclock) == 0);
	if (!rv1)
	{
		if (m_bResumeFromSleep)
		{
			// 休眠恢复后静默重试3次
			for (int i = 0; i < 3; i++)
			{
				Sleep(200);
				if (m_pfnSet_CoreOC(0, GpuOverclock) == 0)
				{
					rv1 = 1;
					break;
				}
			}
			if (!rv1)
				TRACE("Set_CoreOC failed after resume retry\n");
		}
		else
		{
			ReportError(_T("Set_CoreOC失败"));
		}
	}

	// 锁定核心频率
	int rv3 = (m_pfnLock_Frequency(0, GpuClock) == 0x19);
	if (!rv3)
	{
		if (m_bResumeFromSleep)
		{
			// 休眠恢复后静默重试3次
			for (int i = 0; i < 3; i++)
			{
				Sleep(200);
				if (m_pfnLock_Frequency(0, GpuClock) == 0x19)
				{
					rv3 = 1;
					break;
				}
			}
			if (!rv3)
				TRACE("Lock_Frequency failed after resume retry\n");
		}
		else
		{
			ReportError(_T("Lock_Frequency失败"));
		}
	}

	// 更新状态缓存
	if (rv1 && rv3)
	{
		m_nLastSetCoreOC = GpuOverclock;
		m_nLastLockedClock = GpuClock;
	}

	return (rv1 && rv3);
}

BOOL CGPUInfo::SetMemOverclockOffset(int offset)
{
	if (!m_hGPUdll)
		return FALSE;

	if (offset < m_nMemoryRangeMin || offset > m_nMemoryRangeMax)
		return FALSE;

	// 检查缓存，如果已经设置为相同值则跳过
	if (m_bMemOCSet && m_nLastSetMemOC == offset)
		return TRUE;

	int rv = (m_pfnSet_MEMOC(0, offset) == 0);
	if (!rv)
	{
		if (m_bResumeFromSleep)
		{
			// 休眠恢复后静默重试5次
			for (int i = 0; i < 5; i++)
			{
				Sleep(200);
				if (m_pfnSet_MEMOC(0, offset) == 0)
				{
					rv = 1;
					break;
				}
			}

			if (!rv)
			{
				// 静默失败，记录日志，恢复默认
				TRACE("Set_MEMOC failed after resume retry, offset=%d\n", offset);
				m_pfnSet_MEMOC(0, 0);
				m_nLastSetMemOC = 0;
				m_bMemOCSet = TRUE;
				m_nMemOverclockOffset = 0;
				return FALSE;
			}
		}
		else
		{
			ReportError(_T("Set_MEMOC失败"));
			// 失败时自动恢复默认值0，并立即应用到硬件
			m_pfnSet_MEMOC(0, 0);
			m_nLastSetMemOC = 0;
			m_bMemOCSet = TRUE;
			m_nMemOverclockOffset = 0;
			return FALSE;
		}
	}

	// 更新状态缓存
	m_nLastSetMemOC = offset;
	m_bMemOCSet = TRUE;
	m_nMemOverclockOffset = offset;

	return TRUE;
}

CConfig::CConfig()
{
	LoadDefault();
}
void CConfig::LoadDefault()
{
	int i = 0;
	DutyList[0][i++] = 95;//90+
	DutyList[0][i++] = 80;//85+
	DutyList[0][i++] = 70;//80+
	DutyList[0][i++] = 55;//75+
	DutyList[0][i++] = 35;//70+
	DutyList[0][i++] = 30;//65+
	DutyList[0][i++] = 25;//60+
	DutyList[0][i++] = 18;//55+
	DutyList[0][i++] = 18;//50+
	DutyList[0][i++] = 18;//50-
	for (int i = 0; i < 10; i++)
		DutyList[1][i] = DutyList[0][i];

	TransitionTemp = 3;
	UpdateInterval = 2;
	Linear = FALSE;
	TakeOver = FALSE;
	ForceTemp = 50;
	SoftControl = FALSE;
	LockGPUFrequency = FALSE;
	GPUFrequency = 0;
	LockMemOverclock = FALSE;  // 默认关闭显存偏移
	MemOverclockOffset = 0;

	// 默认温度档位
	TempThresholds[0] = 90;
	TempThresholds[1] = 85;
	TempThresholds[2] = 80;
	TempThresholds[3] = 75;
	TempThresholds[4] = 70;
	TempThresholds[5] = 65;
	TempThresholds[6] = 60;
	TempThresholds[7] = 55;
	TempThresholds[8] = 50;
	TempThresholds[9] = 45;
}
void CConfig::LoadConfig()
{
	CString strPath = GetExePath() + _T("\\MyFanControl.cfg");
	CFile file;
	if (!file.Open(strPath, CFile::modeRead | CFile::shareDenyNone))
	{
		SaveConfig();
		if (!file.Open(strPath, CFile::modeRead | CFile::shareDenyNone))
		{
			AfxMessageBox(_T("无法载入配置文件"));
			return;
		}
	}
	if (file.GetLength() != sizeof(*this))
	{
		file.Close();
		DeleteFile(strPath);
		LoadDefault();
		SaveConfig();
		if (!file.Open(strPath, CFile::modeRead | CFile::shareDenyNone))
		{
			AfxMessageBox(_T("重置后仍然无法载入配置文件"));
			return;
		}
		if (file.GetLength() != sizeof(*this))
		{
			AfxMessageBox(_T("配置文件格式不正确"));
			file.Close();
			return;
		}
	}
	file.Read(this, sizeof(*this));
	file.Close();
}
void CConfig::SaveConfig()
{
	FILE* fp = NULL;
	CString strPath = GetExePath() + _T("\\MyFanControl.cfg");
	fp = _tfopen(strPath, _T("wb"));
	if (fp == NULL)
	{
		AfxMessageBox(_T("无法保存配置文件"));
		return;
	}
	fwrite(this, sizeof(*this), 1, fp);
	fclose(fp);
}

CCore::CCore()
{
	m_pfnInitIo = NULL;
	m_pfnSetFanDuty = NULL;
	m_pfnSetFANDutyAuto = NULL;
	m_pfnGetTempFanDuty = NULL;
	m_pfnGetFANCounter = NULL;
	m_pfnGetECVersion = NULL;
	m_pfnGetFANRPM[0] = NULL;
	m_pfnGetFANRPM[1] = NULL;
	//
	m_nInit = 0;
	m_nExit = 0;
	m_hInstDLL = NULL;
	m_nTimerID = 0;  // 初始化定时器ID
	m_hSoftControlThread = NULL;
	m_pParentDlg = NULL;

	// 初始化临界区
	InitializeCriticalSectionEx(&m_csFanControl, 0, 0);

	for (int i = 0; i < 2; i++)
	{
		m_nCurTemp[i] = 0;//当前温度
		m_nLastTemp[i] = 0;//上一次温度
		m_nSetDuty[i] = 0;//设置的负载
		m_nSetDutyLevel[i] = 0;//设置的转速挡位，最低速档为1，最高速档为10
		m_nCurDuty[i] = 0;//当前负载
		m_nCurRPM[i] = 0;//当前转速
		m_nSoftTargetDuty[i] = 0;//软性控制目标
		m_nSoftCurrentDuty[i] = 0;//软性控制当前
	}
	m_bUpdateRPM = 0;//是否更新转速，如果为0，只更新风扇温度和负载
	m_nLastUpdateTime = GetTime(0, -5);
	m_bForcedCooling = FALSE;
	m_bTakeOverStatus = FALSE;
	m_bForcedRefresh = FALSE;
}

CCore::~CCore()
{
	// 确保软控线程退出
	if (m_hSoftControlThread)
	{
		WaitForSingleObject(m_hSoftControlThread, 2000);
		CloseHandle(m_hSoftControlThread);
		m_hSoftControlThread = NULL;
	}

	// 确保定时器被清理
	if (m_nTimerID)
	{
		timeKillEvent(m_nTimerID);
		m_nTimerID = 0;
	}
	Uninit();

	// 删除临界区
	DeleteCriticalSection(&m_csFanControl);
}

BOOL CCore::Init()
{


	if (m_hInstDLL)
	{
		return TRUE;
	}

	TRACE0("内核开始初始化。\n");
	m_nInit = -1;
	//
	CString dllpth = GetExePath() + _T("\\ClevoEcInfo.dll");

	m_hInstDLL = LoadLibrary(dllpth);
	if (m_hInstDLL == NULL)
	{
		AfxMessageBox(_T("无法加载") + dllpth + _T("，请确保该文件在程序目录下，并且已安装NTPortDrv。"));
		return FALSE;
	}

	m_pfnInitIo = (InitIo*)::GetProcAddress(m_hInstDLL, "InitIo");
	m_pfnSetFanDuty = (::SetFanDuty*)::GetProcAddress(m_hInstDLL, "SetFanDuty");
	m_pfnSetFANDutyAuto = (SetFANDutyAuto*)::GetProcAddress(m_hInstDLL, "SetFanDutyAuto");
	m_pfnGetTempFanDuty = (GetTempFanDuty*)::GetProcAddress(m_hInstDLL, "GetTempFanDuty");
	m_pfnGetFANCounter = (GetFANCounter*)::GetProcAddress(m_hInstDLL, "GetFanCount");

	m_pfnGetECVersion = (GetECVersion*)::GetProcAddress(m_hInstDLL, "GetECVersion");
	m_pfnGetFANRPM[0] = (GetFanRpm*)::GetProcAddress(m_hInstDLL, "GetCpuFanRpm");
	m_pfnGetFANRPM[1] = (GetFanRpm*)::GetProcAddress(m_hInstDLL, "GetGpuFanRpm");

	if (m_pfnInitIo == NULL)
	{
		FreeLibrary(m_hInstDLL);
		m_hInstDLL = NULL;
		AfxMessageBox(_T("错误的ClevoEcInfo.dll"));
		return FALSE;
	}

	if (m_pfnInitIo() != 1)
	{
		FreeLibrary(m_hInstDLL);
		m_hInstDLL = NULL;
		AfxMessageBox(_T("接口初始化返回值错误！"));
		return FALSE;
	}

	TRACE0("内核初始化成功。\n");
	m_nInit = 1;
	return TRUE;
}
void CCore::Uninit()
{
	ResetFan();
	if (m_hInstDLL != NULL)
	{
		FreeLibrary(m_hInstDLL);
		m_hInstDLL = NULL;
	}
	m_nInit = 0;
}

// 定时器回调函数
void CALLBACK CCore::TimerCallback(UINT uTimerID, UINT uMsg, DWORD_PTR dwUser, DWORD_PTR dw1, DWORD_PTR dw2)
{
	CCore* pCore = (CCore*)dwUser;
	if (pCore && !pCore->m_nExit)
	{
		// 设置强制刷新标志，让主循环执行Work
		pCore->m_bForcedRefresh = TRUE;
	}
}

// 软性控制独立线程
DWORD WINAPI CCore::SoftControlThreadProc(LPVOID lpParam)
{
	CCore* pCore = (CCore*)lpParam;
	TRACE0("软性控制线程启动。\n");

	while (!pCore->m_nExit)
	{
		if (pCore->m_config.SoftControl && pCore->m_config.TakeOver && !pCore->m_bForcedCooling)
		{
			pCore->SoftControlDuty();
		}
		Sleep(100);  // 每100ms调整1%
	}

	TRACE0("软性控制线程退出。\n");
	return 0;
}

void CCore::Run()
{
	m_config.LoadConfig();

	if (!m_nInit)
		Init();

	// 设置GPU错误通知窗口
	if (m_pParentDlg)
	{
		m_GpuInfo.SetNotifyWindow(m_pParentDlg->GetSafeHwnd());
	}

	if (m_nInit == 1)
	{
		TRACE0("内核开始运行（多媒体定时器模式）。\n");

		// 创建软性控制独立线程
		m_hSoftControlThread = CreateThread(NULL, 0, SoftControlThreadProc, this, 0, NULL);
		if (m_hSoftControlThread)
		{
			SetThreadPriority(m_hSoftControlThread, THREAD_PRIORITY_ABOVE_NORMAL);
		}

		// 获取定时器能力
		TIMECAPS tc;
		if (timeGetDevCaps(&tc, sizeof(TIMECAPS)) == TIMERR_NOERROR)
		{
			// 计算定时器间隔（毫秒）
			UINT nInterval = m_config.UpdateInterval * 1000;
			nInterval = max(nInterval, tc.wPeriodMin);
			nInterval = min(nInterval, tc.wPeriodMax);

			// 创建多媒体定时器
			m_nTimerID = timeSetEvent(
				nInterval,                    // 延时（毫秒）
				tc.wPeriodMin,                // 分辨率
				TimerCallback,                // 回调函数
				(DWORD_PTR)this,              // 用户数据
				TIME_PERIODIC | TIME_CALLBACK_FUNCTION);  // 周期性触发

			if (m_nTimerID)
			{
				TRACE0("多媒体定时器创建成功。\n");

				while (!m_nExit)
				{
					if (m_bForcedRefresh)
					{
						Work();
						m_nLastUpdateTime = GetTime();
						m_bForcedRefresh = FALSE;
					}

					Sleep(500);
				}

				// 清理定时器
				timeKillEvent(m_nTimerID);
				m_nTimerID = 0;
				TRACE0("内核结束运行。\n");
			}
			else
			{
				TRACE0("多媒体定时器创建失败，回退到原始模式。\n");
				m_nInit = 1;
				RunOriginal();
			}
		}
		else
		{
			TRACE0("无法获取定时器能力，回退到原始模式。\n");
			m_nInit = 1;
			RunOriginal();
		}

		// 等待软控线程结束
		if (m_hSoftControlThread)
		{
			WaitForSingleObject(m_hSoftControlThread, 2000);
			CloseHandle(m_hSoftControlThread);
			m_hSoftControlThread = NULL;
		}
	}
	m_nExit = 2;
}

void CCore::RunOriginal()
{
	static int nNextChecktTime = 0;
	static BOOL bSetPriority = FALSE;

	if (m_nInit == 1)
	{
		TRACE0("内核开始运行（原始模式）。\n");

		// 创建软性控制独立线程
		m_hSoftControlThread = CreateThread(NULL, 0, SoftControlThreadProc, this, 0, NULL);
		if (m_hSoftControlThread)
		{
			SetThreadPriority(m_hSoftControlThread, THREAD_PRIORITY_ABOVE_NORMAL);
		}

		int curtime;
		while (!m_nExit)
		{
			curtime = GetTime();
			if (curtime >= nNextChecktTime || m_bForcedRefresh)
			{
				//MessageBox(NULL , "工作中...", "MyFunColtrol" , 0);
				Work();
				m_nLastUpdateTime = curtime;//更新时间
				nNextChecktTime = GetTime(NULL, m_config.UpdateInterval);//下一个更新时间
				m_bForcedRefresh = FALSE;
				if (!bSetPriority)
				{
					bSetPriority = TRUE;
					SetPriorityClass(GetCurrentProcess(), HIGH_PRIORITY_CLASS);//在首次更新成功后才设置高优先级
				}
			}
			Sleep(100);
		}
		TRACE0("内核结束运行。\n");

		// 等待软控线程结束
		if (m_hSoftControlThread)
		{
			WaitForSingleObject(m_hSoftControlThread, 2000);
			CloseHandle(m_hSoftControlThread);
			m_hSoftControlThread = NULL;
		}
	}
	m_nExit = 2;
}

void CCore::Work()
{
	Update();

	// 同步休眠恢复状态
	if (m_pParentDlg)
	{
		m_GpuInfo.m_bResumeFromSleep = m_pParentDlg->m_bResumeFromSleep;
	}

	if (m_bForcedCooling)//强制冷却
	{
		if (m_nCurTemp[0] >= m_config.ForceTemp || m_nCurTemp[1] >= m_config.ForceTemp)
		{
			if (m_nSetDuty[0] < 100 || m_nSetDuty[1] < 100)
			{
				m_nSetDuty[0] = 100;
				m_nSetDutyLevel[0] = 10;
				m_nSetDuty[1] = 100;
				m_nSetDutyLevel[1] = 10;
				// 强制冷却直接设置，不经过软性控制
				EnterCriticalSection(&m_csFanControl);
				for (int i = 0; i < 2; i++)
				{
					m_nSoftTargetDuty[i] = m_nSetDuty[i];
					m_nSoftCurrentDuty[i] = m_nSetDuty[i];
				}
				LeaveCriticalSection(&m_csFanControl);
				SetFanDuty();
			}
			return;
		}
		else
			m_bForcedCooling = FALSE;
	}
	if (m_config.TakeOver)
	{
		Control();
	}
	else
		ResetFan();

	//锁定GPU频率
	if (m_config.LockGPUFrequency)
		m_GpuInfo.LockFrequency(m_config.GPUFrequency);
	else
		m_GpuInfo.LockFrequency(0);

	//设置显存频率偏移
	if (m_config.LockMemOverclock)
	{
		if (!m_GpuInfo.SetMemOverclockOffset(m_config.MemOverclockOffset))
		{
			// 设置失败，自动归零
			m_config.LockMemOverclock = FALSE;
			m_config.MemOverclockOffset = 0;
			m_config.SaveConfig();
		}
	}
	else
		m_GpuInfo.SetMemOverclockOffset(0);
}
void CCore::Update()
{
	ECData data;
	int TempErr = 0;
	for (int i = 0; i < 2; i++)
	{
		data = m_pfnGetTempFanDuty(i + 1);
		if (abs(data.Remote - this->m_nCurTemp[i]) > 30)
		{
			//AfxMessageBox("获取温度有误");
			//温度获取可能有误，重试一次
			if (TempErr++ == 0)
			{
				Sleep(1000);
				i--;
				continue;//重试
			}
		}
		this->m_nLastTemp[i] = this->m_nCurTemp[i];
		this->m_nCurTemp[i] = data.Remote;
		this->m_nCurDuty[i] = int(data.FanDuty * 100 / 255.0 + 0.5);

		if (m_bUpdateRPM)//获取风扇转速
		{
			int val = m_pfnGetFANRPM[i]();
			if (val == 0)
				this->m_nCurRPM[i] = 0;
			if (val > 300 && val < 5000)
				this->m_nCurRPM[i] = 2100000 / val;
		}
		else
		{
			this->m_nCurRPM[i] = -1;
		}
		TempErr = 0;
	}
	if (m_bUpdateRPM)
		m_GpuInfo.Update();
}
void CCore::Control()
{
	if (m_config.Linear)
		CalcLinearDuty();
	else
		CalcStdDuty();
	//设定转速
	if (m_config.SoftControl)
	{
		// 软性控制：设置目标值，由SoftControlThreadProc()控制逐步接近
		EnterCriticalSection(&m_csFanControl);
		for (int i = 0; i < 2; i++)
		{
			m_nSoftTargetDuty[i] = m_nSetDuty[i];
			if (m_nSoftCurrentDuty[i] <= 0)
				m_nSoftCurrentDuty[i] = m_nCurDuty[i];
		}
		LeaveCriticalSection(&m_csFanControl);
	}
	else
	{
		SetFanDuty();
	}
}
void CCore::CalcLinearDuty()
{
	static int nLastTemp[2] = { 0, 0 };//每次用于计算转速的温度

	int duty, dl;
	int j;
	for (int i = 0; i < 2; i++)
	{
		nLastTemp[i] = max(nLastTemp[i], m_nCurTemp[i]);//温度上升时立刻以当前温度计算转速
		nLastTemp[i] = min(nLastTemp[i], m_nCurTemp[i] + m_config.TransitionTemp);//温度下降时以当前温度+过渡温度来计算转速

		j = nLastTemp[i];//计算转速使用的温度

		if (j < m_config.TempThresholds[9])
		{
			duty = m_config.DutyList[i][9];
			dl = 0;
		}
		else if (j >= m_config.TempThresholds[0])
		{
			duty = m_config.DutyList[i][0];
			dl = 10;
		}
		else
		{
			int idx = 0;
			if (j < m_config.TempThresholds[8])
				idx = 8;
			else if (j < m_config.TempThresholds[7])
				idx = 7;
			else if (j < m_config.TempThresholds[6])
				idx = 6;
			else if (j < m_config.TempThresholds[5])
				idx = 5;
			else if (j < m_config.TempThresholds[4])
				idx = 4;
			else if (j < m_config.TempThresholds[3])
				idx = 3;
			else if (j < m_config.TempThresholds[2])
				idx = 2;
			else if (j < m_config.TempThresholds[1])
				idx = 1;
			else// if (j < TempThresholds[0])
				idx = 0;

			int temp_l = m_config.TempThresholds[idx + 1];
			int temp_h = m_config.TempThresholds[idx];
			int duty_l = m_config.DutyList[i][idx + 1];
			int duty_h = m_config.DutyList[i][idx];
			duty = int((duty_h - duty_l) / double(temp_h - temp_l) * (j - temp_l) + 0.5) + duty_l;
			dl = 9 - idx;
		}
		m_nSetDuty[i] = duty;
		m_nSetDutyLevel[i] = dl;
	}
}
void CCore::CalcStdDuty()
{
	int dl;
	int last_dl;
	int j, k;
	for (int i = 0; i < 2; i++)
	{
		j = m_nCurTemp[i];
		last_dl = m_nSetDutyLevel[i];//上一次的负载等级
		for (k = 0; k < 10; k++)
		{
			dl = 10 - k;
			if (j >= m_config.TempThresholds[k])
			{
				break;
			}
			else if (j < m_config.TempThresholds[k] - m_config.TransitionTemp)
			{
				continue;
			}
			else
			{
				//根据上一次的负载挡位决定
				if (last_dl >= dl)
				{
					break;
				}
				continue;
			}
		}
		k = min(9, k);
		m_nSetDuty[i] = m_config.DutyList[i][k];
		m_nSetDutyLevel[i] = dl;
	}
}
void CCore::ResetFan()
{
	if (m_bTakeOverStatus)
	{
		EnterCriticalSection(&m_csFanControl);
		m_nSetDuty[0] = 0;
		m_nSetDutyLevel[0] = 0;
		m_nSetDuty[1] = 0;
		m_nSetDutyLevel[1] = 0;
		m_nSoftTargetDuty[0] = 0;
		m_nSoftTargetDuty[1] = 0;
		m_nSoftCurrentDuty[0] = 0;
		m_nSoftCurrentDuty[1] = 0;
		LeaveCriticalSection(&m_csFanControl);

		m_pfnSetFANDutyAuto(1);
		m_pfnSetFANDutyAuto(2);
		m_pfnSetFANDutyAuto(3);
		m_bTakeOverStatus = FALSE;
	}
}
void CCore::SetFanDuty()
{
	EnterCriticalSection(&m_csFanControl);
	int duty;
	for (int i = 0; i < 2; i++)
	{
		if (m_nCurDuty[i] == m_nSetDuty[i])
			continue;
		duty = int(m_nSetDuty[i] * 255.0 / 100 + 0.5);
		m_pfnSetFanDuty(i + 1, duty);
		if (i == 1)
			m_pfnSetFanDuty(i + 2, duty);//如果存在第3个风扇
	}
	m_bTakeOverStatus = TRUE;
	LeaveCriticalSection(&m_csFanControl);
}

void CCore::SoftControlDuty()
{
	EnterCriticalSection(&m_csFanControl);
	BOOL bChanged = FALSE;
	for (int i = 0; i < 2; i++)
	{
		if (m_nSoftCurrentDuty[i] != m_nSoftTargetDuty[i])
		{
			bChanged = TRUE;
			if (m_nSoftCurrentDuty[i] < m_nSoftTargetDuty[i])
				m_nSoftCurrentDuty[i]++;
			else
				m_nSoftCurrentDuty[i]--;

			m_nSetDuty[i] = m_nSoftCurrentDuty[i];
		}
	}
	LeaveCriticalSection(&m_csFanControl);

	if (bChanged)
		SetFanDuty();

}