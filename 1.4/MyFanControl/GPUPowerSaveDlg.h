#pragma once
#include "afxcmn.h"

class CGPUInfo;

class CGPUPowerSaveDlg : public CDialogEx
{
public:
    CGPUPowerSaveDlg(CWnd* pParent, CGPUInfo& gpuInfo);

    enum { IDD = IDD_GPU_POWERSAVE_DLG };

    int m_nBatteryFreq;
    int m_nBatteryMem;
    int m_nPowerFreq;
    int m_nPowerMem;

protected:
    virtual void DoDataExchange(CDataExchange* pDX);
    virtual BOOL OnInitDialog();
    virtual void OnOK();

    CGPUInfo& m_gpuInfo;  // 引用，用于校验范围

    CEdit m_editBatFreq;
    CEdit m_editBatMem;
    CEdit m_editPwrFreq;
    CEdit m_editPwrMem;

    BOOL ValidateFrequency(int freq);
    BOOL ValidateMemOffset(int offset);

    DECLARE_MESSAGE_MAP()
};