#pragma once
#pragma warning(disable : 4996)
#include"stdafx.h"
#include "CApplication.h"
#include "CDocument0.h"
#include "CDocuments.h"
#include "CSelection.h"

#include <sapi.h>
#include <sphelper.h>
#include <vector>
#include <memory>

class SpeechToText
{
public:
	SpeechToText(LPCWSTR soundFilePath = nullptr);
	void setszFilePath(LPCWSTR);
	virtual ~SpeechToText();
	void CALLBACK start_recognition();
	void CloseDocument();
private:
	BOOL InitNewWordDocument();
	void AddSpeechToDocument(LPWSTR arrayWithResults);
	CApplication m_iAppInterface;
	CDocuments m_iDocuments;
	CDocument0 m_iActiveDocument;
	CComPtr<ISpObjectToken> cpRecognizerToken;
	CComPtr<ISpRecognizer> cpRecognizer;
	CComPtr<ISpRecoContext> cpContext;
	CComPtr<ISpStream> cpAudioIn;
	CComPtr<ISpRecoGrammar> cpGrammar;
	HANDLE hExitEvent;
	HANDLE hSpeechNotifyEvent;
	BOOL fContinue;
	DWORD dwMessage;
	CSpEvent spevent;
	LPCWSTR szFilePath;

protected:
	HRESULT hr;

};