// CMUAPITestDlg.h : header file
//

#if !defined(AFX_CMUAPITESTDLG_H__CD2FC058_075A_4F1B_9BFA_0301B2A64D46__INCLUDED_)
#define AFX_CMUAPITESTDLG_H__CD2FC058_075A_4F1B_9BFA_0301B2A64D46__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CCMUAPITestDlg dialog
#include "cmuapidefs.h"
#include "cmuapi.h"
#pragma comment(lib, "CMUAPI.lib")

#include "LocalLogDLL.h"
#pragma comment(lib, "LocalLog.lib")

class CCMUAPITestDlg : public CDialog
{
// Construction
public:
	CCMUAPITestDlg(CWnd* pParent = NULL);	// standard constructor
	~CCMUAPITestDlg();

	HANDLE m_Thread;
	DWORD  m_ThreadID;
	void GetEvent();

	// CTI Parm
	long OnCstaConfEvent(Csta_Event evt);
	long OnCstaCallEvent(Csta_Event evt);
	long OnCstaDeviceEvent(Csta_Event evt);
	long OnCstaRouteEvent(Csta_Event evt);
	long OnCstaMediaEvent(Csta_Event evt);
	long OnCstaDeviceRecordEvent(Csta_Event evt);
	long OnCstaDeviceMessageEvent(Csta_Event evt);

	// Other Function
	CString GetEvt(long evttype);

// Dialog Data
	//{{AFX_DATA(CCMUAPITestDlg)
	enum { IDD = IDD_CMUAPITEST_DIALOG };
	long	m_Call1;
	long	m_Call2;
	CString	m_CalledNum;
	CString	m_CallerNum;
	CString	m_DataKey;
	CString	m_DataValue;
	CString	m_DN1;
	CString	m_DTMFDigits;
	CString	m_FileName;
	long	m_JoinMode;
	long	m_DTMFCount;
	long	m_RouteID;
	long	m_Timeout;
	long	m_DeviceType;
	CString	m_Message;
	long	m_Channel;
	CString	m_AgentID;
	CString	m_SIPHeaderKey;
	CString	m_SIPHeaderValue;

	long	m_PlayText;
	CString m_PText;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCMUAPITestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CCMUAPITestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnBtnassign();
	afx_msg void OnBtndeassign();
	afx_msg void OnBtnmakecall();
	afx_msg void OnBtnhangupcall();
	afx_msg void OnBtnsenddtmf();
	afx_msg void OnBtnsinglesteptransfer();
	afx_msg void OnBtnsinglestepconference();
	afx_msg void OnBtnrespondtoroute();
	afx_msg void OnBtnstartrec();
	afx_msg void OnBtnstoprec();
	afx_msg void OnBtnsetdata();
	afx_msg void OnBtngetdata();
	afx_msg void OnBtngcopen();
	afx_msg void OnBtngcclose();
	afx_msg void OnBtngcmakecall();
	afx_msg void OnBtnanswercall();
	afx_msg void OnBtngchangupcall();
	afx_msg void OnBtnblindtransfer();
	afx_msg void OnBtndxplay();
	afx_msg void OnBtndxgetdig();
	afx_msg void OnBtndxdial();
	afx_msg void OnBtndxrec();
	afx_msg void OnBtnaddiottdata();
	afx_msg void OnBtnplayiottdata();
	afx_msg void OnBtnfxsendfax();
	afx_msg void OnBtnfxrcvfax();
	afx_msg void OnBtnsubscribecallevent();
	afx_msg void OnBtnunsubscribecallevent();
	afx_msg void OnBtngcdial();
	afx_msg void OnBtnsendmessage();
	afx_msg void OnBtndxsendmessage();
	afx_msg void OnBtnhold();
	afx_msg void OnBtnretrieve();
	afx_msg void OnBtnconsult();
	afx_msg void OnBtntransfer();
	afx_msg void OnBtnconference();
	afx_msg void OnBtnreconnect();
	afx_msg void OnBtndeflect();
	afx_msg void OnBtnpickup();
	afx_msg void OnBtnclearcall();
	afx_msg void OnBtndxsetsipheader();
	afx_msg void OnBtngetsipheader();
	afx_msg void OnBtnPlayText();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMUAPITESTDLG_H__CD2FC058_075A_4F1B_9BFA_0301B2A64D46__INCLUDED_)
