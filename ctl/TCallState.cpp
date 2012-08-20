//***************************************************************************
// TCallState.cpp : implementation file
//***************************************************************************

#include "TCallState.h"
#include "TDeviceState.h"
#include "TConnectionState.h"

TCallState*  g_pCallState = NULL;

/////////////////////////////////////////////////////////////////////////////
// TCallState code
TCallState::TCallState( Smt_String name, 
						Smt_Uint loglevel, Smt_String logpath, 
						Smt_Server* pserver )
: Smt_StateService( name, loglevel, logpath, pserver )
{
	m_CallIndex = 0;
	m_DeviceStateGOR = 0;
}

TCallState::~TCallState()
{
	m_SubMgr.RemoveAll();

	// release call
	TCall* pCall = NULL;
	TCallMap::ITERATOR iter1(m_CallMgr);
	for (TCallMap::ENTRY *entry1 = 0;
		iter1.next (entry1) != 0;
		iter1.advance ())
	{
		pCall = entry1->int_id_;		
		if(pCall != NULL)
		{	
			delete pCall;
			pCall = NULL;
		}
	}	
	
	m_CallMgr.RemoveAll();

	// release connid
	TConnID* pConnID = NULL;
	TConnIDMap::ITERATOR iter2(m_ConnIDMgr);
	for (TConnIDMap::ENTRY *entry2 = 0;
		iter2.next (entry2) != 0;
		iter2.advance ())
	{
		pConnID = entry2->int_id_;		
		if(pConnID != NULL)
		{	
			delete pConnID;
			pConnID = NULL;
		}
	}	
	
	m_ConnIDMgr.RemoveAll();
}

Smt_Uint TCallState::OnUserOnline(Smt_Uint sender, Smt_String sendername)
{
	PrintLog(4,"[TCallState::OnUserOnline ] sender<0x%x>, sendername<%s>.", sender, sendername.c_str());

	if( Smt_String(CMU_DEVICESTATENAME) == sendername )
	{
		m_DeviceStateGOR = sender;
	}

	if( Smt_String(CMU_CONNECTIONSTATE) == sendername )
	{
		m_ConnectionStateGOR = sender;
	}
	
	return Smt_Success;
}

Smt_Uint TCallState::OnUserOffline(Smt_Uint sender, Smt_String sendername)
{
	PrintLog(4,"[TCallState::OnUserOffline] sender<0x%x>, sendername<%s>.", sender, sendername.c_str());

	if( Smt_String(CMU_DEVICESTATENAME) == sendername )
	{
		m_DeviceStateGOR = 0;
	}
	
	if( Smt_String(CMU_CONNECTIONSTATE) == sendername )
	{
		m_ConnectionStateGOR = 0;
	}

	m_SubMgr.Remove( sender );

	return Smt_Success;
}

Smt_Uint TCallState::OnTimer(Smt_Uint& timerid, Smt_Uint& messageid, Smt_Uint& senderobj)
{
	switch (messageid)
	{
	case Evt_ICMU_SSTransferTimer:
		OnEvtSSTransferTimer( timerid );
		break;
	}
	return Smt_Success;
}

Smt_Uint TCallState::InitStates()
{
	m_ShiftTable		
	+ new Smt_ScriptState(CALL_NULL, "CALL_NULL") 
		+ new Smt_ScriptRule(Evt_ICMU_Originated,CauseOriginated,CALL_ORIGINATED, (ST_ACTION)&TCall::EvtCallInitToOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpConferenced,CALL_CONFERENCED, (ST_ACTION)&TCall::EvtCallConferenced )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseTpTransferred,CALL_ORIGINATED, (ST_ACTION)&TCall::EvtCallClearedEx )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseSingleStepTransferred,CALL_INITIATED, (ST_ACTION)&TCall::EvtCallClearedEx )
//		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseSingleStepTransferredMeetme,CALL_INITIATED, (ST_ACTION)&TCall::EvtCallClearedEx )

	+ new Smt_ScriptState(CALL_INITIATED, "CALL_INITIATED") 
		+ new Smt_ScriptRule(Evt_ICMU_Originated,CauseOriginated,CALL_ORIGINATED, (ST_ACTION)&TCall::EvtCallOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNoActiveConnID,CALL_NULL, (ST_ACTION)&TCall::EvtCallCleared )		

	+ new Smt_ScriptState(CALL_ORIGINATED, "CALL_ORIGINATED") 
		+ new Smt_ScriptRule(Evt_ICMU_Alerting,CauseAlerting,CALL_DELIVERED, (ST_ACTION)&TCall::EvtCallDelivered )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseConnected,CALL_CONNECTED, (ST_ACTION)&TCall::EvtCallConnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNoActiveConnID,CALL_NULL, (ST_ACTION)&TCall::EvtCallCleared )
		+ new Smt_ScriptRule(Evt_ICMU_Queued,-1,CALL_QUEUED, (ST_ACTION)&TCall::EvtCallQueued )
		+ new Smt_ScriptRule(Evt_ICMU_Alerting,CauseIVRRequest,CALL_DELIVERED, (ST_ACTION)&TCall::EvtCallDelivered )

	+ new Smt_ScriptState(CALL_DELIVERED, "CALL_DELIVERED") 
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseConnected,CALL_CONNECTED, (ST_ACTION)&TCall::EvtCallConnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNoActiveConnID,CALL_NULL, (ST_ACTION)&TCall::EvtCallCleared )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseAGISuccess,CALL_CONNECTED, (ST_ACTION)&TCall::EvtCallConnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseDial,CALL_CONNECTED, (ST_ACTION)&TCall::EvtCallFailed )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseIVRRequest,CALL_CONNECTED, (ST_ACTION)&TCall::EvtCallConnected )

	+ new Smt_ScriptState(CALL_QUEUED, "CALL_QUEUED") 
		+ new Smt_ScriptRule(Evt_ICMU_Originated,-1,CALL_DELIVERED, (ST_ACTION)&TCall::EvtCallDelivered )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNoActiveConnID,CALL_NULL, (ST_ACTION)&TCall::EvtCallCleared )

	+ new Smt_ScriptState(CALL_CONNECTED, "CALL_CONNECTED") 
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNoActiveConnID,CALL_NULL, (ST_ACTION)&TCall::EvtCallCleared )
		+ new Smt_ScriptRule(Evt_ICMU_Held,-1,CALL_HELD, (ST_ACTION)&TCall::EvtCallHeld )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseListening,CALL_CONFERENCED, (ST_ACTION)&TCall::EvtCallConferenced )
		//+ new Smt_ScriptRule(Evt_ICMU_Released,CauseSingleStepTransferredMeetme,CALL_NULL, (ST_ACTION)&TCall::EvtCallCleared )
		+ new Smt_ScriptRule(Evt_ICMU_Originated,CauseOriginated,CALL_ORIGINATED, (ST_ACTION)&TCall::EvtCallOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_Alerting,CauseAlerting,CALL_DELIVERED, (ST_ACTION)&TCall::EvtCallDelivered )
		+ new Smt_ScriptRule(Evt_ICMU_Originated,CauseSingleStepTransferredEx,CALL_ORIGINATED, (ST_ACTION)&TCall::EvtCallTransferredToOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_Originated,CauseNotKnown,CALL_ORIGINATED, (ST_ACTION)&TCall::EvtCallOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_ClearCallEvent,-1,CALL_NULL, (ST_ACTION)&TCall::EvtCallCleared )
		//+ new Smt_ScriptRule(Evt_ICMU_Released,CauseDial,CALL_NULL, (ST_ACTION)&TCall::EvtCallCleared )

	+ new Smt_ScriptState(CALL_HELD, "CALL_HELD")
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseConnected,CALL_CONNECTED, (ST_ACTION)&TCall::EvtCallConnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNoActiveConnID,CALL_NULL, (ST_ACTION)&TCall::EvtCallCleared )

	+ new Smt_ScriptState(CALL_CONFERENCED, "CALL_CONFERENCED") 
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseConnected,CALL_CONNECTED, (ST_ACTION)&TCall::EvtCallConnected )

	+ new Smt_ScriptState(CALL_FAILED, "CALL_FAILED") 
		//+ new Smt_ScriptRule(Evt_ICMU_Originated,-1,CALL_INITIATED, (ST_ACTION)&TCall::EvtCallInitiated )
	;
	return Smt_Success;
}

Smt_StateObject* TCallState::PrehandleMessage(Smt_Pdu &pdu)
{
	TCall* pCall = NULL;
	switch ( pdu.m_MessageID )
	{
		// IVR Command
		case Cmd_ICMU_SubscribeCallEvent:
			OnCmdSubscribeCallEvent(pdu);
			break;
		case Cmd_ICMU_UnsubscribeCallEvent:
			OnCmdUnsubscribeCallEvent(pdu);
			break;
		case Cmd_ICMU_AssignEx:
			OnCmdAssignEx(pdu);
			break;
		case Cmd_ICMU_DeassignEx:
			OnCmdDeassignEx(pdu);
			break;
 		case Cmd_ICMU_MakeCallEx:
 			OnCmdMakeCallEx( pdu );
 			break;
		case Resp_ICMU_MakeCall:
			OnRespMakeCall( pdu );
 			break;
		case Cmd_ICMU_AnswerCallEx:
			OnCmdAnswerCallEx( pdu );
			break;
		case Cmd_ICMU_HangupCallEx:
			OnCmdHangupCallEx( pdu );
			break;
		case Cmd_ICMU_SingleStepTransferEx:
			OnCmdSingleStepTransferEx( pdu );
			break;
		case Cmd_ICMU_Dial:
			OnCmdDial(pdu);
			break;
		case Cmd_ICMU_SetData:
			OnCmdSetData( pdu );
			break;
		case Cmd_ICMU_GetData:
			OnCmdGetData( pdu );
			break;
		case Cmd_ICMU_ClearCall:
			OnCmdClearCall( pdu );
			break;

		case Cmd_ICMU_SendDTMFEx:
		case Cmd_ICMU_PlayFile:
		case Cmd_ICMU_PlayFileList:
		case Cmd_ICMU_GetDigits:
		case Cmd_ICMU_RecordEx:
		case Cmd_ICMU_SendMessageEx:
		case Cmd_ICMU_TTSPlay:
			OnCmdMediaAction(pdu);
			break;
		case Cmd_ICMU_SetSIPHeader:
		case Cmd_ICMU_GetSIPHeader:
			OnCmdDoSIPHeader(pdu);
			break;
		case Resp_ICMU_SetSIPHeader:
		case Resp_ICMU_GetSIPHeader:
			OnRespDoSIPHeader(pdu);
			break;
		case Resp_ICMU_TTSPlay:
			break;
		// Connection Event
		case Evt_ICMU_Initiated:
			pCall = OnEvtInitiated(pdu);
			break;
		case Evt_ICMU_Originated:
			pCall = OnEvtOriginated(pdu);
			break;
		case Evt_ICMU_Alerting:
			pCall = OnEvtAlerting(pdu);
			break;
		case Evt_ICMU_Connected:
			pCall = OnEvtConnected(pdu);
			break;
		case Evt_ICMU_Disconnected:
			pCall = OnEvtDisconnected(pdu);
			break;
		case Evt_ICMU_Released:
			pCall = OnEvtReleased(pdu);
			break;				
		case Evt_ICMU_Held:
			pCall = OnEvtHeld(pdu);
			break;
		case Evt_ICMU_Queued:
			pCall = OnEvtQueued(pdu);
			break;
		case Evt_ICMU_Failed:
			pCall = OnEvtFailed(pdu);
			break;	
		case Evt_ICMU_ChannelDataReached:
			OnEvtChannelDataReached(pdu);
			break;
		case Evt_ICMU_ClearCallEvent:
			pCall = OnEvtClearCallEvent(pdu);
			break;			

		// Media Event
		case Evt_ICMU_Playing:
		case Evt_ICMU_PlayEnd:
		case Evt_ICMU_Geting:
		case Evt_ICMU_GetEnd:
		case Evt_ICMU_Sending:
		case Evt_ICMU_SendEnd:
		case Evt_ICMU_Recording:
		case Evt_ICMU_RecordEnd:
		case Evt_ICMU_MessageSending:
		case Evt_ICMU_MessageSendEnd:
		//case Evt_ICMU_TTSPlayEnd:
			OnEvtMediaEvent(pdu);
		break;

		case Evt_ICMU_LinkUp:
		case Evt_ICMU_LinkDown:
			OnEvtLinkEvent( pdu );
		break;

		default:
			PrintLog(3, "[TCallState::PrehandleMessage] Receive Unknown Message, PduInf %s .",
				pdu.GetInfo().c_str() );
		break;
	}
	return pCall;
}

