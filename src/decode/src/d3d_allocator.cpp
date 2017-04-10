
#include <objbase.h>
#include <initguid.h>
//#include <assert.h>
#include <d3d9.h>
#include "d3d_allocator.h"

#define D3DFMT_NV12 (D3DFORMAT)MAKEFOURCC('N','V','1','2')
#define D3DFMT_YV12 (D3DFORMAT)MAKEFOURCC('Y','V','1','2')

D3DFORMAT ConvertMfxFourccToD3dFormat(mfxU32 fourcc)
{
    switch (fourcc)
    {
    case MFX_FOURCC_NV12:
        return D3DFMT_NV12;
    case MFX_FOURCC_YV12:
        return D3DFMT_YV12;
    case MFX_FOURCC_YUY2:
        return D3DFMT_YUY2;
    case MFX_FOURCC_RGB3:
        return D3DFMT_R8G8B8;
    case MFX_FOURCC_RGB4:
        return D3DFMT_A8R8G8B8;
    case MFX_FOURCC_P8:
        return D3DFMT_P8;
    default:
        return D3DFMT_UNKNOWN;
    }
}
DXGI_FORMAT ConverColorToDXGIFormat(mfxU32 fourcc)
{
	switch (fourcc)
	{
	case MFX_FOURCC_NV12:
		return DXGI_FORMAT_NV12;

	case MFX_FOURCC_YUY2:
		return DXGI_FORMAT_YUY2;

	case MFX_FOURCC_RGB4:
		return DXGI_FORMAT_B8G8R8A8_UNORM;

	case MFX_FOURCC_P8:
	case MFX_FOURCC_P8_TEXTURE:
		return DXGI_FORMAT_P8;

	case MFX_FOURCC_ARGB16:
	case MFX_FOURCC_ABGR16:
		return DXGI_FORMAT_R16G16B16A16_UNORM;

	case MFX_FOURCC_P010:
		return DXGI_FORMAT_P010;

	case MFX_FOURCC_A2RGB10:
		return DXGI_FORMAT_R10G10B10A2_UNORM;

	case DXGI_FORMAT_AYUV:
		return DXGI_FORMAT_AYUV;

	default:
		return DXGI_FORMAT_UNKNOWN;
	}
}


D3DFrameAllocator::D3DFrameAllocator()
: m_pDecoderService(0), m_pProcessorService(0), m_hDecoder(0), m_hProcessor(0), m_pIND3DDeviceManager9(0)
{    
}

D3DFrameAllocator::~D3DFrameAllocator()
{
    Close();
}

mfxStatus D3DFrameAllocator::Init(mfxAllocatorParams *pParams)
{   
    D3DAllocatorParams *pd3dParams = 0;
    pd3dParams = dynamic_cast<D3DAllocatorParams *>(pParams);
    if (!pd3dParams)
        return MFX_ERR_NOT_INITIALIZED;
   
    m_pIND3DDeviceManager9 = pd3dParams->m_pParaInDeviceManager9;

    return MFX_ERR_NONE;    
}

mfxStatus D3DFrameAllocator::Close()
{   
    if (m_pIND3DDeviceManager9)
    {
        m_pIND3DDeviceManager9->CloseDeviceHandle(m_hDecoder);
        m_pIND3DDeviceManager9 = 0;
        m_hDecoder = 0;
		m_hProcessor = 0;
	}

    return BaseFrameAllocator::Close();
}


mfxStatus D3DFrameAllocator::GetFrameHDL(mfxMemId mid, mfxHDL *handle)
{
	if (NULL == handle)
		return MFX_ERR_INVALID_HANDLE;

	mfxHDLPair *pPair = (mfxHDLPair*)mid;

	*handle = pPair->first;

	pPair = (mfxHDLPair *)handle;
	if (pPair->second == (mfxHDL)SPECIAL_FOR_SHARE_GL_D3D9SURFACE)
	{
		pPair->first = ((mfxHDLPair*)mid)->first;
		pPair->second = ((mfxHDLPair*)mid)->second;
	}
	return MFX_ERR_NONE;
}

mfxStatus D3DFrameAllocator::CheckRequestType(mfxFrameAllocRequest *request)
{    
    mfxStatus sts = BaseFrameAllocator::CheckRequestType(request);
    if (MFX_ERR_NONE != sts)
        return sts;

    if ((request->Type & (MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET | MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET)) != 0)
        return MFX_ERR_NONE;
    else
        return MFX_ERR_UNSUPPORTED;
}

mfxStatus D3DFrameAllocator::ReleaseResponse(mfxFrameAllocResponse *response)
{
    if (!response)
        return MFX_ERR_NULL_PTR;

    mfxStatus sts = MFX_ERR_NONE;

    if (response->mids)
    {
        for (mfxU32 i = 0; i < response->NumFrameActual; i++)
        {
            if (response->mids[i])
            {
				mfxHDLPair handle;
				handle.second = (mfxHDL)SPECIAL_FOR_SHARE_GL_D3D9SURFACE;

                sts = GetFrameHDL(response->mids[i], (mfxHDL *)&handle);
                if (MFX_ERR_NONE != sts)
                    return sts;
				((IDirect3DSurface9*)(handle.first))->Release();

				delete response->mids[i];
            }
        }        
		delete[] response->mids;
	}

    response->mids = 0;
    
    return sts;
}


