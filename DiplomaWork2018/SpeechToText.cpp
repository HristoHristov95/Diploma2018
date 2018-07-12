
#include"SpeechToText.h"
using namespace std;

SpeechToText::SpeechToText(LPCWSTR soundFilePath)
{
	hr = S_OK;
	hSpeechNotifyEvent = INVALID_HANDLE_VALUE;
	szFilePath = soundFilePath;
}

void SpeechToText::setszFilePath(LPCWSTR szFilePath)
{
	if (szFilePath != nullptr)
	{
		this->szFilePath = szFilePath;
	}
}

SpeechToText::~SpeechToText()
{
}

void SpeechToText::start_recognition()
{
	if (InitNewWordDocument() == FALSE)
	{
		return;
	}

	CoInitialize(NULL);
	{
		hr = S_OK;

		// Find the best matching installed en-us recognizer.
		cpRecognizerToken;

		if (SUCCEEDED(hr))
		{
			hr = SpFindBestToken(SPCAT_RECOGNIZERS, L"language=409", NULL, &cpRecognizerToken);
		}

		// Create a recognizer and immediately set its state to inactive.
		cpRecognizer;

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer.CoCreateInstance(CLSID_SpInprocRecognizer);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetRecognizer(cpRecognizerToken);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetRecoState(SPRST_INACTIVE);
		}

		// Create a new recognition context from the recognizer.
		cpContext;

		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->CreateRecoContext(&cpContext);
		}

		// Subscribe to the speech recognition event and end stream event.
		if (SUCCEEDED(hr))
		{
			ULONGLONG ullEventInterest = SPFEI(SPEI_RECOGNITION) | SPFEI(SPEI_END_SR_STREAM);
			hr = cpContext->SetInterest(ullEventInterest, ullEventInterest);
		}

		// Establish a Win32 event to signal when speech events are available.
		hSpeechNotifyEvent = INVALID_HANDLE_VALUE;

		if (SUCCEEDED(hr))
		{
			hr = cpContext->SetNotifyWin32Event();
		}

		if (SUCCEEDED(hr))
		{
			hSpeechNotifyEvent = cpContext->GetNotifyEventHandle();

			if (INVALID_HANDLE_VALUE == hSpeechNotifyEvent)
			{
				// Notification handle unsupported.
				hr = E_NOINTERFACE;
			}
		}

		// Initialize an audio object to use the default audio input of the system and set the recognizer to use it.
		//CComPtr<ISpAudio> cpAudioIn; // input audio
		cpAudioIn;
		if (SUCCEEDED(hr) && szFilePath != nullptr)
		{
			hr = SPBindToFile(szFilePath, SPFM_OPEN_READONLY, &cpAudioIn);
			//hr=ISpStream::SetBaseStream()
			//hr = cpAudioIn.CoCreateInstance(CLSID_SpMMAudioIn);
		}

		// This will typically use the microphone input. 
		// Speak a phrase such as "Find restaurants near Madrid".  
		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetInput(cpAudioIn, TRUE);
		}


		// Create a new grammar and load an SRGS grammar from file.
		cpGrammar;

		if (SUCCEEDED(hr))
		{
			hr = cpContext->CreateGrammar(0, &cpGrammar);
		}

		if (SUCCEEDED(hr))
		{
			hr = cpGrammar->LoadCmdFromFile(L"FindServices8.grxml", SPLO_STATIC);
		}

		// Set all top-level rules in the new grammar to the active state.
		if (SUCCEEDED(hr))
		{
			hr = cpGrammar->SetRuleState(NULL, NULL, SPRS_ACTIVE);
		}

		// Set the recognizer state to active to begin recognition.
		if (SUCCEEDED(hr))
		{
			hr = cpRecognizer->SetRecoState(SPRST_ACTIVE_ALWAYS);
		}

		// Establish a separate win32 event to signal event loop exit.
		hExitEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

		// Collect the events listened for to pump the speech event loop.
		HANDLE rghEvents[] = { hSpeechNotifyEvent, hExitEvent };

		// Speech recognition event loop.
		fContinue = TRUE;

		while (fContinue && SUCCEEDED(hr))
		{
			// Wait for either a speech event or an exit event.
			dwMessage = WaitForMultipleObjects(sp_countof(rghEvents), rghEvents, FALSE, INFINITE);

			switch (dwMessage)
			{
				// With the WaitForMultipleObjects call above, WAIT_OBJECT_0 is a speech event from hSpeechNotifyEvent.
			case WAIT_OBJECT_0:
			{
				// Sequentially grab the available speech events from the speech event queue.
				spevent;

				while (S_OK == spevent.GetFrom(cpContext))
				{
					switch (spevent.eEventId)
					{
					case SPEI_RECOGNITION:
					{
						// Retrieve the recognition result and output the text of that result.
						ISpRecoResult* pResult = spevent.RecoResult();

						LPWSTR pszCoMemResultText = NULL;
						hr = pResult->GetText(SP_GETWHOLEPHRASE, SP_GETWHOLEPHRASE, TRUE, &pszCoMemResultText, NULL);

						if (SUCCEEDED(hr))
						{
								AddSpeechToDocument(pszCoMemResultText);
						}

						if (NULL != pszCoMemResultText)
						{
							CoTaskMemFree(pszCoMemResultText);
						}

						break;
					}
					case SPEI_END_SR_STREAM:
					{
						// The stream has ended; signal the exit event if it hasn't been signaled already.
						//wprintf(L"End stream event received!\r\n");
						SetEvent(hExitEvent);
						break;
					}
					}
				}

				break;
			}
			case WAIT_OBJECT_0 + 1:
			{
				// Exit event; discontinue the speech loop.
				fContinue = FALSE;
				break;
			}
			}
		}

	}CoUninitialize();

	CloseDocument();
	return;
}

BOOL SpeechToText::InitNewWordDocument()
{
	if (!m_iAppInterface.CreateDispatch(_T("Word.Application")))
	{
		AfxMessageBox(_T("Cannot open the Word"));
		return FALSE;
	}

	// Set visibility, show / hide
	m_iAppInterface.put_Visible(TRUE);

	// Create application interface with visibility option
	COleVariant ovOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

	// Get Documents
	m_iDocuments = m_iAppInterface.get_Documents();

	// Create a new Document
	m_iActiveDocument = m_iDocuments.Add(ovOptional, ovOptional, ovOptional, ovOptional);

	return TRUE;
}

void SpeechToText::AddSpeechToDocument(LPWSTR result)
{
	CSelection  iSelection = m_iAppInterface.get_Selection();
		// Add text into the document
	iSelection.TypeText(result);

		// Add a paragraph (return & new line)
	iSelection.TypeParagraph();
		
}
void SpeechToText::CloseDocument()
{
	// Save before closing (Prompt the "Save as" dialog if this is a new document)
	//m_iActiveDocument.Save();

	// Create application interface with visibility option
	//COleVariant ovOptional((long)DISP_E_PARAMNOTFOUND, VT_ERROR);

	// Close the active document
	//m_iActiveDocument.Close(ovOptional, ovOptional, ovOptional);

	// Release dispatch for valid re-creating dispatch in Open / New document activities
	m_iAppInterface.ReleaseDispatch();
}