/****************************************************************************
函 数 名: OnEvtInitiated
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TCall* TCallState::OnEvtInitiated(Smt_Pdu& pdu)
{	
	Smt_String strConnectionID;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceType;
	Smt_String strCallerID;
	Smt_Uint   nConnectionState;
	TConnID* pConnID = NULL;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_ConnectionStatus,&nConnectionState );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_DeviceType, &nDeviceType );
	
	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) == Smt_Success )
	{
		m_ConnIDMgr.Remove( strConnectionID );
		delete pConnID;
		pConnID = NULL;
	}

	pConnID = new TConnID( strConnectionID );
	pConnID->m_ConnState = nConnectionState;
	pConnID->m_DeviceID = strDeviceID;
	pConnID->m_DeviceType = nDeviceType;
	m_ConnIDMgr.SetAt( strConnectionID, pConnID );
	
	SendConnectionEvent( pdu );

	PrintLog(5,"[TCallState::OnEvtInitiated] DeviceID<%s>,ConnectionID<%s>,CallerID<%s>.", 
		strDeviceID.c_str(), strConnectionID.c_str(), strCallerID.c_str() );
	
	return NULL;
}

/****************************************************************************
函 数 名: OnEvtOriginated
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TCall* TCallState::OnEvtOriginated(Smt_Pdu& pdu)
{	
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;
	TConnID* pOtherConnID = NULL;
	
	Smt_String strConnectionID;
	Smt_Uint   nConnState;
	Smt_String strOtherConnectionID;
	Smt_String strDeviceID;
	Smt_String strCallerID;
	Smt_String strCallerIDName;
	Smt_String strSource;
	Smt_String strDestination;
	Smt_String strMeetmeNum;
	Smt_Uint   nReason;
	Smt_Uint   nCallID;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_ConnectionStatus, &nConnState );
	pdu.GetString( Key_ICMU_OtherConnectionID, &strOtherConnectionID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_CallerID, &strCallerID );
	pdu.GetString( Key_ICMU_CallerIDName, &strCallerIDName );
	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetString( Key_ICMU_Destination, &strDestination );
	pdu.GetString( Key_ICMU_MeetmeNum, &strMeetmeNum );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	
	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtOriginated] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}

	Smt_Bool bFindOk = Smt_BoolFALSE;
	do 
	{
		if( m_CallMgr.Lookup(pConnID->m_CallID, pCall) == Smt_Success )
		{
			bFindOk = Smt_BoolTRUE;
			break;
		}

		if( m_ConnIDMgr.Lookup(strOtherConnectionID, pOtherConnID) != Smt_Success )
		{
			break;
		}

		if( m_CallMgr.Lookup(pOtherConnID->m_CallID, pCall) == Smt_Success )
		{
			bFindOk = Smt_BoolTRUE;
			break;
		}
	} while (0);
	
	if( bFindOk == Smt_BoolFALSE )
	{
		nCallID = AllocateCallID();
		pCall = new TCall(HLFormatStr("%d",nCallID), CALL_NULL, this );
		m_CallMgr.SetAt(nCallID, pCall );
	}

	pConnID->m_CallID = pCall->m_CallID;
	pConnID->m_ConnState = nConnState;
	pConnID->m_DeviceID = strDeviceID;
	pConnID->m_CallerID = strCallerID;
	pConnID->m_CallerIDName = strCallerIDName;
	pConnID->m_OtherConnID = strOtherConnectionID;
	pConnID->m_Source = strSource;
	pConnID->m_Destination = strDestination;

	//if( nReason != CauseListening )   ??? 监听后只能挂机了，无法再咨询转移等
	//{	
	//	pCall->AddConnID( pConnID );
	//}
	
	pCall->AddConnID( pConnID );
	pCall->m_InitiatedParty = pConnID->m_Source;

	if( strDeviceID == pConnID->m_Source )
	{
		pCall->m_CallingParty = pConnID->m_CallerID;
		pCall->m_CalledParty = pConnID->m_Destination;
	}
	else if( strDeviceID == pConnID->m_Destination )
	{
		pCall->m_CalledParty = pConnID->m_CallerID;
	}
	else
	{
		pCall->m_CallingParty = pConnID->m_CallerID;
		pCall->m_CalledParty = pConnID->m_Destination;
	}	

	pCall->SendConnectionEvent( pdu );

	do 
	{
		if( pCall->m_LastCommand.m_MessageID == Cmd_ICMU_SingleStepTransferEx )
		{
			pCall->m_LastCommand.Clear();
			pCall->m_CalledParty = "";
			pdu.m_Status = CauseSingleStepTransferredEx;
			break;
		}

		if( pCall->GetConnIDCount() == 1 )
		{
			pdu.m_Status = CauseOriginated;
			break;
		}
	} while (0);

	PrintLog(5,"[TCallState::OnEvtOriginated] DeviceID<%s>, ConnectionID<%s>.", 
		strDeviceID.c_str(), strConnectionID.c_str() );

	return pCall;
}

/****************************************************************************
函 数 名: OnEvtAlerting
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TCall* TCallState::OnEvtAlerting(Smt_Pdu& pdu)
{	
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;
	TConnID* pOtherConnID = NULL;
	Smt_String strConnectionID;
	Smt_Uint   nConnState;
	Smt_String strDeviceID;
	Smt_String strSource;
	Smt_String strDestination;
	Smt_String strCallerID;
	Smt_Uint   nReason;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_ConnectionStatus, &nConnState );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetString( Key_ICMU_Destination, &strDestination );
	pdu.GetString( Key_ICMU_CallerID, &strCallerID );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );

	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtAlerting] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}

	pConnID->m_ConnState = nConnState;
	pConnID->m_Reason = nReason;

	do 
	{
		if( m_CallMgr.Lookup(pConnID->m_CallID, pCall) != Smt_Success )
		{
			break;
		}
		
		// 单步转移时，新的通道缺少 OldCallID，从对方通道获取
		if( m_ConnIDMgr.Lookup(pConnID->m_OtherConnID, pOtherConnID) == Smt_Success )
		{
			if( pOtherConnID->m_OldCallID != 0 )
				pConnID->m_OldCallID = pOtherConnID->m_OldCallID;
		}

		pCall->m_AlertingParty = strDestination;
		pCall->SendConnectionEvent( pdu );

		pConnID->m_OldCallID = 0;  

		if( nReason == CauseIVRRequest )
		{
			pConnID->m_CallerID = strCallerID;
			pConnID->m_Source = strSource;
			pConnID->m_Destination = strDestination;

			pCall->m_CallingParty = pConnID->m_CallerID;
			pCall->m_CalledParty = pConnID->m_Destination;
			pdu.m_Status = CauseIVRRequest;
			break;
		}

		Smt_Uint nCount = 0;
		TConnID* pTmpConnID = NULL;
		for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); iter != pCall->m_ConnIDList.end(); iter++)
		{
			pTmpConnID = (*iter).int_id_;
			if(pTmpConnID->m_ConnState == CONN_ALERTING)
			{
				nCount++;
			}
		}
		
		if( nCount == 2 )
		{
			pdu.m_Status = CauseAlerting;
			break;
		}
		
	} while (0);

	PrintLog(5,"[TCallState::OnEvtAlerting] DeviceID<%s>, ConnectionID<%s>.",
		strDeviceID.c_str(), strConnectionID.c_str() );

	return pCall;
}

/****************************************************************************
函 数 名: OnEvtConnected
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TCall* TCallState::OnEvtConnected(Smt_Pdu& pdu)
{	
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;
	
	Smt_String strConnectionID;
	Smt_Uint   nConnState;
	Smt_String strDeviceID;
	Smt_String strSource;
	Smt_String strDestination;
	Smt_String strMeetmeNum;
	Smt_Uint   nReason;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_ConnectionStatus, &nConnState );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetString( Key_ICMU_Destination, &strDestination );
	pdu.GetString( Key_ICMU_MeetmeNum, &strMeetmeNum );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );

	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtConnected] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}
	
	pConnID->m_ConnState = nConnState;
	pConnID->m_Source = strSource;
	pConnID->m_Destination = strDestination;
	pConnID->m_MeetmeNum = strMeetmeNum;
	pConnID->m_Reason = nReason;

	Smt_Uint nCount = 0;
	Smt_Uint nTransferredCount = 0;
	Smt_Uint nMeetmeCount = 0;
	
	do 
	{
		if( m_CallMgr.Lookup(pConnID->m_CallID, pCall) != Smt_Success )
		{
			break;
		}

		//Smt_Uint nCount = 0;
		TConnID* pTmpConnID = NULL;
		for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
			iter != pCall->m_ConnIDList.end(); 
			iter++)
		{
			pTmpConnID = (*iter).int_id_;
			//if(pTmpConnID->m_ConnState == CONN_CONNECTED)
			//add by caoyj 20111124
			//监听强插方不算在内
			if(pTmpConnID->m_ConnState == CONN_CONNECTED && pTmpConnID->m_Reason != CauseListening )
			////////////////////////////////////////////////////////////////////////////////////////
			{
				nCount++;
			}
		}

		pCall->m_AnsweringParty = strDestination;

		// 转移后话路通话，清除 OldCallID、TransferringParty
		if(nCount == 2 && pCall->m_TransferringParty != "")   
		{
			pCall->SendConnectionEvent( pdu );

			//Smt_Uint nTransferredCount = 0;
			for(TConnIDMap::ITERATOR iter1 = pCall->m_ConnIDList.begin(); 
				iter1 != pCall->m_ConnIDList.end(); 
				iter1++)
			{
				pTmpConnID = (*iter1).int_id_;
				if(pTmpConnID->m_DeviceID == pConnID->m_DeviceID)
				{
					pTmpConnID->m_OldCallID = 0;
					pTmpConnID->m_Transferred = Smt_BoolTRUE;
				}		
				
				if(pTmpConnID->m_Transferred == Smt_BoolTRUE )
				{
					nTransferredCount++;
				}
			}

			if( nTransferredCount == 2 )
			{
				pCall->m_TransferringParty = "";
				pCall->m_ConsultingParty = "";
				pdu.m_Status = CauseConnected;
			}
			break;
		}

		// 检测是否会议
		if( nCount == 2 && pConnID->m_MeetmeNum != "")     
		{
			//Smt_Uint nMeetmeCount = 0;
			for(TConnIDMap::ITERATOR iter2 = m_ConnIDMgr.begin(); 
				iter2 != m_ConnIDMgr.end(); 
				iter2++ )
			{
				pTmpConnID = (*iter2).int_id_;
//				if( pTmpConnID->m_MeetmeNum == pConnID->m_MeetmeNum && 
//					pTmpConnID->m_ConnState == CONN_CONNECTED )
				//add by caoyj 20111124
				//监听强插方不算在内
				if(pTmpConnID->m_MeetmeNum == pConnID->m_MeetmeNum &&  pTmpConnID->m_ConnState == CONN_CONNECTED && pTmpConnID->m_Reason != CauseListening )
				////////////////////////////////////////////////////////////////////////////////////////
				{
					nMeetmeCount++;
				}
			}

			if( nMeetmeCount == 3)
			{
				pCall = ConferencedCall( pConnID, pCall, pdu );
				pdu.m_Status = CauseOpConferenced;
				break;  
			}
		}

		// 单步会议三方通话
		if( nReason == CauseListening )
		{
			pCall->SendConnectionEvent( pdu );
			pdu.m_Status = CauseListening;
			break;
		}

		// AGI 应答成功
		if( nReason == CauseAGISuccess )
		{
			pCall->SendConnectionEvent( pdu );
			pdu.m_Status = CauseAGISuccess;
			break;
		}

		if( nReason == CauseIVRRequest )
		{
			pdu.m_Status = CauseIVRRequest;
			break;
		}

		// 检测是否双方通话
		if( nCount == 2 )
		{
			pCall->SendConnectionEvent( pdu );
			pdu.m_Status = CauseConnected;
			break;
		}

		pCall->SendConnectionEvent( pdu );

	} while (0);

	PrintLog(5,"[TCallState::OnEvtConnected] DeviceID<%s>, ConnectionID<%s>,Status<%d>,nCount<%d>,nTransferredCount<%d>,nMeetmeCount<%d>,nReason<%d>.",
		strDeviceID.c_str(), strConnectionID.c_str(),pdu.m_Status,nCount,nTransferredCount,nMeetmeCount,nReason );
	
	return pCall;
}

/****************************************************************************
函 数 名: OnEvtDisconnected
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TCall* TCallState::OnEvtDisconnected(Smt_Pdu& pdu)
{	
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;
	
	Smt_String strConnectionID;
	Smt_Uint   nConnState;
	Smt_String strDeviceID;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_ConnectionStatus, &nConnState );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );

	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtDisconnected] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}
	
	pConnID->m_ConnState = nConnState;
	
	if( m_CallMgr.Lookup(pConnID->m_CallID, pCall) == Smt_Success )
	{
		pCall->SendConnectionEvent( pdu );
	}
	
	PrintLog(5,"[TCallState::OnEvtDisconnected] DeviceID<%s>, ConnectionID<%s>.", 
		strDeviceID.c_str(), strConnectionID.c_str());
	
	return pCall;
}

/****************************************************************************
函 数 名: OnEvtReleased
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TCall* TCallState::OnEvtReleased(Smt_Pdu& pdu)
{	
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;
	
	Smt_String strConnectionID;
	Smt_Uint   nConnState;
	Smt_String strDeviceID;
	Smt_String strSource;
	Smt_Uint   nReason;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_ConnectionStatus, &nConnState );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );

	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtReleased] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}

	pConnID->m_ConnState = nConnState;

	do 
	{
		if( m_CallMgr.Lookup(pConnID->m_CallID, pCall) != Smt_Success )
		{
			SendConnectionEvent( pdu );
			break;
		}

		if( pCall->GetState() == CALL_CONNECTED && 
			pConnID->m_CallID != 0 && pConnID->m_OldCallID != 0 ) // 咨询通话中，咨询方挂机， 发起转移
		{
			pCall = TransferredCall(pConnID, pdu);
			pdu.m_Status = CauseTpTransferred;
			break;
		}

		if( nReason == CauseSingleStepTransferred )
		{
			pCall = SingleStepTransferredCall(pConnID, pdu);
			pdu.m_Status = CauseSingleStepTransferred;
			break;
		}

		if( nReason == CauseSingleStepTransferredMeetme )
		{	
			TDevice* pDevice = NULL;
			if( g_pDeviceState->m_DeviceMgr.Lookup(strDeviceID, pDevice) == Smt_Success )
			{
				pDevice->ActSingleStepTransfer();
			}	

			pConnID->m_LastEvent = pdu;					
			pConnID->m_SSTransferTimerID = SetSingleTimer( DEFAULT_SHORTTIMERLEN*2, Evt_ICMU_SSTransferTimer, (Smt_Uint)pConnID );

			PrintLog(5, "[TCallState::OnEvtReleased] CallID<%d>, DeviceID<%s>, SSTransferTimerID<%d>. ",
				pCall->m_CallID, strDeviceID.c_str(), pConnID->m_SSTransferTimerID );

			pdu.m_Status = CauseSingleStepTransferredMeetme;
			break;
		}
		
		pCall->m_ReleaseingParty = pConnID->m_DeviceID;
		pCall->RemoveConnID( pConnID );    
		pCall->SendConnectionEvent( pdu );
	
		if( pCall->GetConnIDCount() == 2)                         // 会议 3 方，有一方退出，留下双方通话
		{
			TConnID* pTmpConnID = NULL;
			for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
				iter != pCall->m_ConnIDList.end(); 
				iter++)
			{
				pTmpConnID = (*iter).int_id_;

				//add by caoyj 20111124
				//如果是meteme监听强插时，剩下两方是监听发起方，应该拆线
				if(pTmpConnID->m_Reason == CauseListening)
				{
					CmdHangupCall( pCall->m_CallID, pTmpConnID->m_DeviceID, pTmpConnID->m_ConnID );
				}
                //////////////////////////////////////////////////////////////////////////////////////

				pTmpConnID->m_OldCallID = 0;
				pTmpConnID->m_SecOldCallID = 0;
			}

			pCall->m_ConferencingParty = "";
			pCall->m_ConsultingParty = "";
			pdu.m_Status = CauseConnected;
			break;
		}

		if( pCall->GetConnIDCount() == 1 )
		{
			TConnID* pTmpConnID = LookupOneConnID( pCall );
			if(pTmpConnID == NULL)
			{
				break;
			}

			//if( pCall->m_LastCommand.m_MessageID == Cmd_ICMU_Dial ) // Dial 时不需要挂机，由IVR流程控制是否挂机
			//{
			//	pdu.m_Status = CauseDial;
			//	break;	
			//}

			if( pTmpConnID->m_OldCallID == 0 )
			{
				if( pCall->m_LastCommand.m_MessageID == Cmd_ICMU_SingleStepTransferEx ) // IVR 外拨单步转移
				{
					pdu.m_Status = CauseSingleStepTransferredEx;
					break;	
				}
				CmdHangupCall( pCall->m_CallID, pTmpConnID->m_DeviceID, pTmpConnID->m_ConnID );
			}
			else if( pTmpConnID->m_OldCallID == pCall->m_CallID )  // 被保持方挂机
			{
				TCall* pTmpCall = NULL;
				if(m_CallMgr.Lookup(pTmpConnID->m_CallID, pTmpCall) == Smt_Success)
				{
					pTmpCall->m_ConsultingParty = "";
				}
				pTmpConnID->m_OldCallID = 0;
				pCall->RemoveConnID( pTmpConnID );
				pdu.m_Status = CauseNoActiveConnID;
			}
			else // pTmpConnID->m_OldCallID != pCall->m_CallID     // 被咨询方挂机
			{
				pTmpConnID->m_CallID = pTmpConnID->m_OldCallID;
				pTmpConnID->m_OldCallID = 0;
				pCall->RemoveConnID( pTmpConnID );
				pdu.m_Status = CauseNoActiveConnID;
			}
			break;
		}

		if( pCall->GetConnIDCount() == 0 )
		{
			pdu.m_Status = CauseNoActiveConnID;
			break;
		}

	} while (0);
			
	if( pConnID != NULL )		
	{
		// 以下原因码，需要通过超时删除，以便发送单步转移的 Transferred 事件
		if( nReason != CauseSingleStepTransferred && nReason != CauseSingleStepTransferredMeetme ) 
		{
			RemoveConnIDFromCall( strConnectionID ); // Add by caoyj  2011-03-07 

			m_ConnIDMgr.Remove( strConnectionID );
			delete pConnID;
			pConnID = NULL;
		}	
	}

	PrintLog(5,"[TCallState::OnEvtReleased] DeviceID<%s>, ConnectionID<%s>.", 
		strDeviceID.c_str(), strConnectionID.c_str() );

	return pCall;
}

/****************************************************************************
函 数 名: OnEvtHeld
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TCall* TCallState::OnEvtHeld(Smt_Pdu& pdu)
{	
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;
	
	Smt_String strConnectionID;
	Smt_Uint   nConnState;
	Smt_String strDeviceID;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_ConnectionStatus, &nConnState );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );

	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtHeld] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}
	
	pConnID->m_ConnState = nConnState;
	
	if( m_CallMgr.Lookup(pConnID->m_CallID, pCall) == Smt_Success )
	{
		pCall->m_HoldingParty = pCall->LookupOtherParty( strConnectionID );
		pCall->SendConnectionEvent( pdu );
	}
	
	PrintLog(5,"[TCallState::OnEvtHeld] DeviceID<%s>, ConnectionID<%s>.", 
		strDeviceID.c_str(), strConnectionID.c_str() );
	
	return pCall;
}

/****************************************************************************
函 数 名: OnEvtQueued
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TCall* TCallState::OnEvtQueued(Smt_Pdu& pdu)
{	
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;
	
	Smt_String strConnectionID;
	Smt_Uint   nConnState;
	Smt_String strDeviceID;
	Smt_String strSource;
	Smt_String strDestination;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_ConnectionStatus, &nConnState );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetString( Key_ICMU_Destination, &strDestination );

	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtQueued] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}
	
	pConnID->m_ConnState = nConnState;
	
	if( m_CallMgr.Lookup(pConnID->m_CallID, pCall) == Smt_Success )
	{
		pCall->m_CalledParty = strDestination;
		pCall->SendConnectionEvent( pdu );
	}
	
	PrintLog(5,"[TCallState::OnEvtQueued] DeviceID<%s>, ConnectionID<%s>.", 
		strDeviceID.c_str(), strConnectionID.c_str() );
	
	return pCall;
}

/****************************************************************************
函 数 名: OnEvtFailed
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TCall* TCallState::OnEvtFailed(Smt_Pdu& pdu)
{	
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;
	
	Smt_String strConnectionID;
	Smt_Uint   nConnState;
	Smt_String strDeviceID;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_ConnectionStatus, &nConnState );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );

	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtFailed] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}
	
	pConnID->m_ConnState = nConnState;
	
	if( m_CallMgr.Lookup(pConnID->m_CallID, pCall) == Smt_Success )
	{
		pCall->SendConnectionEvent( pdu );
	}
	
	PrintLog(5,"[TCallState::OnEvtFailed] DeviceID<%s>, ConnectionID<%s>.", 
		strDeviceID.c_str(), strConnectionID.c_str() );
	
	return pCall;
}

/****************************************************************************
函 数 名: AllocateCallID
参    数:
返回数值: 
功能描述: 分配呼叫ID
*****************************************************************************/
Smt_Uint TCallState::AllocateCallID()
{
	Smt_SingleLock sLock(m_Lock);
	
	m_CallIndex++;
	if( m_CallIndex >= 0xFFFF )
	{
		m_CallIndex = 1;
	}
	
	return m_CallIndex;
}

