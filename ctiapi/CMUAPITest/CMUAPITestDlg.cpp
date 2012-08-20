// CMUAPITestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "CMUAPITest.h"
#include "CMUAPITestDlg.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

///////////////////////////////////////////////////////////////////
#ifndef _STR	
#define _STR(x)		_T(#x)
#endif

#ifndef CASE_STR
#define CASE_STR(x)	         case x:	return _T(#x);
#endif

#ifndef CASE_STR_v
#define CASE_STR_v(ret, x)	 case x:	ret = _T(#x); break;				
#endif


DWORD WINAPI ThreadFunc(LPVOID parm )
{
	CCMUAPITestDlg* m_dlg = (CCMUAPITestDlg*)parm;
	m_dlg->GetEvent();
	return 0;
}

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About

class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCMUAPITestDlg dialog

CCMUAPITestDlg::CCMUAPITestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCMUAPITestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CCMUAPITestDlg)
	m_Call1 = 0;
	m_Call2 = 0;
	m_CalledNum = _T("");
	m_CallerNum = _T("");
	m_DataKey = _T("");
	m_DataValue = _T("");
	m_DTMFDigits = _T("");
	m_FileName = _T("");
	m_JoinMode = 2;
	m_DTMFCount = 0;
	m_RouteID = 0;
	m_Timeout = 20000;
	m_DeviceType = 1;
	m_Message = _T("");
	m_Channel = 0;
	m_AgentID = _T("");
	m_CallerNum = _T("");
	m_Channel = 0;
	m_Channel = 0;
	m_SIPHeaderKey = _T("");
	m_SIPHeaderValue = _T("");
	m_PText = _T("");
	//m_PlayText = 0;
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	OpenLog( ".\\logs\\CMUAPITest.log", 5, 50*1024*1024 );
	m_Thread = ::CreateThread(NULL, 0, &ThreadFunc, this, CREATE_SUSPENDED, &m_ThreadID );
	::ResumeThread( m_Thread );
}

CCMUAPITestDlg::~CCMUAPITestDlg()
{
	::CloseHandle( m_Thread );

	CloseLog();
}

void CCMUAPITestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CCMUAPITestDlg)
	DDX_Text(pDX, IDC_EDTCALL1, m_Call1);
	DDX_Text(pDX, IDC_EDTCALL2, m_Call2);
	DDX_Text(pDX, IDC_EDTCALLEDNUM, m_CalledNum);
	DDX_Text(pDX, IDC_EDTCALLERNUM, m_CallerNum);
	DDX_Text(pDX, IDC_EDTDATAKEY, m_DataKey);
	DDX_Text(pDX, IDC_EDTDATAVALUE, m_DataValue);
	DDX_Text(pDX, IDC_EDTDN1, m_DN1);
	DDX_Text(pDX, IDC_EDTDTMFDIGITS, m_DTMFDigits);
	DDX_Text(pDX, IDC_EDTFILENAME, m_FileName);
	DDX_Text(pDX, IDC_EDTJOINMODE, m_JoinMode);
	DDX_Text(pDX, IDC_EDTDTMFCOUNT, m_DTMFCount);
	DDX_Text(pDX, IDC_EDTROUTEID, m_RouteID);
	DDX_Text(pDX, IDC_EDTTIMEOUT, m_Timeout);
	DDX_Text(pDX, IDC_EDTDEVICETYPE, m_DeviceType);
	DDX_Text(pDX, IDC_EDTMESSAGE, m_Message);
	DDX_Text(pDX, IDC_EDTCHANNEL, m_Channel);
	DDX_Text(pDX, IDC_EDTAGENTID, m_AgentID);
	DDX_Text(pDX, IDC_EDTSIPHEADERKEY, m_SIPHeaderKey);
	DDX_Text(pDX, IDC_EDTSIPHEADERVALUE, m_SIPHeaderValue);
	DDX_Text(pDX, IDC_PTEXT, m_PText);
	//DDX_Text(pDX, IDC_PALYTEXT, m_PlayText);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CCMUAPITestDlg, CDialog)
	//{{AFX_MSG_MAP(CCMUAPITestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTNASSIGN, OnBtnassign)
	ON_BN_CLICKED(IDC_BTNDEASSIGN, OnBtndeassign)
	ON_BN_CLICKED(IDC_BTNMAKECALL, OnBtnmakecall)
	ON_BN_CLICKED(IDC_BTNHANGUPCALL, OnBtnhangupcall)
	ON_BN_CLICKED(IDC_BTNSENDDTMF, OnBtnsenddtmf)
	ON_BN_CLICKED(IDC_BTNSINGLESTEPTRANSFER, OnBtnsinglesteptransfer)
	ON_BN_CLICKED(IDC_BTNSINGLESTEPCONFERENCE, OnBtnsinglestepconference)
	ON_BN_CLICKED(IDC_BTNRESPONDTOROUTE, OnBtnrespondtoroute)
	ON_BN_CLICKED(IDC_BTNSTARTREC, OnBtnstartrec)
	ON_BN_CLICKED(IDC_BTNSTOPREC, OnBtnstoprec)
	ON_BN_CLICKED(IDC_BTNSETDATA, OnBtnsetdata)
	ON_BN_CLICKED(IDC_BTNGETDATA, OnBtngetdata)
	ON_BN_CLICKED(IDC_BTNGCOPEN, OnBtngcopen)
	ON_BN_CLICKED(IDC_BTNGCCLOSE, OnBtngcclose)
	ON_BN_CLICKED(IDC_BTNGCMAKECALL, OnBtngcmakecall)
	ON_BN_CLICKED(IDC_BTNANSWERCALL, OnBtnanswercall)
	ON_BN_CLICKED(IDC_BTNGCHANGUPCALL, OnBtngchangupcall)
	ON_BN_CLICKED(IDC_BTNBLINDTRANSFER, OnBtnblindtransfer)
	ON_BN_CLICKED(IDC_BTNDXPLAY, OnBtndxplay)
	ON_BN_CLICKED(IDC_BTNDXGETDIG, OnBtndxgetdig)
	ON_BN_CLICKED(IDC_BTNDXDIAL, OnBtndxdial)
	ON_BN_CLICKED(IDC_BTNDXREC, OnBtndxrec)
	ON_BN_CLICKED(IDC_BTNADDIOTTDATA, OnBtnaddiottdata)
	ON_BN_CLICKED(IDC_BTNPLAYIOTTDATA, OnBtnplayiottdata)
	ON_BN_CLICKED(IDC_BTNFXSENDFAX, OnBtnfxsendfax)
	ON_BN_CLICKED(IDC_BTNFXRCVFAX, OnBtnfxrcvfax)
	ON_BN_CLICKED(IDC_BTNSUBSCRIBECALLEVENT, OnBtnsubscribecallevent)
	ON_BN_CLICKED(IDC_BTNUNSUBSCRIBECALLEVENT, OnBtnunsubscribecallevent)
	ON_BN_CLICKED(IDC_BTNGCDIAL, OnBtngcdial)
	ON_BN_CLICKED(IDC_BTNSENDMESSAGE, OnBtnsendmessage)
	ON_BN_CLICKED(IDC_BTNDXSENDMESSAGE, OnBtndxsendmessage)
	ON_BN_CLICKED(IDC_BTNHOLD, OnBtnhold)
	ON_BN_CLICKED(IDC_BTNRETRIEVE, OnBtnretrieve)
	ON_BN_CLICKED(IDC_BTNCONSULT, OnBtnconsult)
	ON_BN_CLICKED(IDC_BTNTRANSFER, OnBtntransfer)
	ON_BN_CLICKED(IDC_BTNCONFERENCE, OnBtnconference)
	ON_BN_CLICKED(IDC_BTNRECONNECT, OnBtnreconnect)
	ON_BN_CLICKED(IDC_BTNDEFLECT, OnBtndeflect)
	ON_BN_CLICKED(IDC_BTNPICKUP, OnBtnpickup)
	ON_BN_CLICKED(IDC_BTNCLEARCALL, OnBtnclearcall)
	ON_BN_CLICKED(IDC_BTNDXSETSIPHEADER, OnBtndxsetsipheader)
	ON_BN_CLICKED(IDC_BTNGETSIPHEADER, OnBtngetsipheader)
	ON_BN_CLICKED(IDC_PALYTEXT, OnBtnPlayText)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CCMUAPITestDlg message handlers

BOOL CCMUAPITestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon
	
	// TODO: Add extra initialization here
	
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CCMUAPITestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CCMUAPITestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

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
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CCMUAPITestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CCMUAPITestDlg::OnBtnassign() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );

	long nRet;
	nRet = ctcAssign(1000, (char*)(LPCTSTR)m_DN1, m_DeviceType );
}

