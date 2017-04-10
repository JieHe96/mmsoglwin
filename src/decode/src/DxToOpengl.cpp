

#include "DxToOpengl.h"


CDecoderInstance::CDecoderInstance()
{
	m_pDecoder = NULL;
	m_nVppOutSurfaceNums = 0;
	m_eMemType = D3D11_MEMORY;

	m_nSpeedx = 1;

	m_pVppOutSurfaceArray = 0;
	m_pVppOutSurfaceHandleArray = NULL;
	m_pOpenglTextureArray = NULL;
	m_pDxSharedTextureHandleArray = NULL;
}

CDecoderInstance::~CDecoderInstance()
{

}

mfxStatus CDecoderInstance::CloseInstance(HANDLE hShareDxOpenglDevice)
{
	mfxStatus sts = MFX_ERR_NONE;

	m_decFrameList.clear();
	m_vppFrameList.clear();

	UnbindTexture(hShareDxOpenglDevice);

	MSDK_SAFE_DELETE(m_pDecoder);

	MSDK_SAFE_DELETE_ARRAY(m_pVppOutSurfaceArray);
	MSDK_SAFE_DELETE_ARRAY(m_pVppOutSurfaceHandleArray);
	MSDK_SAFE_DELETE_ARRAY(m_pOpenglTextureArray);
	MSDK_SAFE_DELETE_ARRAY(m_pDxSharedTextureHandleArray);


	return sts;
}

mfxStatus CDecoderInstance::BindTexture(HANDLE hShareDxOpenglDevice)
{
	GLuint tmpTextureArray[40];
	glGenTextures(m_nVppOutSurfaceNums, tmpTextureArray);

	if (!WGL_NV_DX_interop2) return  MFX_ERR_UNSUPPORTED;

	if (m_eMemType == D3D9_MEMORY)
	{
		for (UINT32 i = 0; i < m_nVppOutSurfaceNums; i++)
		{
			mfxHDLPair aPair;
			m_pOpenglTextureArray[i] = tmpTextureArray[i];
			(mfxHDLPair *)m_pDecoder->GetVppSuface9Ptr(i, aPair);
			m_pVppOutSurfaceArray[i] = aPair.first;
			m_pVppOutSurfaceHandleArray[i] = aPair.second;
			wglDXSetResourceShareHandleNV(m_pVppOutSurfaceArray[i], m_pVppOutSurfaceHandleArray[i]);
			m_pDxSharedTextureHandleArray[i] = wglDXRegisterObjectNV(hShareDxOpenglDevice,
				m_pVppOutSurfaceArray[i], m_pOpenglTextureArray[i], GL_TEXTURE_2D, WGL_ACCESS_READ_ONLY_NV);

			if (m_pDxSharedTextureHandleArray[i] == 0) return MFX_ERR_NULL_PTR;
		}
	}
	else
	{
		for (UINT32 i = 0; i < m_nVppOutSurfaceNums; i++)
		{
			m_pOpenglTextureArray[i] = tmpTextureArray[i];
			m_pVppOutSurfaceArray[i] = m_pDecoder->GetVppSufacePtr(i);
			m_pDxSharedTextureHandleArray[i] = wglDXRegisterObjectNV(hShareDxOpenglDevice,
				m_pVppOutSurfaceArray[i], m_pOpenglTextureArray[i], GL_TEXTURE_2D, WGL_ACCESS_READ_ONLY_NV);

			if (m_pDxSharedTextureHandleArray[i] == 0) return MFX_ERR_NULL_PTR;
		}
	}

	return MFX_ERR_NONE;
}

mfxStatus CDecoderInstance::UnbindTexture(HANDLE hShareDxOpenglDevice)
{
	if (!WGL_NV_DX_interop2) return  MFX_ERR_UNSUPPORTED;
	for (UINT32 i = 0; i < m_nVppOutSurfaceNums; i++)
	{
		if (m_pDxSharedTextureHandleArray[i]) {
			//???handle leak???
			//wglDXUnlockObjectsNV(hShareDxOpenglDevice, 1, &m_pDxSharedTextureHandleArray[i]);

			wglDXUnregisterObjectNV(hShareDxOpenglDevice, m_pDxSharedTextureHandleArray[i]);
			m_pDxSharedTextureHandleArray[i] = NULL;
			
			DIRECTX_SAFE_RELEASE(m_pVppOutSurfaceArray[i]);
			//m_pVppOutSurfaceArray[i] = NULL;

			glDeleteTextures(1, &m_pOpenglTextureArray[i]);
			m_pOpenglTextureArray[i] = 0;
		}
	}

	return MFX_ERR_NONE;
}