Smt_String TCallState::GetIDName(Smt_Uint msgid)
{
	return g_pConnState->GetIDName( msgid );
}

Smt_Uint TCallState::SendConnectionEvent(Smt_Pdu& pdu)
{
	Smt_String strConnectionID;
	Smt_String strDeviceID;	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );

	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	SendMessage( pdu );

	PrintLog(5,"[TCallState::SendConnectionEvent] ConnectionID<%s>, DeviceID<%s>.", 
		strConnectionID.c_str(), strDeviceID.c_str() );

	return Smt_Success;
}

Smt_Uint TCallState::ReleaseCall(Smt_Uint callid)
{
	TCall* pCall = NULL;
	if( m_CallMgr.Lookup(callid, pCall) == Smt_Success )
	{
		m_CallMgr.Remove( callid );
		delete pCall;
		pCall = NULL;
	}

	return Smt_Success;
}

Smt_Uint TCallState::OnEvtChannelDataReached(Smt_Pdu& pdu)
{
	TConnID* pConnID = NULL;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	Smt_String strVariable;
	Smt_String strConsultingParty;
	Smt_String strConsultedParty;
	Smt_String strTransferredParty;
	Smt_Uint nCallID;
	Smt_Uint nOldCallID;
	TCall* pCall = NULL;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_Variable, &strVariable );
	pdu.GetString( Key_ICMU_ConsultingParty, &strConsultingParty );
	pdu.GetString( Key_ICMU_ConsultedParty, &strConsultedParty );
	pdu.GetString( Key_ICMU_TransferredParty, &strTransferredParty );
	
	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtChannelDataReached] Lookup ConnID Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}

	strDeviceID = pConnID->m_DeviceID;
	nCallID = pConnID->m_CallID;
	pConnID->m_Variable = strVariable;

	if( m_CallMgr.Lookup( nCallID, pCall) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtChannelDataReached] Lookup Call Fail, CallID<%d>.",
			nCallID );
		return NULL;
	}

	if(strConsultingParty != "")
	{
		TConnID* pConsultingConnID = LookupConnIDByDeviceID( strConsultingParty );
		if( pConsultingConnID != NULL )
		{
			nOldCallID = pConsultingConnID->m_CallID;
			
			pConsultingConnID->m_CallID = nCallID;
			pConsultingConnID->m_OldCallID = nOldCallID;
			
			pCall->AddConnID( pConsultingConnID );
			pCall->m_InitiatedParty = strConsultingParty;
			pCall->m_AlertingParty = strDeviceID;
			pCall->m_CallingParty = strConsultingParty;
			pCall->m_CalledParty = strConsultedParty;
			pCall->m_ConsultingParty = strConsultingParty;
		}
	}

	if( strTransferredParty != "" )
	{
		TConnID* pTransferredConnID = LookupConnIDByDeviceID( strTransferredParty );
		if( pTransferredConnID != NULL )
		{
			nOldCallID = pTransferredConnID->m_CallID;

			pTransferredConnID->m_CallID = nCallID;
			pConnID->m_OtherConnID = pTransferredConnID->m_ConnID;
			
			pCall->AddConnID( pTransferredConnID );
			pCall->m_InitiatedParty = strTransferredParty;
			pCall->m_AlertingParty = strDeviceID;
			pCall->m_CallingParty = strTransferredParty;
			pCall->m_CalledParty = strDeviceID;

			// meetme 的单步转移发起方定 OldCallID
			TConnID* pTransferringConnID = LookupOtherConnID(pTransferredConnID->m_ConnID, nOldCallID);
			if( pTransferringConnID != NULL )
			{		
				pTransferringConnID->m_CallID = nCallID;
				pTransferringConnID->m_OldCallID = nOldCallID;

				PrintLog(5,"[TCallState::OnEvtChannelDataReached] TransferredParty<%s>, TransferringParty<%s>, CallID<%d>, OldCallID<%d>.", 
					strTransferredParty.c_str(), pTransferringConnID->m_DeviceID.c_str(), nCallID, nOldCallID );
			}
		}
	}

	pCall->SendConnectionEvent( pdu );

	PrintLog(5,"[TCallState::OnEvtChannelDataReached] ConnectionID<%s>, ConsultingParty<%s>, TransferredParty<%s>, Variable<%s>.", 
		strConnectionID.c_str(), strConsultingParty.c_str(), strTransferredParty.c_str(), strVariable.c_str() );

	return Smt_Success;
}

