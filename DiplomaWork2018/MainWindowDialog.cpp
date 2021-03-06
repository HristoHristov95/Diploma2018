// MainWindowDialog.cpp : implementation file
//
#include <memory>
#include "stdafx.h"
#include "MainWindowDialog.h"
#include "afxdialogex.h"
#include "resource1.h"
#include "afxdlgs.h"



// MainWindowDialog dialog

IMPLEMENT_DYNAMIC(MainWindowDialog, CDialog)
int MainWindowDialog::currentActiveRecordSoundWindow = -1;
static LPCWSTR szFileFilter = _T("Sound Files (*.mp3;*.wma,*.wav)|*.mp3;*.wma;*.wav|All Files (*.*)|*.*");
static CString strPathName;

MainWindowDialog::MainWindowDialog(CWnd* pParent /*=nullptr*/)
	: CDialog(ID_MAIN_DIALOG, pParent)
{
	
}

void MainWindowDialog::OnLoadFile(UINT nID)
{
	CFileDialog dlgFile(TRUE, _T("mp3"), _T(""), OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, szFileFilter, this);
	switch (nID)
	{
	case IDC_RECORD:
		recordSoundWindow.push_back(new RecordSoundWindow());
		currentActiveRecordSoundWindow += 1;
		if (recordSoundWindow[currentActiveRecordSoundWindow]->Create(NULL, L"Record sound", WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MINIMIZEBOX, CRect(32, 64, 462, 410),GetDesktopWindow(),NULL) == 0)
		{
			AfxMessageBox(L"Cannot create recording window !", MB_OK, 0);
		}
		recordSoundWindow[currentActiveRecordSoundWindow]->BringWindowToTop();
		break;
	case IDC_LOAD:
		if (dlgFile.DoModal() == IDOK)
		{
			strPathName = dlgFile.GetPathName();
			recognitionInstance = std::make_unique<SpeechToText>(strPathName);
			recognitionInstance->start_recognition();
		}
		break;
	default:
		break;
	}

}

void MainWindowDialog::OnDestroy()
{
	for (int i = 0; i <= currentActiveRecordSoundWindow; i++)
	{
		delete recordSoundWindow[i];
	}
	recordSoundWindow.erase(recordSoundWindow.begin(), recordSoundWindow.end());
}

MainWindowDialog::~MainWindowDialog()
{
}

void MainWindowDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}

// MainWindowDialog message handlers

BEGIN_MESSAGE_MAP(MainWindowDialog, CDialog)
	ON_COMMAND_RANGE(IDC_RECORD,IDC_LOAD,OnLoadFile)
	ON_WM_DESTROY()
END_MESSAGE_MAP()