void CCMUAPITestDlg::OnBtndeassign() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );

	long nRet;
	nRet = ctcDeassign( m_Channel );
}

void CCMUAPITestDlg::OnBtnmakecall() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );

	long nRet;
	nRet = ctcMakeCall( m_Channel, (char*)(LPCTSTR)m_CalledNum, (char*)(LPCTSTR)m_CallerNum );
}

void CCMUAPITestDlg::OnBtnhangupcall() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	
	long nRet;
	nRet = ctcHangupCall( m_Channel, m_Call1 );
}

void CCMUAPITestDlg::OnBtnsenddtmf() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = ctcSendDTMF( m_Channel, m_Call1, (char*)(LPCTSTR)m_DTMFDigits );
}

void CCMUAPITestDlg::OnBtnsinglesteptransfer() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = ctcSingleStepTransfer( m_Channel, (char*)(LPCTSTR)m_CalledNum, m_Call1 );
}

void CCMUAPITestDlg::OnBtnsinglestepconference() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = ctcSingleStepConference( m_Channel, (char*)(LPCTSTR)m_CalledNum, m_Call1, m_JoinMode );
}

void CCMUAPITestDlg::OnBtnrespondtoroute() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = ctcRouteSelected( m_Channel, m_RouteID, (char*)(LPCTSTR)m_CalledNum, (char*)(LPCTSTR)m_AgentID );
}

void CCMUAPITestDlg::OnBtnstartrec() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = ctcStartRecord( m_Channel, m_Call1, (char*)(LPCTSTR)m_FileName );
}

void CCMUAPITestDlg::OnBtnstoprec() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = ctcStopRecord( m_Channel, m_Call1 );
}

void CCMUAPITestDlg::OnBtnsetdata() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = ctcSetDataValue( m_Channel, m_Call1, (char*)(LPCTSTR)m_DataKey, (char*)(LPCTSTR)m_DataValue );
}

void CCMUAPITestDlg::OnBtngetdata() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = ctcGetDataValue( m_Channel, m_Call1, (char*)(LPCTSTR)m_DataKey );
}

void CCMUAPITestDlg::OnBtnsubscribecallevent() 
{
	// TODO: Add your control notification handler code here
	long nRet = cmSubscribeCallEvent();
}

void CCMUAPITestDlg::OnBtnunsubscribecallevent() 
{
	// TODO: Add your control notification handler code here
	long nRet = cmUnsubscribeCallEvent();
}

void CCMUAPITestDlg::OnBtngcopen() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = gc_open( 10001, (char*)(LPCTSTR)m_DN1, m_DeviceType );
}

void CCMUAPITestDlg::OnBtngcclose() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = gc_close( m_Channel );
}

void CCMUAPITestDlg::OnBtngcmakecall() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = gc_makecall( 10001, (char*)(LPCTSTR)m_DN1, (char*)(LPCTSTR)m_CalledNum, (char*)(LPCTSTR)m_CallerNum, m_Timeout );
}

void CCMUAPITestDlg::OnBtnanswercall() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = gc_answercall( m_Call1 );
}

void CCMUAPITestDlg::OnBtngchangupcall() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = gc_hangupcall( m_Call1 );
}

void CCMUAPITestDlg::OnBtnblindtransfer() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = gc_blindtransfer( m_Call1, (char*)(LPCTSTR)m_CalledNum );
}

void CCMUAPITestDlg::OnBtndxplay() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = dx_play( m_Call1, (char*)(LPCTSTR)m_FileName, (char*)(LPCTSTR)m_DTMFDigits );
}

void CCMUAPITestDlg::OnBtndxgetdig() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = dx_getdig( m_Call1, m_Timeout, m_DTMFCount );
}

void CCMUAPITestDlg::OnBtndxdial() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = dx_dial( m_Call1, (char*)(LPCTSTR)m_DTMFDigits);
}

void CCMUAPITestDlg::OnBtndxrec() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = dx_rec( m_Call1, (char*)(LPCTSTR)m_FileName, (char*)(LPCTSTR)m_DTMFDigits, m_Timeout, 30*1000 );
}

void CCMUAPITestDlg::OnBtnaddiottdata() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = dx_addiottdata( m_Call1, (char*)(LPCTSTR)m_FileName );
}