TConnID* TCallState::LookupConnIDByDeviceID(Smt_String deviceid)
{
	TConnID* pConnID = NULL;
	TConnID* pTmpConnID = NULL;
	for( TConnIDMap::ITERATOR 
		iter = m_ConnIDMgr.begin();
		iter != m_ConnIDMgr.end(); 
		iter++ )
	{
		pTmpConnID = (*iter).int_id_;
		if(pTmpConnID->m_DeviceID == deviceid )
		{
			pConnID = pTmpConnID;
			break;
		}
	}

	return pConnID;
}

TConnID* TCallState::LookupOneConnID(TCall* pcall)
{
	TCall* pCall = pcall;
	if( pCall == NULL || pCall->GetConnIDCount() == 0 )
	{
		return NULL;
	}

	// IVR 呼入类型
	TConnID* pTmpConnID = NULL;
	if( pCall->GetConnIDCount() == 1 )
	{
		for(TConnIDMap::ITERATOR 
			iter = pCall->m_ConnIDList.begin(); 
			iter!= pCall->m_ConnIDList.end(); 
			iter++)
		{
			pTmpConnID = (*iter).int_id_;
			if(pTmpConnID != NULL )  
			{
				return pTmpConnID;
			} 
		} 
	}

	// IVR 呼出类型
	if( pCall->GetConnIDCount() == 2 )
	{
		Smt_String strDeviceID;
		TDevice* pDevice = NULL;

		for(TConnIDMap::ITERATOR 
			iter1 = pCall->m_ConnIDList.begin(); 
			iter1!= pCall->m_ConnIDList.end(); 
			iter1++)
		{
			pTmpConnID = (*iter1).int_id_;
			strDeviceID = pTmpConnID->m_DeviceID;
			if( g_pDeviceState->m_DeviceMgr.Lookup(strDeviceID, pDevice ) == Smt_Success )
			{
				if( pDevice->m_DeviceType == DTYPE_IVR_OUTBOUND) 
					return pTmpConnID;
			}		
		} 
	}

	return NULL;
}

TConnID* TCallState::LookupOtherConnID(Smt_String connid, Smt_Uint callid)
{
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;
	TConnID* pTmpConnID = NULL;
	
	if( (m_CallMgr.Lookup(callid, pCall) == Smt_Success) &&
		(pCall->m_ConnIDList.GetCount() <= 2) )
	{	
		for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
			iter!= pCall->m_ConnIDList.end(); 
			iter++)
		{
			pTmpConnID = (*iter).int_id_;
			if(pTmpConnID->m_ConnID != connid)
			{
				pConnID = pTmpConnID;
				break;
			}
		}		
	}
		
	return pConnID;
}

