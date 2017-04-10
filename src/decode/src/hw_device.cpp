
#if defined(_WIN32) || defined(_WIN64)

#ifndef _PREFAST_
#pragma warning(disable:4068)
#endif

#include <atlbase.h>

#include "hw_device.h"

CD3D9Device::CD3D9Device(void* pD3D9Dev)
{
	m_pIND3DDevice = NULL;
	m_pDeviceManager9 = NULL;

	m_pIND3DDevice = (IDirect3DDevice9Ex *)(pD3D9Dev);

	m_resetToken = 0;
}

CD3D9Device::CD3D9Device()
{
	m_pIND3DDevice = NULL;
	m_pDeviceManager9 = NULL;
	m_resetToken = 0;
}


mfxStatus CD3D9Device::Init(
	mfxHDL /*hWindow*/,
	mfxU16 /*nViews*/,
	mfxU32 nAdapterNum)
{
	mfxStatus sts = MFX_ERR_NONE;

	HRESULT hr = S_OK;

	nAdapterNum = nAdapterNum;

	UINT resetToken = 0;

	hr = DXVA2CreateDirect3DDeviceManager9(&resetToken, &m_pDeviceManager9);
	if (FAILED(hr))
		return MFX_ERR_NULL_PTR;

	hr = m_pDeviceManager9->ResetDevice(m_pIND3DDevice, resetToken);
	if (FAILED(hr))
		return MFX_ERR_UNDEFINED_BEHAVIOR;

	m_resetToken = resetToken;

	return sts;
}

mfxStatus CD3D9Device::Reset()
{
	return MFX_ERR_NONE;
}

void CD3D9Device::Close()
{
	MSDK_SAFE_RELEASE(m_pDeviceManager9);
}

CD3D9Device::~CD3D9Device()
{
	Close();
}

mfxStatus CD3D9Device::GetHandle(mfxHandleType type, mfxHDL *pHdl)
{
	if (MFX_HANDLE_DIRECT3D_DEVICE_MANAGER9 == type && pHdl != NULL)
	{
		*pHdl = m_pDeviceManager9;

		return MFX_ERR_NONE;
	}

	return MFX_ERR_UNSUPPORTED;
}

mfxStatus CD3D9Device::SetHandle(mfxHandleType /*type*/, mfxHDL /*hdl*/)
{
	return MFX_ERR_UNSUPPORTED;
}



#if MFX_D3D11_SUPPORT


CD3D11Device::CD3D11Device()
{
	m_pIND3D11Device = NULL;
}

CD3D11Device::CD3D11Device(ID3D11Device* mDevice)
{
	m_pIND3D11Device = mDevice;
}

CD3D11Device::~CD3D11Device()
{
    Close();
}

mfxStatus CD3D11Device::Init(
    mfxHDL /*hWindow*/,
    mfxU16 /*nViews*/,
    mfxU32 nAdapterNum)
{
    mfxStatus sts = MFX_ERR_NONE;
	nAdapterNum;
    return sts;
}


mfxStatus CD3D11Device::Reset()
{
    return MFX_ERR_NONE;
}


mfxStatus CD3D11Device::GetHandle(mfxHandleType type, mfxHDL *pHdl)
{
    if (MFX_HANDLE_D3D11_DEVICE == type)
    {
        *pHdl = m_pIND3D11Device;
        return MFX_ERR_NONE;
    }
    return MFX_ERR_UNSUPPORTED;
}

mfxStatus CD3D11Device::SetHandle(mfxHandleType /*type*/, mfxHDL /*hdl*/)
{
    return MFX_ERR_UNSUPPORTED;
}


void CD3D11Device::Close()
{
}

#endif // #if MFX_D3D11_SUPPORT
#endif // #if defined(_WIN32) || defined(_WIN64)

IDirect3DDevice9Ex*		SharedDXDevice::GetD3D9Device() { return this->m_pd3dDevice; }

ID3D11Device*			SharedDXDevice::GetD3D11Device() { return m_pd3d11Device; };

SharedDXDevice::SharedDXDevice()
{
	m_pD3d = NULL;
	m_pd3dDevice = NULL;

	m_pd3d11Device = NULL;
	m_pImmediateContext = NULL;

	d3dDebug = NULL;

	m_bDeviceTypeD11 = FALSE;
}

