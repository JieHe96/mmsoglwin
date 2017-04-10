/* ////////////////////////////////////////////////////////////////////////////// */
/*
//
//              INTEL CORPORATION PROPRIETARY INFORMATION
//  This software is supplied under the terms of a license  agreement or
//  nondisclosure agreement with Intel Corporation and may not be copied
//  or disclosed except in  accordance  with the terms of that agreement.
//        Copyright (c) 2005-2013 Intel Corporation. All Rights Reserved.
//
//
*/
#include <math.h>
#include <process.h>

#include "shared_defs.h"
#include "shared_utils.h"

#pragma warning( disable : 4748 )


CSmplBitstreamReader::CSmplBitstreamReader()
{
    m_fSource = NULL;
    m_bInited = false;
}

CSmplBitstreamReader::~CSmplBitstreamReader()
{
    Close();
}

void CSmplBitstreamReader::Close()
{
    if (m_fSource)
    {
        fclose(m_fSource);
        m_fSource = NULL;
    }

    m_bInited = false;
}

void CSmplBitstreamReader::Reset()
{
    fseek(m_fSource, 0, SEEK_SET);
}

mfxStatus CSmplBitstreamReader::Init(const msdk_char *strFileName)
{
    MSDK_CHECK_POINTER(strFileName, MFX_ERR_NULL_PTR);
    MSDK_CHECK_ERROR(msdk_strlen(strFileName), 0, MFX_ERR_NOT_INITIALIZED);

    Close();

    //open file to read input stream
    MSDK_FOPEN(m_fSource, strFileName, MSDK_STRING("rb"));
    MSDK_CHECK_POINTER(m_fSource, MFX_ERR_NULL_PTR);

    m_bInited = true;
    return MFX_ERR_NONE;
}

mfxStatus CSmplBitstreamReader::ReadNextFrame(mfxBitstream *pBS)
{
    MSDK_CHECK_POINTER(pBS, MFX_ERR_NULL_PTR);
    MSDK_CHECK_ERROR(m_bInited, false, MFX_ERR_NOT_INITIALIZED);

    mfxU32 nBytesRead = 0;

    memmove(pBS->Data, pBS->Data + pBS->DataOffset, pBS->DataLength);
    pBS->DataOffset = 0;
    nBytesRead = (mfxU32)fread(pBS->Data + pBS->DataLength, 1, pBS->MaxLength - pBS->DataLength, m_fSource);

    if (0 == nBytesRead)
    {
        return MFX_ERR_MORE_DATA;
    }

    pBS->DataLength += nBytesRead;

    return MFX_ERR_NONE;
}


inline bool isPathDelimiter(msdk_char c)
{
    return c == '/' || c == '\\';
}


mfxStatus InitMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize)
{
    //check input params
    MSDK_CHECK_POINTER(pBitstream, MFX_ERR_NULL_PTR);
    MSDK_CHECK_ERROR(nSize, 0, MFX_ERR_NOT_INITIALIZED);

    //prepare pBitstream
    WipeMfxBitstream(pBitstream);

    //prepare buffer
    pBitstream->Data = new mfxU8[nSize];
    MSDK_CHECK_POINTER(pBitstream->Data, MFX_ERR_MEMORY_ALLOC);

    pBitstream->MaxLength = nSize;

    return MFX_ERR_NONE;
}

mfxStatus ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize)
{
    MSDK_CHECK_POINTER(pBitstream, MFX_ERR_NULL_PTR);

    MSDK_CHECK_ERROR(nSize <= pBitstream->MaxLength, true, MFX_ERR_UNSUPPORTED);

    mfxU8* pData = new mfxU8[nSize];
    MSDK_CHECK_POINTER(pData, MFX_ERR_MEMORY_ALLOC);

    memmove(pData, pBitstream->Data + pBitstream->DataOffset, pBitstream->DataLength);

    WipeMfxBitstream(pBitstream);

    pBitstream->Data       = pData;
    pBitstream->DataOffset = 0;
    pBitstream->MaxLength  = nSize;

    return MFX_ERR_NONE;
}

void WipeMfxBitstream(mfxBitstream* pBitstream)
{
    MSDK_CHECK_POINTER(pBitstream);

    //free allocated memory
    MSDK_SAFE_DELETE_ARRAY(pBitstream->Data);
}

std::basic_string<msdk_char> CodecIdToStr(mfxU32 nFourCC)
{
    std::basic_string<msdk_char> fcc;
    for (size_t i = 0; i < 4; i++)
    {
        fcc.push_back((msdk_char)*(i + (char*)&nFourCC));
    }
    return fcc;
}