Smt_Uint TCallState::CmdHangupCall(Smt_Uint callid, Smt_String deviceid, Smt_String connid )
{
	Smt_Uint   nCallID = callid;
	Smt_String strDeviceID = deviceid;
	Smt_String strConnID = connid;

	Smt_Pdu sendpdu;
	sendpdu.m_Sender = g_pCallState->GetGOR();
	sendpdu.m_Receiver = g_pCallState->m_ConnectionStateGOR;
	sendpdu.m_MessageID = Cmd_ICMU_HangupCall;
	
	sendpdu.PutString( Key_ICMU_DeviceID, strDeviceID );
	sendpdu.PutString( Key_ICMU_ConnectionID, strConnID );
	
	if( sendpdu.m_Receiver > 0 ) g_pCallState->PostMessage( sendpdu );
	
	PrintLog(5, "[TCall::CmdHangupCall] CallID<%d>, DeviceID<%s>, ConnID<%s>. ",
		nCallID, strDeviceID.c_str(), strConnID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: TransferredCall
参    数:
返回数值: 
功能描述: 删除C1/C2，新建 C3
*****************************************************************************/
TCall* TCallState::TransferredCall(TConnID* pconnid, Smt_Pdu &pdu)
{
	Smt_Uint nNewCallID;
	Smt_Uint nCallID = pconnid->m_CallID;
	Smt_Uint nOldCallID = pconnid->m_OldCallID;
	Smt_String strDeviceID = pconnid->m_DeviceID;
	TConnID* pConnID = pconnid;
	TCall* pCall = NULL;
	TCall* pOldCall = NULL;
	TCall* pNewCall = NULL;

	if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success ||
		m_CallMgr.Lookup(nOldCallID, pOldCall) != Smt_Success )
	{
		PrintLog(5, "[TCallState::TransferredCall] Lookup Call Fial, CallID<%d>, OldCallID<%d>. ",
			nCallID, nOldCallID );
		return NULL;
	}

	// 创建 NewCall ，并把其他两方加入到新建 Call
	nNewCallID = AllocateCallID();
	pNewCall = new TCall(HLFormatStr("%d",nNewCallID), CALL_NULL, this);
	m_CallMgr.SetAt(nNewCallID, pNewCall );

	TConnID* pTmpConnID = NULL;
	for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
		iter != pCall->m_ConnIDList.end(); 
		iter++)
	{
		pTmpConnID = (*iter).int_id_;
		//if(pTmpConnID->m_DeviceID != pConnID->m_DeviceID )
		//add by caoyj 20111124
		//如果是监听强插连接不用加
		if(pTmpConnID->m_DeviceID != pConnID->m_DeviceID && pTmpConnID->m_Reason != CauseListening)
		{
			pNewCall->AddConnID( pTmpConnID );
			pTmpConnID->m_OldCallID = pTmpConnID->m_CallID;
			pTmpConnID->m_CallID = nNewCallID;			
			pTmpConnID->m_SecOldCallID = 0;
			pTmpConnID->m_Transferred = Smt_BoolFALSE;
			PrintLog(5, "[TCallState::TransferredCall] Add ConnID 1, NewCallID<%d>, DeviceID<%s>.",
				nNewCallID, pTmpConnID->m_DeviceID.c_str() );
			break;
		}
	}

	for(TConnIDMap::ITERATOR iter1 = pOldCall->m_ConnIDList.begin(); 
		iter1 != pOldCall->m_ConnIDList.end(); 
		iter1++)
	{
		pTmpConnID = (*iter1).int_id_;
		if(pTmpConnID->m_DeviceID != pConnID->m_DeviceID )
		{
			pNewCall->AddConnID( pTmpConnID );
			pTmpConnID->m_OldCallID = pTmpConnID->m_CallID;
			pTmpConnID->m_CallID = nNewCallID;
			pTmpConnID->m_SecOldCallID = 0;
			pTmpConnID->m_Transferred = Smt_BoolFALSE;
			PrintLog(5, "[TCallState::TransferredCall] Add ConnID 2, NewCallID<%d>, DeviceID<%s>.",
				nNewCallID, pTmpConnID->m_DeviceID.c_str() );
			break;
		}
	}

	pConnID->m_CallID = nNewCallID;
	pConnID->m_OldCallID = nOldCallID;
	pConnID->m_SecOldCallID = nCallID;

	pNewCall->m_CallingParty = pOldCall->m_CallingParty;
	pNewCall->m_CalledParty = pCall->m_CalledParty;
	pNewCall->m_TransferringParty = strDeviceID;
	pNewCall->SendConnectionEvent( pdu );

	pCall->m_TransferringParty = strDeviceID;
	pCall->m_ReleaseingParty = strDeviceID;
	//pCall->RemoveAllConnID();

	pOldCall->m_TransferringParty = strDeviceID;
	pOldCall->m_ReleaseingParty = strDeviceID;
	//pOldCall->RemoveAllConnID();

	pdu.PutUint( Key_ICMU_NoUseCallID, nCallID);
	pdu.PutUint( Key_ICMU_NoUseOldCallID, nOldCallID);

	PrintLog(5, "[TCallState::TransferredCall] Create Call Ok, NewCallID<%d>, OldCallID<%d>, SecOldCallID<%d>. ",
		nNewCallID, nOldCallID, nCallID );
	
	return pNewCall;
}

/****************************************************************************
函 数 名: ConferencedCall
参    数:
返回数值: 
功能描述: 删除C1/C2，新建 C3
*****************************************************************************/
TCall* TCallState::ConferencedCall(TConnID* pconnid, TCall* pcall, Smt_Pdu &pdu)
{
	TCall* pHoldCall = pcall;
 	TConnID* pHoldConnID = pconnid;
	TConnID* pConnID = NULL;  // Controller ConnID
	TConnID* pTmpConnID = NULL;

	for(TConnIDMap::ITERATOR iter0 = pHoldCall->m_ConnIDList.begin(); 
		iter0 != pHoldCall->m_ConnIDList.end(); 
		iter0++)
	{
		pTmpConnID = (*iter0).int_id_;
		if( pTmpConnID->m_DeviceID != pHoldConnID->m_DeviceID )
		{
			pConnID = pTmpConnID;
			break;
		}
	}

	if(pConnID == NULL )
	{
		PrintLog(5, "[TCallState::ConferencedCall] Lookup Controller Fial, HoldCallID<%d>, HoldDeviceID<%d>. ",
			pHoldCall->m_CallID, pHoldConnID->m_DeviceID.c_str() );
		return NULL;
	}

	Smt_Uint nNewCallID;
	Smt_Uint nCallID;
	Smt_Uint nOldCallID;
	TCall* pNewCall = NULL;
	TCall* pCall = NULL;
	TCall* pOldCall = NULL;
	Smt_String strDeviceID;
	
	strDeviceID = pConnID->m_DeviceID;
	nCallID = pConnID->m_CallID;
	nOldCallID = pConnID->m_OldCallID;

	if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success ||
		m_CallMgr.Lookup(nOldCallID, pOldCall) != Smt_Success )
	{
		PrintLog(5, "[TCallState::ConferencedCall] Lookup Call Fial, CallID<%d>, OldCallID<%d>. ",
			nCallID, nOldCallID );
		return NULL;
	}

	// 创建 NewCall ，并把 3 方加入到新建 Call
	nNewCallID = AllocateCallID();
	pNewCall = new TCall(HLFormatStr("%d",nNewCallID), CALL_NULL, this);
	m_CallMgr.SetAt(nNewCallID, pNewCall );
	
	pNewCall->AddConnID( pConnID );
	pConnID->m_OldCallID = nOldCallID;
	pConnID->m_SecOldCallID = pConnID->m_CallID;
	pConnID->m_CallID = nNewCallID;			
	
	PrintLog(5, "[TCall::ConferencedCall] Add ConnID 1, NewCallID<%d>, DeviceID<%s>.",
		nNewCallID, pConnID->m_DeviceID.c_str() );

	for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
		iter != pCall->m_ConnIDList.end(); 
		iter++)
	{
		pTmpConnID = (*iter).int_id_;
		if(pTmpConnID->m_DeviceID != pConnID->m_DeviceID )
		{
			pNewCall->AddConnID( pTmpConnID );
			pTmpConnID->m_OldCallID = pTmpConnID->m_CallID;
			pTmpConnID->m_CallID = nNewCallID;			
			pTmpConnID->m_SecOldCallID = 0;
			PrintLog(5, "[TCallState::ConferencedCall] Add ConnID 2, NewCallID<%d>, DeviceID<%s>.",
				nNewCallID, pTmpConnID->m_DeviceID.c_str() );
			break;
		}
	}
	
	for(TConnIDMap::ITERATOR iter1 = pOldCall->m_ConnIDList.begin(); 
		iter1 != pOldCall->m_ConnIDList.end(); 
		iter1++)
	{
		pTmpConnID = (*iter1).int_id_;
		if(pTmpConnID->m_DeviceID != pConnID->m_DeviceID )
		{
			pNewCall->AddConnID( pTmpConnID );
			pTmpConnID->m_OldCallID = pTmpConnID->m_CallID;
			pTmpConnID->m_CallID = nNewCallID;
			pTmpConnID->m_SecOldCallID = 0;
			PrintLog(5, "[TCallState::ConferencedCall] Add ConnID 3, NewCallID<%d>, DeviceID<%s>.",
				nNewCallID, pTmpConnID->m_DeviceID.c_str() );
			break;
		}
	}
		
	pNewCall->m_ConferencingParty = strDeviceID;
	pNewCall->m_CallingParty = pCall->m_CallingParty;
	pNewCall->m_CalledParty = pCall->m_CalledParty;
	pNewCall->SendConnectionEvent( pdu );
		
	pCall->m_ConferencingParty = strDeviceID;
	//pCall->RemoveAllConnID();
	
	pOldCall->m_ConferencingParty = strDeviceID;
	//pOldCall->RemoveAllConnID();
	
	pdu.PutUint( Key_ICMU_NoUseCallID, nCallID);
	pdu.PutUint( Key_ICMU_NoUseOldCallID, nOldCallID);

	PrintLog(5, "[TCallState::ConferencedCall] Create Call Ok, NewCallID<%d>, OldCallID<%d>, SecOldCallID<%d>. ",
		nNewCallID, nOldCallID, nCallID );
	
	return pNewCall;
}

