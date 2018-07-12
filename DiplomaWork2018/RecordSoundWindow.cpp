#define NOMINMAX
#define NDEBUG
#include <assert.h>
#include <afxwin.h>
#include <mmsystem.h>
#include <CommDlg.h>
#include <MMSystem.h>
#include<algorithm>
#include <fstream>
#include <limits>
#include <mmreg.h>
#include <cstddef>
#include <vector>
#include "RecordSoundWindow.h"
#include "resource1.h"
#include "afxdlgs.h"
using namespace std;


#define INP_BUFFER_SIZE 620000


PTCHAR szOpenError = TEXT("Error opening waveform audio!");
static LPCWSTR szSaveFileFilter = _T("Sound File (*.wav)|*.wav|All Files (*.*)|*.*");





BEGIN_MESSAGE_MAP(RecordSoundWindow, CWnd)
	ON_WM_PAINT()
	ON_WM_CREATE()
	ON_MESSAGE(MM_WIM_OPEN, OnWimOpen)
	ON_MESSAGE(MM_WIM_DATA, OnWimData)
	ON_MESSAGE(MM_WIM_CLOSE, OnWimClose)
	ON_COMMAND(IDC_BUTTON_RECORD, OnRecordStart)
	ON_COMMAND(IDC_BUTTON_STOP, OnRecordStop)
	ON_WM_DESTROY()
END_MESSAGE_MAP()

RecordSoundWindow::RecordSoundWindow()
{

}
int RecordSoundWindow::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CWnd::OnCreate(lpCreateStruct) == -1)
		return -1;
	CWnd::OnCreate(lpCreateStruct);
	CRect clientArea;
	GetClientRect(clientArea);
	CRect rectButtonRecord(39, 239, 189, 269);
	m_wndPushButtonRecord.Create(L"Record", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectButtonRecord, this, IDC_BUTTON_RECORD);
	CRect rectButtonStop(210, 239, 360, 269);
	m_wndPushButtonStop.Create(L"Stop Recording", WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON, rectButtonStop, this, IDC_BUTTON_STOP);

	m_wndPushButtonStop.EnableWindow(FALSE);

	sound.pWaveHdr1 = (PWAVEHDR)malloc(sizeof(WAVEHDR));

	sound.pWaveHdr2 = (PWAVEHDR)malloc(sizeof(WAVEHDR));

	sound.pBuffer1 = NULL;
	sound.pBuffer2 = NULL;
	sound.pNewBuffer = NULL;

	// Allocate memory for save buffer
	sound.pSaveBuffer = (PBYTE)malloc(1);

	ModifyStyle(WS_MAXIMIZEBOX, 0);
	return 0;
}

BOOL RecordSoundWindow::PreCreateWindow(CREATESTRUCT & cs)
{
	if (!CWnd::PreCreateWindow(cs))
		return FALSE;
	cs.dwExStyle &= ~WS_EX_CLIENTEDGE;
	cs.style &= (0xFFFFFFFF ^ WS_SIZEBOX);
	cs.style |= WS_BORDER;
	cs.style &= (0xFFFFFFFF ^ WS_MAXIMIZEBOX);
	cs.lpszClass = AfxRegisterWndClass(0);
	return TRUE;
}

LONG RecordSoundWindow::OnWimOpen(UINT wParam, LONG lParam)
{
	sound.pSaveBuffer = (PBYTE)realloc(sound.pSaveBuffer, 1);
	waveInAddBuffer(sound.hWaveIn, sound.pWaveHdr1, sizeof(WAVEHDR));
	waveInAddBuffer(sound.hWaveIn, sound.pWaveHdr2, sizeof(WAVEHDR));
	sound.bEnding = FALSE;
	sound.dwDataLength = 0;
	waveInStart(sound.hWaveIn);
	return TRUE;
}

