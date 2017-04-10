/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2008-2012 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#ifndef __D3D_ALLOCATOR_H__
#define __D3D_ALLOCATOR_H__

#include <atlbase.h>
#include <d3d9.h>
#include <dxva2api.h>
#include "base_allocator.h"

#define SPECIAL_FOR_SHARE_GL_VPP_ALLOC		9999
#define SPECIAL_FOR_SHARE_GL_D3D9SURFACE	0x98981212ULL

enum eTypeHandle
{
    DXVA2_PROCESSOR     = 0x00,
    DXVA2_DECODER       = 0x01
};

struct D3DAllocatorParams : mfxAllocatorParams
{
    IDirect3DDeviceManager9 *m_pParaInDeviceManager9;

	D3DAllocatorParams() : m_pParaInDeviceManager9() { m_pParaInDeviceManager9 = NULL; }

};

class D3DFrameAllocator: public BaseFrameAllocator
{
public:
    D3DFrameAllocator();
    virtual ~D3DFrameAllocator();    

    virtual mfxStatus Init(mfxAllocatorParams *pParams);
    virtual mfxStatus Close();

    virtual IDirect3DDeviceManager9* GetDeviceManager() {return m_pIND3DDeviceManager9; };

    virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL *handle);

protected:
    virtual mfxStatus CheckRequestType(mfxFrameAllocRequest *request);
    virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse *response);
    virtual mfxStatus AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);


    CComPtr<IDirect3DDeviceManager9> m_pIND3DDeviceManager9;
    CComPtr<IDirectXVideoDecoderService> m_pDecoderService;
    CComPtr<IDirectXVideoProcessorService> m_pProcessorService;
    HANDLE m_hDecoder;
    HANDLE m_hProcessor;
};

#endif // __D3D_ALLOCATOR_H__


////////////////////////////////////////

#include "shared_defs.h"

#if MFX_D3D11_SUPPORT

#ifndef __D3D11_ALLOCATOR_H__
#define __D3D11_ALLOCATOR_H__

#include "base_allocator.h"
#include <limits>

#include <d3d11.h>
#include <vector>
#include <map>

struct D3D11AllocatorParams : mfxAllocatorParams
{
	ID3D11Device *m_pParaInD3DDevice;

	D3D11AllocatorParams() : m_pParaInD3DDevice(NULL) {};
	~D3D11AllocatorParams() { m_pParaInD3DDevice = NULL; };
};

class D3D11FrameAllocator : public BaseFrameAllocator
{
public:

	D3D11FrameAllocator();
	virtual ~D3D11FrameAllocator();

	virtual mfxStatus Init(mfxAllocatorParams *pParams);
	virtual mfxStatus Close();
	virtual ID3D11Device * GetD3D11Device() { return m_initParams.m_pParaInD3DDevice; };
	virtual mfxStatus GetFrameHDL(mfxMemId mid, mfxHDL *handle);

protected:
	virtual mfxStatus CheckRequestType(mfxFrameAllocRequest *request);
	virtual mfxStatus ReleaseResponse(mfxFrameAllocResponse *response);
	virtual mfxStatus AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response);

	D3D11AllocatorParams m_initParams;
	ID3D11DeviceContext *m_pDeviceContext;


};

#endif // __D3D11_ALLOCATOR_H__
#endif // #if MFX_D3D11_SUPPORT