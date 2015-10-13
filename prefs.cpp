/*
    Copyright (C) 2014 kaytat

    Originally derived from the example posted here:
        http://blogs.msdn.com/b/matthew_van_eerde/archive/2014/11/05/draining-the-wasapi-capture-buffer-fully.aspx

    Since there is no explict license, the Microsoft LPL applies:
        http://msdn.microsoft.com/en-us/cc300389.aspx
*/

// prefs.cpp

#include <windows.h>
#include <mmsystem.h>
#include <stdio.h>
#include <mmdeviceapi.h>
#include <functiondiscoverykeys_devpkey.h>

#include "prefs.h"

void usage(LPCWSTR exe);
HRESULT get_default_device(IMMDevice **ppMMDevice);
HRESULT list_devices();
HRESULT get_specific_device(LPCWSTR szLongName, IMMDevice **ppMMDevice);

void usage(LPCWSTR exe) {
    printf(
        "%ls -?\n"
        "%ls --list-devices\n"
        "%ls [--device \"Device long name\"] [--mono] [--div divisor]\n"
        "\n"
        "    -? prints this message.\n"
        "    --list-devices displays the long names of all active playback devices.\n"
        "    --device captures from the specified device (default if omitted)\n"
        "    --mono convert from stereo to mono\n",
        "    --div divisor reduce sample rate by a factor of divisor\n",
        exe, exe, exe
    );
}

CPrefs::CPrefs(int argc, LPCWSTR argv[], HRESULT &hr)
: m_pMMDevice(NULL)
, m_bInt16(true)
, m_bMono(false)
, m_iSampleRateDivisor(1)
, m_pwfx(NULL)
{
    switch (argc) {
        case 2:
            if (0 == _wcsicmp(argv[1], L"-?") || 0 == _wcsicmp(argv[1], L"/?")) {
                // print usage but don't actually capture
                hr = S_FALSE;
                usage(argv[0]);
                return;
            } else if (0 == _wcsicmp(argv[1], L"--list-devices")) {
                // list the devices but don't actually capture
                hr = list_devices();

                // don't actually play
                if (S_OK == hr) {
                    hr = S_FALSE;
                    return;
                }
            }
        // intentional fallthrough
        
        default:
            // loop through arguments and parse them
            for (int i = 1; i < argc; i++) {
                
                // --device
                if (0 == _wcsicmp(argv[i], L"--device")) {
                    if (NULL != m_pMMDevice) {
                        printf("Only one --device switch is allowed\n");
                        hr = E_INVALIDARG;
                        return;
                    }

                    if (i++ == argc) {
                        printf("--device switch requires an argument\n");
                        hr = E_INVALIDARG;
                        return;
                    }

                    hr = get_specific_device(argv[i], &m_pMMDevice);
                    if (FAILED(hr)) {
                        return;
                    }

                    continue;
                }

                // --int-16
                if (0 == _wcsicmp(argv[i], L"--int-16")) {
                    if (m_bInt16) {
                        printf("Only one --int-16 switch is allowed\n");
                        hr = E_INVALIDARG;
                        return;
                    }

                    m_bInt16 = true;
                    continue;
                }

                // --mono
                if (0 == _wcsicmp(argv[i], L"--mono")) {
                    m_bMono = true;
                    continue;
                }

                // --div divisor
                if (0 == _wcsicmp(argv[i], L"--div")) {
                    if (i++ == argc) {
                        printf("--div switch requires an argument\n");
                        hr = E_INVALIDARG;
                        return;
                    }

                    m_iSampleRateDivisor = _wtoi(argv[i]);
                    continue;
                }

                printf("Invalid argument %ls\n", argv[i]);
                hr = E_INVALIDARG;
                return;
            }

            // open default device if not specified
            if (NULL == m_pMMDevice) {
                hr = get_default_device(&m_pMMDevice);
                if (FAILED(hr)) {
                    return;
                }
            }
    }
}

CPrefs::~CPrefs() {
    if (NULL != m_pMMDevice) {
        m_pMMDevice->Release();
    }

    if (NULL != m_pwfx) {
        CoTaskMemFree(m_pwfx);
    }
}

HRESULT get_default_device(IMMDevice **ppMMDevice) {
    HRESULT hr = S_OK;
    IMMDeviceEnumerator *pMMDeviceEnumerator;

    // activate a device enumerator
    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator
    );
    if (FAILED(hr)) {
        printf("CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x\n", hr);
        return hr;
    }

    // get the default render endpoint
    hr = pMMDeviceEnumerator->GetDefaultAudioEndpoint(eRender, eConsole, ppMMDevice);
    pMMDeviceEnumerator->Release();
    if (FAILED(hr)) {
        printf("IMMDeviceEnumerator::GetDefaultAudioEndpoint failed: hr = 0x%08x\n", hr);
        return hr;
    }

    return S_OK;
}