const msdk_char* ColorFormatToStr(mfxU32 format)
{
    switch(format)
    {
    case MFX_FOURCC_NV12:
        return MSDK_STRING("NV12");
    case MFX_FOURCC_YV12:
        return MSDK_STRING("YUV420");
    default:
        return MSDK_STRING("unsupported");
    }
}

const
struct
{
    // actual implementation
    mfxIMPL impl;
    // adapter's number
    mfxU32 adapterID;

} implTypes[] =
{
    {MFX_IMPL_HARDWARE, 0},
    {MFX_IMPL_SOFTWARE, 0},
    {MFX_IMPL_HARDWARE2, 1},
    {MFX_IMPL_HARDWARE3, 2},
    {MFX_IMPL_HARDWARE4, 3}
};

// returns the number of adapter associated with MSDK session, 0 for SW session
mfxU32 GetMSDKAdapterNumber(mfxSession session)
{
    mfxU32 adapterNum = 0; // default
    mfxIMPL impl = MFX_IMPL_SOFTWARE; // default in case no HW IMPL is found

    // we don't care for error codes in further code; if something goes wrong we fall back to the default adapter
    if (session)
    {
        MFXQueryIMPL(session, &impl);
    }
    else
    {
        // an auxiliary session, internal for this function
        mfxSession auxSession;
        memset(&auxSession, 0, sizeof(auxSession));

        mfxVersion ver = {1, 1}; // minimum API version which supports multiple devices
        MFXInit(MFX_IMPL_HARDWARE_ANY, &ver, &auxSession);
        MFXQueryIMPL(auxSession, &impl);
        MFXClose(auxSession);
    }

    // extract the base implementation type
    mfxIMPL baseImpl = MFX_IMPL_BASETYPE(impl);

    // get corresponding adapter number
    for (mfxU8 i = 0; i < sizeof(implTypes)/sizeof(implTypes[0]); i++)
    {
        if (implTypes[i].impl == baseImpl)
        {
            adapterNum = implTypes[i].adapterID;
            break;
        }
    }

    return adapterNum;
}




msdk_so_handle msdk_so_load(const msdk_char *file_name)
{
	if (!file_name) return NULL;
	return (msdk_so_handle)LoadLibrary((LPCTSTR)file_name);
}

msdk_func_pointer msdk_so_get_addr(msdk_so_handle handle, const char *func_name)
{
	if (!handle) return NULL;
	return (msdk_func_pointer)GetProcAddress((HMODULE)handle, /*(LPCSTR)*/func_name);
}

void msdk_so_free(msdk_so_handle handle)
{
	if (!handle) return;
	FreeLibrary((HMODULE)handle);
}

/////////////////////////////
//atomic.cpp

#define _interlockedbittestandset      fake_set
#define _interlockedbittestandreset    fake_reset
#define _interlockedbittestandset64    fake_set64
#define _interlockedbittestandreset64  fake_reset64
#include <intrin.h>
#undef _interlockedbittestandset
#undef _interlockedbittestandreset
#undef _interlockedbittestandset64
#undef _interlockedbittestandreset64
#pragma intrinsic (_InterlockedIncrement16)
#pragma intrinsic (_InterlockedDecrement16)

mfxU16 msdk_atomic_inc16(volatile mfxU16 *pVariable)
{
	return _InterlockedIncrement16((volatile short*)pVariable);
}

/* Thread-safe 16-bit variable decrementing */
mfxU16 msdk_atomic_dec16(volatile mfxU16 *pVariable)
{
	return _InterlockedDecrement16((volatile short*)pVariable);
}

///////////////////////////////
//thread.cpp
MSDKMutex::MSDKMutex(void)
{
	m_bInitialized = true;
	InitializeCriticalSection(&m_CritSec);
}

MSDKMutex::~MSDKMutex(void)
{
	if (m_bInitialized)
	{
		DeleteCriticalSection(&m_CritSec);
	}
}

mfxStatus MSDKMutex::Lock(void)
{
	mfxStatus sts = MFX_ERR_NONE;
	if (m_bInitialized)
	{
		EnterCriticalSection(&m_CritSec);
	}
	return sts;
}

mfxStatus MSDKMutex::Unlock(void)
{
	mfxStatus sts = MFX_ERR_NONE;
	if (m_bInitialized)
	{
		LeaveCriticalSection(&m_CritSec);
	}
	return sts;
}

int MSDKMutex::Try(void)
{
	int res = 0;
	if (m_bInitialized)
	{
		res = TryEnterCriticalSection(&m_CritSec);
	}
	return res;
}

