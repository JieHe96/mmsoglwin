//
//               INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in accordance with the terms of that agreement.
//        Copyright (c) 2005-2013 Intel Corporation. All Rights Reserved.
//

#include <tchar.h>
#include <windows.h>
#include <numeric>
#include <ctime>
#include <algorithm>
#include "pipeline_decode.h"
#include "d3d_allocator.h"
#include "hw_device.h"

#pragma warning(disable : 4100)


mfxU16 CDecodingPipeline::GetFreeSurfaceIndex(mfxFrameSurface1* pSurfacesPool, mfxU16 nPoolSize)
{
	if (pSurfacesPool)
	{
		for (mfxU16 i = 0; i < nPoolSize; i++)
		{
			//hcg
			//if (0 == pSurfacesPool[i].Data.Locked)
			if (0 == pSurfacesPool[i].Data.Locked && 0 == pSurfacesPool[i].Data.reserved[8])
			{
				return i;
			}
		}
	}

	return MSDK_INVALID_SURF_IDX;
}

mfxStatus CDecodingPipeline::InitDecParams()
{
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_NULL_PTR);
    mfxStatus sts = MFX_ERR_NONE;

	// specify memory type 
	m_mfxVideoParams.IOPattern = (mfxU16)MFX_IOPATTERN_OUT_VIDEO_MEMORY;
	m_mfxVideoParams.AsyncDepth = M_DECAsyncDepth;

    // try to find a sequence header in the stream
    // if header is not found this function exits with error (e.g. if device was lost and there's no header in the remaining stream)
    for(;;)
    {
        // parse bit stream and fill mfx params
        sts = m_pmfxDEC->DecodeHeader(&m_mfxBS, &m_mfxVideoParams);
        if (MFX_ERR_MORE_DATA == sts)
        {
            if (m_mfxBS.MaxLength == m_mfxBS.DataLength)
            {
                sts = ExtendMfxBitstream(&m_mfxBS, m_mfxBS.MaxLength * 2); 
                MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
            }
            // read a portion of data             
            sts = m_FileReader->ReadNextFrame(&m_mfxBS);
            if (MFX_ERR_MORE_DATA == sts && 
                !(m_mfxBS.DataFlag & MFX_BITSTREAM_EOS))
            {
                m_mfxBS.DataFlag |= MFX_BITSTREAM_EOS;
                sts = MFX_ERR_NONE;
            }
            MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
            continue;
        }
		else break;
    }

    // check DecodeHeader status
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return MFX_ERR_NONE;
}