mfxStatus CDecoderInstance::SetupInstance(structDecoderParams* param, HANDLE hShareDxOpenglDevice)
{
	mfxStatus sts = MFX_ERR_NONE;

	m_eMemType = param->memType;
	m_pDecoder = new CDecodingPipeline();
	sts = m_pDecoder->Init(param);

	if (sts == MFX_ERR_NONE)
	{
		m_nVppOutSurfaceNums = m_pDecoder->GetVppOutSurfaceNums();
		if (m_nVppOutSurfaceNums == 0) return MFX_ERR_NULL_PTR;

		m_pVppOutSurfaceArray = new void *[m_nVppOutSurfaceNums];
		m_pVppOutSurfaceHandleArray = new HANDLE[m_nVppOutSurfaceNums];
		m_pOpenglTextureArray = new GLuint[m_nVppOutSurfaceNums];
		m_pDxSharedTextureHandleArray = new HANDLE[m_nVppOutSurfaceNums];

		sts = BindTexture(hShareDxOpenglDevice);
	}

	return sts;
}

mfxStatus CDecoderInstance::SetupInstanceForVppResize(structDecoderParams *param, HANDLE hShareDxOpenglDevice)
{
	mfxStatus sts = MFX_ERR_NONE;
	EMPTY_FRAME(m_vppFrameListLock, m_vppFrameList);

	sts = UnbindTexture(hShareDxOpenglDevice);

	if (sts == MFX_ERR_NONE)
	{
		sts = m_pDecoder->RecreateVppForResize(param->vppOutWidth, param->vppOutHeight);
		if (sts != MFX_ERR_NONE) return sts;

		sts = BindTexture(hShareDxOpenglDevice);
	}

	return sts;
}

mfxStatus CDecoderInstance::RunDec()
{
	mfxStatus sts = MFX_ERR_NONE;

	void *videoframeptrs = NULL;

	for (UINT32 i = 0; i < m_nSpeedx; i++)
	{
		void *tmp = NULL;
		tmp = m_pDecoder->RunSingleFrameDecode(sts);
		videoframeptrs = tmp ? tmp : videoframeptrs;
	}

	if (videoframeptrs==NULL) return sts;
	
	((mfxFrameSurface1 *)videoframeptrs)->Data.reserved[8] = 1;

	PUSH_FRAME(m_decFrameListLock, m_decFrameList, videoframeptrs);

	return sts;
}

mfxStatus CDecoderInstance::RunVpp()
{
	mfxStatus sts = MFX_ERR_NONE;

	void *vppinframeptrs = NULL;
	void *vppoutframeptrs = NULL;

	POP_FRAME(m_decFrameListLock, m_decFrameList, vppinframeptrs);

	if(vppinframeptrs == NULL) return MFX_ERR_NULL_PTR;

	vppoutframeptrs = (mfxFrameSurface1 *)(m_pDecoder->RunSingleFrameVPP((mfxFrameSurface1*)vppinframeptrs, sts));
	if (vppoutframeptrs == NULL) return sts;

	((mfxFrameSurface1 *)vppoutframeptrs)->Data.reserved[8] = 1;
	((mfxFrameSurface1 *)vppinframeptrs)->Data.reserved[8] = 0;
	if(((mfxFrameSurface1 *)vppinframeptrs)->Data.Locked == 0)
		m_pDecoder->PushDecSurfaceList((mfxFrameSurface1 *)vppinframeptrs);

	PUSH_FRAME(m_vppFrameListLock, m_vppFrameList, vppoutframeptrs)


	return sts;
}

void * CDecoderInstance::PopupFrameForDisplay()
{
	void *vppoutframeptrs = NULL;

	POP_FRAME(m_vppFrameListLock, m_vppFrameList, vppoutframeptrs);
	
	//if(vppoutframeptrs)
	//	((mfxFrameSurface1 *)vppoutframeptrs)->Data.reserved[8] = 0;

	return vppoutframeptrs;
}

void CDecoderInstance::ClearAllFrameList()
{
	void *videoframeptrs = NULL;
	void *vppoutframeptrs = NULL;

	do{
		POP_FRAME(m_decFrameListLock, m_decFrameList, videoframeptrs);
		if (videoframeptrs)
		{
			((mfxFrameSurface1 *)videoframeptrs)->Data.reserved[8] = 0;
			if (((mfxFrameSurface1 *)videoframeptrs)->Data.Locked == 0)
				m_pDecoder->PushDecSurfaceList((mfxFrameSurface1 *)videoframeptrs);
		}
	} while (videoframeptrs != NULL);

	do {
		POP_FRAME(m_vppFrameListLock, m_vppFrameList, vppoutframeptrs);
		if (vppoutframeptrs)
		{
			((mfxFrameSurface1 *)vppoutframeptrs)->Data.reserved[8] = 0;
			if (((mfxFrameSurface1 *)vppoutframeptrs)->Data.Locked == 0)
				m_pDecoder->PushVppSurfaceList((mfxFrameSurface1 *)vppoutframeptrs);
		}
	} while (vppoutframeptrs != NULL);
}

