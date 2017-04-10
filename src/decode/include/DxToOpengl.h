
#pragma once

#pragma warning(disable : 4201)
#include "GL/glew.h"
#include "GL/wglew.h"


#include <windows.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <iostream>
#include <fstream>

#include "shared_defs.h"

#include "hw_device.h"
#include "pipeline_decode.h"
#include "shared_defs.h"

#define PUSH_FRAME(mLock, mList, ptrFrame) \
{\
	mLock.Lock();\
	mList.push_back((void *)ptrFrame);\
	mLock.Unlock(); \
}

#define POP_FRAME(mLock, mList, ptrFrame) \
{\
	mLock.Lock();\
	ptrFrame = NULL;\
	if( !(mList.empty()) ) {\
		ptrFrame = (void *)*(mList.begin());\
		mList.pop_front();\
	}\
	mLock.Unlock(); \
}

#define EMPTY_FRAME(mLock, mList) \
{\
	mLock.Lock();\
	mList.clear(); \
	mLock.Unlock(); \
}

#define USESHADERFILE 1
//update:0408
#define MAXVIDEOCLIPS 9
#define NUMVIDEOROW 3
#define NUMVIDEOCOL 3

class CDecoderInstance
{
public:
	CDecoderInstance();
	~CDecoderInstance();

	mfxStatus SetupInstance(structDecoderParams *param, HANDLE);
	mfxStatus SetupInstanceForVppResize(structDecoderParams *param, HANDLE);
	mfxStatus CloseInstance(HANDLE hShareDxOpenglDevice);

	mfxStatus BindTexture(HANDLE);
	mfxStatus UnbindTexture(HANDLE);

	mfxStatus RunDec();
	mfxStatus RunVpp();
	void *	  PopupFrameForDisplay();
	void	  ClearAllFrameList();

	UINT32	  GetVppFrameListSize() { return (UINT32)(m_vppFrameList.size()); }
	GLuint    GetTextureVppFrame(int no) { return m_nVppOutSurfaceNums ? m_pOpenglTextureArray[no] : 0; }
	HANDLE    GetSharedHandleVppFrame(int no) { return m_nVppOutSurfaceNums ? m_pDxSharedTextureHandleArray[no] : 0; }


	CDecodingPipeline *	m_pDecoder;
	UINT32				m_nVppOutSurfaceNums;
	MemType				m_eMemType;

	UINT32				m_nSpeedx;

	void **				m_pVppOutSurfaceArray;
	HANDLE*				m_pVppOutSurfaceHandleArray;
	GLuint*				m_pOpenglTextureArray;
	HANDLE*				m_pDxSharedTextureHandleArray;

	MSDKMutex			m_decFrameListLock, m_vppFrameListLock;
	std::list<void*>	m_decFrameList, m_vppFrameList;
};

enum E_DISPLAY_MOVE_DIR
{
	M_LEFT = 0x1,
	M_RIGHT,
	M_UP,
	M_DOWN
};

struct CGLShaderParameters
{

	float xoffsetcurrent;
	float yoffsetcurrent;
	float offsetincrement;
	float apptexcoordscalar;
	float appxtexcoordmultiplier;
	float appytexcoordmultiplier;
	int   appnumvideos;
	float nZoom;

	CGLShaderParameters()
	{
		xoffsetcurrent = 0.0f;
		yoffsetcurrent = 0.0f;
		offsetincrement = 0.01f;
		apptexcoordscalar = 100000.0f;
		appxtexcoordmultiplier = (float)NUMVIDEOROW;
		appytexcoordmultiplier = (float)NUMVIDEOCOL;
		//update:0408
		//appnumvideos = 13;
		appnumvideos = MAXVIDEOCLIPS;
		nZoom = 1.0;

	}

	void SetZoom(float fZoom) { nZoom += fZoom; }
	void SetMoveOffset(E_DISPLAY_MOVE_DIR eDir, float fOffset) {
		if (eDir == M_LEFT) xoffsetcurrent -= fOffset;
		if (eDir == M_RIGHT) xoffsetcurrent += fOffset;
		if (eDir == M_UP) yoffsetcurrent -= fOffset;
		if (eDir == M_DOWN) yoffsetcurrent += fOffset;
	}

};

