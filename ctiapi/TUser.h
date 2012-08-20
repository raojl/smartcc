//***************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CMUAPI1.0
//      Create Date : 2009.11.27												
//      Author      : shenzj@Smtcrm.com
//      Discription : CMUDLL Macro and Event Struct Defines
//      Modify By   :
//***************************************************************************

#ifndef H_Smt_TCMUCLIENTUSER_H
#define H_Smt_TCMUCLIENTUSER_H

#include "CMUInterface.h"
#include "include/cmuapidefs.h"

#define StrAssign(dst,src)  strncpy(dst,src,CMU_STRING_LENGTH-1);dst[CMU_STRING_LENGTH-1]=0
#define LStrAssign(dst,src) strncpy(dst,src,CMU_LONG_STRING_LENGTH-1);dst[CMU_LONG_STRING_LENGTH-1]=0

/////////////////////////////////////////////////////////////////////////////
// TPlayList code
class TPlayList
{
public:
	TPlayList(Smt_Uint callid)
	{
		m_callid = callid; 
		m_PlayIndex = 0;
	}
	~TPlayList(){;}
	void AddFile(Smt_String filename)
	{
		m_PlayList[m_PlayIndex] = filename;
		m_PlayIndex++;
	}
	void Clear(){m_PlayIndex=0;}
public:
	Smt_Uint   m_callid;
	Smt_Uint   m_PlayIndex;
	Smt_String m_PlayList[MAX_FILENAME_ARRAY];
};

/////////////////////////////////////////////////////////////////////////////
// TUser code
class TUser: public Smt_User
{
public:
	typedef Smt_Map<Smt_Uint, TPlayList*> TPlayListMgr;

public:
	TUser( Smt_String name,
		  Smt_Uint loglevel, Smt_String logpath,
		  Smt_Server* pserver );
	~TUser();

public:
	Smt_Uint HandleMessage(Smt_Pdu& pdu);
	Smt_Uint OnUserOnline(Smt_Uint sender, Smt_String sendername);
	Smt_Uint OnUserOffline(Smt_Uint sender, Smt_String sendername);
	Smt_Uint OnTimer(Smt_Uint& timerid, Smt_Uint& messageid, Smt_Uint& senderobj);

public:
	Smt_String m_DeviceStateName;
	Smt_Uint   m_DeviceStateGOR;
	Smt_String m_CallStateName;
	Smt_Uint   m_CallStateGOR;

	Smt_MessageQueue m_EventQueue;
	TPlayListMgr m_PlayList;
	Smt_Uint   m_QueryLinkTimer;
	Smt_Uint   m_LastLinkState;

	Smt_Uint PutEventQueue(Csta_Event* pevt);
	Smt_Uint GetEventQueue(Csta_Event* pevt, Smt_Uint timeout );
	Smt_Uint GetEvtType(Smt_Uint messageid);
	Smt_String GetEvtName(Smt_Uint evttype);

	Smt_Uint OnEvtCallEvent(Smt_Pdu& pdu);
	Smt_Uint OnEvtDeviceBackIn_OutOfService(Smt_Pdu& pdu);
	Smt_Uint OnEvtDeviceEvent(Smt_Pdu& pdu);
	Smt_Uint OnEvtRouteEvent(Smt_Pdu& pdu);
	Smt_Uint OnEvtMediaEvent(Smt_Pdu& pdu);
	Smt_Uint OnEvtDeviceRecordEvent(Smt_Pdu& pdu);\
	Smt_Uint OnEvtMessageReceived(Smt_Pdu& pdu);
	Smt_Uint OnEvtLinkUp(Smt_Pdu& pdu);
	Smt_Uint OnEvtLinkDown(Smt_Pdu& pdu);
	Smt_Uint OnEvtQueryLinkTimer();
	Smt_Uint OnEvtTTSPlayEnd(Smt_Pdu& pdu);

