//***************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CMU
//      Create Date : 2010.9.3												
//      Author      : Shenzj								
//      Discription : Call ¿‡
//      Modify By   :											
//***************************************************************************

#ifndef Smt_TCALL_HH
#define Smt_TCALL_HH

#include "CMUInterface.h"
#include "TConnID.h"

class TCallState;
/////////////////////////////////////////////////////////////////////////////
// TCall code

class TCall: public Smt_DLList_Node, public Smt_StateObject
{
public:
	typedef Smt_Map<Smt_String, Smt_String> TStringsMap;
public:
	TCall(Smt_String objid, Smt_Uint initstate, TCallState* powner );
	~TCall();

public: 
	// virtual method
	Smt_Uint OnUnexpectedEvent(Smt_Pdu &pdu);
	Smt_Uint OnStateExit(Smt_Pdu &pdu);
	Smt_Uint OnStateEnter(Smt_Pdu &pdu);

	// action method
	Smt_Uint EvtCallInitToOriginated(Smt_Pdu &pdu);
	Smt_Uint EvtCallOriginated(Smt_Pdu &pdu);
	Smt_Uint EvtCallDelivered(Smt_Pdu &pdu);
	Smt_Uint EvtCallConnected(Smt_Pdu &pdu);
	Smt_Uint EvtCallCleared(Smt_Pdu &pdu);
	Smt_Uint EvtCallHeld(Smt_Pdu &pdu);
	Smt_Uint EvtCallClearedEx(Smt_Pdu &pdu);
	Smt_Uint EvtCallConferenced(Smt_Pdu &pdu);
	Smt_Uint EvtCallQueued(Smt_Pdu &pdu);
	Smt_Uint EvtCallFailed(Smt_Pdu &pdu);
	Smt_Uint EvtCallTransferredToOriginated(Smt_Pdu &pdu);

	Smt_Uint AddConnID(TConnID* pconnid);
	Smt_Uint RemoveConnID(TConnID* pconnid);
	Smt_Uint RemoveAllConnID();
	Smt_Uint GetConnIDCount();
	Smt_Uint SendConnectionEvent(Smt_Pdu& pdu);
	Smt_Uint SendCallEvent(Smt_Uint evtid, Smt_Uint reason);
	void PrintLog(Smt_Uint loglevel, const char* fmt, ... );
	Smt_String LookupOtherParty(Smt_String localconnid);
	Smt_Uint SetData(Smt_String key, Smt_String val );
	Smt_String GetData(Smt_String key );

public:
	Smt_Uint   m_CallID;
	Smt_String m_CallingParty;
	Smt_String m_CalledParty;
	Smt_String m_InitiatedParty;
	Smt_String m_AlertingParty;
	Smt_String m_AnsweringParty;
	Smt_String m_HoldingParty;
	Smt_String m_RetrievingParty;
	Smt_String m_ConsultingParty;
	Smt_String m_TransferringParty;
	Smt_String m_ConferencingParty;
	Smt_String m_ReleaseingParty;
	Smt_Pdu    m_LastCommand;
	TConnIDMap   m_ConnIDList;
	TStringsMap  m_PrivateData;		
};

typedef Smt_Map<Smt_Uint,TCall*> TCallMap;

/************************************************************************/

#endif // Smt_TCALL_HH
