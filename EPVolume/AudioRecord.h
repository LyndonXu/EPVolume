#pragma once
#include <string>
#include <Mmsystem.h>

using namespace std;
class CAudioRecord
{
public:
	CAudioRecord();
	~CAudioRecord();

	HWND m_hWnd;
#ifdef UNICODE
	wstring m_csDeviceName;
#else
	string m_csDeviceName;
#endif

	HWAVEIN m_hWaveHandle;
	WAVEFORMATEX m_stFormate;

	static const INT32 c_s_s32BufCount = 8;
	char *m_pRecoderBuf;
	WAVEHDR *m_pWaveHDR;


	INT32 Start(TCHAR *pDeviceName, HWND hWndMsg,
		INT32 s32BitsPerSample = 16, 
		INT32 s32SamplesPerSec = 8000, INT32 s32Channel = 2);
	INT32 Stop();
};