mfxStatus CDecodingPipeline::InitVppParams()
{
	m_mfxVppVideoParams.IOPattern = (mfxU16)MFX_IOPATTERN_IN_VIDEO_MEMORY;

	m_mfxVppVideoParams.IOPattern |= (mfxU16)MFX_IOPATTERN_OUT_VIDEO_MEMORY;

	MSDK_MEMCPY_VAR(m_mfxVppVideoParams.vpp.In, &m_mfxVideoParams.mfx.FrameInfo, sizeof(mfxFrameInfo));
	//for hevc must setting, otherwise query return error ???
	m_mfxVppVideoParams.vpp.In.FrameRateExtN = 25;
	m_mfxVppVideoParams.vpp.In.FrameRateExtD = 1;
	MSDK_MEMCPY_VAR(m_mfxVppVideoParams.vpp.Out, &m_mfxVppVideoParams.vpp.In, sizeof(mfxFrameInfo));

	m_mfxVppVideoParams.vpp.Out.FourCC = MFX_FOURCC_RGB4;

	m_vppOutWidth = m_sparams->vppOutWidth; m_vppOutHeight = m_sparams->vppOutHeight;

	if (m_vppOutWidth && m_vppOutHeight)
	{

		m_mfxVppVideoParams.vpp.Out.CropW = m_vppOutWidth;
		m_mfxVppVideoParams.vpp.Out.Width = MSDK_ALIGN16(m_vppOutWidth);
		m_mfxVppVideoParams.vpp.Out.CropH = m_vppOutHeight;
		m_mfxVppVideoParams.vpp.Out.Height = MSDK_ALIGN16(m_vppOutHeight);
	}

	m_mfxVppVideoParams.AsyncDepth = M_VPPAsyncDepth;// m_mfxVideoParams.AsyncDepth;


	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipeline::CreateFromExternalD3DDevice(void* pD3DDevice)
{
    mfxStatus sts = MFX_ERR_NONE;

    HWND window = NULL;

	if(D3D11_MEMORY == m_memType)
		m_hwdev = new CD3D11Device((ID3D11Device*)pD3DDevice);
	else
	{ 
		if (this->m_sparams->m_pParaInDDevice)
			m_hwdev = new CD3D9Device(pD3DDevice);
		else
			m_hwdev = new CD3D9Device();
	}
	
    if (NULL == m_hwdev)
        return MFX_ERR_MEMORY_ALLOC;

	sts = m_hwdev->Init(window, 0, GetMSDKAdapterNumber(m_mfxSession));
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return MFX_ERR_NONE;
}

mfxStatus CDecodingPipeline::CreateHWDevice()
{
    mfxStatus sts = MFX_ERR_NONE;

    HWND window = NULL;

	if (D3D11_MEMORY == m_memType)
		m_hwdev = new CD3D11Device();
	else
		m_hwdev = new CD3D9Device();

    if (NULL == m_hwdev)
        return MFX_ERR_MEMORY_ALLOC;

	sts = m_hwdev->Init(window, 0, GetMSDKAdapterNumber(m_mfxSession));
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return MFX_ERR_NONE;
}

mfxStatus CDecodingPipeline::AllocFramesDec()
{
	MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_NULL_PTR);

	mfxStatus sts = MFX_ERR_NONE;

	mfxFrameAllocRequest Request;
	mfxU16 nSurfNum = 0; // number of surfaces for decoder
	MSDK_ZERO_MEMORY(Request);

	//sts = m_pmfxDEC->Query(&m_mfxVideoParams, &m_mfxVideoParams);
	//MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM); 
	//MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	// calculate number of surfaces required for decoder
	sts = m_pmfxDEC->QueryIOSurf(&m_mfxVideoParams, &Request);
	if (MFX_WRN_PARTIAL_ACCELERATION == sts)
	{
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// surfaces are shared between vpp input and decode output
	//Request.Type |= MFX_MEMTYPE_EXTERNAL_FRAME | MFX_MEMTYPE_FROM_DECODE | MFX_MEMTYPE_FROM_VPPIN;

	if (Request.NumFrameSuggested < m_mfxVideoParams.AsyncDepth)
		return MFX_ERR_MEMORY_ALLOC;

	Request.NumFrameSuggested += m_mfxVideoParams.AsyncDepth;

	// alloc frames for decoder
	sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis, &Request, &m_mfxResponse);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// prepare mfxFrameSurface1 array for decoder
	nSurfNum = m_mfxResponse.NumFrameActual;

	m_pmfxSurfaces = new mfxFrameSurface1[nSurfNum];
	MSDK_CHECK_POINTER(m_pmfxSurfaces, MFX_ERR_MEMORY_ALLOC);
	for (int i = 0; i < nSurfNum; i++)
	{
		MSDK_ZERO_MEMORY(m_pmfxSurfaces[i]);
		MSDK_MEMCPY_VAR(m_pmfxSurfaces[i].Info, &(Request.Info), sizeof(mfxFrameInfo));
		m_pmfxSurfaces[i].Data.MemId = m_mfxResponse.mids[i];
	}

	InitDecSurfaceList();

	return MFX_ERR_NONE;
}

