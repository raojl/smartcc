//***************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CMU
//      Create Date : 2010.9.3												
//      Author      : Shenzj								
//      Discription : CallState ¿‡
//      Modify By   :											
//***************************************************************************

#ifndef Smt_TCALLSTATE_HH
#define Smt_TCALLSTATE_HH

#include "CMUInterface.h"
#include "TCall.h"
#include "TConnID.h"

/////////////////////////////////////////////////////////////////////////////
// TCallState code
class TCallState: public Smt_StateService
{
public:
	TCallState( Smt_String name,
				 Smt_Uint loglevel, Smt_String logpath,
				 Smt_Server* pserver);

	~TCallState();

public: // virtual method
	Smt_Uint OnUserOnline(Smt_Uint sender, Smt_String sendername);
	Smt_Uint OnUserOffline(Smt_Uint sender, Smt_String sendername);
	Smt_Uint OnTimer(Smt_Uint& timerid, Smt_Uint& messageid, Smt_Uint& senderobj);
	Smt_Uint InitStates();
	Smt_StateObject * PrehandleMessage(Smt_Pdu &pdu);
	
public: 
	// Event handle method
	TCall* OnEvtInitiated(Smt_Pdu& pdu);
	TCall* OnEvtOriginated(Smt_Pdu& pdu);
	TCall* OnEvtAlerting(Smt_Pdu& pdu);
	TCall* OnEvtConnected(Smt_Pdu& pdu);
	TCall* OnEvtDisconnected(Smt_Pdu& pdu);
	TCall* OnEvtReleased(Smt_Pdu& pdu);
	TCall* OnEvtHeld(Smt_Pdu& pdu);
	TCall* OnEvtQueued(Smt_Pdu& pdu);
	TCall* OnEvtFailed(Smt_Pdu& pdu);
	TCall* TransferredCall(TConnID* pconnid, Smt_Pdu &pdu);
	TCall* ConferencedCall(TConnID* pconnid, TCall* pcall, Smt_Pdu &pdu);
	TCall* SingleStepTransferredCall(TConnID* pconnid, Smt_Pdu &pdu);
	TCall* OnEvtClearCallEvent(Smt_Pdu &pdu);

	Smt_Uint OnCmdMakeCallEx(Smt_Pdu& pdu);
	Smt_Uint OnRespMakeCall(Smt_Pdu& pdu);
	Smt_Uint OnCmdAnswerCallEx(Smt_Pdu& pdu);
	Smt_Uint OnCmdHangupCallEx(Smt_Pdu& pdu);
	Smt_Uint OnCmdSingleStepTransferEx(Smt_Pdu& pdu);
	Smt_Uint OnCmdDial(Smt_Pdu& pdu);
	Smt_Uint RespFailure(Smt_Uint evtid, Smt_Uint error, Smt_Pdu pdu);
	Smt_Uint RespCmd(Smt_Uint evtid, Smt_Pdu pdu);
	Smt_Uint OnCmdMediaAction(Smt_Pdu& pdu);
	Smt_Uint OnEvtMediaEvent(Smt_Pdu& pdu);
	Smt_Uint OnEvtChannelDataReached(Smt_Pdu& pdu);
	Smt_Uint CmdHangupCall(Smt_Uint callid, Smt_String deviceid, Smt_String connid );
	Smt_Uint OnCmdSubscribeCallEvent(Smt_Pdu& pdu);
	Smt_Uint OnCmdUnsubscribeCallEvent(Smt_Pdu& pdu);
	Smt_Uint PublishEvent(Smt_Pdu& pdu);
	Smt_Uint OnCmdAssignEx(Smt_Pdu& pdu);
	Smt_Uint OnCmdDeassignEx(Smt_Pdu& pdu);
	Smt_Uint OnEvtLinkEvent(Smt_Pdu& pdu);
	Smt_Uint OnCmdSetData(Smt_Pdu& pdu);
	Smt_Uint OnCmdGetData(Smt_Pdu& pdu);
	Smt_Uint OnEvtSSTransferTimer(Smt_Uint timerid);
	Smt_Uint OnCmdClearCall(Smt_Pdu& pdu);
	Smt_Uint OnCmdDoSIPHeader(Smt_Pdu& pdu);
	Smt_Uint OnRespDoSIPHeader(Smt_Pdu& pdu);

	TConnID* LookupConnIDByDeviceID(Smt_String deviceid);
	TConnID* LookupOneConnID(TCall* pcall);
	TConnID* LookupOtherConnID(Smt_String connid, Smt_Uint callid);

public:
	Smt_Lock   m_Lock;
	Smt_Uint   m_CallIndex;
	Smt_Uint   m_ConnectionStateGOR;
	Smt_Uint   m_DeviceStateGOR;
	TConnIDMap   m_ConnIDMgr;
	TCallMap     m_CallMgr;
	Smt_Map<Smt_Uint, Smt_String> m_SubMgr;

	Smt_Uint   AllocateCallID();
	Smt_String GetIDName(Smt_Uint msgid);
	Smt_Uint   ReleaseCall(Smt_Uint callid);
	Smt_Uint   SendConnectionEvent(Smt_Pdu& pdu);
	//Add by caoyj 2011-03-07
	Smt_Uint   RemoveConnIDFromCall(Smt_String connid);
};

extern TCallState*  g_pCallState;
/************************************************************************/

#endif // Smt_TCALLSTATE_HH