	Smt_Uint CmdAssign(Smt_Uint invokeid, Smt_String deviceid, Smt_Uint devicetype );
	Smt_Uint OnRespAssign(Smt_Pdu& pdu);
	Smt_Uint CmdDeassign(Smt_Uint channelid );
	Smt_Uint OnRespCtcResponse(Smt_Pdu& pdu);
	Smt_Uint CmdMakeCall(Smt_Uint channelid, Smt_String callednum, Smt_String callernum );
	Smt_Uint CmdHangupCall(Smt_Uint channelid, Smt_Uint callid );
	Smt_Uint CmdClearCall(Smt_Uint channelid, Smt_Uint callid );
	Smt_Uint CmdHoldCall(Smt_Uint channelid, Smt_Uint callid );
	Smt_Uint CmdRetrieveCall(Smt_Uint channelid, Smt_Uint callid );
	Smt_Uint CmdConsultationCall(Smt_Uint channelid, Smt_Uint callid, Smt_String callednum, Smt_String callernum, Smt_Uint timeout);
	Smt_Uint CmdConferenceCall(Smt_Uint channelid, Smt_Uint holdcallid, Smt_Uint activecallid );
	Smt_Uint CmdTransferCall(Smt_Uint channelid, Smt_Uint holdcallid, Smt_Uint activecallid );
	Smt_Uint CmdReconnectCall(Smt_Uint channelid, Smt_Uint holdcallid, Smt_Uint activecallid );
	Smt_Uint CmdSingleStepTransfer(Smt_Uint channelid, Smt_String callednum, Smt_Uint callid );
	Smt_Uint CmdSingleStepConference(Smt_Uint channelid, Smt_String callednum, Smt_Uint callid, Smt_Uint mode );
	Smt_Uint CmdRouteSelected(Smt_Uint channelid, Smt_Uint routeid, Smt_String callednum, Smt_String agentid );
	Smt_Uint CmdSendDTMF(Smt_Uint channelid, Smt_Uint callid, Smt_String digits );
	Smt_Uint CmdStartRecord(Smt_Uint channelid, Smt_Uint callid, Smt_String filename );
	Smt_Uint CmdStopRecord(Smt_Uint channelid, Smt_Uint callid );
	Smt_Uint CmdSetData(Smt_Uint channelid, Smt_Uint callid, Smt_String key, Smt_String value );
	Smt_Uint CmdGetData(Smt_Uint channelid, Smt_Uint callid, Smt_String key );
	Smt_Uint OnRespGetData(Smt_Pdu& pdu);
	Smt_Uint CmdDeviceSnapshot(Smt_Uint channelid);
	Smt_Uint OnRespDeviceSnapshot(Smt_Pdu& pdu);
	Smt_Uint CmdSendMessage(Smt_Uint channelid, Smt_Uint callid, Smt_String message );

	Smt_Uint CmdAssignEx(Smt_Uint invokeid, Smt_String deviceid, Smt_Uint devicetype );
	Smt_Uint OnRespAssignEx(Smt_Pdu& pdu);
	Smt_Uint CmdDeassignEx(Smt_Uint channelid );
	Smt_Uint OnRespGcResponse(Smt_Pdu& pdu);
	Smt_Uint CmdMakeCallEx(Smt_Uint invokeid, Smt_String deviceid, Smt_String callednum, Smt_String callernum, Smt_Uint timeout );
	Smt_Uint OnRespMakeCallEx(Smt_Pdu& pdu);
	Smt_Uint CmdAnswerCallEx(Smt_Uint callid );
	Smt_Uint CmdHangupCallEx(Smt_Uint callid );
	Smt_Uint CmdSingleStepTransferEx(Smt_Uint callid, Smt_String callednum );
	Smt_Uint CmdPlayFile(Smt_Uint callid, Smt_String filename, Smt_String escapedigits );
	Smt_Uint CmdAddFileList(Smt_Uint callid, Smt_String filename );
	Smt_Uint CmdPlayFileList(Smt_Uint callid, Smt_String escapedigits );
	Smt_Uint CmdGetDigits(Smt_Uint callid, Smt_Uint timeout, Smt_Uint maxcount );
	Smt_Uint CmdSendDTMFEx(Smt_Uint callid, Smt_String digits );
	Smt_Uint CmdRecordEx(Smt_Uint callid, Smt_String filename, Smt_String escapedigits, Smt_Uint timeout, Smt_Uint silence );
	Smt_Uint CmdSendFax(Smt_Uint channelid,Smt_Uint callid,Smt_String message);
	Smt_Uint CmdReceiveFax(Smt_Uint callid );
	Smt_Uint CmdDial(Smt_Uint callid, Smt_String callednum, Smt_Uint timeout );
	Smt_Uint CmdSendMessageEx(Smt_Uint callid, Smt_String message );
	Smt_Uint CmdSetSIPHeader(Smt_Uint callid, Smt_String headerkey, Smt_String headervalue ); 
	Smt_Uint CmdGetSIPHeader(Smt_Uint callid, Smt_String headerkey );
	Smt_Uint OnRespGetSIPHeader(Smt_Pdu& pdu);

	Smt_Uint CmdSubscribeCallEvent( );
	Smt_Uint CmdUnsubscribeCallEvent( );
	Smt_Uint CmdQueryLinkState( );
	Smt_Uint OnRespQueryLinkState(Smt_Pdu& pdu);

	Smt_Uint CmdTTSPlay(Smt_Uint callid,Smt_String message);
};

extern  TUser* g_pCMUDLLUser;
/************************************************************************/

#endif // H_Smt_TCMUCLIENTUSER_H
