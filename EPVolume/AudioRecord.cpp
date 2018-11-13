#include "stdafx.h"
#include "AudioRecord.h"

#pragma comment(lib, "Winmm.lib")


void PrintLog(const char *pFormat, ...)
{
	char c8log[1024];
	{
		va_list args;
		va_start(args, pFormat);
		vsprintf_s(c8log, 1000, pFormat, args);
		va_end(args);
	}

	OutputDebugStringA(c8log);

}

#define PRINT(x, ...) PrintLog("%s[%d]"##x, __FILE__, __LINE__, ##__VA_ARGS__)


CAudioRecord::CAudioRecord()
	:  m_hWnd(NULL)
#ifdef UNICODE
	, m_csDeviceName(L"")
#else
	, m_csDevice("")
#endif
	, m_hWaveHandle(NULL)
	, m_pRecoderBuf(NULL)
{

}


CAudioRecord::~CAudioRecord()
{
	Stop();

}

void CALLBACK waveInProc(
	HWAVEIN   hwi,
	UINT      uMsg,
	DWORD_PTR dwInstance,
	DWORD_PTR dwParam1,
	DWORD_PTR dwParam2
)
{
	CAudioRecord *pThis = (CAudioRecord *)dwInstance;
	if (pThis->m_hWnd != NULL)
	{
		::PostMessage(pThis->m_hWnd, uMsg, (WPARAM)dwParam1, (WPARAM)dwParam2);
	}
}

INT32 CAudioRecord::Start(TCHAR *pDeviceName, HWND hWndMsg, 
	INT32 s32BitsPerSample/* = 16*/,
	INT32 s32SamplesPerSec/* = 8000*/, INT32 s32Channel/* = 2*/)
{
	if (pDeviceName == NULL || hWndMsg == NULL)
	{
		return -1;
	}
	if (m_hWaveHandle != NULL)
	{
		if (m_csDeviceName == pDeviceName && m_hWnd == hWndMsg)
		{
			return 0;
		}
	}

	Stop();


	m_csDeviceName = pDeviceName;
	m_hWnd = hWndMsg;

	UINT u32DevNum = waveInGetNumDevs();
	INT s32DevMatchedNum = -1;
	WAVEINCAPS stCaps = { 0 };
	MMRESULT  result = MMSYSERR_NOERROR;
	for (UINT i = 0; i < u32DevNum; i++)
	{
		memset(&stCaps, 0, sizeof(WAVEINCAPS));
		result = waveInGetDevCaps(i, &stCaps, sizeof(WAVEINCAPS));
		if (result != MMSYSERR_NOERROR)
		{
			PRINT("waveInGetDevCaps error: %08x\n", result);
			continue;
		}

#ifdef UNICODE
		wstring csDeviceName;
#else
		string csDeviceName;
#endif

		csDeviceName = stCaps.szPname;

		if (csDeviceName.find(m_csDeviceName) != string::npos)
		{
			s32DevMatchedNum = i;
			break;
		}
	}
	if (s32DevMatchedNum < 0)
	{
		Stop();
		return -1;
	}

	memset(&m_stFormate, 0, sizeof(WAVEFORMATEX));

	m_stFormate.wFormatTag = WAVE_FORMAT_PCM;
	m_stFormate.wBitsPerSample = s32BitsPerSample;
	m_stFormate.nSamplesPerSec = s32SamplesPerSec;
	m_stFormate.nChannels = s32Channel;
	m_stFormate.cbSize = 0;
	m_stFormate.nBlockAlign = m_stFormate.nChannels * m_stFormate.wBitsPerSample / 8;
	m_stFormate.nAvgBytesPerSec = m_stFormate.nSamplesPerSec * m_stFormate.nBlockAlign;

	result = waveInOpen(&m_hWaveHandle, s32DevMatchedNum, &m_stFormate,
		(DWORD_PTR)waveInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
	if (result != MMSYSERR_NOERROR)
	{
		PRINT("waveInOpen error: %08x\n", result);
		Stop();
		return -1;
	}

	INT32 s32BufLength = m_stFormate.nAvgBytesPerSec / 40;
	/* 25ms */
	m_pRecoderBuf = (char *)malloc(c_s_s32BufCount * s32BufLength);
	if (m_pRecoderBuf == NULL)
	{
		PRINT("malloc error\n");
		Stop();
		return -1;
	}

	m_pWaveHDR = (WAVEHDR *)calloc(c_s_s32BufCount, sizeof(WAVEHDR));
	if (m_pWaveHDR == NULL)
	{
		PRINT("malloc error\n");
		Stop();
		return -1;
	}

	for (INT32 i = 0; i < c_s_s32BufCount; i++)
	{
		m_pWaveHDR[i].lpData = m_pRecoderBuf + i * s32BufLength;
		m_pWaveHDR[i].dwBufferLength = s32BufLength;
		m_pWaveHDR[i].dwFlags = 0;
		m_pWaveHDR[i].dwUser = (DWORD)this;
		result = waveInPrepareHeader(m_hWaveHandle, m_pWaveHDR + i, sizeof(WAVEHDR));
		if (result != MMSYSERR_NOERROR)
		{
			PRINT("waveInPrepareHeader error: %08x\n", result);
			Stop();
			return -1;
		}
		result = waveInAddBuffer(m_hWaveHandle, m_pWaveHDR + i, sizeof(WAVEHDR));
		if (result != MMSYSERR_NOERROR)
		{
			PRINT("waveInAddBuffer error: %08x\n", result);
			Stop();
			return -1;
		}

	}


	result = waveInStart(m_hWaveHandle);
	if (result != MMSYSERR_NOERROR)
	{
		PRINT("waveInAddBuffer error: %08x\n", result);
		Stop();
		return -1;
	}

	return 0;
}

INT32 CAudioRecord::Stop()
{
	MMRESULT  result = MMSYSERR_NOERROR;
	if (m_hWaveHandle != NULL)
	{
		result = waveInReset(m_hWaveHandle);

		if (result != MMSYSERR_NOERROR)
		{
			PRINT("waveInReset error: %08x\n", result);
		}

		if (m_pWaveHDR != NULL)
		{
			for (INT32 i = 0; i < c_s_s32BufCount; i++)
			{
				result = waveInUnprepareHeader(m_hWaveHandle, m_pWaveHDR + i, sizeof(WAVEHDR));
				if (result != MMSYSERR_NOERROR)
				{
					PRINT("waveInUnprepareHeader error: %08x\n", result);
				}
			}
			free(m_pWaveHDR);
			m_pWaveHDR = NULL;
		}

		result = waveInClose(m_hWaveHandle);
		if (result != MMSYSERR_NOERROR)
		{
			PRINT("waveInClose error: %08x\n", result);
		}

		m_hWaveHandle = NULL;

	}

	if (m_pRecoderBuf != NULL)
	{
		free(m_pRecoderBuf);
		m_pRecoderBuf = NULL;
	}

	memset(&m_stFormate, 0, sizeof(WAVEFORMATEX));
#ifdef UNICODE
	m_csDeviceName = L"";
#else
	m_csDeviceName = "";
#endif
	m_hWnd = NULL;

	return 0;
}