COpenglDisplayD3D::COpenglDisplayD3D()
{
	m_pShareD3DDevice = NULL;
	m_hShareDxOpenglDevice = NULL;
	m_hInHwnd = NULL;
	m_hDC = NULL;
	m_hRC = NULL;
	m_nMemType = D3D11_MEMORY;

	m_glvertexshader = 0;
	m_glfragmentshader = 0;
	m_shader_programme = 0;
	m_hVao = 0;
	m_hVbo = 0;

	m_tDec = m_tVpp = m_tDis = NULL;

}

COpenglDisplayD3D::~COpenglDisplayD3D()
{
	wglMakeCurrent(m_hDC, m_hRC);
	
	for (UINT32 i = 0; i < m_nDecNums; i++)
	{
		m_pDecArray[i]->CloseInstance(m_hShareDxOpenglDevice);
		delete m_pDecArray[i];
	}
	m_pDecArray.clear();

	wglDXCloseDeviceNV(m_hShareDxOpenglDevice);
	m_hShareDxOpenglDevice = NULL;

	glDetachShader(m_shader_programme, m_glfragmentshader);
	glDetachShader(m_shader_programme, m_glvertexshader);
	glDeleteProgram(m_shader_programme);
	m_shader_programme = NULL;
	m_glfragmentshader = NULL;
	m_glvertexshader = NULL;

	glDeleteBuffers(1, &m_hEab);
	glDeleteBuffers(1, &m_hVbo);
	glDeleteVertexArrays(1, &m_hVao);

	wglMakeCurrent(NULL, NULL);

	wglDeleteContext(m_hRC);
	ReleaseDC(m_hInHwnd, m_hDC);

	if (m_nMemType == D3D11_MEMORY)
		m_pShareD3DDevice->D3D11DeviceContextFlush();

	delete m_pShareD3DDevice;

	cleanDecoderParams();
}
mfxStatus COpenglDisplayD3D::Close()
{
	return MFX_ERR_NONE;
}

HRESULT COpenglDisplayD3D::InitD3DDevice(HWND inHwnd, MemType mMemType)
{
	m_pShareD3DDevice = new SharedDXDevice();
	m_nMemType = mMemType;
	m_hInHwnd = inHwnd;

	HRESULT hr = S_OK;
	if (m_nMemType == D3D9_MEMORY)
		hr = m_pShareD3DDevice->CreateDevice(0, 0, m_hInHwnd);
	else
		hr = m_pShareD3DDevice->CreateDX11device();

	return hr;
}

mfxStatus COpenglDisplayD3D::CreateDecoders(UINT32 decoderNums)
{
	m_nDecNums = decoderNums;

	wglMakeCurrent(m_hDC, m_hRC);

	for (UINT32 i = 0; i < m_nDecNums; i++)
	{
		CDecoderInstance* decoderInst = new CDecoderInstance();
		if (m_nMemType == D3D11_MEMORY)
			m_decoderParams[i]->m_pParaInDDevice = m_pShareD3DDevice->GetD3D11Device();
		else
			m_decoderParams[i]->m_pParaInDDevice = m_pShareD3DDevice->GetD3D9Device();

		decoderInst->SetupInstance(m_decoderParams[i], m_hShareDxOpenglDevice);
		//test
		decoderInst->m_nSpeedx = 1 +(i % 2);
		m_pDecArray.push_back(decoderInst);
	}

	wglMakeCurrent(NULL, NULL);

	return MFX_ERR_NONE;
}

mfxStatus COpenglDisplayD3D::UpdateDecoders(UINT32 firstDecoder, UINT32 decoderNums)
{
	mfxStatus sts = MFX_ERR_NONE;

	if (firstDecoder + decoderNums >= m_nDecNums) return MFX_ERR_NULL_PTR;

	for (UINT32 i = 0; i < m_nDecNums; i++)
	{
		m_pDecArray[i]->ClearAllFrameList();
	}

	wglMakeCurrent(m_hDC, m_hRC);
	for (UINT32 i = firstDecoder; i < firstDecoder + decoderNums; i++)
	{
		m_pDecArray[i]->CloseInstance(m_hShareDxOpenglDevice);
		delete m_pDecArray[i];
		m_pDecArray[i] = new CDecoderInstance();
		if (m_nMemType == D3D11_MEMORY)
			m_decoderParams[i]->m_pParaInDDevice = m_pShareD3DDevice->GetD3D11Device();
		else
			m_decoderParams[i]->m_pParaInDDevice = m_pShareD3DDevice->GetD3D9Device();

		m_pDecArray[i]->SetupInstance(m_decoderParams[i], m_hShareDxOpenglDevice);
		//test
		m_pDecArray[i]->m_nSpeedx = 1 +(i % 2);
	}
	wglMakeCurrent(NULL, NULL);

	return sts;
}