/****************************************************************************
函 数 名: SingleStepTransferredCall
参    数:
返回数值: 
功能描述: 删除C1，新建C2
*****************************************************************************/
TCall* TCallState::SingleStepTransferredCall(TConnID* pconnid, Smt_Pdu &pdu)
{
	Smt_Uint nNewCallID;
	Smt_Uint nCallID;
	TCall* pNewCall = NULL;
	TCall* pCall = NULL;
	Smt_String strDeviceID;
	TConnID* pConnID = pconnid;
	
	strDeviceID = pConnID->m_DeviceID;
	nCallID = pConnID->m_CallID;
	if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success )
	{
		PrintLog(5, "[TCallState::SingleStepTransferredCall] Lookup Call Fial, CallID<%d>. ",
			nCallID );
		return NULL;
	}
	
	// 创建 NewCall ，并把对方加入到新建 Call
	nNewCallID = AllocateCallID();
	pNewCall = new TCall(HLFormatStr("%d",nNewCallID), CALL_NULL, this);
	m_CallMgr.SetAt(nNewCallID, pNewCall );
	
	TConnID* pTmpConnID = NULL;
	for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
	iter != pCall->m_ConnIDList.end(); 
	iter++)
	{
		pTmpConnID = (*iter).int_id_;
		//if(pTmpConnID->m_DeviceID != pConnID->m_DeviceID )
		//add by caoyj 20111124
		//如果是监听强插连接不用加
		if(pTmpConnID->m_DeviceID != pConnID->m_DeviceID && pTmpConnID->m_Reason != CauseListening)
		{
			pNewCall->AddConnID( pTmpConnID );
			pTmpConnID->m_CallID = nNewCallID;	
			pTmpConnID->m_OldCallID = nCallID;					
			PrintLog(5, "[TCallState::SingleStepTransferredCall] Add ConnID 1, NewCallID<%d>, DeviceID<%s>.",
				nNewCallID, pTmpConnID->m_DeviceID.c_str() );
			break;
		}
	}
	
	pConnID->m_CallID = nNewCallID;
	pConnID->m_OldCallID = nCallID;
	pConnID->m_LastEvent = pdu;
	pConnID->m_SSTransferTimerID = SetSingleTimer( DEFAULT_SHORTTIMERLEN*2, Evt_ICMU_SSTransferTimer, (Smt_Uint)pConnID );

	pNewCall->m_CallingParty = pCall->m_CallingParty;
	pNewCall->m_CalledParty = pCall->m_CalledParty;
	
	pCall->m_ReleaseingParty = strDeviceID;
	//pCall->RemoveConnID( pConnID );
	
	//pdu.PutUint( Key_ICMU_NoUseCallID, nCallID);
		
	PrintLog(5, "[TCallState::SingleStepTransferredCall] Create Call Ok, NewCallID<%d>, CallID<%d>, DeviceID<%s>, SSTransferTimerID<%d>. ",
		nNewCallID, nCallID, strDeviceID.c_str(), pConnID->m_SSTransferTimerID );
	
	return pNewCall;
}

