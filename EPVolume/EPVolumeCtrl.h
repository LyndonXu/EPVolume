#pragma once

#include <windows.h>
#include <commctrl.h>
#include <mmdeviceapi.h>
#include <endpointvolume.h>
#include <Functiondiscoverykeys_devpkey.h>


enum
{
	_DEV_Audio_Type_IN,
	_DEV_Audio_Type_OUT,

	_DEV_Audio_Type_Reserved
};

enum
{
	_Volume_Ctrl_Type_Mute,
	_Volume_Ctrl_Type_Volume,

	_Volume_Ctrl_Type_Reserved
};

static const UINT s_c_u32MaxVol = 100;

class CMyAudioEndpointVolumeCallback : public IAudioEndpointVolumeCallback
{
private:
	LONG m_lRef;
	HWND m_hWnd;
	UINT m_u32MsgNumber;
public:
	CMyAudioEndpointVolumeCallback()
		: m_lRef(1)
		, m_hWnd(NULL)
		, m_u32MsgNumber(0)
	{
	}

	~CMyAudioEndpointVolumeCallback()
	{
	}

	// IUnknown methods -- AddRef, Release, and QueryInterface

	ULONG STDMETHODCALLTYPE AddRef()
	{
		return InterlockedIncrement(&m_lRef);
	}

	ULONG STDMETHODCALLTYPE Release()
	{
		ULONG ulRef = InterlockedDecrement(&m_lRef);
		if (0 == ulRef)
		{
			delete this;
		}
		return ulRef;

	}

	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, VOID **ppvInterface)
	{
		if (IID_IUnknown == riid)
		{
			AddRef();
			*ppvInterface = (IUnknown*)this;
		}
		else if (__uuidof(IAudioEndpointVolumeCallback) == riid)
		{
			AddRef();
			*ppvInterface = (IAudioEndpointVolumeCallback*)this;
		}
		else
		{
			*ppvInterface = NULL;
			return E_NOINTERFACE;
		}
		return S_OK;
	}
	HRESULT STDMETHODCALLTYPE OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify);
	HRESULT SetHWNDAndMsg(HWND hWnd, UINT u32Msg);
};


class CVolumeCtrl
{
public:
	CVolumeCtrl();
	~CVolumeCtrl();
	INT32 StartVolumeCtrl(IMMDeviceEnumerator *pEnumerator, const WCHAR *pDevID, 
		HWND hWnd, UINT u32MsgNumber);
	INT32 EndVolumeCtrl(void);
	INT32 SetVolume(INT32 s32Volume);
	INT32 SetMute(BOOL boIsMute);
	INT32 GetVolume(INT32 *pVolume);
	INT32 GetMute(BOOL *pIsMute);

	const WCHAR *GetDevID(void)
	{
		return m_wcDevID;
	}

private:
	IMMDevice *m_pDevice;
	IAudioEndpointVolume *m_pEndptVol;
	CMyAudioEndpointVolumeCallback *m_pCallback;
	WCHAR m_wcDevID[256];

};

typedef enum _tagEmAudioDeviceType
{
	_Audio_Device_Default,
	_Audio_Device_Normal,
}EmAudioDeviceType;

typedef INT32(*PFUN_EnumAudioDeviceCB)(EmAudioDeviceType emType, const WCHAR *pDevName, const WCHAR *pDevID, void *pContext);


class CEPVolumeCtrl
{
public:
	CEPVolumeCtrl();
	~CEPVolumeCtrl();
	INT32 SetDefaultDeivce(INT32 s32DevType, const WCHAR *pDevName);
	INT32 EnumAudioDevice(INT32 s32DevType, PFUN_EnumAudioDeviceCB pCB, void *pContext);
	INT32 GetVolumeCtrlID(INT32 s32DevType, const WCHAR *pDevName, WCHAR wcDEVID[256]);
	INT32 StartVolumeCtrl(INT32 s32DevType, const WCHAR *pDevName, HWND hWnd, UINT u32MsgNumber);
	INT32 EndVolumeCtrl(INT32 s32DevType);
	INT32 SetVolume(INT32 s32DevType, INT32 s32Volume);
	INT32 SetMute(INT32 s32DevType, BOOL boIsMute);
	INT32 GetVolume(INT32 s32DevType, INT32 *pVolume);
	INT32 GetMute(INT32 s32DevType, BOOL *pIsMute);
	const LPWSTR GetDevID(INT32 s32DevType)
	{
		if (s32DevType == _DEV_Audio_Type_IN)
		{
			return (const LPWSTR)(m_csIn.GetDevID());
		}
		else
		{
			return (const LPWSTR)(m_csOut.GetDevID());
		}
	}

	friend class CMyAudioEndpointVolumeCallback;
	friend class CVolumeCtrl;

private:
	static GUID s_m_stGuidContext;

	IMMDeviceEnumerator *m_pEnumerator;
	CVolumeCtrl m_csIn;
	CVolumeCtrl m_csOut;

};

