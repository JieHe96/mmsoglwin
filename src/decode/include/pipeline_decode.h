//* ////////////////////////////////////////////////////////////////////////////// */
//*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//        Copyright (c) 2005-2013 Intel Corporation. All Rights Reserved.
//
//
//*/
   
#ifndef __PIPELINE_DECODE_H__
#define __PIPELINE_DECODE_H__

#define USE_VPP

#define M_VPPAsyncDepth 4

//setting different asyncdepth will not get same suggestion frame number in QueryIOSurf 
//if
//set 4 => 9
//set 0 => 10
#define M_DECAsyncDepth 4

#include "shared_defs.h"

#if D3D_SURFACES_SUPPORT
#pragma warning(disable : 4201)
#include <d3d9.h>
#include <dxva2api.h>
#endif

#include <vector>
#include <memory>

#include "hw_device.h"
#include "shared_utils.h"
#include "base_allocator.h"

#include "mfxvideo.h"
#include "mfxvideo++.h"
#include "mfxplugin.h"


enum MemType {
    SYSTEM_MEMORY = 0x00,
    D3D9_MEMORY   = 0x01,
    D3D11_MEMORY  = 0x02,
};

enum DecoderResetMode {
	NO_RESET = 0x00,
	VPP_RESET,
	ALL_RESET
};

struct structDecoderParams
{
	mfxU32  decoderID;
    mfxU32  videoType;
    MemType memType; 
    bool    bUseHWLib; // true if application wants to use HW mfx library


	mfxU16  vppOutWidth, vppOutHeight;

    msdk_char  strSrcFile[MSDK_MAX_FILENAME_LEN];

	void* m_pParaInDDevice;

    structDecoderParams()
    {
        MSDK_ZERO_MEMORY(*this);
    }
	~structDecoderParams()
	{
		m_pParaInDDevice = NULL;
	}

	DecoderResetMode GetResetMode(structDecoderParams *inParam)
	{
		BOOL vppSizeChanged = (inParam->vppOutWidth != vppOutWidth) | (inParam->vppOutHeight != vppOutHeight);
		BOOL fileChanged = strcmp((const char *)strSrcFile, (const char *)(inParam->strSrcFile)) != 0;

		if (fileChanged) return ALL_RESET;
		if (vppSizeChanged) return VPP_RESET;

		return NO_RESET;
	}
};

class CDecodingPipeline
{
public:

    CDecodingPipeline();
    virtual ~CDecodingPipeline();

    virtual mfxStatus	Init(structDecoderParams *pParams);
	virtual void*		RunSingleFrameDecode(mfxStatus& sts);
	void *				RunSingleFrameVPP(mfxFrameSurface1*, mfxStatus& sts);
	virtual void		Close();
    virtual mfxStatus	ResetDecoder(structDecoderParams *pParams);

    virtual mfxStatus	ResetDevice()		{ return m_hwdev->Reset(); }
	virtual int			GetVideoWidth()		{ return m_mfxVideoParams.mfx.FrameInfo.Width;  }
	virtual int			GetVideoHeight()	{ return m_mfxVideoParams.mfx.FrameInfo.Height; }

	mfxStatus			ResetVppForResize();
	mfxStatus			RecreateVppForResize(mfxU16 nWidth, mfxU16 nHeight);

	void *				GetVppSufacePtr(int no);
	void *				GetVppSuface9Ptr(int no, mfxHDLPair& handle);
	void *				GetVppSufacePtr(mfxFrameData* pSurface);
	UINT32				GetVppOutSurfaceNums() { return m_pmfxVpp ? m_mfxVppResponse.NumFrameActual : 0; }

	DecoderResetMode    DoResetQuery(structDecoderParams*inParam) { return m_sparams->GetResetMode(inParam); }
protected:
    std::auto_ptr<CSmplBitstreamReader>  m_FileReader;
    mfxBitstream            m_mfxBS;			// contains encoded data

	MemType                 m_memType;			// memory type of surfaces to use
	bool                    m_bExternalAlloc;	// use memory allocator as external for Media SDK

	CHWDevice*				m_hwdev;

	MFXVideoSession			m_mfxSession;
	MFXFrameAllocator*      m_pMFXAllocator;
	mfxAllocatorParams*     m_pmfxAllocatorParams;

	MFXVideoDECODE*			m_pmfxDEC;
    mfxVideoParam			m_mfxVideoParams; 
    mfxFrameSurface1*       m_pmfxSurfaces;		// frames array
    mfxFrameAllocResponse   m_mfxResponse;		// memory allocation response for decoder  
	std::vector<mfxExtBuffer *> m_ExtBuffers;

	MFXVideoVPP*            m_pmfxVpp;
	mfxVideoParam           m_mfxVppVideoParams;
	mfxFrameAllocResponse   m_mfxVppResponse;   // memory allocation response for vpp
	mfxFrameSurface1*       m_pmfxVppSurfaces;	// frames array
	std::vector<mfxExtBuffer*> m_VppExtParams;
	mfxExtVPPDoNotUse       m_VppDoNotUse;      // for disabling VPP algorithms

	mfxU16                  m_vppOutWidth;
	mfxU16                  m_vppOutHeight;

	MSDKMutex				m_decSurfaceListLock, m_vppSurfaceListLock;
	std::list<mfxFrameSurface1*>		m_decSurfaceList, m_vppSurfaceList;

	mfxSyncPoint			m_decSync, m_vppSync;

	void	InitDecSurfaceList();
	void	InitVppSurfaceList();
	void	ReinitDecSurfaceList();
	void	ReinitVppSurfaceList();

	mfxFrameSurface1*	PopupDecSurfaceList();
	mfxFrameSurface1*	PopupVppSurfaceList();

	void	ClearDecSurfaceList();
	void	ClearVppSurfaceList();

public:
	void	PushDecSurfaceList(mfxFrameSurface1* pSurface);
	void	PushVppSurfaceList(mfxFrameSurface1* pSurface);

protected:
	virtual mfxStatus	InitDecParams();
	virtual mfxStatus	InitVppParams();
	virtual mfxStatus	AllocFramesDec();
	virtual mfxStatus	AllocFramesVpp();

	virtual mfxStatus	CreateAllocator();
    virtual mfxStatus	CreateHWDevice();
	virtual mfxStatus	CreateFromExternalD3DDevice(void* pD3DDevice);
    virtual void		DeleteFrames();         
    virtual void		DeleteAllocator();     

	mfxU16 GetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize);


	structDecoderParams* m_sparams;
};

#endif // __PIPELINE_DECODE_H__ 