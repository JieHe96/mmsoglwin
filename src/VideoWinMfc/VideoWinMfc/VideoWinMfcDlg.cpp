
// VideoWinMfcDlg.cpp : implementation file
//

#include "stdafx.h"
#include "VideoWinMfc.h"
#include "VideoWinMfcDlg.h"
#include "afxdialogex.h"


#include "DxToOpengl.h"
#include "shared_defs.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVideoWinMfcDlg dialog

#define NUM_INSTANCE	4
bool m_bUseD3D11 = true;

mfxStatus sts = MFX_ERR_NONE;						/*!< Media SDK return value check */

COpenglDisplayD3D *m_DecAndDisplay[NUM_INSTANCE] = { NULL, NULL, NULL, NULL };
int	m_arrIDCtrl[NUM_INSTANCE] = { IDC_VIDEOBOX1,IDC_VIDEOBOX2,IDC_VIDEOBOX3,IDC_VIDEOBOX4 };

//! Video clip test content array.
//! Global array to hold test clips for the purposes of the sample application. See FULLHD_GP, UHD_GP, MIX4K_GP in stdafx.h.  These defines control while list of clips is used at runtime.  Uncomment the one you want to try and make sure the rest are commented out.  You will notice that the texture sizes in simpleDevice::CreateDevice() are hard coded to the video resolution size.  I did not have a convenient way to get the width / height at runtime.  By hard coding them I was able to be sure that the actual texture being displayed contained the true number of pixels in the video.  If you were to make the textures smaller than the video size, drivers / hw will resize the image and quality will be lost. To examine the image quality, try using the mouse wheel to zoom on the textures, use the arrow keys to pan as needed.*/
wchar_t *videoclips[MAXPOSSIBLEVIDEOS] = {
#if MIX4K_GP
	L"c:/MyProject/MMSF/testcontent/test5.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test0.264", /**< Clip 2 */
	L"c:/MyProject/MMSF/testcontent/test1.264", /**< Clip 3 */
	L"c:/MyProject/MMSF/testcontent/test3.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test4.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test4.264", /**< Clip 4 */
	L"c:/MyProject/MMSF/testcontent/test0.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test1.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test2.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test3.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test4.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test5.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test0.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test1.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test2.264", /**< Clip 5 */
	L"c:/MyProject/MMSF/testcontent/test3.264", /**< Clip 5 */
												//L"../../testcontent/test0.264", /**< Clip 2 */
												//L"../../testcontent/test1.264", /**< Clip 3 */
												//L"../../testcontent/test3.264", /**< Clip 5 */
												//L"../../testcontent/test4.264", /**< Clip 5 */
												//L"../../testcontent/test4.264", /**< Clip 4 */
												//L"../../testcontent/test5.264", /**< Clip 5 */
												//L"../../testcontent/test0.264", /**< Clip 5 */
												//L"../../testcontent/test1.264", /**< Clip 5 */
												//L"../../testcontent/test2.264", /**< Clip 5 */
												//L"../../testcontent/test3.264", /**< Clip 5 */
												//L"../../testcontent/test4.264", /**< Clip 5 */
												//L"../../testcontent/test5.264", /**< Clip 5 */
												//L"../../testcontent/test0.264", /**< Clip 5 */
												//L"../../testcontent/test1.264", /**< Clip 5 */
												//L"../../testcontent/test2.264", /**< Clip 5 */
#endif
#if MIX1280
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
												L"c:/MyProject/VideoClip/New_1280.mp4.h264",
#endif
#if MIX1080
#if 1
												L"c:/MyProject/VideoClip/test-1080M.mp4.h264",
												//L"c:/MyProject/VideoClip/afd.hevc",
												L"c:/MyProject/VideoClip/test-1080M.mp4.h264",
												L"c:/MyProject/VideoClip/test-1080M.mp4.h264",
												L"c:/MyProject/VideoClip/test-1080M.mp4.h264",
												L"c:/MyProject/VideoClip/test-1080M.mp4.h264",//4
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/test-1080M.mp4.h264",//8
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/test-1080M.mp4.h264",//12
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
#else
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",//
												L"c:/MyProject/VideoClip/New_1080_High.mp4.h264",
#endif
#endif
#if MIX2560
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
												L"c:/MyProject/VideoClip/New_2560.mp4.h264",
#endif
};

CVideoWinMfcDlg::CVideoWinMfcDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_VIDEOWINMFC_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CVideoWinMfcDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CVideoWinMfcDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDOK, &CVideoWinMfcDlg::OnBnClickedOk)
	ON_BN_CLICKED(IDCANCEL, &CVideoWinMfcDlg::OnBnClickedCancel)
	ON_WM_CLOSE()
	ON_WM_MOUSEWHEEL()
END_MESSAGE_MAP()


// CVideoWinMfcDlg message handlers

BOOL CVideoWinMfcDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	for (int i = 0; i < NUM_INSTANCE; i++)
	{
		HWND hWnd = GetDlgItem(m_arrIDCtrl[i])->m_hWnd;
		m_DecAndDisplay[i] = new COpenglDisplayD3D();
		if (m_bUseD3D11 == true)
			m_DecAndDisplay[i]->InitD3DDevice(hWnd, D3D11_MEMORY);
		else
			m_DecAndDisplay[i]->InitD3DDevice(hWnd, D3D9_MEMORY);
		m_DecAndDisplay[i]->BindWindowDC_Opengl("vert_mix.glsl", "frag_mix.glsl");
		m_DecAndDisplay[i]->initDecoderParams();
		m_DecAndDisplay[i]->CreateDecoders(MAXVIDEOCLIPS);
	}


	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CVideoWinMfcDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CVideoWinMfcDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CVideoWinMfcDlg::OnBnClickedOk()
{
	// TODO: Add your control notification handler code here

	for (int i = 0; i < NUM_INSTANCE; i++)
		m_DecAndDisplay[i]->CreateTask();

	//CDialogEx::OnOK();
}


void CVideoWinMfcDlg::OnBnClickedCancel()
{
	// TODO: Add your control notification handler code here
	for (int i = 0; i < NUM_INSTANCE; i++)
	{
		m_DecAndDisplay[i]->WaitTaskEnd(); 
		delete m_DecAndDisplay[i]; m_DecAndDisplay[i] = NULL;
	}

	CDialogEx::OnCancel();
}


void CVideoWinMfcDlg::OnClose()
{
	// TODO: Add your message handler code here and/or call default

	//CDialogEx::OnClose();
}

BOOL CVideoWinMfcDlg::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	// TODO: Add your message handler code here and/or call default
	float zoom = 0;
	if (zDelta < 0) zoom = -0.2f;
	else zoom = 0.2f;

	CWnd *ptWnd = ChildWindowFromPoint(pt);
	for (int i = 0; i < NUM_INSTANCE; i++)
	{
		if (GetDlgItem(m_arrIDCtrl[i]) == ptWnd)
		{
			m_DecAndDisplay[i]->m_glParameters.SetZoom(zoom);
			break;
		}
	}

	return CDialogEx::OnMouseWheel(nFlags, zDelta, pt);
}