mfxStatus CDecodingPipeline::AllocFramesVpp()
{
	mfxStatus sts = MFX_ERR_NONE;

	mfxFrameAllocRequest VppRequest[2];
	mfxU16 nVppSurfNum = 0; // number of surfaces for vpp
	MSDK_ZERO_MEMORY(VppRequest[0]);
	MSDK_ZERO_MEMORY(VppRequest[1]);

	//sts = m_pmfxVpp->Query(&m_mfxVppVideoParams, &m_mfxVppVideoParams);
	//MSDK_IGNORE_MFX_STS(sts, MFX_WRN_INCOMPATIBLE_VIDEO_PARAM);
	//MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// VppRequest[0] for input frames request, VppRequest[1] for output frames request
	sts = m_pmfxVpp->QueryIOSurf(&m_mfxVppVideoParams, VppRequest);
	if (MFX_WRN_PARTIAL_ACCELERATION == sts) {
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	if ((VppRequest[0].NumFrameSuggested < m_mfxVppVideoParams.AsyncDepth) ||
		(VppRequest[1].NumFrameSuggested < m_mfxVppVideoParams.AsyncDepth))
		return MFX_ERR_MEMORY_ALLOC;

	// The number of surfaces for vpp output
	nVppSurfNum = VppRequest[1].NumFrameSuggested;
	VppRequest[1].NumFrameSuggested = VppRequest[1].NumFrameMin = nVppSurfNum;
	MSDK_MEMCPY_VAR(VppRequest[1].Info, &(m_mfxVppVideoParams.vpp.Out), sizeof(mfxFrameInfo));
	//special setting: will no mem calloc
	m_mfxVppResponse.NumFrameActual = SPECIAL_FOR_SHARE_GL_VPP_ALLOC;
	sts = m_pMFXAllocator->Alloc(m_pMFXAllocator->pthis, &VppRequest[1], &m_mfxVppResponse);
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	// prepare mfxFrameSurface1 array for decoder
	nVppSurfNum = m_mfxVppResponse.NumFrameActual;

#if 0
	m_VppExtParams.clear();

	m_VppDoNotUse.NumAlg = 3;

	m_VppDoNotUse.AlgList = new mfxU32[m_VppDoNotUse.NumAlg];
	if (!m_VppDoNotUse.AlgList) return MFX_ERR_NULL_PTR;

	m_VppDoNotUse.AlgList[0] = MFX_EXTBUFF_VPP_DENOISE; // turn off denoising (on by default)
	m_VppDoNotUse.AlgList[1] = MFX_EXTBUFF_VPP_SCENE_ANALYSIS; // turn off scene analysis (on by default)
	m_VppDoNotUse.AlgList[2] = MFX_EXTBUFF_VPP_DETAIL; // turn off detail enhancement (on by default)
	//m_VppDoNotUse.AlgList[3] = MFX_EXTBUFF_VPP_PROCAMP; // turn off processing amplified (on by default)

	m_VppExtParams.push_back((mfxExtBuffer*)&m_VppDoNotUse);

	m_mfxVppVideoParams.ExtParam = &m_VppExtParams[0];
	m_mfxVppVideoParams.NumExtParam = (mfxU16)m_VppExtParams.size();
#endif
	// AllocVppBuffers should call before AllocBuffers to set the value of m_OutputSurfacesNumber
	m_pmfxVppSurfaces = new mfxFrameSurface1[nVppSurfNum];
	MSDK_CHECK_POINTER(m_pmfxVppSurfaces, MFX_ERR_MEMORY_ALLOC);
	for (int i = 0; i < nVppSurfNum; i++)
	{
		MSDK_ZERO_MEMORY(m_pmfxVppSurfaces[i]);
		MSDK_MEMCPY_VAR(m_pmfxVppSurfaces[i].Info, &(VppRequest[1].Info), sizeof(mfxFrameInfo));
		m_pmfxVppSurfaces[i].Data.MemId = m_mfxVppResponse.mids[i];
	}

	InitVppSurfaceList();

	return MFX_ERR_NONE;
}

void *CDecodingPipeline::GetVppSufacePtr(int no)
{
	m_pmfxVppSurfaces[no].Data.reserved[7] = (UINT16)no;
	mfxHDLPair handle;

	m_pMFXAllocator->GetFrameHDL(m_pmfxVppSurfaces[no].Data.MemId, (mfxHDL *)&handle);
	return handle.first;
}

void *CDecodingPipeline::GetVppSuface9Ptr(int no, mfxHDLPair& handle)
{
	m_pmfxVppSurfaces[no].Data.reserved[7] = (UINT16)no;

	handle.second = (mfxHDL)SPECIAL_FOR_SHARE_GL_D3D9SURFACE;
	m_pMFXAllocator->GetFrameHDL(m_pmfxVppSurfaces[no].Data.MemId, (mfxHDL *)&handle);
	return NULL;
}

void *CDecodingPipeline::GetVppSufacePtr(mfxFrameData* pSurface)
{
	mfxHDLPair handle;
	m_pMFXAllocator->GetFrameHDL(pSurface->MemId, (mfxHDL *)&handle);
	return handle.first;
}


mfxStatus CDecodingPipeline::CreateAllocator()
{   
    mfxStatus sts = MFX_ERR_NONE;
 
    if (m_memType != SYSTEM_MEMORY)
    {
		if(m_sparams->m_pParaInDDevice)
			sts = CreateFromExternalD3DDevice(m_sparams->m_pParaInDDevice);
		else 
			sts = CreateHWDevice();
	
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        // provide device manager to MediaSDK
        mfxHDL hdl = NULL;
        mfxHandleType hdl_t = (D3D11_MEMORY == m_memType ? MFX_HANDLE_D3D11_DEVICE : MFX_HANDLE_D3D9_DEVICE_MANAGER);

        sts = m_hwdev->GetHandle(hdl_t, &hdl);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
        sts = m_mfxSession.SetHandle(hdl_t, hdl);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

		// create D3D allocator
		if (D3D11_MEMORY == m_memType)
		{
			m_pMFXAllocator = new D3D11FrameAllocator;
			MSDK_CHECK_POINTER(m_pMFXAllocator, MFX_ERR_MEMORY_ALLOC);

			D3D11AllocatorParams *pd3dAllocParams = new D3D11AllocatorParams;
			MSDK_CHECK_POINTER(pd3dAllocParams, MFX_ERR_MEMORY_ALLOC);
			pd3dAllocParams->m_pParaInD3DDevice = reinterpret_cast<ID3D11Device *>(hdl);

			m_pmfxAllocatorParams = pd3dAllocParams;
		}
		else
		{
			m_pMFXAllocator = new D3DFrameAllocator;
			MSDK_CHECK_POINTER(m_pMFXAllocator, MFX_ERR_MEMORY_ALLOC);

			D3DAllocatorParams *pd3dAllocParams = new D3DAllocatorParams;
			MSDK_CHECK_POINTER(pd3dAllocParams, MFX_ERR_MEMORY_ALLOC);
			pd3dAllocParams->m_pParaInDeviceManager9 = reinterpret_cast<IDirect3DDeviceManager9 *>(hdl);
			m_pmfxAllocatorParams = pd3dAllocParams;
		}


        /* In case of video memory we must provide MediaSDK with external allocator 
        thus we demonstrate "external allocator" usage model.
        Call SetAllocator to pass allocator to mediasdk */
        sts = m_mfxSession.SetFrameAllocator(m_pMFXAllocator);
        MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

        m_bExternalAlloc = true;
    }

    // initialize memory allocator
    sts = m_pMFXAllocator->Init(m_pmfxAllocatorParams);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return MFX_ERR_NONE;
}

void CDecodingPipeline::DeleteFrames()
{
    // delete surfaces array
	ClearDecSurfaceList();
    MSDK_SAFE_DELETE_ARRAY(m_pmfxSurfaces);    

    // delete frames
    if (m_pMFXAllocator)
        m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_mfxResponse);

	// delete surfaces array
	ClearVppSurfaceList();
	MSDK_SAFE_DELETE_ARRAY(m_pmfxVppSurfaces);

	// delete frames
	if (m_pMFXAllocator)
		m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_mfxVppResponse);

	return;
}