LONG RecordSoundWindow::OnWimData(UINT wParam, LONG lParam)
{
	sound.pNewBuffer = (PBYTE)realloc(sound.pSaveBuffer, sound.dwDataLength + ((PWAVEHDR)lParam)->dwBytesRecorded);
	if (sound.pNewBuffer == NULL) {
		waveInClose(sound.hWaveIn);
		MessageBeep(MB_ICONEXCLAMATION);
		AfxMessageBox(L"Error Allocating Memory!", MB_OK, 0L);
		return TRUE;
	}
	sound.pSaveBuffer = sound.pNewBuffer;
	// тук става прехвърлянето на данните в pSaveBuffer
	CopyMemory(sound.pSaveBuffer + sound.dwDataLength, ((PWAVEHDR)lParam)->lpData, ((PWAVEHDR)lParam)->dwBytesRecorded);
	sound.dwDataLength += ((PWAVEHDR)lParam)->dwBytesRecorded;
	if (sound.bEnding) {
		waveInClose(sound.hWaveIn);
		return TRUE;
	}
	waveInAddBuffer(sound.hWaveIn, (PWAVEHDR)lParam, sizeof(WAVEHDR));
	return TRUE;
}

LONG RecordSoundWindow::OnWimClose(UINT wParam, LONG lParam)
{
	waveInUnprepareHeader(sound.hWaveIn, sound.pWaveHdr1, sizeof(WAVEHDR));
	waveInUnprepareHeader(sound.hWaveIn, sound.pWaveHdr2, sizeof(WAVEHDR));
	free(sound.pBuffer1);
	sound.pBuffer1 = NULL;
	free(sound.pBuffer2);
	sound.pBuffer2 = NULL;

	return TRUE;
}

void RecordSoundWindow::OnPaint()
{
	CPaintDC dc(this);
	CRect rect;
	if (dc)
	{
		rect.top = 0;
		rect.left = 0;
		rect.bottom = 185;
		rect.right = 425;
		FillRect(dc, &rect, (HBRUSH)(COLOR_WINDOW + 2));
		if (sound.bEnding == TRUE)
		{
			hPen = CreatePen(PS_SOLID, 1, RGB(0, 200, 0));

			SelectObject(dc, hPen);
			SetMapMode(dc, MM_ISOTROPIC);

			SetWindowExtEx(dc, 400, 300, NULL);

			SetViewportExtEx(dc, 200, 180, NULL);

			SetViewportOrgEx(dc, 0, 0, NULL);
			int sample = 0;
			int i = 1;
			int num = 60000;
			sample = readSample(i, TRUE);
			// scale the sample
			sound.pt[i].x = i / 40;

			sound.pt[i].y = (int)((sample)*1.5);

			MoveToEx(dc, sound.pt[i].x, sound.pt[i].y, NULL);

			while (i < num && sample != (int)0xefffffff)
			{
				// scale the sample
				sound.pt[i].x = i / 40;

				sound.pt[i].y = (int)((sample)*1.5);

				LineTo(dc, sound.pt[i].x, sound.pt[i].y);

				i++;

				sample = readSample(i, TRUE);

			}
		}
		DeleteObject(hPen);
	}
}