void CCMUAPITestDlg::OnBtnplayiottdata() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = dx_playiottdata( m_Call1, (char*)(LPCTSTR)m_DTMFDigits );
}

void CCMUAPITestDlg::OnBtnfxsendfax() 
{
	// TODO: Add your control notification handler code here
	// not support
}

void CCMUAPITestDlg::OnBtnfxrcvfax() 
{
	// TODO: Add your control notification handler code here
	// not support
}

void CCMUAPITestDlg::OnBtnPlayText()
{
	UpdateData( true );
	long nRet;
	nRet = fx_playtext(m_Call1,m_PText.GetBuffer(m_PText.GetLength()) );
}

void CCMUAPITestDlg::GetEvent()
{
	Csta_Event event;
	long nTimeout = 1000;
	unsigned long nRet = 0;
	while(true)
	{
		nRet = ctcGetEvent(&event, nTimeout );
		if( nRet == 0 )
		{
			switch (event.evtclass)
			{
			case CSTA_CONFEVENT:
				OnCstaConfEvent( event );
				break;
			case CSTA_CALLEVENT:
				OnCstaCallEvent( event );
				break;
			case CSTA_DEVICEEVENT:
				OnCstaDeviceEvent( event );
				break;
			case CSTA_ROUTEEVENT:
				OnCstaRouteEvent( event );
				break;
			case CSTA_MEDIAEVENT:
				OnCstaMediaEvent( event );
				break;
			case CSTA_DEVICERECORDEVENT:
				OnCstaDeviceRecordEvent( event );
				break;
			case CSTA_DEVICEMESSAGEEVENT:
				OnCstaDeviceMessageEvent( event );
				break;
			case CSTA_SYSTEMEVENT:
				break;
			default:
				break;
			}		
		}
		else
		{
			// PrintLog(5,"[ctcGetEvent] nRet<0x%x>.",	nRet );
		}
	}
}

long CCMUAPITestDlg::OnCstaConfEvent(Csta_Event evt)
{
	PrintLog(5,"[OnCstaConfEvent] Event<%s>.", 
		(char*)(LPCTSTR)GetEvt( evt.evttype) );

	switch (evt.evttype)
	{
	case CONF_CTCASSIGN:		
		PrintLog(5,"[confAssign] Channel<%d>, DeviceID<%s>, Reason<%d>.",
			evt.u.evtconf.u.confAssign.ChannelID, 
			evt.u.evtconf.u.confAssign.DeviceID,
			evt.u.evtconf.u.confAssign.Reason );

		m_Channel = evt.u.evtconf.u.confAssign.ChannelID;
		break;

	case CONF_CTCMAKECALL:
		PrintLog(5,"[confMakeCall] Channel<%d>, CallID<%d>, Reason<%d>.", 
			evt.u.evtconf.u.confctcResponse.ChannelID, 
			evt.u.evtconf.u.confctcResponse.CallID,
			evt.u.evtconf.u.confctcResponse.Reason );
		break;

	case CONF_CTCGETDATAVALUE:
		PrintLog(5,"[confGetDataValue] Channel<%d>, DataValue<%s>, Reason<%d>.", 
			evt.u.evtconf.u.confGetDataValue.ChannelID, 
			evt.u.evtconf.u.confGetDataValue.DataValue,
			evt.u.evtconf.u.confGetDataValue.Reason );
		break;

	case CONF_GC_OPEN:
		PrintLog(5,"[confgc_open] Channel<%d>, InvokeID<%d>, Reason<%d>.", 
			evt.u.evtconf.u.confgc_open.ChannelID, 
			evt.u.evtconf.u.confgc_open.InvokeID,
			evt.u.evtconf.u.confgc_open.Reason );
		break;	
	case CONF_GC_MAKECALL:
		PrintLog(5,"[confgc_makecall] CallID<%d>, InvokeID<%d>, Reason<%d>.", 
			evt.u.evtconf.u.confgc_makecall.CallID, 
			evt.u.evtconf.u.confgc_makecall.InvokeID,
			evt.u.evtconf.u.confgc_makecall.Reason );
		break;	
	case CONF_DX_GETSIPHEADER:
		PrintLog(5,"[confdx_getsipheader] CallID<%d>, HeaderKey<%s>, HeaderValue<%s>,  Reason<%d>.", 
			evt.u.evtconf.u.confdx_getsipheader.CallID, 
			evt.u.evtconf.u.confdx_getsipheader.HeaderKey ,
			evt.u.evtconf.u.confdx_getsipheader.HeaderValue ,
			evt.u.evtconf.u.confdx_getsipheader.Reason );

		m_SIPHeaderValue = evt.u.evtconf.u.confdx_getsipheader.HeaderValue;
		break;	
	}

	return 0;
}