SharedDXDevice::~SharedDXDevice()
{

	if (m_pd3dDevice)
		m_pd3dDevice->Release();

	if (m_pD3d)
		m_pD3d->Release();

#if defined(_DEBUG)
	HRESULT hr;
	if (d3dDebug) hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif

	if (m_pImmediateContext != NULL) {
		m_pImmediateContext->Flush();
		m_pImmediateContext->ClearState();
		m_pImmediateContext->Release();
	}
	m_pImmediateContext = NULL;

	if (m_pd3d11Device) {
		m_pd3d11Device->Release();
#if defined(_DEBUG)
		if (d3dDebug) hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
	}

#if defined(_DEBUG)
	if (d3dDebug) d3dDebug->Release();
#endif
	m_pd3d11Device = NULL;
}

HRESULT SharedDXDevice::CreateDevice(UINT adapterNum, HINSTANCE /*hInstance*/, HWND eHwnd)
{
	D3DPRESENT_PARAMETERS	m_d3dpp;
	ZeroMemory(&m_d3dpp, sizeof(m_d3dpp));
	m_d3dpp.Windowed = TRUE;
	m_d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
	m_d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;
	m_d3dpp.hDeviceWindow = eHwnd;

	Direct3DCreate9Ex(D3D_SDK_VERSION, &m_pD3d);

	HRESULT hr = m_pD3d->CreateDeviceEx(adapterNum,
		D3DDEVTYPE_HAL,
		NULL,
		D3DCREATE_HARDWARE_VERTEXPROCESSING | D3DCREATE_MULTITHREADED,
		&m_d3dpp,
		NULL,
		&m_pd3dDevice);

	hr = m_pd3dDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	hr = m_pd3dDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);

	hr = m_pd3dDevice->ResetEx(&m_d3dpp, NULL);
	if (FAILED(hr)) return hr;

	hr = m_pd3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	if (FAILED(hr)) return hr;

	return S_OK;
}



//
// =========================== DX11 ================================
//

// Notes for DX11 : https://www.opengl.org/registry/specs/NV/DX_interop2.txt
// Create DX11 device
HRESULT SharedDXDevice::CreateDX11device()
{
	HRESULT hr = S_OK;
	D3D_FEATURE_LEVEL	m_featureLevel;
	UINT createDeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
	// If the project is in a debug build, enable the debug layer.
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	static D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1,
		D3D_FEATURE_LEVEL_11_0,
		D3D_FEATURE_LEVEL_10_1,
		D3D_FEATURE_LEVEL_10_0
	};
	UINT numFeatureLevels = ARRAYSIZE(featureLevels);

	IDXGIFactory1 * m_pDXGIFactory;
	hr = CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)(&m_pDXGIFactory));

	if (FAILED(hr))
		return MFX_ERR_DEVICE_FAILED;

	IDXGIAdapter * m_pAdapter = NULL;

	hr = m_pDXGIFactory->EnumAdapters(0/*nAdapterNum*/, &m_pAdapter);
	if (FAILED(hr))
		return MFX_ERR_DEVICE_FAILED;


	hr = D3D11CreateDevice(m_pAdapter,
		D3D_DRIVER_TYPE_UNKNOWN,//D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		createDeviceFlags,
		featureLevels,
		numFeatureLevels,
		D3D11_SDK_VERSION,
		&m_pd3d11Device,
		&m_featureLevel,
		&m_pImmediateContext);

	if (FAILED(hr))
		return MFX_ERR_DEVICE_FAILED;

	// turn on multithreading for the Context
	CComQIPtr<ID3D10Multithread> p_mt(m_pImmediateContext);

	if (p_mt)
		p_mt->SetMultithreadProtected(true);
	else
		return MFX_ERR_DEVICE_FAILED;
#if defined(_DEBUG)
	hr = m_pd3d11Device->QueryInterface(__uuidof(ID3D11Debug), reinterpret_cast<void**>(&d3dDebug));
	//if (d3dDebug) hr = d3dDebug->ReportLiveDeviceObjects(D3D11_RLDO_DETAIL);
#endif
	m_bDeviceTypeD11 = TRUE;
	m_pDXGIFactory->Release();
	m_pAdapter->Release();
	// All OK
	return hr;

} // end CreateDX11device

void SharedDXDevice::D3D11DeviceContextFlush()
{
	m_pImmediateContext->ClearState();
	m_pImmediateContext->Flush();
}