void RecordSoundWindow::OnRecordStart()
{
	m_wndPushButtonRecord.EnableWindow(FALSE);
	m_wndPushButtonStop.EnableWindow(TRUE);


	sound.pBuffer1 = reinterpret_cast <PBYTE> (malloc(INP_BUFFER_SIZE));
	sound.pBuffer2 = reinterpret_cast <PBYTE> (malloc(INP_BUFFER_SIZE));

	if (!sound.pBuffer1 || !sound.pBuffer2)
	{
		if (sound.pBuffer1)
		{
			free(sound.pBuffer1);
			sound.pBuffer1 = NULL;
		}
		if (sound.pBuffer2)
		{
			free(sound.pBuffer2);
			sound.pBuffer1 = NULL;
		}
		AfxMessageBox(L"Error Allocating Memory!", MB_OK, 0L);
		return;

	}
	// Open waveform audio for input 11 kHz 8-bit mono
	sound.waveform.wFormatTag = WAVE_FORMAT_PCM;
	sound.waveform.nChannels = 1;
	sound.waveform.nSamplesPerSec = 11025L;
	sound.waveform.nAvgBytesPerSec = 11025L;
	sound.waveform.nBlockAlign = 1;
	sound.waveform.wBitsPerSample = 8;
	sound.waveform.cbSize = 0;
	if (waveInOpen(&sound.hWaveIn, WAVE_MAPPER, &sound.waveform, (DWORD)this->GetSafeHwnd(), 0, CALLBACK_WINDOW))
	{
		AfxMessageBox(szOpenError, MB_OK, 0);
		free(sound.pBuffer1);
		sound.pBuffer1 = NULL;
		free(sound.pBuffer2);
		sound.pBuffer1 = NULL;
	}
	// Set up headers and prepare them
	sound.pWaveHdr1->lpData = (LPSTR)sound.pBuffer1;
	sound.pWaveHdr1->dwBufferLength = INP_BUFFER_SIZE;
	sound.pWaveHdr1->dwBytesRecorded = 0;
	sound.pWaveHdr1->dwUser = 0;
	sound.pWaveHdr1->dwFlags = 0;
	sound.pWaveHdr1->dwLoops = 1;
	sound.pWaveHdr1->lpNext = NULL;
	sound.pWaveHdr1->reserved = 0;

	waveInPrepareHeader(sound.hWaveIn, sound.pWaveHdr1, sizeof(WAVEHDR));

	sound.pWaveHdr2->lpData = (LPSTR)sound.pBuffer2;
	sound.pWaveHdr2->dwBufferLength = INP_BUFFER_SIZE;
	sound.pWaveHdr2->dwBytesRecorded = 0;
	sound.pWaveHdr2->dwUser = 0;
	sound.pWaveHdr2->dwFlags = 0;
	sound.pWaveHdr2->dwLoops = 1;
	sound.pWaveHdr2->lpNext = NULL;
	sound.pWaveHdr2->reserved = 0;

	waveInPrepareHeader(sound.hWaveIn, sound.pWaveHdr2, sizeof(WAVEHDR));
}

void RecordSoundWindow::OnRecordStop()
{
	waveInReset(sound.hWaveIn);
	_fcloseall();
	CFileDialog fileDlg(FALSE, _T("wav"), NULL, OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, szSaveFileFilter, this, 0, TRUE);
	
	m_wndPushButtonRecord.EnableWindow(TRUE);
	m_wndPushButtonStop.EnableWindow(FALSE);

	//Pop-up save sound dialog
	if (fileDlg.DoModal() == IDOK)
	{
		sound.pathName = fileDlg.GetPathName();
		SaveWavFile(sound.pathName, sound.pWaveHdr1);

		Wav();
		this->Invalidate(TRUE);
		//this->UpdateWindow();
	}
	sound.bEnding = TRUE;

}

void RecordSoundWindow::OnDestroy()
{
	_fcloseall();
	if (sound.pSaveBuffer != NULL)
		free(sound.pSaveBuffer);
	if (sound.pBuffer1 != NULL)
		free(sound.pBuffer1);
	if (sound.pBuffer2 != NULL)
		free(sound.pBuffer2);
	free(sound.pWaveHdr1);
	free(sound.pWaveHdr2);
}

void RecordSoundWindow::SaveWavFile(CString pathAndFileName, PWAVEHDR WaveHeader)
{
	fstream myFile(pathAndFileName, fstream::out | fstream::binary);
	int chunksize, pcmsize, NumSamples, subchunk1size;
	int audioFormat = 1;
	int numChannels = 1;
	int bitsPerSample = 8;

	NumSamples = ((long)(11025*10 / sound.sampleRate) * 1000);
	pcmsize = sizeof(PCMWAVEFORMAT);

	subchunk1size = 16;
	int byteRate = sound.sampleRate * numChannels*bitsPerSample / 8;
	int blockAlign = numChannels * bitsPerSample / 8;
	int subchunk2size = WaveHeader->dwBufferLength*numChannels;
	chunksize = (36 + subchunk2size);
	// write the wav file per the wav file format
	myFile.seekp(0, ios::beg);
	myFile.write("RIFF", 4);                   // chunk id
	myFile.write((char*)&chunksize, 4);               // chunk size (36 + SubChunk2Size))
	myFile.write("WAVE", 4);                   // format
	myFile.write("fmt ", 4);                   // subchunk1ID
	myFile.write((char*)&subchunk1size, 4);           // subchunk1size (16 for PCM)
	myFile.write((char*)&audioFormat, 2);         // AudioFormat (1 for PCM)
	myFile.write((char*)&numChannels, 2);         // NumChannels
	myFile.write((char*)&sound.sampleRate, 4);          // sample rate
	myFile.write((char*)&byteRate, 4);            // byte rate (SampleRate * NumChannels * BitsPerSample/8)
	myFile.write((char*)&blockAlign, 2);          // block align (NumChannels * BitsPerSample/8)
	myFile.write((char*)&bitsPerSample, 2);           // bits per sample
	myFile.write("data", 4);                   // subchunk2ID
	myFile.write((char*)&subchunk2size, 4);           // subchunk2size (NumSamples * NumChannels * BitsPerSample/8)
													  //char* byte =reinterpret_cast<char*>(WaveHeader->lpData);
	myFile.write((char*)sound.pSaveBuffer, sound.dwDataLength); //actual sound data

	myFile.close();
}

