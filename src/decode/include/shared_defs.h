/* ****************************************************************************** *\

INTEL CORPORATION PROPRIETARY INFORMATION
This software is supplied under the terms of a license agreement or nondisclosure
agreement with Intel Corporation and may not be copied or disclosed except in
accordance with the terms of that agreement
Copyright(c) 2012 Intel Corporation. All Rights Reserved.

\* ****************************************************************************** */

#ifndef __SO_DEFS_H__
#define __SO_DEFS_H__

#include "mfxdefs.h"
/////////////////////////////////////////////////////////

#ifndef __SAMPLE_DEFS_H__
#define __SAMPLE_DEFS_H__

#include <memory.h>

#include "mfxdefs.h"


#ifndef D3D_SURFACES_SUPPORT
#define D3D_SURFACES_SUPPORT 1
#endif

#if defined(_WIN32) && !defined(MFX_D3D11_SUPPORT)
#include <sdkddkver.h>
//#if (NTDDI_VERSION >= NTDDI_VERSION_FROM_WIN32_WINNT2(0x0602)) // >= _WIN32_WINNT_WIN8
//    #define MFX_D3D11_SUPPORT 1 // Enable D3D11 support if SDK allows
//#else
#define MFX_D3D11_SUPPORT 1
#define D3DNOTVIADXUT
//#endif
#endif //defined(WIN32) || defined(WIN64)


#define MSDK_DEC_WAIT_INTERVAL 60000
#define MSDK_ENC_WAIT_INTERVAL 10000
#define MSDK_VPP_WAIT_INTERVAL 60000
#define MSDK_WAIT_INTERVAL MSDK_DEC_WAIT_INTERVAL+3*MSDK_VPP_WAIT_INTERVAL+MSDK_ENC_WAIT_INTERVAL // an estimate for the longest pipeline we have in samples

#define MSDK_INVALID_SURF_IDX 0xFFFF

#define MSDK_MAX_FILENAME_LEN 1024

#define MSDK_PRINT_RET_MSG(ERR) {msdk_printf(MSDK_STRING("\nReturn on error: error code %d,\t%s\t%d\n\n"), ERR, MSDK_STRING(__FILE__), __LINE__);}

#define MSDK_CHECK_ERROR(P, X, ERR)              {if ((X) == (P)) {MSDK_PRINT_RET_MSG(ERR); return ERR;}}
#define MSDK_CHECK_NOT_EQUAL(P, X, ERR)          {if ((X) != (P)) {MSDK_PRINT_RET_MSG(ERR); return ERR;}}
#define MSDK_CHECK_RESULT(P, X, ERR)             {if ((X) > (P)) {MSDK_PRINT_RET_MSG(ERR); return ERR;}}
#define MSDK_CHECK_PARSE_RESULT(P, X, ERR)       {if ((X) > (P)) {return ERR;}}
#define MSDK_CHECK_RESULT_SAFE(P, X, ERR, ADD)   {if ((X) > (P)) {ADD; MSDK_PRINT_RET_MSG(ERR); return ERR;}}
#define MSDK_IGNORE_MFX_STS(P, X)                {if ((X) == (P)) {P = MFX_ERR_NONE;}}
#define MSDK_CHECK_POINTER(P, ...)               {if (!(P)) {return __VA_ARGS__;}}
#define MSDK_CHECK_POINTER_NO_RET(P)             {if (!(P)) {return;}}
#define MSDK_CHECK_POINTER_SAFE(P, ERR, ADD)     {if (!(P)) {ADD; return ERR;}}
#define MSDK_BREAK_ON_ERROR(P)                   {if (MFX_ERR_NONE != (P)) break;}
#define MSDK_SAFE_DELETE_ARRAY(P)                {if (P) {delete[] P; P = NULL;}}
#define MSDK_SAFE_RELEASE(X)                     {if (X) { X->Release(); X = NULL; }}
#define MSDK_SAFE_FREE(X)                        {if (X) { free(X); X = NULL; }}

#ifndef MSDK_SAFE_DELETE
#define MSDK_SAFE_DELETE(P)                      {if (P) {delete P; P = NULL;}}
#endif // MSDK_SAFE_DELETE

#define MSDK_ZERO_MEMORY(VAR)                    {memset(&VAR, 0, sizeof(VAR));}
#define MSDK_MAX(A, B)                           (((A) > (B)) ? (A) : (B))
#define MSDK_MIN(A, B)                           (((A) < (B)) ? (A) : (B))
#define MSDK_ALIGN16(value)                      (((value + 15) >> 4) << 4) // round up to a multiple of 16
#define MSDK_ALIGN32(value)                      (((value + 31) >> 5) << 5) // round up to a multiple of 32
#define MSDK_ALIGN(value, alignment)             (alignment) * ( (value) / (alignment) + (((value) % (alignment)) ? 1 : 0))
#define MSDK_ARRAY_LEN(value)                    (sizeof(value) / sizeof(value[0]))

#define MSDK_MEMCPY_BITSTREAM(bitstream, offset, src, count) memcpy_s((bitstream).Data + (offset), (bitstream).MaxLength - (offset), (src), (count))

#define MSDK_MEMCPY_BUF(bufptr, offset, maxsize, src, count) memcpy_s((bufptr)+ (offset), (maxsize) - (offset), (src), (count))

#define MSDK_MEMCPY_VAR(dstVarName, src, count) memcpy_s(&(dstVarName), sizeof(dstVarName), (src), (count))

#ifndef UNREFERENCED_PARAMETER
#define UNREFERENCED_PARAMETER(par) (par)
#endif

#ifndef MFX_PRODUCT_VERSION
#define MFX_PRODUCT_VERSION "4.0.760.60435"
#endif

#define MSDK_SAMPLE_VERSION MSDK_STRING(MFX_PRODUCT_VERSION)

#endif //__SAMPLE_DEFS_H__

///////////////////////////////////////////////////////

#ifndef __STRING_DEFS_H__
#define __STRING_DEFS_H__

#include <stdio.h>
#include <stdlib.h>
#include <memory.h>

#include <tchar.h>

#define MSDK_STRING(x) _T(x)
#define MSDK_CHAR(x) _T(x)
typedef TCHAR msdk_char;

#define msdk_printf   _tprintf
#define msdk_fprintf  _ftprintf
#define msdk_sprintf  _stprintf_s
#define msdk_vprintf  _vtprintf
#define msdk_strlen   _tcslen
#define msdk_strcmp   _tcscmp
#define msdk_strncmp  _tcsnicmp
#define msdk_strstr   _tcsstr
#define msdk_sscanf   _stscanf_s
#define msdk_atoi     _ttoi
#define msdk_strtol   _tcstol
#define msdk_strtod   _tcstod
#define msdk_strchr   _tcschr
#define msdk_itoa_decimal(value, str)   _itow_s(value, str, 4, 10)

// msdk_strcopy is intended to be used with 2 parmeters, i.e. msdk_strcopy(dst, src)
// for _tcscpy_s that's possible if DST is declared as: TCHAR DST[n];
#define msdk_strcopy _tcscpy_s

#endif //__STRING_DEFS_H__

#endif // #ifndef __SO_DEFS_H__

