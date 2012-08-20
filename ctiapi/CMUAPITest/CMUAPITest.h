// CMUAPITest.h : main header file for the CMUAPITEST application
//

#if !defined(AFX_CMUAPITEST_H__4A2F79CF_C778_48B7_823E_D4B3F1953F93__INCLUDED_)
#define AFX_CMUAPITEST_H__4A2F79CF_C778_48B7_823E_D4B3F1953F93__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "testresource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CCMUAPITestApp:
// See CMUAPITest.cpp for the implementation of this class
//

class CCMUAPITestApp : public CWinApp
{
public:
	CCMUAPITestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CCMUAPITestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CCMUAPITestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_CMUAPITEST_H__4A2F79CF_C778_48B7_823E_D4B3F1953F93__INCLUDED_)