int RecordSoundWindow::readSample(int number, bool leftchannel)
{
	/*
	Reads sample number, returns it as an int, if
	this.mono==false we look at the leftchannel bool
	to determine which to return.
	number is in the range [0,length/byte_samp]
	returns 0xefffffff on failure
	*/
	if (number >= 0 && number<sound.length / sound.byte_samp)
	{
		// go to beginning of the file
		rewind(sound.stream);
		// we start reading at sample_number * sample_size + header length

		int offset = number * 1 + 44;

		// unless this is a stereo file and the rightchannel is requested.
		if (!sound.mono && !leftchannel)
		{
			offset += sound.byte_samp / 2;
		}
		// read this many bytes;
		int amount;
		amount = sound.byte_samp;

		fseek(sound.stream, offset, SEEK_CUR);
		short sample = 0;
		fread((void *)&sample, 1, amount, sound.stream);
		return sample;
	}
	else
	{
		// return 0xefffffff if failed
		return (int)0xefffffff;
	}
}
// Read the wav file and get needed information
void RecordSoundWindow::Wav()
{
	// open filepointer readonly

	int s_rate = 11025,byte_sec,bit_samp;
	CT2A ascii(sound.pathName);
	fopen_s(&sound.stream, ascii.m_psz, "r");
	if (sound.stream == NULL)
	{
		AfxMessageBox(L"Could not open file !", MB_OK, 0L);
	}
	else
	{
		// declare a char buff to store some values in
		char *buff = new char[5];
		buff[4] = '\0';
		// read the first 4 bytes
		fread((void *)buff, 1, 4, sound.stream);
		// the first four bytes should be 'RIFF'
		if (strcmp((char *)buff, "RIFF") == 0)
		{
			// read byte 8,9,10 and 11
			fseek(sound.stream, 4, SEEK_CUR);
			fread((void *)buff, 1, 4, sound.stream);
			// this should read "WAVE"
			if (strcmp((char *)buff, "WAVE") == 0)
			{
				// read byte 12,13,14,15
				fread((void *)buff, 1, 4, sound.stream);
				// this should read "fmt "
				if (strcmp((char *)buff, "fmt ") == 0)
				{
					fseek(sound.stream, 20, SEEK_CUR);
					// final one read byte 36,37,38,39
					fread((void *)buff, 1, 4, sound.stream);
					if (strcmp((char *)buff, "data") == 0)
					{
						// Now we know it is a wav file, rewind the stream
						rewind(sound.stream);
						// now is it mono or stereo ?
						fseek(sound.stream, 22, SEEK_CUR);
						fread((void *)buff, 1, 2, sound.stream);
						if (buff[0] == 0x02)
						{
							sound.mono = false;
						}
						else
						{
							sound.mono = true;
						}
						// read the sample rate
						fread((void *)&s_rate, 1, 4, sound.stream);
						fread((void *)&byte_sec, 1, 4, sound.stream);
						sound.byte_samp = 0;
						fread((void *)&sound.byte_samp, 1, 2, sound.stream);
						bit_samp = 0;
						fread((void *)&bit_samp, 1, 2, sound.stream);
						fseek(sound.stream, 4, SEEK_CUR);
						fread((void *)&sound.length, 1, 4, sound.stream);
					}
				}
			}
		}
		delete buff;
	}
}