mfxStatus COpenglDisplayD3D::BindWindowDC_Opengl(const char *path_vert_shader, const char *path_frag_shader)
{
	int         pf;

	if (m_hInHwnd == NULL) {
		return MFX_ERR_NULL_PTR;
	}
	m_hDC = GetDC(m_hInHwnd);
	RECT rcClient;
	GetClientRect(m_hInHwnd, &rcClient);
	m_viewWidth = rcClient.right - rcClient.left + 1;
	m_viewHeight = rcClient.bottom - rcClient.top + 1;

	static	PIXELFORMATDESCRIPTOR pfd =
	{
		sizeof(PIXELFORMATDESCRIPTOR),  1,
		PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER,
		PFD_TYPE_RGBA,
		32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 16, 0, 0,
		PFD_MAIN_PLANE,
		0, 0, 0, 0
	};
	pf = ChoosePixelFormat(m_hDC, &pfd);
	if (pf == 0) 
	{
		ReleaseDC(m_hInHwnd, m_hDC);
		return MFX_ERR_UNSUPPORTED;
	}
	if (SetPixelFormat(m_hDC, pf, &pfd) == FALSE) 
	{
		ReleaseDC(m_hInHwnd, m_hDC);
		return MFX_ERR_UNSUPPORTED;
	}
	int ret=DescribePixelFormat(m_hDC, pf, sizeof(PIXELFORMATDESCRIPTOR), &pfd);

	if (!ret)
	{
		ReleaseDC(m_hInHwnd, m_hDC);
		return MFX_ERR_UNSUPPORTED;
	}

	m_hRC = wglCreateContext(m_hDC);

	wglMakeCurrent(m_hDC, m_hRC);
	glewExperimental = true; // Needed for core profile
	GLenum err = glewInit();
	if (err != GLEW_OK) throw std::runtime_error("glewInit failed");

	//GLboolean ver40 = GLEW_VERSION_4_4;

	if(m_nMemType == D3D9_MEMORY)
		m_hShareDxOpenglDevice = wglDXOpenDeviceNV(m_pShareD3DDevice->GetD3D9Device());
	else
		m_hShareDxOpenglDevice = wglDXOpenDeviceNV(m_pShareD3DDevice->GetD3D11Device());

	wglMakeCurrent(NULL, NULL);

	if (!m_hShareDxOpenglDevice) return MFX_ERR_UNSUPPORTED;

	bool bRet = createshaders(path_vert_shader, path_frag_shader);

	if (!bRet) return MFX_ERR_UNSUPPORTED;

	return MFX_ERR_NONE;
}

void COpenglDisplayD3D::SetupGlViewPort(GLint x, GLint y, GLsizei width, GLsizei height)
{
	wglMakeCurrent(m_hDC, m_hRC);
	glViewport(x, y, width, height);
	wglMakeCurrent(NULL, NULL);

	m_viewWidth = width;
	m_viewHeight = height;
}

mfxStatus COpenglDisplayD3D::Decoding()
{
	mfxStatus sts = MFX_ERR_NONE;

	for (UINT32 i = 0; i < m_nDecNums; i++)
	{
		mfxStatus ret = m_pDecArray[i]->RunDec();
		sts = (ret != MFX_ERR_NONE) ? ret : sts;
	}
	
	return sts;
}

mfxStatus COpenglDisplayD3D::Vpping()
{
	mfxStatus sts = MFX_ERR_NONE;

	for (UINT32 i = 0; i < m_nDecNums; i++)
	{
		mfxStatus ret = m_pDecArray[i]->RunVpp();
		sts = (ret != MFX_ERR_NONE) ? ret : sts;
	}

	return sts;
}

mfxStatus COpenglDisplayD3D::SetupDisplayMode(UINT32 decoderNums)
{
	mfxStatus sts = MFX_ERR_NONE;

	wglMakeCurrent(m_hDC, m_hRC);

	for (UINT32 i = 0; i < decoderNums; i++)
	{
		if (m_pDecArray[i]->m_pDecoder->DoResetQuery(m_decoderParams[i]) == VPP_RESET)
		{
			m_pDecArray[i]->SetupInstanceForVppResize(m_decoderParams[i], m_hShareDxOpenglDevice);
		}
	}

	wglMakeCurrent(NULL, NULL);

	if (m_nMemType == D3D11_MEMORY)
		m_pShareD3DDevice->D3D11DeviceContextFlush();

	return sts;
}


