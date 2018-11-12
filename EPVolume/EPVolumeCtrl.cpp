#include "stdafx.h"
#include "EPVolumeCtrl.h"
#include "PolicyConfig.h"

#pragma comment(lib, "comctl32.lib")   

#define EXIT_ON_ERROR(hr);  \
              if (FAILED(hr)) { goto Exit; }

#define SAFE_RELEASE(punk)  \
              if ((punk) != NULL)  \
                { (punk)->Release(); (punk) = NULL; }

const CLSID CLSID_MMDeviceEnumerator = __uuidof(MMDeviceEnumerator);
const IID IID_IMMDeviceEnumerator = __uuidof(IMMDeviceEnumerator);


// Callback method for endpoint-volume-change notifications.

HRESULT STDMETHODCALLTYPE CMyAudioEndpointVolumeCallback::OnNotify(PAUDIO_VOLUME_NOTIFICATION_DATA pNotify)
{
	if (pNotify == NULL)
	{
		return E_INVALIDARG;
	}
	if (m_hWnd != NULL && pNotify->guidEventContext != CEPVolumeCtrl::s_m_stGuidContext)
	{

		PostMessage(m_hWnd, m_u32MsgNumber, _Volume_Ctrl_Type_Mute, pNotify->bMuted);
		PostMessage(m_hWnd, m_u32MsgNumber, _Volume_Ctrl_Type_Volume,
			LPARAM((UINT32)(s_c_u32MaxVol * pNotify->fMasterVolume + 0.5)));
	}
	return S_OK;
}
HRESULT CMyAudioEndpointVolumeCallback::SetHWNDAndMsg(HWND hWnd, UINT u32Msg)
{
	m_hWnd = hWnd;
	m_u32MsgNumber = u32Msg;

	return S_OK;
}


CVolumeCtrl::CVolumeCtrl()
	: m_pDevice(NULL)
	, m_pEndptVol(NULL)
	, m_pCallback(NULL)

{
	memset(m_wcDevID, 0, 256 * sizeof(WCHAR));
}

CVolumeCtrl::~CVolumeCtrl()
{
	EndVolumeCtrl();
}

INT32 CVolumeCtrl::StartVolumeCtrl(IMMDeviceEnumerator *pEnumerator,
	const WCHAR *pDevID, HWND hWnd, UINT u32MsgNumber)
{
	if (pEnumerator == NULL || pDevID == NULL)
	{
		return -1;
	}
	HRESULT hr = S_OK;

	hr = pEnumerator->GetDevice(pDevID, &m_pDevice);
	EXIT_ON_ERROR(hr);

	hr = m_pDevice->Activate(__uuidof(IAudioEndpointVolume),
		CLSCTX_INPROC_SERVER, NULL, (void**)&m_pEndptVol);
	EXIT_ON_ERROR(hr);

	if (hWnd != NULL)
	{
		if (m_pCallback == NULL)
		{
			m_pCallback = new CMyAudioEndpointVolumeCallback;
		}
		if (m_pCallback != NULL)
		{
			m_pCallback->SetHWNDAndMsg(hWnd, u32MsgNumber);

			hr = m_pEndptVol->RegisterControlChangeNotify(
				(IAudioEndpointVolumeCallback*)m_pCallback);
		}
	}
	EXIT_ON_ERROR(hr);

	wcscpy_s(m_wcDevID, 256, pDevID);

Exit:
	return hr;
}

INT32 CVolumeCtrl::EndVolumeCtrl(void)
{
	if (m_pCallback != NULL && m_pEndptVol != NULL)
	{
		m_pEndptVol->UnregisterControlChangeNotify(
			(IAudioEndpointVolumeCallback*)m_pCallback);
	}
	SAFE_RELEASE(m_pDevice);
	SAFE_RELEASE(m_pEndptVol);

	if (m_pCallback != NULL)
	{
		delete m_pCallback;
		m_pCallback = NULL;
	}

	memset(m_wcDevID, 0, 256 * sizeof(WCHAR));
	return 0;
}