mfxStatus D3DFrameAllocator::AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
    HRESULT hr;

    D3DFORMAT format = ConvertMfxFourccToD3dFormat(request->Info.FourCC);

    if (format == D3DFMT_UNKNOWN)
        return MFX_ERR_UNSUPPORTED;
    
    safe_array<mfxMemId> mids(new mfxMemId[request->NumFrameSuggested]);
    if (!mids.get())
        return MFX_ERR_MEMORY_ALLOC;

    DWORD   target;

    if (MFX_MEMTYPE_DXVA2_DECODER_TARGET & request->Type)
    {
        target = DXVA2_VideoDecoderRenderTarget;
    }
    else if (MFX_MEMTYPE_DXVA2_PROCESSOR_TARGET & request->Type)
    {
        target = DXVA2_VideoProcessorRenderTarget;
    }
    else
        return MFX_ERR_UNSUPPORTED;

	IDirectXVideoAccelerationService* videoService = NULL;

	if (target == DXVA2_VideoProcessorRenderTarget) {
		if (!m_hProcessor) {
			hr = m_pIND3DDeviceManager9->OpenDeviceHandle(&m_hProcessor);
			if (FAILED(hr))
				return MFX_ERR_MEMORY_ALLOC;

			hr = m_pIND3DDeviceManager9->GetVideoService(m_hProcessor, IID_IDirectXVideoProcessorService, (void**)&m_pProcessorService);
			if (FAILED(hr))
				return MFX_ERR_MEMORY_ALLOC;
		}
		videoService = m_pProcessorService;
	}
	else {
		if (!m_hDecoder)
		{
			hr = m_pIND3DDeviceManager9->OpenDeviceHandle(&m_hDecoder);
			if (FAILED(hr))
				return MFX_ERR_MEMORY_ALLOC;

			hr = m_pIND3DDeviceManager9->GetVideoService(m_hDecoder, IID_IDirectXVideoDecoderService, (void**)&m_pDecoderService);
			if (FAILED(hr))
				return MFX_ERR_MEMORY_ALLOC;
		}
		videoService = m_pDecoderService;
	}

	HANDLE *handles = (new HANDLE[request->NumFrameSuggested]);
	memset(handles, 0, sizeof(HANDLE)*request->NumFrameSuggested);

	if (response->NumFrameActual != SPECIAL_FOR_SHARE_GL_VPP_ALLOC)
		hr = videoService->CreateSurface(request->Info.Width, request->Info.Height, request->NumFrameSuggested - 1,
			format, D3DPOOL_DEFAULT, 0, target, (IDirect3DSurface9 **)mids.get(), NULL);
	else
	{
		IDirect3DDevice9*		m_pd3dDevice = NULL;

		hr = m_pIND3DDeviceManager9->LockDevice(m_hProcessor, &m_pd3dDevice, FALSE);
		for (int i = 0; i < request->NumFrameSuggested; i++)
		{
			IDirect3DTexture9 *texture = NULL;
			hr = m_pd3dDevice->CreateTexture(
				request->Info.Width, request->Info.Height, 1, D3DUSAGE_RENDERTARGET,
				D3DFMT_X8R8G8B8,//rtDesc.Format,
				D3DPOOL_DEFAULT,
				&texture,
				&handles[i]);
			hr = texture->GetSurfaceLevel(0, (IDirect3DSurface9**)&mids.get()[i]);
			texture->Release();
		}
		hr = m_pIND3DDeviceManager9->UnlockDevice(m_hProcessor, FALSE);
		m_pd3dDevice = NULL;
	}
	//hr = ((IDirectXVideoProcessorService *)videoService)->CreateSurface(request->Info.Width, request->Info.Height, request->NumFrameSuggested - 1,
	//	D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT, 0, target, (IDirect3DSurface9 **)mids.get(), handles);
	if (FAILED(hr))
		return MFX_ERR_MEMORY_ALLOC;

	mfxHDLPair** buffers = (new mfxHDLPair*[request->NumFrameSuggested]);

	for (int i = 0; i < request->NumFrameSuggested; i++)
	{
		buffers[i] = new mfxHDLPair;
		(buffers[i])->first = mids.get()[i];
		if (response->NumFrameActual != SPECIAL_FOR_SHARE_GL_VPP_ALLOC)
			(buffers[i])->second = 0;
		else
			(buffers[i])->second = handles[i];
	}
	delete[]mids.release(); delete[]handles;
	response->mids = (mfxMemId*)(buffers);

	response->NumFrameActual = request->NumFrameSuggested;

    return MFX_ERR_NONE;
}


#include "shared_defs.h"

#if MFX_D3D11_SUPPORT

D3D11FrameAllocator::D3D11FrameAllocator()
{
	m_pDeviceContext = NULL;
}

D3D11FrameAllocator::~D3D11FrameAllocator()
{
	Close();
}