/****************************************************************************
函 数 名: OnCmdMakeCallEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TCallState::OnCmdMakeCallEx(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	
	Smt_String strDeviceID;
	Smt_String strCallerNum;
	Smt_String strCalledNum;
	Smt_Uint   nInvokeID;
	Smt_Uint   nDeviceType;
	
	pdu.GetUint  ( Key_ICMU_InvokeID,  &nInvokeID );
	pdu.GetString( Key_ICMU_DeviceID,  &strDeviceID );
	pdu.GetString( Key_ICMU_CalledNum,  &strCalledNum );

	do
	{
		if( g_pDeviceState->m_DeviceMgr.Lookup(strDeviceID, pDevice) != Smt_Success )
		{
			RespFailure( Resp_ICMU_MakeCallEx, CauseInvDeviceID, pdu );
			break;
		}

		nDeviceType = pDevice->m_DeviceType;
		
		Smt_Pdu pduSend = pdu;
		pduSend.PutUint( Key_ICMU_DeviceType, nDeviceType );
		pduSend.PutUint( Key_ICMU_OriginalSender, pdu.m_Sender );
		
		pduSend.m_Sender = GetGOR();
		pduSend.m_Receiver = m_ConnectionStateGOR;
		pduSend.m_MessageID = Cmd_ICMU_MakeCall;
		if( pduSend.m_Receiver > 0 )PostMessage( pduSend );			
	}
	while( 0 );
	
	PrintLog(5, "[TCallState::OnCmdMakeCallEx] InvokeID<%d>,DeviceID<%s>,DeviceType<%d>, CalledNum<%s>.",
		nInvokeID, strDeviceID.c_str(), nDeviceType, strCalledNum.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnRespMakeCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TCallState::OnRespMakeCall(Smt_Pdu& pdu)
{
	TConnID* pConnID = NULL;
	Smt_String strConnectionID;
	Smt_String strDeviceID;
	Smt_Uint   nOriSender;
	Smt_Uint   nCallID;
	Smt_Uint   nInvokeID;

	pdu.GetUint  ( Key_ICMU_InvokeID,  &nInvokeID );
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_OriginalSender, &nOriSender );

	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnRespMakeCall] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}

	nCallID = pConnID->m_CallID;

	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = nOriSender;
	pdu.m_MessageID = Resp_ICMU_MakeCallEx;
	pdu.PutUint( Key_ICMU_CallID, nCallID );
	pdu.PutUint( Key_ICMU_Reason, CauseSuccess );
	if( pdu.m_Receiver > 0 ) PostMessage( pdu );

	PrintLog(5, "[TCallState::OnRespMakeCall] DeviceID<%s>, ConnectionID<%s>, CallID<%d>.",
		strDeviceID.c_str(), strConnectionID.c_str(), nCallID );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdAnswerCallEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TCallState::OnCmdAnswerCallEx(Smt_Pdu& pdu)
{
	TCall*   pCall   = NULL;
	TConnID* pConnID = NULL;	
	Smt_String strConnectionID;
	Smt_String strDeviceID;
	Smt_Uint   nCallID;

	pdu.GetUint( Key_ICMU_CallID, &nCallID );
	
	do 
	{
		if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success )
		{
			RespFailure( Resp_ICMU_AnswerCallEx, CauseInvCallID, pdu );
			break;
		}

		pConnID = LookupOneConnID( pCall );
		if( pConnID == NULL )
		{		
			RespFailure( Resp_ICMU_AnswerCallEx, CauseInvCallID, pdu );
			break;	
		}
	
		RespCmd( Resp_ICMU_AnswerCallEx, pdu );

		pCall->m_LastCommand = pdu;
		strDeviceID = pConnID->m_DeviceID;
		strConnectionID = pConnID->m_ConnID;
	
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_AnswerCallEx;
		pdu.PutString( Key_ICMU_ConnectionID, strConnectionID );
	
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}while( 0 );

	PrintLog(5, "[TCallState::OnCmdAnswerCallEx] CallID<%d>, DeviceID<%s>, ConnectionID<%s>.", 
		nCallID, strDeviceID.c_str(), strConnectionID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdHangupCallEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TCallState::OnCmdHangupCallEx(Smt_Pdu& pdu)
{
	TCall*   pCall   = NULL;
	TConnID* pConnID = NULL;	
	Smt_String strConnectionID;
	Smt_String strDeviceID;
	Smt_Uint   nCallID;
	
	pdu.GetUint( Key_ICMU_CallID, &nCallID );
	
	do 
	{
		if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success )
		{
			RespFailure( Resp_ICMU_HangupCallEx, CauseInvCallID, pdu );
			break;
		}

		pConnID = LookupOneConnID( pCall );
		if( pConnID == NULL )
		{		
			RespFailure( Resp_ICMU_HangupCallEx, CauseInvCallID, pdu );
			break;	
		}
		
		RespCmd( Resp_ICMU_HangupCallEx, pdu );

		pCall->m_LastCommand = pdu;
		strDeviceID = pConnID->m_DeviceID;
		strConnectionID = pConnID->m_ConnID;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_HangupCallEx;
		pdu.PutString( Key_ICMU_ConnectionID, strConnectionID );

		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}while( 0 );

	PrintLog(5, "[TCallState::OnCmdHangupCallEx] CallID<%d>, DeviceID<%s>, ConnectionID<%s>.", 
		nCallID, strDeviceID.c_str(), strConnectionID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdSingleStepTransferEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TCallState::OnCmdSingleStepTransferEx(Smt_Pdu& pdu)
{
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;	
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	Smt_String strCalledNum;
	TDevice* pCalledDevice = NULL;

	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetString( Key_ICMU_CalledNum, &strCalledNum );
	
	do 
	{
		if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success )
		{
			RespFailure( Resp_ICMU_SingleStepTransferEx, CauseInvCallID, pdu );
			break;
		}
		
		if( g_pDeviceState->m_DeviceMgr.Lookup(strCalledNum, pCalledDevice) == Smt_Success )
		{
			if( pCalledDevice->GetState() != DST_IDLE && pCalledDevice->m_DeviceType == DTYPE_EXTENSION )
			{
				RespFailure( Resp_ICMU_SingleStepTransferEx, CauseDestBusy, pdu );
				break;
			}
		}

		pConnID = LookupOneConnID( pCall );
		if( pConnID == NULL )
		{		
			RespFailure( Resp_ICMU_SingleStepTransferEx, CauseInvCallID, pdu );
			break;	
		}
		
		RespCmd(Resp_ICMU_SingleStepTransferEx, pdu);

		pCall->m_LastCommand = pdu;
		strDeviceID = pConnID->m_DeviceID;
		strConnectionID = pConnID->m_ConnID;

		if( pCall->m_ConnIDList.GetCount() == 1 )  // 呼入类型的单步转移
		{
			pdu.m_Sender = GetGOR();
			pdu.m_Receiver = m_ConnectionStateGOR;
			pdu.m_MessageID = Cmd_ICMU_SingleStepTransfer;		
			pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
			pdu.PutString( Key_ICMU_ConnectionID, strConnectionID );
			pdu.PutString( Key_ICMU_CalledNum, strCalledNum );
			
			if( pdu.m_Receiver > 0 ) PostMessage( pdu );
		}
		else// pCall->m_ConnIDList.GetCount() == 2  // IVR 外呼单步转移
		{
			pdu.m_Sender = GetGOR();
			pdu.m_Receiver = m_ConnectionStateGOR;
			pdu.m_MessageID = Cmd_ICMU_Dial;		
			pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
			pdu.PutString( Key_ICMU_ConnectionID, strConnectionID );	
			if( pdu.m_Receiver > 0 ) PostMessage( pdu );
		}
	}
	while( 0 );
	PrintLog(5, "[TCallState::OnCmdSingleStepTransferEx] CallID<%d>, DeviceID<%s>, CalledNum<%s>.",
		nCallID, strDeviceID.c_str(), strCalledNum.c_str() );
	
	return Smt_Success;
}

Smt_Uint TCallState::RespCmd(Smt_Uint evtid, Smt_Pdu pdu)
{
	Smt_Uint nOriSender;
	Smt_Uint nCallID;
	
	pdu.GetUint( Key_ICMU_CallID, &nCallID );	

	nOriSender = pdu.m_Sender;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = nOriSender;
	pdu.m_MessageID = evtid;
	pdu.m_Status = Smt_Success;
	pdu.PutUint( Key_ICMU_Reason, CauseSuccess );
	if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	
	PrintLog(5, "[TCallState::RespCmd] Resp<%s>, CallID<%d>.", 
		GetIDName(evtid).c_str(), nCallID );
	
	return Smt_Success;
}

Smt_Uint TCallState::RespFailure(Smt_Uint evtid, Smt_Uint error, Smt_Pdu pdu)
{
	Smt_Uint nOriSender;
	Smt_Uint nCallID;
	
	pdu.GetUint( Key_ICMU_CallID, &nCallID );
	
	nOriSender = pdu.m_Sender;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = nOriSender;
	pdu.m_MessageID = evtid;
	pdu.m_Status = Smt_Success;
	pdu.PutUint( Key_ICMU_Reason, error );

	if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	
	PrintLog(5, "[TCallState::RespFailure] RespFail<%s>, CallID<%d>, Reason<%d>.", 
		GetIDName(evtid).c_str(), nCallID, error);
	
	return Smt_Success;
}

Smt_Uint TCallState::OnCmdMediaAction(Smt_Pdu& pdu)
{
	TCall*   pCall   = NULL;
	TConnID* pConnID = NULL;
	
	Smt_Uint   nRespMsg;
	Smt_Uint   nCallID;
	Smt_String strConnectionID;
	
	pdu.GetUint( Key_ICMU_CallID, &nCallID );
	
	switch (pdu.m_MessageID)
	{
	case Cmd_ICMU_SendDTMFEx:
		nRespMsg = Resp_ICMU_SendDTMFEx;
		break;
	case Cmd_ICMU_PlayFile:
		nRespMsg = Resp_ICMU_PlayFile;
		break;
	case Cmd_ICMU_PlayFileList:
		nRespMsg = Resp_ICMU_PlayFileList;
		break;
	case Cmd_ICMU_GetDigits:
		nRespMsg = Resp_ICMU_GetDigits;
		break;
	case Cmd_ICMU_RecordEx:
		nRespMsg = Resp_ICMU_RecordEx;
		break;
	case Cmd_ICMU_SendMessageEx:
		nRespMsg = Resp_ICMU_SendMessageEx;
		break;
	case Cmd_ICMU_TTSPlay:
		nRespMsg = Resp_ICMU_TTSPlay;
		break;
	default:
		break;
	}
	
	do 
	{
		if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success )
		{
			RespFailure( nRespMsg, CauseInvCallID, pdu );
			break;
		}
		
		pConnID = LookupOneConnID( pCall );
		if( pConnID == NULL )
		{
			RespFailure( nRespMsg, CauseInvalidConnID, pdu );
			break;
		}

		RespCmd(nRespMsg, pdu);

		strConnectionID = pConnID->m_ConnID;
		pdu.PutString( Key_ICMU_ConnectionID, strConnectionID );
		
		pCall->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}while( 0 );

	PrintLog(5, "[TCallState::OnCmdMediaAction] MessageID<%s>, CallID<%d>, ConnectionID<%s>.",
		GetIDName(pdu.m_MessageID).c_str(), nCallID, strConnectionID.c_str() );		
	
	return Smt_Success;
}

Smt_Uint TCallState::OnEvtMediaEvent(Smt_Pdu& pdu)
{
	TCall*   pCall = NULL;
	TConnID* pConnID = NULL;
	Smt_String strConnectionID;
	Smt_Uint nCallID;
	Smt_Uint nReason;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	
	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnEvtMediaEvent] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return NULL;
	}
	
	nCallID = pConnID->m_CallID;
	if( m_CallMgr.Lookup(nCallID, pCall) == Smt_Success )
	{
		pdu.PutUint( Key_ICMU_CallID, nCallID );
		
		pdu.m_Sender   = GetGOR();
		pdu.m_Receiver = pCall->m_LastCommand.m_Sender;
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	
	PrintLog(5, "[TCallState::OnEvtMediaEvent] MessageID<%s>, CallID<%d>, ConnectionID<%s>, Reason<%d>",
		GetIDName(pdu.m_MessageID).c_str(), nCallID, strConnectionID.c_str(), nReason );
	
	return Smt_Success;
}


/****************************************************************************
函 数 名: OnCmdSubscribeCallEvent
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TCallState::OnCmdSubscribeCallEvent(Smt_Pdu& pdu)
{
	Smt_String strSubscriberName;
	Smt_Uint   nSubscriberGOR;
	
	pdu.GetString( Key_ICMU_SubscriberName, &strSubscriberName );
	nSubscriberGOR = pdu.m_Sender;
	
	Smt_String strTemp = "";
	if( m_SubMgr.Lookup(nSubscriberGOR, strTemp) != Smt_Success )
	{
		m_SubMgr.SetAt(nSubscriberGOR, strSubscriberName );
	}
	
	RespCmd(Resp_ICMU_SubscribeCallEvent, pdu);
	
	PrintLog(5, "[TCallState::OnCmdSubscribeCallEvent] Add Subscriber, SubscriberGOR<0x%x>, SubscriberName<%s>.",
		nSubscriberGOR, strSubscriberName.c_str() );
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdUnsubscribeCallEvent
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TCallState::OnCmdUnsubscribeCallEvent(Smt_Pdu& pdu)
{
	Smt_String strSubscriberName;
	Smt_Uint   nSubscriberGOR;
	
	pdu.GetString( Key_ICMU_SubscriberName, &strSubscriberName );
	nSubscriberGOR = pdu.m_Sender;
	
	if( m_SubMgr.Lookup(nSubscriberGOR, strSubscriberName) == Smt_Success )
	{
		m_SubMgr.Remove( nSubscriberGOR );
	}

	RespCmd(Resp_ICMU_UnsubscribeCallEvent, pdu);

	PrintLog(5, "[TCallState::OnCmdUnsubscribeCallEvent] SubscriberGOR<%#x>,SubscriberName<%s>.", nSubscriberGOR, strSubscriberName.c_str() );

	return Smt_Success;
}

Smt_Uint TCallState::PublishEvent(Smt_Pdu& pdu)
{
	Smt_Uint nSubscriberGOR = 0;	
	for( Smt_Map<Smt_Uint, Smt_String>::ITERATOR 
		iter = m_SubMgr.begin();
		iter != m_SubMgr.end(); 
		iter++ )
	{
		nSubscriberGOR = (*iter).ext_id_;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = nSubscriberGOR;
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	return Smt_Success;
}

Smt_Uint TCallState::OnCmdAssignEx(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceType;
	Smt_Uint   nDeviceRefID;
	
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_DeviceType, &nDeviceType );
	
	if( g_pDeviceState->m_DeviceMgr.Lookup(strDeviceID, pDevice) != Smt_Success )
	{
		pDevice = new TDevice(strDeviceID, nDeviceType, DST_IDLE, g_pDeviceState );
		g_pDeviceState->m_DeviceMgr.SetAt(strDeviceID, pDevice );
	}
	
	nDeviceRefID = pDevice->AddMonitor( pdu.m_Sender );
	pdu.PutUint( Key_ICMU_DeviceRefID, nDeviceRefID );

	RespCmd(Resp_ICMU_AssignEx, pdu);

	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_ConnectionStateGOR;
	pdu.m_MessageID = Cmd_ICMU_Assign;
	pdu.m_Status  = Smt_Success;	
	if( pdu.m_Receiver > 0 ) PostMessage( pdu );

	PrintLog(5, "[TCallState::OnCmdAssignEx] DeviceRefID<%d>,DeviceID<%s>,DeviceType<%d>.", 
		nDeviceRefID, strDeviceID.c_str(), nDeviceType );

	return Smt_Success;
}

Smt_Uint TCallState::OnCmdDeassignEx(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceRefID;
	
	pdu.GetUint( Key_ICMU_DeviceRefID, &nDeviceRefID );
	
	pDevice = g_pDeviceState->LookupDeviceByDeviceRefID(nDeviceRefID);
	if( pDevice == NULL )
	{
		RespFailure( Resp_ICMU_DeassignEx, CauseInvChannelID, pdu );
		return Smt_Fail;
	}
	
	pDevice->RemoveMonitor( nDeviceRefID );
	
	RespCmd(Resp_ICMU_DeassignEx, pdu);
	
	PrintLog(5, "[TTDeviceState::OnCmdDeassignEx] DeviceRefID<%d>, DeviceID<%s>.",
		nDeviceRefID, pDevice->GetID().c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtLinkEvent
参    数:
返回数值: 
功能描述: 发送Link事件到订阅者 和 DeviceState
*****************************************************************************/
Smt_Uint TCallState::OnEvtLinkEvent(Smt_Pdu& pdu)
{
	PublishEvent( pdu );

	pdu.m_Receiver = m_DeviceStateGOR;
	if( pdu.m_Receiver > 0 ) PostMessage( pdu );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdDial
参    数:
返回数值: 
功能描述: 共振,如手机、分机共振 
*****************************************************************************/
Smt_Uint TCallState::OnCmdDial(Smt_Pdu& pdu)
{
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;	
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	Smt_String strCalledNum;
	
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	
	do 
	{
		if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success )
		{
			RespFailure( Resp_ICMU_Dial, CauseInvCallID, pdu );
			break;
		}
		
		pConnID = LookupOneConnID( pCall );
		if( pConnID == NULL )
		{		
			RespFailure( Resp_ICMU_Dial, CauseInvCallID, pdu );
			break;	
		}
		
		RespCmd(Resp_ICMU_Dial, pdu);
		
		pCall->m_LastCommand = pdu;
		strDeviceID = pConnID->m_DeviceID;
		strConnectionID = pConnID->m_ConnID;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_Dial;		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strConnectionID );	
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TCallState::OnCmdDial] CallID<%d>, DeviceID<%s>.",
		nCallID, strDeviceID.c_str());

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdSetData
参    数:
返回数值: 
功能描述: 设置或获取呼叫数据
*****************************************************************************/
Smt_Uint TCallState::OnCmdSetData(Smt_Pdu& pdu)
{
	Smt_Uint nCallID;
	Smt_String strDataKey;
	Smt_String strDataValue;
	TCall* pCall = NULL;

	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetString( Key_ICMU_DataKey, &strDataKey );
	pdu.GetString( Key_ICMU_DataValue, &strDataValue );

	do 
	{
		if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success )
		{
			RespFailure( Resp_ICMU_SetData, CauseInvCallID, pdu );
			break;
		}
		
		pCall->SetData( strDataKey, strDataValue );	

		RespCmd( Resp_ICMU_SetData, pdu );
	}while( 0 );
	
	PrintLog(5, "[TCallState::OnCmdSetData] CallID<%d>, DataKey<%s>, DataValue<%s>.", 
		nCallID, strDataKey.c_str(), strDataValue.c_str() );

	return Smt_Success;
}