mfxStatus COpenglDisplayD3D::Display()
{
	mfxStatus sts = MFX_ERR_NONE;
	BOOL hasFrame = TRUE;
	UINT32 aaa[25];

	for (UINT32 i = 0; i < m_nDecNums && hasFrame; i++)
	{
		hasFrame = m_pDecArray[i]->GetVppFrameListSize() > 0;
		aaa[i] = m_pDecArray[i]->GetVppFrameListSize();
	}

	if (!hasFrame) return MFX_ERR_NULL_PTR;

	wglMakeCurrent(m_hDC, m_hRC);

	//glViewport(0, 0, m_cx, m_cy);
	//glMatrixMode(GL_PROJECTION);
	//glLoadIdentity();
	//glOrtho(0, m_cx, m_cy, 0, -1, 1);
	glDisable(GL_CULL_FACE);
	glDisable(GL_DEPTH_TEST);
	//glMatrixMode(GL_MODELVIEW);
	//glLoadIdentity();
	glEnable(GL_TEXTURE_2D);
	//glPushMatrix();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


	glUseProgram(m_shader_programme);

	float box = m_glParameters.nZoom;
	float aspect = 1.0f;
	float xval = box;
	float yval = box*aspect;

	//settingNewDisplaySize(shader_programme, xval, yval);

	for (UINT32 i = 0; i < m_nDecNums; i++)
	{
		void *vppOutframeptrs = NULL;
		vppOutframeptrs = m_pDecArray[i]->PopupFrameForDisplay();

		glActiveTexture(GL_TEXTURE0 + i);
		//wglDXLockObjectsNV(m_hShareDxOpenglDevice, 1, &m_hShareDxOpenglTextureArray[i]);

		int vppFrameNo = ((mfxFrameSurface1 *)vppOutframeptrs)->Data.reserved[7];

		glBindTexture(GL_TEXTURE_2D, m_pDecArray[i]->GetTextureVppFrame(vppFrameNo));
		//wglDXUnlockObjectsNV(m_hShareDxOpenglDevice, 1, &m_hShareDxOpenglTextureArray[i]);
		glUniform1i(testdtexArray[i], i);

		((mfxFrameSurface1 *)vppOutframeptrs)->Data.reserved[8] = 0;
		if (((mfxFrameSurface1 *)vppOutframeptrs)->Data.Locked == 0)
			m_pDecArray[i]->m_pDecoder->PushVppSurfaceList((mfxFrameSurface1 *)vppOutframeptrs);
	}
	//init_gl = true;
	glUniform1f(testxoffset, m_glParameters.xoffsetcurrent);
	glUniform1f(testyoffset, m_glParameters.yoffsetcurrent);
	//glUniform1f(texcoordscalar, apptexcoordscalar);
	glUniform1f(xtexcoordmultiplier, m_glParameters.appxtexcoordmultiplier);
	glUniform1f(ytexcoordmultiplier, m_glParameters.appytexcoordmultiplier);
	glUniform1i(numvideos, m_glParameters.appnumvideos);
	GLenum err = glGetError();
	if (err) msdk_printf(MSDK_STRING("glTexImage2D\n"));

	glBindVertexArray(m_hVao);

	settingNewDisplaySize(xval, yval);
	//or use
	//glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	err = glGetError(); if (err) msdk_printf(MSDK_STRING("glTexImage2D\n"));


	//glPopMatrix();
	SwapBuffers(m_hDC);
	//glFlush();
	wglMakeCurrent(NULL, NULL);



	return sts;

}

//for shader
bool COpenglDisplayD3D::createshaders(const char *path_vert_shader, const char *path_frag_shader)
{
	wglMakeCurrent(m_hDC, m_hRC);
#if (USESHADERFILE==0)
	m_glvertexshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(m_glvertexshader, 1, &path_vert_shader, NULL);
	glCompileShader(m_glvertexshader);
	GLenum err = glGetError();
	if (err) msdk_printf(MSDK_STRING("fragment_shader\n"));
	m_glfragmentshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(m_glfragmentshader, 1, &path_frag_shader, NULL);
	glCompileShader(m_glfragmentshader);
	err = glGetError();
	if (err) msdk_printf(MSDK_STRING("fragment_shader\n"));
	m_shader_programme = glCreateProgram();
	glAttachShader(m_shader_programme, m_glfragmentshader);
	glAttachShader(m_shader_programme, m_glvertexshader);
	glLinkProgram(m_shader_programme);
	// Flag the shaders for deletion
	glDeleteShader(m_glvertexshader);
	glDeleteShader(m_glfragmentshader);

	glUseProgram(m_shader_programme);
#else
	m_shader_programme = create_program(path_vert_shader, path_frag_shader);
	if (m_shader_programme == 0) {
		wglMakeCurrent(NULL, NULL);
		return false;
	}
#endif

	initialize(m_shader_programme, m_hVao);


	wglMakeCurrent(NULL, NULL);

	return true;
}
GLuint COpenglDisplayD3D::read_shader_src(const char *fname, std::vector<char> &buffer)
{
	std::ifstream in;
	in.open(fname, std::ios::binary);

	if (in.is_open()) {
		// Get the number of bytes stored in this file
		in.seekg(0, std::ios::end);
		size_t length = (size_t)in.tellg();

		// Go to start of the file
		in.seekg(0, std::ios::beg);

		// Read the content of the file in a buffer
		buffer.resize(length + 1);
		in.read(&buffer[0], length);
		in.close();
		// Add a valid C - string end
		buffer[length] = '\0';
	}
	else {
		//std::cerr << "Unable to open " << fname << " I'm out!" << std::endl;
		return 0;
	}

	return 1;
}