mfxStatus D3D11FrameAllocator::Init(mfxAllocatorParams *pParams)
{
	D3D11AllocatorParams *pd3d11Params = 0;
	pd3d11Params = dynamic_cast<D3D11AllocatorParams *>(pParams);

	if (NULL == pd3d11Params ||
		NULL == pd3d11Params->m_pParaInD3DDevice)
	{
		return MFX_ERR_NOT_INITIALIZED;
	}

	m_initParams = *pd3d11Params;
	MSDK_SAFE_RELEASE(m_pDeviceContext);
	pd3d11Params->m_pParaInD3DDevice->GetImmediateContext(&m_pDeviceContext);

	return MFX_ERR_NONE;
}

mfxStatus D3D11FrameAllocator::Close()
{
	mfxStatus sts = BaseFrameAllocator::Close();
	MSDK_SAFE_RELEASE(m_pDeviceContext);

	return sts;
}


mfxStatus D3D11FrameAllocator::GetFrameHDL(mfxMemId mid, mfxHDL *handle)
{
	if (NULL == handle)
		return MFX_ERR_INVALID_HANDLE;

	mfxHDLPair *pPair = (mfxHDLPair*)handle;

	pPair->first = mid;
	pPair->second = NULL;

	return MFX_ERR_NONE;
}

mfxStatus D3D11FrameAllocator::CheckRequestType(mfxFrameAllocRequest *request)
{
	mfxStatus sts = BaseFrameAllocator::CheckRequestType(request);
	if (MFX_ERR_NONE != sts)
		return sts;

	if ((request->Type & (MFX_MEMTYPE_VIDEO_MEMORY_DECODER_TARGET | MFX_MEMTYPE_VIDEO_MEMORY_PROCESSOR_TARGET)) != 0)
		return MFX_ERR_NONE;
	else
		return MFX_ERR_UNSUPPORTED;
}

mfxStatus D3D11FrameAllocator::ReleaseResponse(mfxFrameAllocResponse *response)
{
	if (!response)
		return MFX_ERR_NULL_PTR;

	mfxStatus sts = MFX_ERR_NONE;

	if (response->mids)
	{
		for (mfxU32 i = 0; i < response->NumFrameActual; i++)
		{
			if (response->mids[i])
			{
				mfxHDLPair handle;
				sts = GetFrameHDL(response->mids[i], (mfxHDL *)&handle);
				if (MFX_ERR_NONE != sts)
					return sts;
				((ID3D11Texture2D *)(handle.first))->Release();
				response->mids[i] = NULL;
			}
		}
		delete[] response->mids;
	}

	response->mids = 0;

	return sts;
}

mfxStatus D3D11FrameAllocator::AllocImpl(mfxFrameAllocRequest *request, mfxFrameAllocResponse *response)
{
	HRESULT hRes;
	DXGI_FORMAT colorFormat = ConverColorToDXGIFormat(request->Info.FourCC);

	if (DXGI_FORMAT_UNKNOWN == colorFormat)
	{
		msdk_printf(MSDK_STRING("D3D11 Allocator: invalid fourcc is provided (%#X), exitting\n"), request->Info.FourCC);
		return MFX_ERR_UNSUPPORTED;
	}


	//safe_array<mfxMemId> mids(new mfxMemId[request->NumFrameSuggested]);
	mfxMemId *mids = (new mfxMemId[request->NumFrameSuggested]);
	//if (!mids.get())
	//	return MFX_ERR_MEMORY_ALLOC;


	D3D11_TEXTURE2D_DESC desc = { 0 };

	desc.Width = request->Info.Width;
	desc.Height = request->Info.Height;

	desc.MipLevels = 1;
	//number of subresources is 1 in case of not single texture
	desc.ArraySize = 1;
	desc.Format = ConverColorToDXGIFormat(request->Info.FourCC);
	desc.SampleDesc.Count = 1;
	desc.Usage = D3D11_USAGE_DEFAULT;
	//if (response->NumFrameActual == 9999)
	//	desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED | D3D11_RESOURCE_MISC_SHARED_NTHANDLE;
	//else
	desc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
	//D3D11_RESOURCE_MISC_SHARED;
	if (response->NumFrameActual == 9999)
		desc.BindFlags = D3D11_BIND_RENDER_TARGET;
	else
		desc.BindFlags = D3D11_BIND_DECODER;

	//ID3D11Texture2D* pTexture2D=NULL;

	for (size_t i = 0; i < request->NumFrameSuggested / desc.ArraySize; i++)
	{
		mids[i] = NULL;
		hRes = m_initParams.m_pParaInD3DDevice->CreateTexture2D(&desc, NULL, (ID3D11Texture2D**)&mids[i]);// &pTexture2D);

		if (FAILED(hRes))
		{
			msdk_printf(MSDK_STRING("CreateTexture2D(%lld) failed, hr = 0x%08lx\n"), (long long)i, hRes);
			return MFX_ERR_MEMORY_ALLOC;
		}

		//mids.get()[i] = pTexture2D;
	}
	response->mids = mids;// .release();

	response->NumFrameActual = request->NumFrameSuggested;

	return MFX_ERR_NONE;
}


#endif // #if MFX_D3D11_SUPPORT
