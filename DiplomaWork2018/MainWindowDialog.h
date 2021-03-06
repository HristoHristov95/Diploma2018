#pragma once


// MainWindowDialog dialog
#include "SpeechToText.h"
#include "RecordSoundWindow.h"
#include <memory>

class MainWindowDialog : public CDialog
{
	DECLARE_DYNAMIC(MainWindowDialog)
private:
	static int currentActiveRecordSoundWindow;

	//Функционалността на превода в текст
	std::unique_ptr<SpeechToText> recognitionInstance;

	//Прозорците за записването на звук
	std::vector<RecordSoundWindow*> recordSoundWindow;

public:
	MainWindowDialog(CWnd* pParent = nullptr);   // standard constructor
	afx_msg void OnLoadFile(UINT nID);
	afx_msg void OnDestroy();
	virtual ~MainWindowDialog();

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = ID_MAIN_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()
};