HRESULT list_devices() {
    HRESULT hr = S_OK;

    // get an enumerator
    IMMDeviceEnumerator *pMMDeviceEnumerator;

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator
    );
    if (FAILED(hr)) {
        printf("CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x\n", hr);
        return hr;
    }

    IMMDeviceCollection *pMMDeviceCollection;

    // get all the active render endpoints
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(
        eRender, DEVICE_STATE_ACTIVE, &pMMDeviceCollection
    );
    pMMDeviceEnumerator->Release();
    if (FAILED(hr)) {
        printf("IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x\n", hr);
        return hr;
    }

    UINT count;
    hr = pMMDeviceCollection->GetCount(&count);
    if (FAILED(hr)) {
        pMMDeviceCollection->Release();
        printf("IMMDeviceCollection::GetCount failed: hr = 0x%08x\n", hr);
        return hr;
    }
    printf("Active render endpoints found: %u\n", count);

    for (UINT i = 0; i < count; i++) {
        IMMDevice *pMMDevice;

        // get the "n"th device
        hr = pMMDeviceCollection->Item(i, &pMMDevice);
        if (FAILED(hr)) {
            pMMDeviceCollection->Release();
            printf("IMMDeviceCollection::Item failed: hr = 0x%08x\n", hr);
            return hr;
        }

        // open the property store on that device
        IPropertyStore *pPropertyStore;
        hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
        pMMDevice->Release();
        if (FAILED(hr)) {
            pMMDeviceCollection->Release();
            printf("IMMDevice::OpenPropertyStore failed: hr = 0x%08x\n", hr);
            return hr;
        }

        // get the long name property
        PROPVARIANT pv; PropVariantInit(&pv);
        hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
        pPropertyStore->Release();
        if (FAILED(hr)) {
            pMMDeviceCollection->Release();
            printf("IPropertyStore::GetValue failed: hr = 0x%08x\n", hr);
            return hr;
        }

        if (VT_LPWSTR != pv.vt) {
            printf("PKEY_Device_FriendlyName variant type is %u - expected VT_LPWSTR", pv.vt);

            PropVariantClear(&pv);
            pMMDeviceCollection->Release();
            return E_UNEXPECTED;
        }

        printf("    %ls\n", pv.pwszVal);
        
        PropVariantClear(&pv);
    }    
    pMMDeviceCollection->Release();
    
    return S_OK;
}

HRESULT get_specific_device(LPCWSTR szLongName, IMMDevice **ppMMDevice) {
    HRESULT hr = S_OK;

    *ppMMDevice = NULL;
    
    // get an enumerator
    IMMDeviceEnumerator *pMMDeviceEnumerator;

    hr = CoCreateInstance(
        __uuidof(MMDeviceEnumerator), NULL, CLSCTX_ALL, 
        __uuidof(IMMDeviceEnumerator),
        (void**)&pMMDeviceEnumerator
    );
    if (FAILED(hr)) {
        printf("CoCreateInstance(IMMDeviceEnumerator) failed: hr = 0x%08x\n", hr);
        return hr;
    }

    IMMDeviceCollection *pMMDeviceCollection;

    // get all the active render endpoints
    hr = pMMDeviceEnumerator->EnumAudioEndpoints(
        eRender, DEVICE_STATE_ACTIVE, &pMMDeviceCollection
    );
    pMMDeviceEnumerator->Release();
    if (FAILED(hr)) {
        printf("IMMDeviceEnumerator::EnumAudioEndpoints failed: hr = 0x%08x\n", hr);
        return hr;
    }

    UINT count;
    hr = pMMDeviceCollection->GetCount(&count);
    if (FAILED(hr)) {
        pMMDeviceCollection->Release();
        printf("IMMDeviceCollection::GetCount failed: hr = 0x%08x\n", hr);
        return hr;
    }

    for (UINT i = 0; i < count; i++) {
        IMMDevice *pMMDevice;

        // get the "n"th device
        hr = pMMDeviceCollection->Item(i, &pMMDevice);
        if (FAILED(hr)) {
            pMMDeviceCollection->Release();
            printf("IMMDeviceCollection::Item failed: hr = 0x%08x\n", hr);
            return hr;
        }

        // open the property store on that device
        IPropertyStore *pPropertyStore;
        hr = pMMDevice->OpenPropertyStore(STGM_READ, &pPropertyStore);
        if (FAILED(hr)) {
            pMMDevice->Release();
            pMMDeviceCollection->Release();
            printf("IMMDevice::OpenPropertyStore failed: hr = 0x%08x\n", hr);
            return hr;
        }

        // get the long name property
        PROPVARIANT pv; PropVariantInit(&pv);
        hr = pPropertyStore->GetValue(PKEY_Device_FriendlyName, &pv);
        pPropertyStore->Release();
        if (FAILED(hr)) {
            pMMDevice->Release();
            pMMDeviceCollection->Release();
            printf("IPropertyStore::GetValue failed: hr = 0x%08x\n", hr);
            return hr;
        }

        if (VT_LPWSTR != pv.vt) {
            printf("PKEY_Device_FriendlyName variant type is %u - expected VT_LPWSTR", pv.vt);

            PropVariantClear(&pv);
            pMMDevice->Release();
            pMMDeviceCollection->Release();
            return E_UNEXPECTED;
        }

        // is it a match?
        if (0 == _wcsicmp(pv.pwszVal, szLongName)) {
            // did we already find it?
            if (NULL == *ppMMDevice) {
                *ppMMDevice = pMMDevice;
                pMMDevice->AddRef();
            } else {
                printf("Found (at least) two devices named %ls\n", szLongName);
                PropVariantClear(&pv);
                pMMDevice->Release();
                pMMDeviceCollection->Release();
                return E_UNEXPECTED;
            }
        }
        
        pMMDevice->Release();
        PropVariantClear(&pv);
    }
    pMMDeviceCollection->Release();
    
    if (NULL == *ppMMDevice) {
        printf("Could not find a device named %ls\n", szLongName);
        return HRESULT_FROM_WIN32(ERROR_NOT_FOUND);
    }

    return S_OK;
}