void CDecodingPipeline::DeleteAllocator()
{
    // delete allocator
    MSDK_SAFE_DELETE(m_pMFXAllocator);   
    MSDK_SAFE_DELETE(m_pmfxAllocatorParams);
    MSDK_SAFE_DELETE(m_hwdev);
}

CDecodingPipeline::CDecodingPipeline()
{
    m_pmfxDEC = NULL;
    m_pMFXAllocator = NULL;
    m_pmfxAllocatorParams = NULL;
    m_memType = SYSTEM_MEMORY;
    m_bExternalAlloc = false;
    m_pmfxSurfaces = NULL; 

    m_hwdev = NULL;

    MSDK_ZERO_MEMORY(m_mfxVideoParams);
    MSDK_ZERO_MEMORY(m_mfxResponse);
    MSDK_ZERO_MEMORY(m_mfxBS);

#ifdef USE_VPP
	m_pmfxVppSurfaces = NULL;
	MSDK_ZERO_MEMORY(m_mfxVppVideoParams);
	MSDK_ZERO_MEMORY(m_mfxVppResponse);

	//m_bVppFullColorRange = false;
	MSDK_ZERO_MEMORY(m_VppDoNotUse);
	m_VppDoNotUse.Header.BufferId = MFX_EXTBUFF_VPP_DONOTUSE;
	m_VppDoNotUse.Header.BufferSz = sizeof(m_VppDoNotUse);

#endif
}