INT32 CVolumeCtrl::SetVolume(INT32 s32Volume)
{
	if (m_pEndptVol != NULL)
	{
		float f32Volume = (float)s32Volume;
		f32Volume = f32Volume / s_c_u32MaxVol;
		return m_pEndptVol->SetMasterVolumeLevelScalar(f32Volume, &(CEPVolumeCtrl::s_m_stGuidContext));
	}

	return -1;
}
INT32 CVolumeCtrl::SetMute(BOOL boIsMute)
{
	if (m_pEndptVol != NULL)
	{
		return m_pEndptVol->SetMute(boIsMute, &(CEPVolumeCtrl::s_m_stGuidContext));
	}
	return -1;

}
INT32 CVolumeCtrl::GetVolume(INT32 *pVolume)
{
	if (m_pEndptVol != NULL && pVolume != NULL)
	{
		float f32Volume = 0;

		m_pEndptVol->GetMasterVolumeLevelScalar(&f32Volume);
		*pVolume = (INT32)(s_c_u32MaxVol * f32Volume + 0.5);
		
		return 0;
	}

	return -1;

}
INT32 CVolumeCtrl::GetMute(BOOL *pIsMute)
{
	if (m_pEndptVol != NULL && pIsMute != NULL)
	{
		return m_pEndptVol->GetMute(pIsMute);
	}
	return -1;

}








GUID CEPVolumeCtrl::s_m_stGuidContext = GUID_NULL;

CEPVolumeCtrl::CEPVolumeCtrl()
	: m_pEnumerator(NULL)
{
	HRESULT hr = S_OK;
	if (s_m_stGuidContext == GUID_NULL)
	{

		CoInitialize(NULL);

		hr = CoCreateGuid(&s_m_stGuidContext);
		EXIT_ON_ERROR(hr);

		InitCommonControls();
	}

	if (m_pEnumerator == NULL)
	{
		//hr = CoCreateInstance(
		//	CLSID_MMDeviceEnumerator, NULL,
		//	CLSCTX_ALL, IID_IMMDeviceEnumerator,
		//	(void**)&m_pEnumerator);
		hr = CoCreateInstance(
			CLSID_MMDeviceEnumerator, NULL,
			CLSCTX_INPROC_SERVER, IID_IMMDeviceEnumerator,
			(void**)&m_pEnumerator);
		EXIT_ON_ERROR(hr);
	}
Exit:
	return;
}


CEPVolumeCtrl::~CEPVolumeCtrl()
{
	SAFE_RELEASE(m_pEnumerator);
}

INT32 CEPVolumeCtrl::SetDefaultDeivce(INT32 s32DevType, const WCHAR *pDevName)
{
	WCHAR wcDevID[256];
	INT32 s32Rslt = GetVolumeCtrlID(s32DevType, pDevName, wcDevID);
	if (s32Rslt != 0)
	{
		return s32Rslt;
	}

	IPolicyConfig *pPolicyConfig;

	HRESULT hr = CoCreateInstance(__uuidof(CPolicyConfigClient),
		NULL, CLSCTX_ALL, __uuidof(IPolicyConfig), (LPVOID *)&pPolicyConfig);
	if (SUCCEEDED(hr))
	{
		hr = pPolicyConfig->SetDefaultEndpoint(wcDevID, eConsole);
		pPolicyConfig->Release();
	}
	return hr;
}


