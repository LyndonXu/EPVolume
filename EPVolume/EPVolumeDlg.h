
// EPVolumeDlg.h : ͷ�ļ�
//

#pragma once

#include "EPVolumeCtrl.h"
#include "FitComboBox.h"
#include "afxwin.h"
#include <string>
#include <map>
#include "afxbutton.h"
#include "afxcolorbutton.h"
#include "AudioRecord.h"

using namespace std;

typedef map<wstring, wstring> CMapAudioDev;
typedef map<wstring, wstring>::iterator CMapAudioDevIter;

// CEPVolumeDlg �Ի���
class CEPVolumeDlg : public CDialogEx
{
// ����
public:
	CEPVolumeDlg(CWnd* pParent = NULL);	// ��׼���캯��

// �Ի�������
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EPVOLUME_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnHScroll(UINT nSBCode, UINT nPos, CScrollBar* pScrollBar);
	afx_msg void OnBnClickedOk();

	CEPVolumeCtrl *m_pVolumeCtrl;
	afx_msg void OnBnClickedCheckSpeaker();
	afx_msg void OnBnClickedCheckPhone();
	LRESULT MsgSpeaker(WPARAM wParam, LPARAM lParam);
	LRESULT MsgPhone(WPARAM wParam, LPARAM lParam);
	afx_msg void OnDestroy();

private:
	NOTIFYICONDATA m_NID;
	int NotifyInit();
	int NotifyDestory();
	LRESULT OnShowTask(WPARAM wParam, LPARAM lParam);

public:
	afx_msg void OnBnClickedBtnReg();
	CFitComboBox m_csComboDev;
	CMapAudioDev m_csMapAudioOutDev;
	afx_msg void OnCbnSelchangeComboDev();
	CMFCColorButton m_csMFCColorBtn;
	afx_msg void OnBnClickedMfcColorBtn();
	COLORREF m_s32ChosenColor;

	CToolTipCtrl m_csToolTips;
	afx_msg BOOL OnToolTipNotify(UINT id, NMHDR *pNMHDR, LRESULT *pResult);

	virtual BOOL PreTranslateMessage(MSG* pMsg);

	LRESULT MsgWaveInOpen(WPARAM wParam, LPARAM lParam);
	LRESULT MsgWaveInClose(WPARAM wParam, LPARAM lParam);
	LRESULT MsgWaveInData(WPARAM wParam, LPARAM lParam);

	CAudioRecord m_csAudioRecord;

	afx_msg void OnBnClickedBtnRecord();
};