CDecodingPipeline::~CDecodingPipeline()
{
    Close();
	delete m_sparams;
}

mfxStatus CDecodingPipeline::Init(structDecoderParams *pParams)
{
    MSDK_CHECK_POINTER(pParams, MFX_ERR_NULL_PTR);

    mfxStatus sts = MFX_ERR_NONE;
	m_sparams = new structDecoderParams();
	*m_sparams = *pParams;

	m_FileReader.reset(new CSmplBitstreamReader());

    sts = m_FileReader->Init(pParams->strSrcFile);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);     

	mfxVersion version = { { 3, 1 } };
	mfxIMPL impl = MFX_IMPL_HARDWARE_ANY;

    // Init session
    if (pParams->bUseHWLib)
    {             
        // try searching on all display adapters
                
        // if d3d11 surfaces are used ask the library to run acceleration through D3D11
        // feature may be unsupported due to OS or MSDK API version
        if (D3D11_MEMORY == pParams->memType) 
            impl |= MFX_IMPL_VIA_D3D11;

		if (D3D9_MEMORY == pParams->memType)
			impl |= MFX_IMPL_VIA_D3D9;

        sts = m_mfxSession.Init(impl, &version);

        // MSDK API version may not support multiple adapters - then try initialize on the default
        if (MFX_ERR_NONE != sts)
            sts = m_mfxSession.Init(impl & !MFX_IMPL_HARDWARE_ANY | MFX_IMPL_HARDWARE, &version);        
    }
    else
        sts = m_mfxSession.Init(MFX_IMPL_SOFTWARE, &version);    

	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = m_mfxSession.QueryVersion(&version); // get real API version of the loaded library
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = m_mfxSession.QueryIMPL(&impl); // get actual library implementation
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	//for hevc&h265
	if (pParams->videoType == MFX_CODEC_HEVC)
	{
		sts = MFXVideoUSER_Load(m_mfxSession, &MFX_PLUGINID_HEVCD_HW, 1);
		MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);
	}



    // create decoder
    m_pmfxDEC = new MFXVideoDECODE(m_mfxSession);
    MSDK_CHECK_POINTER(m_pmfxDEC, MFX_ERR_MEMORY_ALLOC);    

	m_pmfxVpp = new MFXVideoVPP(m_mfxSession);
	if (!m_pmfxVpp) return MFX_ERR_MEMORY_ALLOC;

    // set video type in parameters
    m_mfxVideoParams.mfx.CodecId = pParams->videoType;
    // set memory type
    m_memType = pParams->memType;

    // prepare bit stream
    sts = InitMfxBitstream(&m_mfxBS, 1024 * 1024);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);    

	// create device and allocator. SetHandle must be called after session Init and before any other MSDK calls, 
    // otherwise an own device will be created by MSDK
    sts = CreateAllocator();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // Populate parameters. Involves DecodeHeader call
    sts = InitDecParams();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = InitVppParams();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // in case of HW accelerated decode frames must be allocated prior to decoder initialization
    sts = AllocFramesDec();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = AllocFramesVpp();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    sts = m_pmfxDEC->Init(&m_mfxVideoParams);
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = m_pmfxVpp->Init(&m_mfxVppVideoParams);
	if (MFX_WRN_PARTIAL_ACCELERATION == sts)
	{
		msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
		MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
	}
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return MFX_ERR_NONE;
}