INT32 CEPVolumeCtrl::EnumAudioDevice(INT32 s32DevType, PFUN_EnumAudioDeviceCB pCB, void *pContext)
{
	if (s32DevType >= _DEV_Audio_Type_Reserved)
	{
		return -1;
	}

	if (m_pEnumerator == NULL)
	{
		return -1;
	}

	HRESULT hr = S_OK;
	//IMMDeviceEnumerator *pEnumerator = NULL;
	IMMDeviceCollection *pCollection = NULL;
	IMMDevice *pEndpoint = NULL;
	IPropertyStore *pProps = NULL;
	LPWSTR pwszID = NULL;
	INT ret = 0;
	if (s32DevType == _DEV_Audio_Type_IN)
	{
		hr = m_pEnumerator->GetDefaultAudioEndpoint(eCapture, eConsole, &pEndpoint);
	}
	else
	{
		hr = m_pEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, &pEndpoint);
	}
	if (hr == S_OK)
	{
		hr = pEndpoint->OpenPropertyStore(
			STGM_READ, &pProps);
		EXIT_ON_ERROR(hr);

		PROPVARIANT varName;
		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the endpoint's friendly-name property.
		hr = pProps->GetValue(
			PKEY_Device_FriendlyName, &varName);
		EXIT_ON_ERROR(hr);

		if (pCB != NULL)
		{
			ret = pCB(_Audio_Device_Default, varName.pwszVal, NULL, pContext);
		}

		PropVariantClear(&varName);
		SAFE_RELEASE(pProps);
		SAFE_RELEASE(pEndpoint);
	}

	if (ret != 0)
	{
		goto Exit;
	}

	pEndpoint = NULL;

	if (s32DevType == _DEV_Audio_Type_IN)
	{
		hr = m_pEnumerator->EnumAudioEndpoints(
			eCapture, DEVICE_STATE_ACTIVE,
			&pCollection);
	}
	else
	{
		hr = m_pEnumerator->EnumAudioEndpoints(
			eRender, DEVICE_STATE_ACTIVE,
			&pCollection);
	}
	EXIT_ON_ERROR(hr);

	UINT  count;
	hr = pCollection->GetCount(&count);
	EXIT_ON_ERROR(hr);

	// Each loop prints the name of an endpoint device.
	for (ULONG i = 0; i < count; i++)
	{
		// Get pointer to endpoint number i.
		hr = pCollection->Item(i, &pEndpoint);
		EXIT_ON_ERROR(hr);

		// Get the endpoint ID string.
		hr = pEndpoint->GetId(&pwszID);
		EXIT_ON_ERROR(hr);


		hr = pEndpoint->OpenPropertyStore(
			STGM_READ, &pProps);
		EXIT_ON_ERROR(hr);

		PROPVARIANT varName;
		// Initialize container for property value.
		PropVariantInit(&varName);

		// Get the endpoint's friendly-name property.
		hr = pProps->GetValue(
			PKEY_Device_FriendlyName, &varName);
		EXIT_ON_ERROR(hr);


		if (pCB != NULL)
		{
			ret = pCB(_Audio_Device_Normal, varName.pwszVal, pwszID, pContext);
		}

		CoTaskMemFree(pwszID);
		pwszID = NULL;
		PropVariantClear(&varName);
		SAFE_RELEASE(pProps);
		SAFE_RELEASE(pEndpoint);

		if (ret != 0)
		{
			break;
		}
	}
	//SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pCollection);

Exit:
	CoTaskMemFree(pwszID);
	//SAFE_RELEASE(pEnumerator);
	SAFE_RELEASE(pCollection);
	SAFE_RELEASE(pEndpoint);
	SAFE_RELEASE(pProps);

	return hr;

}


typedef struct _tagStGetVolumeCtrlIDContext
{
	const WCHAR *pDevName;
	WCHAR *pDevID;
}StGetVolumeCtrlIDContext;

INT32 GetVolumeCtrlIDCB(EmAudioDeviceType emType, 
	const WCHAR *pDevName, 
	const WCHAR *pDevID, 
	void *pContext)
{
	if (emType == _Audio_Device_Normal)
	{
		StGetVolumeCtrlIDContext *pCompare = (StGetVolumeCtrlIDContext *)pContext;
		if (wcsstr(pDevName, pCompare->pDevName) != NULL)
		{
			wcscpy_s(pCompare->pDevID, 256, pDevID);
			return 1;
		}
	}

	return 0;
}

