#include<afxwin.h>
#include "AppInstance.h"
#include "MainWindowDialog.h"

AppInstance appInstance;

BOOL AppInstance::InitInstance()
{
	MainWindowDialog mainDialog;
	m_pMainWnd = &mainDialog;
	mainDialog.DoModal();
	return TRUE;
}