void CDecodingPipeline::Close()
{
	mfxStatus sts = MFX_ERR_NONE;
	// close decoder
	sts = m_pmfxDEC->Close();
	sts = m_pmfxVpp->Close();


	WipeMfxBitstream(&m_mfxBS);
    

    DeleteFrames();
    
    // allocator if used as external for MediaSDK must be deleted after decoder
    DeleteAllocator();

	MSDK_SAFE_DELETE(m_pmfxDEC);
	MSDK_SAFE_DELETE(m_pmfxVpp);

	//for hevc&h265
	if (m_mfxVideoParams.mfx.CodecId == MFX_CODEC_HEVC)
	{
		sts = MFXVideoUSER_UnLoad(m_mfxSession, &MFX_PLUGINID_HEVCD_HW);
	}
	m_mfxSession.Close();

    if (m_FileReader.get())
        m_FileReader->Close();

	MSDK_SAFE_DELETE(m_VppDoNotUse.AlgList);

    return;
}

mfxStatus CDecodingPipeline::ResetDecoder(structDecoderParams *pParams)
{
    mfxStatus sts = MFX_ERR_NONE;    

    // close decoder
    sts = m_pmfxDEC->Close();
    MSDK_IGNORE_MFX_STS(sts, MFX_ERR_NOT_INITIALIZED);
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // free allocated frames
    DeleteFrames();
    
    // initialize parameters with values from parsed header 
    sts = InitDecParams();
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // in case of HW accelerated decode frames must be allocated prior to decoder initialization
	sts = AllocFramesDec();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = AllocFramesVpp();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    // init decoder
    sts = m_pmfxDEC->Init(&m_mfxVideoParams);
    if (MFX_WRN_PARTIAL_ACCELERATION == sts)
    {
        msdk_printf(MSDK_STRING("WARNING: partial acceleration\n"));
        MSDK_IGNORE_MFX_STS(sts, MFX_WRN_PARTIAL_ACCELERATION);
    }
    MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

    return MFX_ERR_NONE;
}

void* CDecodingPipeline::RunSingleFrameDecode(mfxStatus& sts)
{   
	mfxFrameSurface1    *pmfxOutSurface = NULL;
	mfxFrameSurface1    *pmfxInSurface = NULL;
 
	void *retval = NULL; //pointer to dx surface for manipulation / blt via stretchrect

	while (MFX_ERR_NONE <= sts || MFX_ERR_MORE_DATA == sts || MFX_ERR_MORE_SURFACE == sts)
    {
        if (MFX_WRN_DEVICE_BUSY == sts)
        {
            MSDK_SLEEP(1); // just wait and then repeat the same call to DecodeFrameAsync
        }
        else if (MFX_ERR_MORE_DATA == sts)
        {
            sts = m_FileReader->ReadNextFrame(&m_mfxBS); // read more data to input bit stream

            //videowall mode: decoding in a loop
            if (MFX_ERR_MORE_DATA == sts )//&& m_bIsVideoWall)
            {
                m_FileReader->Reset();
                sts = MFX_ERR_NONE;
                continue;
            }
		
			MSDK_BREAK_ON_ERROR(sts);
		}
        
		if (MFX_ERR_MORE_SURFACE == sts || MFX_ERR_NONE == sts)
		{
			pmfxInSurface = PopupDecSurfaceList();

			if (pmfxInSurface == NULL)
			{
				if(m_decSync) m_mfxSession.SyncOperation(m_decSync, MSDK_DEC_WAIT_INTERVAL);
				ReinitDecSurfaceList();
				pmfxInSurface = PopupDecSurfaceList();

				if (pmfxInSurface == NULL)
				{
					sts = MFX_ERR_MORE_SURFACE;
					return 0;//MFX_ERR_MEMORY_ALLOC;            
				}
			}
		}

		sts = m_pmfxDEC->DecodeFrameAsync(&m_mfxBS, pmfxInSurface, &pmfxOutSurface, &m_decSync);

        // ignore warnings if output is available, 
        // if no output and no action required just repeat the same call
        if (MFX_ERR_NONE < sts && m_decSync)
        {
            sts = MFX_ERR_NONE;
        }
        
        if (MFX_ERR_NONE == sts)
        {                
            //sts = m_mfxSession.SyncOperation(m_decSync, MSDK_DEC_WAIT_INTERVAL);
			//retval = (void *)&(pmfxOutSurface->Data);// .MemId;
			retval = (void *)pmfxOutSurface;
			break;
        }

    } //while processing    

	//if (retval)
	//	retval = RunSingleFrameVPP(pmfxOutSurface);

    return retval;//sts; // ERR_NONE or ERR_INCOMPATIBLE_VIDEO_PARAM
}

