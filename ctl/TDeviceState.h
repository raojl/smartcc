//***************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CTL
//      Create Date : 2010.9.3												
//      Author      : Shenzj								
//      Discription : DeviceState ¿‡
//      Modify By   :											
//***************************************************************************

#ifndef Smt_TDEVICESTATE_HH
#define Smt_TDEVICESTATE_HH

#include "CMUInterface.h"
#include "TDevice.h"
#include "TConnID.h"

/////////////////////////////////////////////////////////////////////////////
// TDeviceState code
class TDeviceState: public Smt_StateService
{
public:
	TDeviceState( Smt_String name,
				 Smt_Uint loglevel, Smt_String logpath,
				 Smt_Server* pserver);

	~TDeviceState();

public: // virtual method
	Smt_Uint OnUserOnline(Smt_Uint sender, Smt_String sendername);
	Smt_Uint OnUserOffline(Smt_Uint sender, Smt_String sendername);
	Smt_Uint OnTimer(Smt_Uint& timerid, Smt_Uint& messageid, Smt_Uint& senderobj);
	Smt_Uint InitStates();
	Smt_StateObject * PrehandleMessage(Smt_Pdu &pdu);
	
public: 
	// Event handle method
	TDevice* OnEvtInitiated(Smt_Pdu& pdu);
	TDevice* OnEvtOriginated(Smt_Pdu& pdu);
	TDevice* OnEvtAlerting(Smt_Pdu& pdu);
	TDevice* OnEvtConnected(Smt_Pdu& pdu);
	TDevice* OnEvtDisconnected(Smt_Pdu& pdu);
	TDevice* OnEvtReleased(Smt_Pdu& pdu);
	TDevice* OnEvtHeld(Smt_Pdu& pdu);
	TDevice* OnEvtQueued(Smt_Pdu& pdu);
	TDevice* OnEvtFailed(Smt_Pdu& pdu);
	TDevice* OnEvtChannelDataReached(Smt_Pdu &pdu);
	TDevice* OnEvtDeviceTimerExpired(Smt_Pdu &pdu);

	Smt_Uint OnCmdAssign(Smt_Pdu& pdu);
	Smt_Uint OnRespAssign( Smt_Pdu& pdu);
	Smt_Uint OnCmdDeassign(Smt_Pdu& pdu);
	Smt_Uint RespFailure(Smt_Uint evtid, Smt_Uint error, Smt_Pdu pdu);
	Smt_Uint RespCmd(Smt_Uint evtid, Smt_Pdu pdu);
	Smt_Uint OnCmdMakeCall(Smt_Pdu& pdu);
	Smt_Uint OnCmdHangupCall(Smt_Pdu& pdu);
	Smt_Uint OnCmdHoldCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdRetrieveCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdConsultationCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdTransferCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdConferenceCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdReconnectCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdSingleStepTransfer(Smt_Pdu &pdu);
	Smt_Uint OnCmdSingleStepConference(Smt_Pdu &pdu);
	Smt_Uint OnCmdRouteSelected(Smt_Pdu &pdu);
	Smt_Uint OnCmdSendDTMF(Smt_Pdu &pdu);
	Smt_Uint OnCmdRecord(Smt_Pdu &pdu);
	Smt_Uint OnCmdSendMessage(Smt_Pdu &pdu);
	Smt_Uint OnEvtLinkEvent(Smt_Pdu &pdu);
	Smt_Uint OnCmdDeviceSnapshot(Smt_Pdu &pdu);
	Smt_Uint OnCmd_To_CallState(Smt_Pdu &pdu);
	Smt_Uint OnEvtOtherDeviceEvent(Smt_Pdu &pdu);
	Smt_Uint OnEvtMessageReceived(Smt_Pdu &pdu);

	Smt_Uint AllocateDeviceRefID();
	Smt_Uint AllocateRouteID();
	Smt_String AllocateMeetmeNum();	
	Smt_String GetIDName(Smt_Uint msgid);
	Smt_String GetStateName(Smt_Uint stateid);
	Smt_Uint   OnEvtDeviceTimer(Smt_Uint senderobj);
	TDevice* LookupDeviceByDeviceRefID( Smt_Uint devicerefid );
	TDevice* LookupDeviceByCallID(Smt_Uint callid, Smt_String otherdeviceid );
	Smt_String LookupOtherPartyByCallID(Smt_String deviceid, Smt_Uint callid);
	Smt_String LookupDeviceIDByConnID(Smt_String connid);
	TRouteID* LookupRouteIDByCallID(Smt_Uint callid);
	Smt_Uint OnCmdQueryLinkState(Smt_Pdu &pdu);

public:
	Smt_Lock   m_Lock;
	Smt_Uint   m_ConnectionStateGOR;
	Smt_Uint   m_CallStateGOR;
	TDeviceMap   m_DeviceMgr;
	Smt_Uint   m_MonitorIndex;
	Smt_Uint   m_MeetmeIndex;	
	Smt_Uint   m_RouteIndex; 
	TRouteIDMap  m_RouteIDMgr;
	Smt_Uint   m_LinkState;
	Smt_Uint   m_LinkReason;
};

extern TDeviceState*  g_pDeviceState;
/************************************************************************/

#endif // Smt_TDEVICESTATE_HH