Smt_Uint TCallState::OnCmdGetData(Smt_Pdu& pdu)
{
	Smt_Uint nCallID;
	Smt_String strDataKey;
	Smt_String strDataValue;
	TCall* pCall = NULL;
	
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetString( Key_ICMU_DataKey, &strDataKey );
	
	do 
	{
		if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success )
		{
			RespFailure( Resp_ICMU_GetData, CauseInvCallID, pdu );
			break;
		}
		
		strDataValue = pCall->GetData( strDataKey );	
		pdu.PutString( Key_ICMU_DataValue, strDataValue );

		RespCmd( Resp_ICMU_GetData, pdu );
	}while( 0 );
	
	PrintLog(5, "[TCallState::OnCmdGetData] CallID<%d>, DataKey<%s>, DataValue<%s>.", 
		nCallID, strDataKey.c_str(), strDataValue.c_str() );
	
	return Smt_Success;
}

Smt_Uint TCallState::OnEvtSSTransferTimer(Smt_Uint timerid)
{
	Smt_Uint nTimerID = timerid;
	TConnID* pConnID = NULL;
	TConnID* pTmpConnID = NULL;
	for( TConnIDMap::ITERATOR 
		iter = m_ConnIDMgr.begin();
		iter != m_ConnIDMgr.end(); 
		iter++ )
	{
		pTmpConnID = (*iter).int_id_;
		if(pTmpConnID->m_SSTransferTimerID == nTimerID )
		{
			pConnID = pTmpConnID;
			break;
		}
	}

	Smt_String strDeviceID="";
	Smt_String strConnID="";
	Smt_Uint   nOldCallID = 0;
	TCall* pCall = NULL;
	if( pConnID != NULL )
	{
		strConnID = pConnID->m_ConnID;
		strDeviceID = pConnID->m_DeviceID;
		nOldCallID = pConnID->m_OldCallID;
/*
		if( m_CallMgr.Lookup(nOldCallID, pCall) == Smt_Success )
		{
			pCall->SendConnectionEvent( pConnID->m_LastEvent );

			Smt_Pdu pdu;
			pdu.m_Receiver = GetGOR();
			pdu.m_MessageID = Evt_ICMU_ClearCallEvent;
			pdu.PutUint( Key_ICMU_CallID, nOldCallID );
			if( pdu.m_Receiver > 0 ) PostMessage( pdu );
		}
*/
		//Add by caoyj 20110917
		if( m_CallMgr.Lookup(nOldCallID, pCall) == Smt_Success )
		{
			pCall->SendConnectionEvent( pConnID->m_LastEvent );
			
			Smt_Pdu pdu;
			pdu.m_Receiver = GetGOR();
			pdu.m_MessageID = Evt_ICMU_ClearCallEvent;
			pdu.PutUint( Key_ICMU_CallID, nOldCallID );
			if( pdu.m_Receiver > 0 ) PostMessage( pdu );
		}
		else
		{//meetme后单步转移发送设备事件，如保持取保持后的单步转移IVR的挂机事件
			Smt_Uint   nCallID = 0;
			nCallID    = pConnID->m_CallID;
			if( m_CallMgr.Lookup(nCallID, pCall) == Smt_Success )
			{
				pCall->SendConnectionEvent( pConnID->m_LastEvent );
			}
		}

		RemoveConnIDFromCall( pConnID->m_ConnID ); // Add by caoyj  2011-03-07 

		m_ConnIDMgr.Remove( pConnID->m_ConnID );
		delete pConnID;
		pConnID = NULL;
	}

	PrintLog(5, "[TCallState::OnEvtSSTransferTimer] CallID<%d>, DeviceID<%s>, ConnectionID<%s>, SSTransferTimerID<%d>.", 
		nOldCallID, strDeviceID.c_str(), strConnID.c_str(), nTimerID );

	return Smt_Success;
}

TCall* TCallState::OnEvtClearCallEvent(Smt_Pdu &pdu)
{
	Smt_Uint nCallID;
	Smt_Bool bFind = Smt_BoolFALSE;
	TCall* pCall = NULL;

	pdu.GetUint( Key_ICMU_CallID, &nCallID );

	if( m_CallMgr.Lookup( nCallID, pCall) == Smt_Success)
	{
		bFind = Smt_BoolTRUE;
	}

	PrintLog(5, "[TCallState::OnEvtClearCallEvent] CallID<%d>, bFind<%d>.", nCallID, bFind );

	return pCall;
}

Smt_Uint TCallState::OnCmdClearCall(Smt_Pdu& pdu)
{
	TCall* pCall = NULL;
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strConnID;
	
	pdu.GetUint( Key_ICMU_CallID, &nCallID );
	
	do 
	{
		if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success )
		{
			RespFailure( Resp_ICMU_ClearCall, CauseInvCallID, pdu );
			break;
		}
				
		RespCmd(Resp_ICMU_ClearCall, pdu);
		
		pCall->m_LastCommand = pdu;

		TConnID* pTmpConnID = NULL;	
		if( pCall->GetState() == CALL_CONFERENCED )
		{
			for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
				iter!= pCall->m_ConnIDList.end(); 
				iter++)
			{
				pTmpConnID = (*iter).int_id_;
								
				strConnID = pTmpConnID->m_ConnID;
				strDeviceID = pTmpConnID->m_DeviceID;
				
				pdu.m_Sender = GetGOR();
				pdu.m_Receiver = m_ConnectionStateGOR;
				pdu.m_MessageID = Cmd_ICMU_HangupCall;
				
				pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
				pdu.PutString( Key_ICMU_ConnectionID, strConnID );
				
				if( pdu.m_Receiver > 0 ) PostMessage( pdu );

				PrintLog(5, "[TCallState::OnCmdClearCall] Call-Conferenced, DeviceID<%s>, CallID<%d>, OldCallID<%d>.", 
					pTmpConnID->m_DeviceID.c_str(), pTmpConnID->m_CallID, pTmpConnID->m_OldCallID );
			} // end-for
				
			break;
		}
		
		if( pCall->GetState() == CALL_CONNECTED )
		{
			for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
				iter!= pCall->m_ConnIDList.end(); 
				iter++)
			{
				pTmpConnID = (*iter).int_id_;				
				if( pTmpConnID->m_OldCallID == 0 )
				{
					strConnID = pTmpConnID->m_ConnID;
					strDeviceID = pTmpConnID->m_DeviceID;
					
					pdu.m_Sender = GetGOR();
					pdu.m_Receiver = m_ConnectionStateGOR;
					pdu.m_MessageID = Cmd_ICMU_HangupCall;
					
					pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
					pdu.PutString( Key_ICMU_ConnectionID, strConnID );
					
					if( pdu.m_Receiver > 0 ) PostMessage( pdu );
				}

				PrintLog(5, "[TCallState::OnCmdClearCall] Call-Connected, DeviceID<%s>, CallID<%d>, OldCallID<%d>.", 
					pTmpConnID->m_DeviceID.c_str(), pTmpConnID->m_CallID, pTmpConnID->m_OldCallID );
			} // end-for

			break;
		}
	}
	while( 0 );
	
	PrintLog(5, "[TCallState::OnCmdClearCall] CallID<%d>.", nCallID );

	return Smt_Success;
}

Smt_Uint TCallState::OnCmdDoSIPHeader(Smt_Pdu& pdu)
{
	TCall*   pCall   = NULL;
	TConnID* pConnID = NULL;
	Smt_Uint   nRespMsg;
	Smt_Uint   nCallID;
	Smt_String strConnectionID;
	
	pdu.GetUint( Key_ICMU_CallID, &nCallID );

	if( pdu.m_MessageID == Cmd_ICMU_SetSIPHeader)
	{
		nRespMsg = Resp_ICMU_SetSIPHeader;
	}
	else // Cmd_ICMU_GetSIPHeader
	{
		nRespMsg = Resp_ICMU_GetSIPHeader;
	}

	do 
	{
		if( m_CallMgr.Lookup(nCallID, pCall) != Smt_Success )
		{
			RespFailure( nRespMsg, CauseInvCallID, pdu );
			break;
		}
		
		pConnID = LookupOneConnID( pCall );
		if( pConnID == NULL )
		{
			RespFailure( nRespMsg, CauseInvalidConnID, pdu );
			break;
		}
			
		strConnectionID = pConnID->m_ConnID;
		pdu.PutString( Key_ICMU_ConnectionID, strConnectionID );
		
		pCall->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );

	}while( 0 );

	PrintLog(5, "[TCallState::OnCmdDoSIPHeader] MessageID<%s>, CallID<%d>, ConnectionID<%s>.",
		GetIDName(pdu.m_MessageID).c_str(), nCallID, strConnectionID.c_str() );	

	return Smt_Success;
}

Smt_Uint TCallState::OnRespDoSIPHeader(Smt_Pdu& pdu)
{
	TCall* pCall = NULL;
	TConnID* pConnID = NULL;
	Smt_Uint   nCallID;
	Smt_String strConnectionID;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );	
	if( m_ConnIDMgr.Lookup(strConnectionID, pConnID) != Smt_Success )
	{
		PrintLog(3,"[TCallState::OnRespDoSIPHeader] Lookup Fail, ConnectionID<%s>.", strConnectionID.c_str());
		return Smt_Fail;
	}
		
	nCallID = pConnID->m_CallID;

	if( m_CallMgr.Lookup(nCallID, pCall) == Smt_Success )
	{
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = pCall->m_LastCommand.m_Sender;

		pdu.PutUint( Key_ICMU_CallID, nCallID);

		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	
	PrintLog(5,"[TCallState::OnRespDoSIPHeader] MessageID<%s>, CallID<%d>, ConnectionID<%s>.", 
		GetIDName(pdu.m_MessageID).c_str(), nCallID, strConnectionID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: RemoveConnIDFromCall
参    数:
返回数值: 
功能描述: 容错，删除Call的ConnID关联
*****************************************************************************/
Smt_Uint TCallState::RemoveConnIDFromCall(Smt_String connid)
{
 TCall* pCall = NULL;
 TConnID* pConnID = NULL;
 for(TCallMap::ITERATOR iter = m_CallMgr.begin(); 
  iter!= m_CallMgr.end(); 
  iter++)
 {
  pCall = (*iter).int_id_;  
  
  if( pCall->m_ConnIDList.Lookup(connid, pConnID) == Smt_Success )
  {
   pCall->RemoveConnID( pConnID );
   PrintLog(3, "[TCallState::RemoveConnIDFromCall] CallID<%d>, ConnID<%s>.", pCall->m_CallID, connid.c_str() );
  }
 } 
 return Smt_Success;
}
