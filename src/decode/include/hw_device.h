/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2011 - 2012 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#pragma once

#pragma warning(disable : 4201)

#include "mfxvideo++.h"

#include "shared_defs.h"

#include <windows.h>

#if MFX_D3D11_SUPPORT
#include <d3d11.h>
#include <dxgi1_2.h>
#endif

#include <d3d9.h>
#include <dxva2api.h>
#include <dxva.h>

#define DIRECTX_SAFE_RELEASE(pSurface) \
{\
	if (pSurface)\
	{\
		UINT64 cnt = 0;\
		cnt = ((IUnknown*)pSurface)->AddRef();\
		for (int n = 0; n < cnt-1; n++) ((IUnknown*)pSurface)->Release();\
	}\
}

/// Base class for hw device
class CHWDevice
{
public:
	CHWDevice() {}
    virtual ~CHWDevice(){}

	virtual mfxStatus	Init(mfxHDL hWindow, mfxU16 nViews, mfxU32 nAdapterNum) = 0;
    
    virtual mfxStatus	Reset() = 0;
    
	/// Get handle can be used for MFX session SetHandle calls
    virtual mfxStatus	GetHandle(mfxHandleType type, mfxHDL *pHdl) = 0;
    
	virtual mfxStatus	SetHandle(mfxHandleType type, mfxHDL hdl) = 0;

	virtual void		Close() = 0;
	virtual void*		GetDevice() = 0;
};

#define OVERLAY_BACKBUFFER_FORMAT D3DFMT_X8R8G8B8
#define VIDEO_MAIN_FORMAT D3DFMT_YUY2

/** Direct3D 9 device implementation.
@note Can be initilized for only 1 or two 2 views. Handle to
MFX_HANDLE_GFXS3DCONTROL must be set prior if initializing for 2 views.

@note Device always set D3DPRESENT_PARAMETERS::Windowed to TRUE.
*/
class CD3D9Device : public CHWDevice
{

public:
	CD3D9Device();
	CD3D9Device(void* pD3D9Dev);
	virtual ~CD3D9Device();
	virtual void*		GetDevice() { return m_pIND3DDevice; }

	virtual mfxStatus	Init(mfxHDL hWindow, mfxU16 nViews, mfxU32 nAdapterNum);
	virtual mfxStatus	Reset();
	virtual mfxStatus	GetHandle(mfxHandleType type, mfxHDL *pHdl);
	virtual mfxStatus	SetHandle(mfxHandleType type, mfxHDL hdl);
	virtual void		Close();

protected:
	IDirect3DDevice9Ex*         m_pIND3DDevice;
	IDirect3DDeviceManager9*    m_pDeviceManager9;
	UINT                        m_resetToken;

};

#if MFX_D3D11_SUPPORT
class CD3D11Device : public CHWDevice
{
public:
	CD3D11Device();
	CD3D11Device(ID3D11Device* mDevice);
	virtual ~CD3D11Device();
	virtual void *		GetDevice() { return m_pIND3D11Device; }

	virtual mfxStatus	Init(mfxHDL hWindow, mfxU16 nViews, mfxU32 nAdapterNum);
	virtual mfxStatus	Reset();
	virtual mfxStatus	GetHandle(mfxHandleType type, mfxHDL *pHdl);
	virtual mfxStatus	SetHandle(mfxHandleType type, mfxHDL hdl);
	virtual void		Close();

protected:
	ID3D11Device*                   m_pIND3D11Device;

};
#endif

class SharedDXDevice
{
public:
	SharedDXDevice();
	~SharedDXDevice();

	IDirect3DDevice9Ex*		GetD3D9Device();
	HRESULT					CreateDevice(UINT adapterNum, HINSTANCE hInstance, HWND eHwnd);


	HRESULT					CreateDX11device(); // Create a DX11 device
	ID3D11Device*			GetD3D11Device();
	void					D3D11DeviceContextFlush();

protected:
	IDirect3DDevice9Ex*		m_pd3dDevice;
	IDirect3D9Ex*			m_pD3d;

	ID3D11Device*			m_pd3d11Device;
	ID3D11DeviceContext*	m_pImmediateContext;

	ID3D11Debug *d3dDebug;

	bool					m_bDeviceTypeD11;
};
