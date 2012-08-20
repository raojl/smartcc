//***************************************************************************
// TConnID.cpp : implementation file
//***************************************************************************

#include "TConnID.h"

/////////////////////////////////////////////////////////////////////////////
// TConnID code
TConnID::TConnID()
{
	InitData();
}

TConnID::TConnID(Smt_String connid)
{
	InitData();
	m_ConnID = connid;
}

void TConnID::InitData()
{
	m_ConnID = "";
	m_ConnState = 0;
	m_CallerID = "";
	m_CallerIDName = "";
	m_DeviceID = "";
	m_DeviceType = 0;
	m_OtherConnID = "";
	m_Source = "";
	m_Destination = "";
	m_MeetmeNum = "";
	m_CallID = 0;
	m_OldCallID = 0;
	m_SecOldCallID = 0;
	m_Reason = 0; 
	m_Variable = "";
	m_Transferred = Smt_BoolFALSE;
	m_SSTransferTimerID = 0;
}

TConnID::~TConnID()
{
}