AutomaticMutex::AutomaticMutex(MSDKMutex& mutex)
{
	m_pMutex = &mutex;
	m_bLocked = false;
	Lock();
};
AutomaticMutex::~AutomaticMutex(void)
{
	Unlock();
}

void AutomaticMutex::Lock(void)
{
	if (!m_bLocked)
	{
		if (!m_pMutex->Try())
		{
			m_pMutex->Lock();
		}
		m_bLocked = true;
	}
}

void AutomaticMutex::Unlock(void)
{
	if (m_bLocked)
	{
		m_pMutex->Unlock();
		m_bLocked = false;
	}
}

MSDKEvent::MSDKEvent(mfxStatus &sts, bool manual, bool state)
{
	sts = MFX_ERR_NONE;
	m_event = CreateEvent(NULL, manual, state, NULL);
	if (!m_event) sts = MFX_ERR_UNKNOWN;
}

MSDKEvent::~MSDKEvent(void)
{
	if (m_event) CloseHandle(m_event);
}

void MSDKEvent::Signal(void)
{
	if (m_event) SetEvent(m_event);
}

void MSDKEvent::Reset(void)
{
	if (m_event) ResetEvent(m_event);
}

void MSDKEvent::Wait(void)
{
	if (m_event) WaitForSingleObject(m_event, MFX_INFINITE);
}

mfxStatus MSDKEvent::TimedWait(mfxU32 msec)
{
	if (MFX_INFINITE == msec) return MFX_ERR_UNSUPPORTED;
	mfxStatus mfx_res = MFX_ERR_NOT_INITIALIZED;
	if (m_event)
	{
		DWORD res = WaitForSingleObject(m_event, msec);
		if (WAIT_OBJECT_0 == res) mfx_res = MFX_ERR_NONE;
		else if (WAIT_TIMEOUT == res) mfx_res = MFX_TASK_WORKING;
		else mfx_res = MFX_ERR_UNKNOWN;
	}
	return mfx_res;
}

MSDKSema::MSDKSema(mfxStatus &sts, long init_cnt, long max_cnt)
{
	sts = MFX_ERR_NONE;
	m_sema = CreateSemaphore(NULL, init_cnt, max_cnt, NULL);
	if (!m_sema) sts = MFX_ERR_UNKNOWN;
}

MSDKSema::~MSDKSema()
{
	if (m_sema) CloseHandle(m_sema);
}

DWORD MSDKSema::Wait(void)
{
	DWORD ret = 0;
	if (m_sema) ret = WaitForSingleObject(m_sema, MFX_INFINITE);

	return ret;
}

BOOL MSDKSema::Signal(int cnt)
{
	BOOL ret=TRUE;
	if (m_sema) {
		ret = ReleaseSemaphore(m_sema, cnt, NULL);
	}
	return ret;
}

MSDKThread::MSDKThread(mfxStatus &sts, msdk_thread_callback func, void* arg)
{
	sts = MFX_ERR_UNKNOWN;
	m_thread = (void*)_beginthreadex(NULL, 0, func, arg, 0, NULL);
	if (m_thread) sts = MFX_ERR_NONE;
}

MSDKThread::~MSDKThread(void)
{
	if (m_thread) CloseHandle(m_thread);
}

void MSDKThread::Wait(void)
{
	if (m_thread) WaitForSingleObject(m_thread, MFX_INFINITE);
}

mfxStatus MSDKThread::TimedWait(mfxU32 msec)
{
	if (MFX_INFINITE == msec) return MFX_ERR_UNSUPPORTED;
	mfxStatus mfx_res = MFX_ERR_NONE;
	if (m_thread)
	{
		DWORD res = WaitForSingleObject(m_thread, msec);
		if (WAIT_OBJECT_0 == res) mfx_res = MFX_ERR_NONE;
		else if (WAIT_TIMEOUT == res) mfx_res = MFX_TASK_WORKING;
		else mfx_res = MFX_ERR_UNKNOWN;
	}
	return mfx_res;
}

mfxStatus MSDKThread::GetExitCode()
{
	mfxStatus mfx_res = MFX_ERR_NOT_INITIALIZED;
	if (m_thread)
	{
		DWORD code = 0;
		int sts = 0;
		sts = GetExitCodeThread(m_thread, &code);
		if (sts == 0) mfx_res = MFX_ERR_UNKNOWN;
		else if (STILL_ACTIVE == code) mfx_res = MFX_TASK_WORKING;
		else mfx_res = MFX_ERR_NONE;
	}
	return mfx_res;
}



