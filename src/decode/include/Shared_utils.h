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

#ifndef __SAMPLE_UTILS_H__
#define __SAMPLE_UTILS_H__


#include <stdio.h>
#include <string>
#include <vector>

#include "mfxstructures.h"
#include "mfxvideo.h"

#include "shared_defs.h"


class CSmplBitstreamReader
{
public :

    CSmplBitstreamReader();
    virtual ~CSmplBitstreamReader();

    //resets position to file begin
    virtual void      Reset();
    virtual void      Close();
    virtual mfxStatus Init(const msdk_char *strFileName);
    virtual mfxStatus ReadNextFrame(mfxBitstream *pBS);

protected:
    FILE*     m_fSource;
    bool      m_bInited;
};


#include "windows.h"


mfxStatus InitMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize);
mfxStatus ExtendMfxBitstream(mfxBitstream* pBitstream, mfxU32 nSize);

void WipeMfxBitstream(mfxBitstream* pBitstream);

//serialization fnc set
std::basic_string<msdk_char> CodecIdToStr(mfxU32 nFourCC);
const msdk_char* ColorFormatToStr(mfxU32 format);


// returns the number of adapter associated with MSDK session, 0 for SW session
mfxU32 GetMSDKAdapterNumber(mfxSession session = 0);

#endif //__SAMPLE_UTILS_H__

/////////////////////////////////////////////////
//////////////////////////////////////////////////

/* Declare shared object handle */
typedef void * msdk_so_handle;
typedef void(*msdk_func_pointer)(void);

msdk_so_handle msdk_so_load(const msdk_char *file_name);
msdk_func_pointer msdk_so_get_addr(msdk_so_handle handle, const char *func_name);
void msdk_so_free(msdk_so_handle handle);


#ifndef __ATOMIC_DEFS_H__
#define __ATOMIC_DEFS_H__

#include "mfxdefs.h"

/* Thread-safe 16-bit variable incrementing */
mfxU16 msdk_atomic_inc16(volatile mfxU16 *pVariable);

/* Thread-safe 16-bit variable decrementing */
mfxU16 msdk_atomic_dec16(volatile mfxU16 *pVariable);

#endif // #ifndef __ATOMIC_DEFS_H__

/////////////////////////////////////////////////

#ifndef __FILE_DEFS_H__
#define __FILE_DEFS_H__

#include "mfxdefs.h"

#include <stdio.h>

#define MSDK_FOPEN(file, name, mode) _tfopen_s(&file, name, mode)

#define msdk_fgets  _fgetts

#endif // #ifndef __FILE_DEFS_H__

/////////////////////////////////////////////////

#ifndef __TIME_DEFS_H__
#define __TIME_DEFS_H__

#include "mfxdefs.h"

#include <Windows.h>
#define MSDK_SLEEP(msec) Sleep(msec)

#endif // #ifndef __TIME_DEFS_H__

//////////////////////////////////////////////////

#ifndef __THREAD_DEFS_H__
#define __THREAD_DEFS_H__

#include "mfxdefs.h"
#include <windows.h>
#include <process.h>

class MSDKMutex
{
public:
	MSDKMutex(void);
	~MSDKMutex(void);

	mfxStatus Lock(void);
	mfxStatus Unlock(void);
	int Try(void);

private:
	bool m_bInitialized;
	CRITICAL_SECTION m_CritSec;
};

class AutomaticMutex
{
public:
	AutomaticMutex(MSDKMutex& mutex);
	~AutomaticMutex(void);

private:
	void Lock(void);
	void Unlock(void);

	MSDKMutex* m_pMutex;
	bool m_bLocked;
};

class MSDKEvent
{
public:
	MSDKEvent(mfxStatus &sts, bool manual, bool state);
	~MSDKEvent(void);

	void Signal(void);
	void Reset(void);
	void Wait(void);
	mfxStatus TimedWait(mfxU32 msec);

private:
	void* m_event;
};

class MSDKSema
{
public:
	MSDKSema(mfxStatus &sts, long init_cnt, long max_cnt);
	~MSDKSema(void);

	BOOL Signal(int cnt=1);
	void Reset(void) {};
	DWORD Wait(void);
	//mfxStatus TimedWait(mfxU32 msec);

private:
	void* m_sema;
};

#define MSDK_THREAD_CALLCONVENTION __stdcall

typedef unsigned int (MSDK_THREAD_CALLCONVENTION * msdk_thread_callback)(void*);

class MSDKThread
{
public:
	MSDKThread(mfxStatus &sts, msdk_thread_callback func, void* arg);
	~MSDKThread(void);

	void Wait(void);
	mfxStatus TimedWait(mfxU32 msec);
	mfxStatus GetExitCode();

//protected:
	void* m_thread;
};

#endif //__THREAD_DEFS_H__