GLuint COpenglDisplayD3D::load_and_compile_shader(const char *fname, GLenum shaderType)
{
	// Load a shader from an external file
	std::vector<char> buffer;
	GLuint ret = read_shader_src(fname, buffer);
	if (ret == 0) return 0;

	const char *src = &buffer[0];
	// Compile the shader
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &src, NULL);
	glCompileShader(shader);
	// Check the result of the compilation
	GLint test;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &test);
	if (!test) {
		std::cerr << "Shader compilation failed with this message:" << std::endl;
		std::vector<char> compilation_log(4096);
		glGetShaderInfoLog(shader, (GLsizei)compilation_log.size(), NULL, &compilation_log[0]);
		std::cerr << &compilation_log[0] << std::endl;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

GLuint COpenglDisplayD3D::create_program(const char *path_vert_shader, const char *path_frag_shader)
{
	// Load and compile the vertex and fragment shaders
	GLuint vertexShader = load_and_compile_shader(path_vert_shader, GL_VERTEX_SHADER);
	if (vertexShader == 0) return 0;

	GLuint fragmentShader = load_and_compile_shader(path_frag_shader, GL_FRAGMENT_SHADER);
	if (fragmentShader == 0)
	{
		if (vertexShader) glDeleteShader(vertexShader);
		return 0;
	}

	// Attach the above shader to a program
	GLuint shaderProgram = glCreateProgram();
	glAttachShader(shaderProgram, vertexShader);
	glAttachShader(shaderProgram, fragmentShader);

	//
	//glBindAttribLocation(shaderProgram, 1, "pos");
	//glBindAttribLocation(shaderProgram, 0, "vTexCoord");

	// Link and use the program
	glLinkProgram(shaderProgram);

	// Flag the shaders for deletion
	glDeleteShader(vertexShader);
	glDeleteShader(fragmentShader);

	// Check the result of the linking
	GLint test;
	glGetProgramiv(shaderProgram, GL_LINK_STATUS, &test);
	if (!test) {
		std::cerr << "Shader compilation failed with this message:" << std::endl;
		std::vector<char> linking_log(4096);
		glGetShaderInfoLog(shaderProgram, (GLsizei)linking_log.size(), NULL, &linking_log[0]);
		std::cerr << &linking_log[0] << std::endl;
		glDeleteShader(vertexShader);
		glDeleteShader(fragmentShader);
		glDeleteProgram(shaderProgram);
		return 0;
	}

	return shaderProgram;
}

void COpenglDisplayD3D::initialize(GLuint shaderProgram, GLuint &vao)
{
	// Use a Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	GLfloat tex_position[8] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
	};

	// 1 square (made by 2 triangles) to be rendered
	GLfloat vertices_position[8] = {
		-1.0f, -1.0f,
		 1.0f, -1.0f,
		-1.0f,  1.0f,
		 1.0f,  1.0f
	};

	GLuint indices[6] = {
		0, 1, 2, 2, 1, 3
		//0, 1, 2, 2, 3, 0
	};


	// Create a Vector Buffer Object that will store the vertices on video memory
	glGenBuffers(1, &m_hVbo);

	// Allocate space for vertex positions and colors
	glBindBuffer(GL_ARRAY_BUFFER, m_hVbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices_position) + sizeof(tex_position), NULL, GL_STATIC_DRAW);

	// Transfer the vertex positions:
	glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(tex_position), tex_position);

	// Transfer the vertex colors:
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(tex_position), sizeof(vertices_position), vertices_position);

	// Create an Element Array Buffer that will store the indices array:
	glGenBuffers(1, &m_hEab);

	// Transfer the data from indices to eab
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_hEab);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);


	//GLuint shaderProgram = create_program("shaders/vert.shader", "shaders/frag.shader");

	// Get the location of the attributes that enters in the vertex shader
	GLint position_attribute = glGetAttribLocation(shaderProgram, "texCoord");

	// Specify how the data for position can be accessed
	glVertexAttribPointer(position_attribute, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Enable the attribute
	glEnableVertexAttribArray(position_attribute);

	// Color attribute
	GLint verCoord_attribute = glGetAttribLocation(shaderProgram, "verCoord");
	glVertexAttribPointer(verCoord_attribute, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(tex_position));
	glEnableVertexAttribArray(verCoord_attribute);

	//update:0408
	char texsampler[MAXVIDEOCLIPS];
	for (int i = 0; i < MAXVIDEOCLIPS; i++) {
		sprintf_s(texsampler, 16, "dtex[%d]", i);
		//sprintf(texsampler, "dtex");
		testdtexArray[i] = glGetUniformLocation(m_shader_programme, texsampler);
	}
	testxoffset = glGetUniformLocation(m_shader_programme, "xoffset");
	testyoffset = glGetUniformLocation(m_shader_programme, "yoffset");
	//texcoordscalar = glGetUniformLocation(shader_programme, "texcoordscalar");
	xtexcoordmultiplier = glGetUniformLocation(m_shader_programme, "xtexcoordmultiplier");
	ytexcoordmultiplier = glGetUniformLocation(m_shader_programme, "ytexcoordmultiplier");
	numvideos = glGetUniformLocation(m_shader_programme, "numvideos");

}

