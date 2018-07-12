#pragma once
static const int NUMBER_POINTS = 60000;
struct Sound {
	DWORD dwDataLength;
	HWAVEIN hWaveIn;
	PBYTE pBuffer1;
	PBYTE pBuffer2;
	PBYTE pSaveBuffer;
	PBYTE pNewBuffer;
	PWAVEHDR pWaveHdr1, pWaveHdr2;
	BOOL bEnding, mono = TRUE;
	WAVEFORMATEX waveform;
	CString pathName;
	POINT pt[NUMBER_POINTS];
	FILE * stream;
	int byte_samp, length, sampleRate = 11025;
};