long CCMUAPITestDlg::OnCstaCallEvent(Csta_Event evt)
{
	PrintLog(5,"[OnCstaCallEvent] Event<%s>, CallID<%d>, CallingParty<%s>, CalledParty<%s>, InitiatedParty<%s>, AlertingParty<%s>, HoldingParty<%s>, RetrievingParty<%s>, TransferringParty<%s>, ConferencingParty<%s>.", 
		(char*)(LPCTSTR)GetEvt(evt.evttype),  
		evt.u.evtcall.CallID,
		evt.u.evtcall.CallingParty,
		evt.u.evtcall.CalledParty,
		evt.u.evtcall.InitiatedParty,
		evt.u.evtcall.AlertingParty,
		evt.u.evtcall.HoldingParty,
		evt.u.evtcall.RetrievingParty,
		evt.u.evtcall.TransferringParty,
		evt.u.evtcall.ConferencingParty
		);
	return 0;
}

long CCMUAPITestDlg::OnCstaDeviceEvent(Csta_Event evt)
{
	PrintLog(5,"[OnCstaDeviceEvent] Event<%s>, Device<%s>, Channel<%d>, CallID<%d>, OtherParty<%s>.", 
		(char*)(LPCTSTR)GetEvt(evt.evttype),
		(char*)(LPCTSTR)evt.u.evtdevice.MonitorParty,
		evt.u.evtdevice.ChannelID, 
		evt.u.evtdevice.CallID, 
		evt.u.evtdevice.OtherParty );
	return 0;
}

long CCMUAPITestDlg::OnCstaRouteEvent(Csta_Event evt)
{
	PrintLog(5,"[OnCstaRouteEvent] Event<%s>, RouteID<%d>, CallID<%d>, DeviceID<%s>, OtherParty<%s>.", 
		(char*)(LPCTSTR)GetEvt(evt.evttype), 
		evt.u.evtroute.RouteID, 
		evt.u.evtroute.CallID,
		evt.u.evtroute.DeviceID,
		evt.u.evtroute.OtherParty );
	return 0;
}

long CCMUAPITestDlg::OnCstaMediaEvent(Csta_Event evt)
{
	PrintLog(5,"[OnCstaMediaEvent] Event<%s>, CallID<%d>, DTMF<%s>, FileName<%s>, Reason<%d>.", 
		(char*)(LPCTSTR)GetEvt(evt.evttype), 
		evt.u.evtmedia.CallID, 
		evt.u.evtmedia.DTMFDigits, 
		evt.u.evtmedia.FileName,
		evt.u.evtmedia.Reason );

	return 0;
}

long CCMUAPITestDlg::OnCstaDeviceRecordEvent(Csta_Event evt)
{
	PrintLog(5,"[OnCstaDeviceRecordEvent] Event<%s>, Channel<%d>, FileName<%s>, Reason<%d>.", 
		(char*)(LPCTSTR)GetEvt(evt.evttype), 
		evt.u.evtdevicerecord.ChannelID, 
		evt.u.evtdevicerecord.FileName,
		evt.u.evtdevicerecord.Reason );
	return 0;
}

long CCMUAPITestDlg::OnCstaDeviceMessageEvent(Csta_Event evt)
{
	PrintLog(5,"[OnCstaDeviceRecordEvent] Event<%s>, Channel<%d>, Message<%s>, Reason<%d>.", 
		(char*)(LPCTSTR)GetEvt(evt.evttype), 
		evt.u.evtdevicemessage.ChannelID, 
		evt.u.evtdevicemessage.Message,
		evt.u.evtdevicemessage.Reason );
	return 0;
}