INT32 CEPVolumeCtrl::GetVolumeCtrlID(INT32 s32DevType, const WCHAR *pDevName, WCHAR wcDEVID[256])
{
	if (s32DevType >= _DEV_Audio_Type_Reserved ||
		pDevName == NULL || wcDEVID == NULL)
	{
		return -1;
	}

	if (m_pEnumerator == NULL)
	{
		return -1;
	}

	wcDEVID[0] = 0;

	StGetVolumeCtrlIDContext stContext = { pDevName, wcDEVID };
	return EnumAudioDevice(s32DevType, GetVolumeCtrlIDCB, &stContext);
}


INT32 CEPVolumeCtrl::StartVolumeCtrl(INT32 s32DevType, const WCHAR *pDevName, HWND hWnd, UINT u32MsgNumber)
{
	WCHAR wcDevID[256];
	INT32 s32Rslt = GetVolumeCtrlID(s32DevType, pDevName, wcDevID);
	if (s32Rslt != 0)
	{
		return s32Rslt;
	}

	if (s32DevType == _DEV_Audio_Type_IN)
	{
		if (wcscmp(wcDevID, m_csIn.GetDevID()) == 0)
		{
			return 0;
		}
		else
		{
			m_csIn.EndVolumeCtrl();
			return m_csIn.StartVolumeCtrl(m_pEnumerator, wcDevID, hWnd, u32MsgNumber);
		}
	}
	else if (s32DevType == _DEV_Audio_Type_OUT)
	{
		if (wcscmp(wcDevID, m_csOut.GetDevID()) == 0)
		{
			return 0;
		}
		else
		{
			m_csOut.EndVolumeCtrl();
			return m_csOut.StartVolumeCtrl(m_pEnumerator, wcDevID, hWnd, u32MsgNumber);
		}
	}
	return -1;

}

INT32 CEPVolumeCtrl::EndVolumeCtrl(INT32 s32DevType)
{
	if (s32DevType == _DEV_Audio_Type_IN)
	{
		return m_csIn.EndVolumeCtrl();
	}
	else if (s32DevType == _DEV_Audio_Type_OUT)
	{
		return m_csOut.EndVolumeCtrl();
	}
	return -1;
}

INT32 CEPVolumeCtrl::SetVolume(INT32 s32DevType, INT32 s32Volume)
{
	if (s32DevType == _DEV_Audio_Type_IN)
	{
		return m_csIn.SetVolume(s32Volume);
	}
	else if (s32DevType == _DEV_Audio_Type_OUT)
	{
		return m_csOut.SetVolume(s32Volume);
	}
	return -1;
}

INT32 CEPVolumeCtrl::SetMute(INT32 s32DevType, BOOL boIsMute)
{
	if (s32DevType == _DEV_Audio_Type_IN)
	{
		return m_csIn.SetMute(boIsMute);
	}
	else if (s32DevType == _DEV_Audio_Type_OUT)
	{
		return m_csOut.SetMute(boIsMute);
	}
	return -1;
}

INT32 CEPVolumeCtrl::GetVolume(INT32 s32DevType, INT32 *pVolume)
{
	if (s32DevType == _DEV_Audio_Type_IN)
	{
		return m_csIn.GetVolume(pVolume);
	}
	else if (s32DevType == _DEV_Audio_Type_OUT)
	{
		return m_csOut.GetVolume(pVolume);
	}
	return -1;
}

INT32 CEPVolumeCtrl::GetMute(INT32 s32DevType, BOOL *pIsMute)
{
	if (s32DevType == _DEV_Audio_Type_IN)
	{
		return m_csIn.GetMute(pIsMute);
	}
	else if (s32DevType == _DEV_Audio_Type_OUT)
	{
		return m_csOut.GetMute(pIsMute);
	}
	return -1;
}