void *CDecodingPipeline::RunSingleFrameVPP(mfxFrameSurface1* pmfxOutSurface, mfxStatus& sts)
{
	//mfxSyncPoint        syncp;
	//mfxStatus           sts = MFX_ERR_NONE;
	//mfxU16              nIndex = 0; // index of free surface   
	mfxFrameSurface1    *pmfxVppInSurface = NULL;
	void *retval = NULL; //pointer to dx surface for manipulation / blt via stretchrect

	do {
		pmfxVppInSurface = PopupVppSurfaceList();
		
		if (pmfxVppInSurface == NULL)
		{
			if(m_vppSync)  m_mfxSession.SyncOperation(m_vppSync, MSDK_DEC_WAIT_INTERVAL);
			ReinitVppSurfaceList();
			pmfxVppInSurface = PopupVppSurfaceList();
			if (pmfxVppInSurface == NULL)
			{
				sts = MFX_ERR_MORE_SURFACE;
				return 0;//MFX_ERR_MEMORY_ALLOC;            
			}
		}

		//if ((m_pmfxVppSurfaces[nIndex].Info.CropW == 0) || (m_pmfxVppSurfaces[nIndex].Info.CropH == 0))
		//{
		//	m_pmfxVppSurfaces[nIndex].Info.CropW = pmfxOutSurface->Info.CropW;
		//	m_pmfxVppSurfaces[nIndex].Info.CropH = pmfxOutSurface->Info.CropH;
		//	m_pmfxVppSurfaces[nIndex].Info.CropX = pmfxOutSurface->Info.CropX;
		//	m_pmfxVppSurfaces[nIndex].Info.CropY = pmfxOutSurface->Info.CropY;
		//}
		//if (pmfxOutSurface->Info.PicStruct != m_pmfxVppSurfaces[nIndex].Info.PicStruct)
		//{
		//	m_pmfxVppSurfaces[nIndex].Info.PicStruct = pmfxOutSurface->Info.PicStruct;
		//}
		//if ((pmfxOutSurface->Info.PicStruct == 0) && (m_pmfxVppSurfaces[nIndex].Info.PicStruct == 0))
		//{
		//	m_pmfxVppSurfaces[nIndex].Info.PicStruct = pmfxOutSurface->Info.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;
		//}

		//m_pmfxVppSurfaces[nIndex].Info.PicStruct = MFX_PICSTRUCT_PROGRESSIVE;

		sts = m_pmfxVpp->RunFrameVPPAsync(pmfxOutSurface, pmfxVppInSurface, NULL, &m_vppSync);

		if (MFX_WRN_DEVICE_BUSY == sts)
		{
			MSDK_SLEEP(1); // just wait and then repeat the same call to RunFrameVPPAsync
		}

		if (MFX_ERR_NONE == sts)
		{
			sts = m_mfxSession.SyncOperation(m_vppSync, MSDK_DEC_WAIT_INTERVAL);
			retval = (void *)pmfxVppInSurface;
		}

	} while (MFX_WRN_DEVICE_BUSY == sts);

	return retval;
}

mfxStatus CDecodingPipeline::ResetVppForResize()
{
	mfxStatus           sts = MFX_ERR_NONE;
	return sts;
}