class COpenglDisplayD3D
{
public:
	COpenglDisplayD3D();
	~COpenglDisplayD3D();

	mfxStatus BindWindowDC_Opengl(const char *path_vert_shader, const char *path_frag_shader);

	//for shader
	bool	createshaders(const char *path_vert_shader, const char *path_frag_shader);
	GLuint	read_shader_src(const char *fname, std::vector<char> &buffer);
	GLuint	load_and_compile_shader(const char *fname, GLenum shaderType);
	GLuint	create_program(const char *path_vert_shader, const char *path_frag_shader);

	void	initialize(GLuint shaderProgram, GLuint &vao);

	void	SetupGlViewPort(GLint x, GLint y, GLsizei width, GLsizei height);

	mfxStatus	Display();
	mfxStatus	Decoding();
	mfxStatus	Vpping();
	mfxStatus   Close();
	mfxStatus   SetupDisplayMode(UINT32 decoderNums);
	//for decoder
	//mfxStatus BindDecoder(int decoderNums, structDecoderParams* inDecoderParams);
	//mfxStatus InitDecoder();
	//mfxStatus AddDecoder(int decoderNums, structDecoderParams* inDecoderParams);
	//mfxStatus DelDecoder(int decoderNo);
	//mfxStatus ResizeVppOutput(int decoderNo, mfxU16 width, mfxU16 height);
	mfxStatus CreateDecoders(UINT32 decoderNums);
	mfxStatus UpdateDecoders(UINT32 firstDecoder, UINT32 decoderNums);

	//decoder buffer manage


	//for D3D
	HRESULT InitD3DDevice(HWND inHwnd, MemType);

	static unsigned int MFX_STDCALL dec_thread0(void* pThis);
	static unsigned int MFX_STDCALL vpp_thread0(void* pThis);
	static unsigned int MFX_STDCALL dis_thread0(void* pThis);

	mfxStatus ThreadFuncLoop_Dec();
	mfxStatus ThreadFuncLoop_Vpp();
	mfxStatus ThreadFuncLoop_Dis();

	void CreateTask();
	void WaitTaskEnd();

protected:
	HWND		m_hInHwnd;
	HDC			m_hDC;
	HGLRC		m_hRC;

	UINT32		m_viewWidth, m_viewHeight;

	MemType     m_nMemType;
	SharedDXDevice	*m_pShareD3DDevice;

	HANDLE		m_hShareDxOpenglDevice;

	GLuint		m_glvertexshader;
	GLuint		m_glfragmentshader;
	GLuint		m_shader_programme;
	GLuint		m_hVao;
	GLuint		m_hVbo;
	GLuint      m_hEab; //elements array buffer

	UINT32		m_nDecNums;
	std::vector<CDecoderInstance *>m_pDecArray;


	MSDKSema *m_eResizeFromOpenglToVpp, *m_eResizeFromDecToVpp;
	MSDKSema *m_eResizeFromVppToDec, *m_eResizeFromVppToOpengl;
	MSDKThread *m_tDec, *m_tVpp, *m_tDis;
	BOOL exit_app, exit_disp ,exit_vpp;
	BOOL m_bWillUpdate0, m_bWillUpdate1;

public:
	//for test:opengl
	GLuint testdtexArray[MAXVIDEOCLIPS];

	GLuint testxoffset;					
	GLuint testyoffset;					

	GLuint texcoordscalar;
	GLuint xtexcoordmultiplier;
	GLuint ytexcoordmultiplier;
	GLuint numvideos;

	CGLShaderParameters m_glParameters;

	void settingNewDisplaySize(float xVal, float yVal);

	//for decoder para
	structDecoderParams *m_decoderParams[MAXVIDEOCLIPS];			/*!< Global pointer array for decode configuration. */

	void initDecoderParams();
	void updateDecoderParams();
	void cleanDecoderParams();

};