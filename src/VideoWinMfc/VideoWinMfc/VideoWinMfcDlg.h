
// VideoWinMfcDlg.h : header file
//

#pragma once


// CVideoWinMfcDlg dialog
class CVideoWinMfcDlg : public CDialogEx
{
// Construction
public:
	CVideoWinMfcDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VIDEOWINMFC_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedOk();
	afx_msg void OnBnClickedCancel();
	afx_msg void OnClose();
	afx_msg BOOL OnMouseWheel(UINT nFlags, short zDelta, CPoint pt);
};

#define MAXPOSSIBLEVIDEOS 25

#define MIX4K_GP 0
#define MIX1280 0
#define MIX1080 1
#define MIX2560 0


#define DEC1280 0
#define DEC1920 1
#define DEC2560 0
#define DEC4k 0


#if (DEC1280==1)
#define WINDOWWIDTH 1280
#define WINDOWHEIGHT 720
#else
#define WINDOWWIDTH 1920
#define WINDOWHEIGHT 1080
#endif