mfxStatus CDecodingPipeline::RecreateVppForResize(mfxU16 nWidth, mfxU16 nHeight)
{
	mfxStatus           sts = MFX_ERR_NONE;

	// delete surfaces array
	for (int i = 0; i < m_mfxVppResponse.NumFrameActual; i++)
	{
		//((ID3D11Texture2D*)(m_pmfxVppSurfaces[i].Data.MemId))->Release();
		(m_pmfxVppSurfaces[i].Data.MemId) = NULL;
	}

	ClearVppSurfaceList();

	MSDK_SAFE_DELETE_ARRAY(m_pmfxVppSurfaces);

	// delete frames
	if (m_pMFXAllocator)
		m_pMFXAllocator->Free(m_pMFXAllocator->pthis, &m_mfxVppResponse);

	m_pmfxVpp->Close();

	m_sparams->vppOutHeight = nHeight;
	m_sparams->vppOutWidth = nWidth;
	m_vppOutWidth = m_sparams->vppOutWidth; m_vppOutHeight = m_sparams->vppOutHeight;

	if (m_vppOutWidth && m_vppOutHeight)
	{

		m_mfxVppVideoParams.vpp.Out.CropW = m_vppOutWidth;
		m_mfxVppVideoParams.vpp.Out.Width = MSDK_ALIGN16(m_vppOutWidth);
		m_mfxVppVideoParams.vpp.Out.CropH = m_vppOutHeight;
		m_mfxVppVideoParams.vpp.Out.Height = MSDK_ALIGN16(m_vppOutHeight);
	}

	sts = AllocFramesVpp();
	MSDK_CHECK_RESULT(sts, MFX_ERR_NONE, sts);

	sts = m_pmfxVpp->Init(&m_mfxVppVideoParams);

	return sts;
}

void CDecodingPipeline::InitDecSurfaceList()
{
	for (int i = 0; i < m_mfxResponse.NumFrameActual; i++)
		m_decSurfaceList.push_back(&(m_pmfxSurfaces[i]));
}

void CDecodingPipeline::InitVppSurfaceList()
{
	for (int i = 0; i < m_mfxVppResponse.NumFrameActual; i++)
		m_vppSurfaceList.push_back(&(m_pmfxVppSurfaces[i]));
}

void CDecodingPipeline::ReinitDecSurfaceList()
{
	m_decSurfaceListLock.Lock();
	for (int i = 0; i < m_mfxResponse.NumFrameActual; i++)
	{
		if(m_pmfxSurfaces[i].Data.reserved[8] == 0 && m_pmfxSurfaces[i].Data.Locked == 0)
			m_decSurfaceList.push_back(&(m_pmfxSurfaces[i]));
	}
	m_decSurfaceListLock.Unlock();
}

void CDecodingPipeline::ReinitVppSurfaceList()
{
	m_vppSurfaceListLock.Lock();
	for (int i = 0; i < m_mfxVppResponse.NumFrameActual; i++)
	{
		if (m_pmfxVppSurfaces[i].Data.reserved[8] == 0 && m_pmfxVppSurfaces[i].Data.Locked == 0)
			m_vppSurfaceList.push_back(&(m_pmfxVppSurfaces[i]));
	}
	m_vppSurfaceListLock.Unlock();
}

mfxFrameSurface1* CDecodingPipeline::PopupDecSurfaceList()
{
	mfxFrameSurface1* pSurface = NULL;
	m_decSurfaceListLock.Lock();
	if (!(m_decSurfaceList.empty()))
	{
		pSurface = m_decSurfaceList.front();
		m_decSurfaceList.pop_front();
	}
	m_decSurfaceListLock.Unlock();

	return pSurface;
}

mfxFrameSurface1* CDecodingPipeline::PopupVppSurfaceList()
{
	mfxFrameSurface1* pSurface = NULL;
	m_vppSurfaceListLock.Lock();
	if (!(m_vppSurfaceList.empty()))
	{
		pSurface = m_vppSurfaceList.front();
		m_vppSurfaceList.pop_front();
	}
	m_vppSurfaceListLock.Unlock();

	return pSurface;
}

void CDecodingPipeline::PushDecSurfaceList(mfxFrameSurface1* pSurface)
{
	m_decSurfaceListLock.Lock();
	m_decSurfaceList.push_back(pSurface);
	m_decSurfaceListLock.Unlock();
}

void CDecodingPipeline::PushVppSurfaceList(mfxFrameSurface1* pSurface)
{
	m_vppSurfaceListLock.Lock();
	m_vppSurfaceList.push_back(pSurface);
	m_vppSurfaceListLock.Unlock();
}

void CDecodingPipeline::ClearDecSurfaceList()
{
	m_decSurfaceListLock.Lock();
	m_decSurfaceList.clear();
	m_decSurfaceListLock.Unlock();
}

void CDecodingPipeline::ClearVppSurfaceList()
{
	m_vppSurfaceListLock.Lock();
	m_vppSurfaceList.clear();
	m_vppSurfaceListLock.Unlock();
}

