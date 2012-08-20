//***************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CMU
//      Create Date : 2010.9.9												
//      Author      : Shenzj								
//      Discription : TConnID ¿‡
//      Modify By   :											
//***************************************************************************

#ifndef Smt_TCONNID_HH
#define Smt_TCONNID_HH

#include "CMUInterface.h"

/////////////////////////////////////////////////////////////////////////////
// TConnID code

class TConnID: public Smt_Node
{
public:
	TConnID();
	TConnID(Smt_String connid);
	~TConnID();
	void InitData();

public:
	Smt_String m_ConnID;
	Smt_Uint   m_ConnState;
	Smt_String m_CallerID;
	Smt_String m_CallerIDName;
	Smt_String m_DeviceID;
	Smt_Uint   m_DeviceType;
	Smt_String m_OtherConnID;
	Smt_String m_Source;
	Smt_String m_Destination;
	Smt_String m_MeetmeNum;
	Smt_Uint   m_CallID;         // current call-id
	Smt_Uint   m_OldCallID;      // previous call-id
	Smt_Uint   m_SecOldCallID;   // consultation call-id
	Smt_Uint   m_Reason;   
	Smt_String m_Variable;
	Smt_Bool   m_Transferred;
	Smt_Pdu    m_LastEvent;
	Smt_Uint   m_SSTransferTimerID; 
};

typedef Smt_Map<Smt_String,TConnID*> TConnIDMap;

#endif // Smt_TCONNID_HH
