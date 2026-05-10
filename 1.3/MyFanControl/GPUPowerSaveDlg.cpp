#include "stdafx.h"
#include "GPUPowerSaveDlg.h"
#include "Core.h"          // 包含 CGPUInfo
#include "afxdialogex.h"

CGPUPowerSaveDlg::CGPUPowerSaveDlg(CWnd* pParent, CGPUInfo& gpuInfo)
    : CDialogEx(CGPUPowerSaveDlg::IDD, pParent)
    , m_gpuInfo(gpuInfo)
    , m_nBatteryFreq(0)
    , m_nBatteryMem(0)
    , m_nPowerFreq(0)
    , m_nPowerMem(0)
{}

void CGPUPowerSaveDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_BAT_GPU_FREQ, m_nBatteryFreq);
    DDX_Text(pDX, IDC_EDIT_BAT_MEM_OFFSET, m_nBatteryMem);
    DDX_Text(pDX, IDC_EDIT_PWR_GPU_FREQ, m_nPowerFreq);
    DDX_Text(pDX, IDC_EDIT_PWR_MEM_OFFSET, m_nPowerMem);
}

BOOL CGPUPowerSaveDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    m_editBatFreq.SubclassDlgItem(IDC_EDIT_BAT_GPU_FREQ, this);
    m_editBatMem.SubclassDlgItem(IDC_EDIT_BAT_MEM_OFFSET, this);
    m_editPwrFreq.SubclassDlgItem(IDC_EDIT_PWR_GPU_FREQ, this);
    m_editPwrMem.SubclassDlgItem(IDC_EDIT_PWR_MEM_OFFSET, this);

    return TRUE;
}

BOOL CGPUPowerSaveDlg::ValidateFrequency(int freq)
{
    if (freq < 0 || freq > m_gpuInfo.m_nMaxFrequency)
    {
        CString msg;
        msg.Format(_T("GPU 频率必须为 0~%d，0 代表默认频率 %d"),
            m_gpuInfo.m_nMaxFrequency, m_gpuInfo.m_nStandardFrequency);
        AfxMessageBox(msg);
        return FALSE;
    }
    return TRUE;
}

BOOL CGPUPowerSaveDlg::ValidateMemOffset(int offset)
{
    if (offset < m_gpuInfo.m_nMemoryRangeMin || offset > m_gpuInfo.m_nMemoryRangeMax)
    {
        CString msg;
        msg.Format(_T("显存偏移必须为 %d~%d"),
            m_gpuInfo.m_nMemoryRangeMin, m_gpuInfo.m_nMemoryRangeMax);
        AfxMessageBox(msg);
        return FALSE;
    }
    return TRUE;
}

void CGPUPowerSaveDlg::OnOK()
{
    if (!UpdateData(TRUE))
        return;

    if (!ValidateFrequency(m_nBatteryFreq))
        return;
    if (!ValidateFrequency(m_nPowerFreq))
        return;
    if (!ValidateMemOffset(m_nBatteryMem))
        return;
    if (!ValidateMemOffset(m_nPowerMem))
        return;

    CDialogEx::OnOK();
}

BEGIN_MESSAGE_MAP(CGPUPowerSaveDlg, CDialogEx)
END_MESSAGE_MAP()