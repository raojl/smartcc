//***************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CMU
//      Create Date : 2009.10.21												
//      Author      : Shenzj								
//      Discription : Connection 类
//      Modify By   :											
//***************************************************************************

#ifndef Smt_TCONNECTION_HH
#define Smt_TCONNECTION_HH

#include "CMUInterface.h"

class TConnectionState;

/////////////////////////////////////////////////////////////////////////////
// TConnection code
class TConnection:  public Smt_DLList_Node, public Smt_StateObject
{
public:
	TConnection(){;}
	TConnection(Smt_String objid, Smt_Uint initstate, TConnectionState* powner );
	~TConnection();

public: 
	// virtual method
	Smt_Uint OnUnexpectedEvent(Smt_Pdu &pdu);
	Smt_Uint OnStateExit(Smt_Pdu &pdu);
	Smt_Uint OnStateEnter(Smt_Pdu &pdu);

	// action method
	Smt_Uint EvtInitiated(Smt_Pdu &pdu);
	Smt_Uint EvtOriginated(Smt_Pdu &pdu);
	Smt_Uint EvtAlerting(Smt_Pdu &pdu);
	Smt_Uint EvtInitToQueued(Smt_Pdu &pdu);
	Smt_Uint EvtConnected(Smt_Pdu &pdu);
	Smt_Uint EvtHeld(Smt_Pdu &pdu);
	Smt_Uint EvtRetrieved(Smt_Pdu &pdu);
	Smt_Uint EvtDisconnected(Smt_Pdu &pdu);
	Smt_Uint EvtReleased(Smt_Pdu &pdu);
	Smt_Uint SendEvent(Smt_Uint evtid );	
	Smt_Uint EvtRecord( Smt_Uint evtid );
	Smt_Uint EvtMedia( Smt_Uint evtid );
	Smt_Uint EvtQueued(Smt_Pdu &pdu);
	Smt_Uint EvtInitToAlerting(Smt_Pdu &pdu);
	Smt_Uint EvtOriToConnected(Smt_Pdu &pdu);
	Smt_Uint EvtDialResponse();
	Smt_Uint SendMessageEvent(Smt_Pdu &pdu);
	Smt_Uint EvtMeetmeConnected(Smt_Pdu &pdu);
	Smt_Uint EvtMeetmeDisconnected(Smt_Pdu &pdu);
	Smt_Uint EvtFailed(Smt_Pdu &pdu);
	Smt_Uint RespSetSIPHeader(Smt_Uint reason);
	Smt_Uint RespGetSIPHeader(Smt_Uint reason, Smt_String headervalue);
	Smt_Uint SetTimer(Smt_Pdu &pdu);//add by caoyj 20120315
	Smt_Uint ClearTimer();//add by caoyj 20120315

public:
	Smt_Uint   m_AMIHandle;
	Smt_Uint   m_AGIHandle;
	Smt_String m_Channel;
	Smt_Uint   m_ChannelType;       // 在 Asterisk Dial 事件内 Channel 的类型
	Smt_String m_Uniqueid;
	Smt_String m_CallerID;
	Smt_String m_CallerIDName;
	Smt_Uint   m_DeviceType;
	Smt_String m_DeviceID;
	Smt_String m_FullDeviceID;
	Smt_String m_Source;
	Smt_String m_Destination;
	Smt_Pdu    m_LastCommand;
	Smt_String m_OtherConnectionID;
	Smt_Uint   m_Reason;
	Smt_Bool   m_Recording;
	Smt_String m_RecordFileName;
	Smt_String m_PlayFileName;
	Smt_String m_AGIScriptName;
	Smt_Uint   m_AGIEvtTimerID;
	Smt_String m_AGIData;
	Smt_Uint   m_AGITimeLen;
	Smt_Uint   m_AGIReason;
	Smt_Uint   m_AGIAction;
	Smt_DateTime m_StartRecordTime;
	Smt_String m_PlayList[MAX_FILENAME_ARRAY];
	Smt_Uint   m_PlayListCurrIndex;
	Smt_Uint   m_PlayListIndex;
	Smt_String m_PlayListEscapeDigit;
	Smt_Uint   m_PlayListTimeLen;
	Smt_String m_TACCode;
	Smt_String m_Message;
	Smt_String m_MeetmeNum;
	Smt_Uint   m_UserNum;
	Smt_String m_Variable;
	Smt_Uint   m_TimerID;//add by caoyj 20120315
	Smt_String m_CurrContext;//add by caoyj 20120315

private:
	Smt_Uint   m_LastState;
};

typedef Smt_Map<Smt_String, TConnection*> TConnectionMap;
/************************************************************************/

#endif // Smt_TCONNECTION_HH