CString CCMUAPITestDlg::GetEvt(long evttype)
{
	CString strType = "";

	switch(evttype)
    {
		CASE_STR(CONF_CTCASSIGN                 ) 
		CASE_STR(CONF_CTCDEASSIGN               ) 
		CASE_STR(CONF_CTCMAKECALL               ) 
		CASE_STR(CONF_CTCHANGUPCALL             ) 
		CASE_STR(CONF_CTCHOLDCALL               ) 
		CASE_STR(CONF_CTCRETRIEVECALL           ) 
		CASE_STR(CONF_CTCCONSULTATIONCALL       ) 
		CASE_STR(CONF_CTCCONFERENCECALL         ) 
		CASE_STR(CONF_CTCTRANSFERCALL           ) 
		CASE_STR(CONF_CTCRECONNECTCALL          ) 
		CASE_STR(CONF_CTCSINGLESTEPTRANSFER     ) 
		CASE_STR(CONF_CTCSINGLESTEPCONFERENCE   ) 
		CASE_STR(CONF_CTCROUTESELECTED          ) 
		CASE_STR(CONF_CTCSENDDTMF               ) 
		CASE_STR(CONF_CTCSTARTRECORD            ) 
		CASE_STR(CONF_CTCSTOPRECORD             ) 
		CASE_STR(CONF_CTCSETDATAVALUE           ) 
		CASE_STR(CONF_CTCGETDATAVALUE           ) 
		CASE_STR(CONF_CTCSNAPSHOT               ) 
		CASE_STR(CONF_CTCSENDMESSAGE            ) 
		CASE_STR(CONF_GC_OPEN                   ) 
		CASE_STR(CONF_GC_CLOSE                  ) 
		CASE_STR(CONF_GC_MAKECALL               ) 
		CASE_STR(CONF_GC_ANSWERCALL             ) 
		CASE_STR(CONF_GC_HANGUPCALL             ) 
		CASE_STR(CONF_GC_BLINDTRANSFER          ) 
		CASE_STR(CONF_GC_DIAL                   ) 
		CASE_STR(CONF_DX_PLAY                   ) 
		CASE_STR(CONF_DX_ADDIOTTDATA            ) 
		CASE_STR(CONF_DX_PLAYIOTTDATA           ) 
		CASE_STR(CONF_DX_GETDIG                 ) 
		CASE_STR(CONF_DX_DIAL                   ) 
		CASE_STR(CONF_DX_REC                    ) 
		CASE_STR(CONF_DX_SENDMESSAGE            ) 
		CASE_STR(CONF_FX_SENDFAX                ) 
		CASE_STR(CONF_FX_RCVFAX                 ) 
		CASE_STR(CONF_SUBSCRIBECALLEVENT        ) 
		CASE_STR(CONF_UNSUBSCRIBECALLEVENT      ) 
		CASE_STR(CSTA_CALLINITIATED             ) 
		CASE_STR(CSTA_CALLDELIVERED             ) 
		CASE_STR(CSTA_CALLCONNECTED             ) 
		CASE_STR(CSTA_CALLHELD                  ) 
		CASE_STR(CSTA_CALLRETRIEVED             ) 
		CASE_STR(CSTA_CALLCONFERENCED           ) 
		CASE_STR(CSTA_CALLQUEUED                ) 
		CASE_STR(CSTA_CALLTRANSFERRED           ) 
		CASE_STR(CSTA_CALLCLEARED               ) 
		CASE_STR(CSTA_CALLFAILED                ) 
		CASE_STR(CSTA_DEVICE_BACKINSERVICE      ) 
		CASE_STR(CSTA_DEVICE_OUTOFSERVICE       ) 
		CASE_STR(CSTA_DEVICE_OFFHOOK            ) 
		CASE_STR(CSTA_DEVICE_INBOUNDCALL        ) 
		CASE_STR(CSTA_DEVICE_DESTSEIZED         ) 
		CASE_STR(CSTA_DEVICE_TPANSWERED         ) 
		CASE_STR(CSTA_DEVICE_OPANSWERED         ) 
		CASE_STR(CSTA_DEVICE_TPSUSPENDED        ) 
		CASE_STR(CSTA_DEVICE_OPHELD             ) 
		CASE_STR(CSTA_DEVICE_TPRETRIEVED        ) 
		CASE_STR(CSTA_DEVICE_OPRETRIEVED        ) 
		CASE_STR(CSTA_DEVICE_TPDISCONNECTED     ) 
		CASE_STR(CSTA_DEVICE_OPDISCONNECTED     ) 
		CASE_STR(CSTA_DEVICE_TPTRANSFERRED      ) 
		CASE_STR(CSTA_DEVICE_OPTRANSFERRED      ) 
		CASE_STR(CSTA_DEVICE_TPCONFERENCED      ) 
		CASE_STR(CSTA_DEVICE_OPCONFERENCED      ) 
		CASE_STR(CSTA_DEVICE_DESTBUSY           ) 
		CASE_STR(CSTA_DEVICE_DESTINVALID        ) 
		CASE_STR(CSTA_DEVICE_ROUTEREQUEST       ) 
		CASE_STR(CSTA_DEVICE_ROUTEEND           ) 
		CASE_STR(CSTA_DEVICE_QUEUED             ) 
		CASE_STR(CSTA_MEDIA_PLAYING             ) 
		CASE_STR(CSTA_MEDIA_PLAYEND             ) 
		CASE_STR(CSTA_MEDIA_SENDING             ) 
		CASE_STR(CSTA_MEDIA_SENDEND             ) 
		CASE_STR(CSTA_MEDIA_GETING              ) 
		CASE_STR(CSTA_MEDIA_GETEND              ) 
		CASE_STR(CSTA_MEDIA_RECORDING           ) 
		CASE_STR(CSTA_MEDIA_RECORDEND           ) 
		CASE_STR(CSTA_MEDIA_MESSAGESENDING      ) 
		CASE_STR(CSTA_MEDIA_MESSAGESENDEND      ) 
		CASE_STR(CSTA_DEVICE_RECORDING          ) 
		CASE_STR(CSTA_DEVICE_RECORDEND          ) 
		CASE_STR(CSTA_DEVICE_MESSAGE            ) 
		CASE_STR(CSTA_LINKDOWN                  ) 
		CASE_STR(CSTA_LINKUP                    )  
            
	default:
		strType.Format("_?(%d)", evttype );
		return strType;
    }
    return "";
}
void CCMUAPITestDlg::OnBtngcdial() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = gc_dial( m_Call1, (char*)(LPCTSTR)m_CalledNum, m_Timeout );
}