void COpenglDisplayD3D::settingNewDisplaySize(float xVal, float yVal)
{
	glBindVertexArray(m_hVao);

	GLfloat tex_position[8] = {
		0.0f, 0.0f,
		1.0f, 0.0f,
		0.0f, 1.0f,
		1.0f, 1.0f,
	};

	// 1 square (made by 2 triangles) to be rendered
	GLfloat vertices_position[8] = {
		-1.0f*xVal, -1.0f*yVal,
		 1.0f*xVal, -1.0f*yVal,
		-1.0f*xVal,  1.0f*yVal,
		 1.0f*xVal,  1.0f*yVal
	};
	glBindBuffer(GL_ARRAY_BUFFER, m_hVbo);

	// Transfer the vertex colors:
	glBufferSubData(GL_ARRAY_BUFFER, sizeof(tex_position), sizeof(vertices_position), vertices_position);
	// Color attribute
	GLint verCoord_attribute = glGetAttribLocation(m_shader_programme, "verCoord");
	glVertexAttribPointer(verCoord_attribute, 2, GL_FLOAT, GL_FALSE, 0, (GLvoid *)sizeof(tex_position));
	glEnableVertexAttribArray(verCoord_attribute);
}


mfxStatus COpenglDisplayD3D::ThreadFuncLoop_Dec()
{
	mfxStatus sts = MFX_ERR_NONE;
	while (exit_app == false)
	{
		//if (m_bWillResize)
		//{
		//	//m_eResizeFromDecToVpp->Signal();
		//	m_eResizeFromVppToDec->Wait();
		//}

		if (m_bWillUpdate1)
		{
			m_bWillUpdate1 = FALSE;
			UpdateDecoders(4, 4);

			m_eResizeFromVppToOpengl->Signal(2);
		}

		sts = Decoding();
		if (sts == MFX_ERR_NONE)
		{
			m_tDec->TimedWait(5);
			//framecounter++;
		}
		else
		{
			m_tDec->TimedWait(5);
			//INDEBUG("t_dec ...");
		}
		//Sleep(15);
	}

	exit_disp = TRUE;
	exit_vpp = TRUE;
	return sts;
}
mfxStatus COpenglDisplayD3D::ThreadFuncLoop_Vpp()
{
	mfxStatus sts = MFX_ERR_NONE;
	while (exit_vpp == false)
	{
		//if (m_bWillResize)
		//{
		//	//m_eResizeFromDecToVpp->Wait();
		//	updateDecoderParams();
		//	m_DecAndDisplay->SetupDisplayMode(MAXVIDEOCLIPS, ParamsPtrs);

		//	m_bWillResize = FALSE;
		//	m_eResizeFromVppToDec->Signal();
		//	m_eResizeFromVppToOpengl->Signal();
		//}

		if (m_bWillUpdate0)
		{
			m_bWillUpdate0 = FALSE;
			m_bWillUpdate1 = TRUE;


			m_eResizeFromVppToOpengl->Wait();
		}

		sts = Vpping();

		if (sts == MFX_ERR_NONE) m_tVpp->TimedWait(5);
		else m_tVpp->TimedWait(5);
		//Sleep(10);
	}

	return sts;
}

mfxStatus COpenglDisplayD3D::ThreadFuncLoop_Dis()
{
	int m_nCountDisplayFrames = 0;
	mfxStatus sts = MFX_ERR_NONE;
	while (exit_disp == false)
	{
		sts = Display();

		if (sts != MFX_ERR_NONE)
		{
			m_tDis->TimedWait(10);
			//Sleep(10);
			continue;
		}

		if ((++m_nCountDisplayFrames) % 50 == 0)
		//if(0)
		{
			m_bWillUpdate0 = TRUE;
			//m_bWillResize = TRUE;

			//appnumvideos = (appnumvideos == 25) ? 13 : 25;
			//if (appnumvideos == 25)
			//{
			//	m_nScaler[0] = m_nScaler[1] = m_nScaler[2] = m_nScaler[3] = 1;
			//}
			//else
			//{
			//	m_nScaler[0] = m_nScaler[1] = m_nScaler[2] = m_nScaler[3] = 2;
			//}
			m_eResizeFromVppToOpengl->Wait();
		}
	}
	return sts;
}

