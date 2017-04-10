#include "mmsoglwin2.h"

#include "DxToOpengl.h"
#include "shared_defs.h"


bool m_bUseD3D11 = true ;

mfxStatus sts = MFX_ERR_NONE;						/*!< Media SDK return value check */
COpenglDisplayD3D *m_DecAndDisplay = NULL;


HWND hWnd;
HINSTANCE hInst;									/*!< Current hInst for our application.  */
TCHAR szTitle[MAX_LOADSTRING];						/*!< Title bar text for this application. */
TCHAR szWindowClass[MAX_LOADSTRING];				/*!< The main window class name. */

int fps = 0;										/*!< Fps tracker variable. */
int framecounter = 0;								/*!< Framecounter for computing fps. */




ATOM MyRegisterClass(HINSTANCE hInstance);
BOOL InitInstance(HINSTANCE, int);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);
HWND CreateOpenGLWindow(TCHAR* title, int x, int y, int width, int height, BYTE type, DWORD flags, HINSTANCE hInstance);




#if 0
std::vector<std::string> debuginfo;
MSDKMutex dLock;

#define INDEBUG(x) \
{\
	dLock.Lock();\
	debuginfo.push_back(std::string(x));\
	dLock.Unlock();\
}
#else
#define INDEBUG(x)
#endif


int APIENTRY _tWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPTSTR lpCmdLine, _In_ int nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);
	MSG msg;
	HACCEL hAccelTable;
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_MMSOGLWIN, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	if (!InitInstance (hInstance, nCmdShow)) {
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_MMSOGLWIN));
	unsigned int Last = 0;
	unsigned int Now = 0;
	Last = GetTickCount();
	wchar_t buffer[48];


	m_DecAndDisplay->CreateTask();

	while (GetMessage(&msg, NULL, 0, 0)) {
		//display();
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//InvalidateRect( msg.hwnd, NULL, FALSE );
		Now = GetTickCount();
		if((Now - Last) > 1000) {
			fps = framecounter/((Now - Last)/1000);
			Last = Now;
			framecounter = 0;
			wsprintfW(buffer, L"mmsoglwin %d fps", fps);
			SetWindowText(msg.hwnd,buffer);
		}
		Sleep(10);
	}
	return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MMSOGLWIN));
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDC_MMSOGLWIN);
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));
	return RegisterClassEx(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;
	hWnd = CreateOpenGLWindow(szTitle, 0, 0, WINDOWWIDTH, WINDOWHEIGHT, PFD_TYPE_RGBA, 0, hInstance);
	if (!hWnd) {
		return FALSE;
	}


	m_DecAndDisplay = new COpenglDisplayD3D();
	if(m_bUseD3D11 == true)
		m_DecAndDisplay->InitD3DDevice(hWnd, D3D11_MEMORY);
	else
		m_DecAndDisplay->InitD3DDevice(hWnd, D3D9_MEMORY);
	m_DecAndDisplay->BindWindowDC_Opengl("vert_mix.glsl", "frag_mix.glsl");
	m_DecAndDisplay->initDecoderParams();
	m_DecAndDisplay->CreateDecoders(MAXVIDEOCLIPS);


	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	switch (message) {
	case WM_COMMAND:
		wmId    = LOWORD(wParam);
		wmEvent = HIWORD(wParam);
		switch (wmId) {
		case IDM_ABOUT:
			break;
		case IDM_EXIT:
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
		break;
	case WM_PAINT:
		break;
    case WM_SIZE:
		m_DecAndDisplay->SetupGlViewPort(0, 0, LOWORD(lParam), HIWORD(lParam));
		PostMessage(hWnd, WM_PAINT, 0, 0);
		break;
	case WM_CLOSE:
		m_DecAndDisplay->WaitTaskEnd();
		delete m_DecAndDisplay;

		//?close
		PostQuitMessage(0);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
    case WM_MOUSEWHEEL:
            if((short) HIWORD(wParam)< 0) {
				m_DecAndDisplay->m_glParameters.SetZoom(-0.25f);
			}
			else {
				m_DecAndDisplay->m_glParameters.SetZoom(0.25f);
			}
            break;
	case WM_KEYDOWN: 
		switch (wParam) { 
			case VK_LEFT: 
				m_DecAndDisplay->m_glParameters.SetMoveOffset(M_LEFT, 0.01f);
				break; 
			case VK_RIGHT: 
				m_DecAndDisplay->m_glParameters.SetMoveOffset(M_RIGHT, 0.01f);
				break; 
			case VK_UP: 
				m_DecAndDisplay->m_glParameters.SetMoveOffset(M_UP, 0.01f);
				break; 
			case VK_DOWN: 
				m_DecAndDisplay->m_glParameters.SetMoveOffset(M_DOWN, 0.01f);
				break; 
		}
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

HWND CreateOpenGLWindow(TCHAR* title, int x, int y, int width, int height, 
		   BYTE type, DWORD flags, HINSTANCE hInstance)
{
    HWND        hWnd;

	hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN,
			x, y, width, height, NULL, NULL, hInstance, NULL);

    if (hWnd == NULL) 
	{
		MessageBox(NULL, L"CreateWindow() failed:  Cannot create a window.", L"Error", MB_OK);
		return NULL;
    }
    return hWnd;
}