void CCMUAPITestDlg::OnBtnsendmessage() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = ctcSendMessage( m_Channel, m_Call1, (char*)(LPCTSTR)m_Message );
}

void CCMUAPITestDlg::OnBtndxsendmessage() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	long nRet;
	nRet = dx_sendmessage( m_Call1, (char*)(LPCTSTR)m_Message );
}

void CCMUAPITestDlg::OnBtnhold() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	
	long nRet;
	nRet = ctcHoldCall( m_Channel, m_Call1 );
}

void CCMUAPITestDlg::OnBtnretrieve() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	
	long nRet;
	nRet = ctcRetrieveCall( m_Channel, m_Call1 );
}

void CCMUAPITestDlg::OnBtnconsult() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	
	long nRet;
	nRet = ctcConsultationCall( m_Channel, m_Call1, 
		(char*)(LPCTSTR)m_CalledNum, 
		(char*)(LPCTSTR)m_CallerNum, 
		m_Timeout );
}

void CCMUAPITestDlg::OnBtntransfer() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	
	long nRet;
	nRet = ctcTransferCall( m_Channel, m_Call2, m_Call1 );
}

void CCMUAPITestDlg::OnBtnconference() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	
	long nRet;
	nRet = ctcConferenceCall( m_Channel, m_Call2, m_Call1  );
}

void CCMUAPITestDlg::OnBtnreconnect() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	
	long nRet;
	nRet = ctcReconnectCall( m_Channel, m_Call2, m_Call1  );
}

void CCMUAPITestDlg::OnBtndeflect() 
{
	// TODO: Add your control notification handler code here
	
}

void CCMUAPITestDlg::OnBtnpickup() 
{
	// TODO: Add your control notification handler code here
	
}

void CCMUAPITestDlg::OnBtnclearcall() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	
	long nRet;
	nRet = ctcClearCall( m_Channel, m_Call1 );
}

void CCMUAPITestDlg::OnBtndxsetsipheader() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	
	long nRet;
	nRet = dx_setsipheader( m_Call1, (char*)(LPCTSTR)m_SIPHeaderKey, (char*)(LPCTSTR)m_SIPHeaderValue );
}

void CCMUAPITestDlg::OnBtngetsipheader() 
{
	// TODO: Add your control notification handler code here
	UpdateData( true );
	
	long nRet;
	nRet = dx_getsipheader( m_Call1, (char*)(LPCTSTR)m_SIPHeaderKey );
}
