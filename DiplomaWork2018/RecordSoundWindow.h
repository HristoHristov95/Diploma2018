#pragma once
#include "SoundData.h"

class RecordSoundWindow : public CWnd
{

private:
	HPEN hPen;
	CButton m_wndPushButtonRecord;
	CButton m_wndPushButtonStop;
	void SaveWavFile(CString pathAndFileName, PWAVEHDR WaveHeader);
	int readSample(int number, bool leftchannel);
	void Wav();
public:
	Sound sound;
	RecordSoundWindow();

protected:
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg BOOL PreCreateWindow(CREATESTRUCT& cs);
	afx_msg LONG OnWimOpen(UINT wParam, LONG lParam);
	afx_msg LONG OnWimData(UINT wParam, LONG lParam);
	afx_msg LONG OnWimClose(UINT wParam, LONG lParam);
	afx_msg void OnPaint();
	afx_msg void OnRecordStart();
	afx_msg void OnRecordStop();
	afx_msg void OnDestroy();
	DECLARE_MESSAGE_MAP()
};