unsigned int MFX_STDCALL COpenglDisplayD3D::dec_thread0(void* pThis)
{
	((COpenglDisplayD3D *)pThis)->ThreadFuncLoop_Dec();
	return 0;
}

unsigned int MFX_STDCALL COpenglDisplayD3D::vpp_thread0(void* pThis)
{
	((COpenglDisplayD3D *)pThis)->ThreadFuncLoop_Vpp();
	return 0;
}

unsigned int MFX_STDCALL COpenglDisplayD3D::dis_thread0(void* pThis)
{
	((COpenglDisplayD3D *)pThis)->ThreadFuncLoop_Dis();
	return 0;
}

void COpenglDisplayD3D::CreateTask()
{
	mfxStatus t_sts;

	m_tDec = m_tVpp = m_tDis = NULL;
	exit_app = exit_disp = exit_vpp = FALSE;
	m_bWillUpdate0 = m_bWillUpdate1 = FALSE;

	m_eResizeFromOpenglToVpp = new MSDKSema(t_sts, 0, 2);
	m_eResizeFromDecToVpp = new MSDKSema(t_sts, 0, 2);
	m_eResizeFromVppToDec = new MSDKSema(t_sts, 0, 2);
	m_eResizeFromVppToOpengl = new MSDKSema(t_sts, 0, 2);

	if (m_tDec == NULL) m_tDec = new MSDKThread(t_sts, COpenglDisplayD3D::dec_thread0, this);
	if (m_tVpp == NULL) m_tVpp = new MSDKThread(t_sts, COpenglDisplayD3D::vpp_thread0, this);
	if (m_tDis == NULL) m_tDis = new MSDKThread(t_sts, COpenglDisplayD3D::dis_thread0, this);

}

void COpenglDisplayD3D::WaitTaskEnd()
{
	if (m_tDec == NULL || m_tVpp == NULL || m_tDis == NULL) return;

	HANDLE mThreads[3];

	mThreads[0] = m_tDec->m_thread;
	mThreads[1] = m_tVpp->m_thread;
	mThreads[2] = m_tDis->m_thread;

	exit_app = TRUE;

	WaitForMultipleObjects(3, mThreads, TRUE, MFX_INFINITE);

	exit_app = exit_disp = exit_vpp = FALSE;
	m_bWillUpdate0 = m_bWillUpdate1 = FALSE;

	delete m_eResizeFromDecToVpp;
	delete m_eResizeFromOpenglToVpp;
	delete m_eResizeFromVppToDec;
	delete m_eResizeFromVppToOpengl;

	delete m_tDec;
	delete m_tVpp;
	delete m_tDis;

	m_tDec = m_tVpp = m_tDis = NULL;
}

int  m_nScaler[25] = {
	//2, 2, 2, 2, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
	1, 1, 1, 1, 1,
};
extern wchar_t *videoclips[];

void COpenglDisplayD3D::initDecoderParams()
{
	for (int no = 0; no < MAXVIDEOCLIPS; no++) {
		m_decoderParams[no] = NULL;
		m_decoderParams[no] = new structDecoderParams();
		
		//update:0408
		//test for hevc
		//if (no == 0)
		//	m_decoderParams[no]->videoType = MFX_CODEC_HEVC;
		//else
		//	m_decoderParams[no]->videoType = MFX_CODEC_AVC;

		m_decoderParams[no]->videoType = MFX_CODEC_AVC;

		m_decoderParams[no]->memType = m_nMemType;
		m_decoderParams[no]->bUseHWLib = true;
		m_decoderParams[no]->m_pParaInDDevice = NULL;

		m_decoderParams[no]->vppOutWidth = (UINT16)((m_viewWidth / (UINT16)(m_glParameters.appxtexcoordmultiplier))*m_nScaler[no]);
		m_decoderParams[no]->vppOutHeight = (UINT16)((m_viewHeight / (UINT16)(m_glParameters.appytexcoordmultiplier))*m_nScaler[no]);
		msdk_strcopy(m_decoderParams[no]->strSrcFile, videoclips[no]);
	}
}

void COpenglDisplayD3D::updateDecoderParams()
{
	for (int no = 0; no < MAXVIDEOCLIPS; no++) {
		m_decoderParams[no]->vppOutWidth = (UINT16)((m_viewWidth / (UINT16)(m_glParameters.appxtexcoordmultiplier))*m_nScaler[no]);
		m_decoderParams[no]->vppOutHeight = (UINT16)((m_viewHeight / (UINT16)(m_glParameters.appytexcoordmultiplier))*m_nScaler[no]);
		msdk_strcopy(m_decoderParams[no]->strSrcFile, videoclips[no]);
	}
}
void COpenglDisplayD3D::cleanDecoderParams()
{
	for (int no = 0; no < MAXVIDEOCLIPS; no++) {
		if(m_decoderParams[no]) 
			delete m_decoderParams[no];
		m_decoderParams[no] = NULL;
	}
}
