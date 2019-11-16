#include "stdafx.h"
#include "DS_AudioVideoDevices.h"
#include <sstream>

#pragma comment(lib, "Strmiids.lib")

// ����Ԥ����
BOOL WINAPI Prepare() { return TRUE; }

HRESULT WINAPI DS_GetAudioVideoInputDevices(OUT std::vector<TDeviceName> &devices, REFGUID guid)
{
	devices.clear(); // ��ʼ��

	// ��ʼ��COM
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{	// ����ϵͳ�豸ö����ʵ��
		ICreateDevEnum *pSysDevEnum = NULL;
		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
		if (SUCCEEDED(hr))
		{	// ��ȡ�豸��ö����
			IEnumMoniker *pEnumCat = NULL;
			hr = pSysDevEnum->CreateClassEnumerator(guid, &pEnumCat, 0);
			if (hr == S_OK)
			{
				LPOLESTR pOleDisplayName = reinterpret_cast<LPOLESTR>(CoTaskMemAlloc(MAX_MONIKER_NAME_LENGTH * 2));
				if (pOleDisplayName == NULL)
				{
					hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
				}
				else
				{	// ö���豸����
					TDeviceName name;
					IMoniker *pMoniker = NULL;
					ULONG cFetched;
					while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
					{
						IPropertyBag *pPropBag;
						hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void **)&pPropBag);
						if (SUCCEEDED(hr))
						{
							VARIANT varName;
							VariantInit(&varName);
							hr = pPropBag->Read(L"FriendlyName", &varName, NULL); // ��ȡ�豸�Ѻ���
							if (SUCCEEDED(hr))
							{
								hr = pMoniker->GetDisplayName(NULL, NULL, &pOleDisplayName); // ��ȡ�豸Moniker��
								if (SUCCEEDED(hr))
								{
									StringCchCopy(name.Name, MAX_FRIENDLY_NAME_LENGTH, varName.bstrVal);
									StringCchCopy(name.Moniker, MAX_MONIKER_NAME_LENGTH, pOleDisplayName);
									devices.push_back(name);
								}
							}

							VariantClear(&varName);
							pPropBag->Release();
						}
						pMoniker->Release();
					} // End for While
					CoTaskMemFree(pOleDisplayName);
				}
				pEnumCat->Release();
			}
			pSysDevEnum->Release();
		}
		CoUninitialize();
	}

	return hr;
}

HRESULT WINAPI DS_GetDeviceSources(OUT TCHAR* &devices, OUT int &cch, BOOL video)
{
	// ��ʼ��COM
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (SUCCEEDED(hr))
	{	// ����ϵͳ�豸ö����ʵ��
		ICreateDevEnum *pSysDevEnum = NULL;
		hr = CoCreateInstance(CLSID_SystemDeviceEnum, NULL, CLSCTX_INPROC_SERVER, IID_ICreateDevEnum, (void **)&pSysDevEnum);
		if (SUCCEEDED(hr))
		{	// ��ȡ�豸��ö����
			IEnumMoniker *pEnumCat = NULL;
			if (video)
			{	// ��Ƶ�豸
				hr = pSysDevEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, &pEnumCat, 0);
			}
			else
			{	// ��Ƶ�豸
				hr = pSysDevEnum->CreateClassEnumerator(CLSID_AudioInputDeviceCategory, &pEnumCat, 0);
			}

			if (hr == S_OK)
			{
				LPOLESTR pOleDisplayName = reinterpret_cast<LPOLESTR>(CoTaskMemAlloc(MAX_MONIKER_NAME_LENGTH * 2));
				if (pOleDisplayName == NULL)
				{
					hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
				}
				else
				{	// ö���豸����
					BOOL first = TRUE;
					std::wostringstream os;
					os << L"[";

					IMoniker *pMoniker = NULL;
					ULONG cFetched;
					while (pEnumCat->Next(1, &pMoniker, &cFetched) == S_OK)
					{
						IPropertyBag *pPropBag;
						hr = pMoniker->BindToStorage(NULL, NULL, IID_IPropertyBag, (void **)&pPropBag);
						if (SUCCEEDED(hr))
						{
							VARIANT varName;
							VariantInit(&varName);
							hr = pPropBag->Read(L"FriendlyName", &varName, NULL); // ��ȡ�豸�Ѻ���
							if (SUCCEEDED(hr))
							{
								hr = pMoniker->GetDisplayName(NULL, NULL, &pOleDisplayName); // ��ȡ�豸Moniker��
								if (SUCCEEDED(hr))
								{
									if (first) first = FALSE; else os << L",";
									os << L"{\"Name\":\"" << varName.bstrVal << L"\",\"Moniker\":\"" << pOleDisplayName << L"\"}";
								}
							}
							VariantClear(&varName);
							pPropBag->Release();
						}
						pMoniker->Release();
					} // End for While
					CoTaskMemFree(pOleDisplayName);
					os << L"]";

					std::wstring ws = os.str();
					cch = (int)ws.size();
					devices = (TCHAR*)CoTaskMemAlloc((cch + 1) << 1);
					if (devices == NULL)
					{
						hr = HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);
					}
					else
					{
						wcscpy_s(devices, cch + 1, ws.c_str());
					}
				}
				pEnumCat->Release();
			}
			pSysDevEnum->Release();
		}
		CoUninitialize();
	}

	return hr;
}

VOID WINAPI FreeMemory(LPVOID p)
{
	if (p != NULL) CoTaskMemFree(p);
}
