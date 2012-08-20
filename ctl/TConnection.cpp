//***************************************************************************
// TConnection.cpp : implementation file
//***************************************************************************

#include "TConnection.h"
#include "TConnectionState.h"
#include "TASTConnector.h"

/////////////////////////////////////////////////////////////////////////////
// TConnection code

TConnection::TConnection( Smt_String objid, Smt_Uint initstate, TConnectionState* powner )
:Smt_StateObject( objid, initstate, powner )
{
	m_Channel = "";
	m_Uniqueid = "";
	m_CallerID = "";
	m_CallerIDName = "";
	m_DeviceType = 0;
	m_DeviceID = "";
	m_FullDeviceID = "";
	m_Source = "";
	m_Destination = "";
	m_LastState = 0;
	m_OtherConnectionID = "";
	m_ChannelType = 0;
	m_Reason = 0;
	m_AMIHandle = 0;
	m_AGIHandle = 0;
	m_Recording = Smt_BoolFALSE;
	m_RecordFileName = "";
	m_AGIScriptName = "";
	m_PlayFileName = "";
	m_AGIEvtTimerID = 0;
	m_AGIData = "";
	m_AGITimeLen = 0;
	m_AGIReason = 0;
	m_AGIAction = 0;
	m_PlayListIndex = 0;
	m_PlayListCurrIndex = 0;
	m_PlayListEscapeDigit = "";
	m_PlayListTimeLen = 0;
	m_Message = "";
	m_MeetmeNum = "";
	m_UserNum = 0;
	m_Variable = "";
	m_TimerID = 0;//add by caoyj 20120315
	m_CurrContext = "";//add by caoyj 20120315
}

TConnection::~TConnection()
{
}

Smt_Uint TConnection::OnUnexpectedEvent(Smt_Pdu &pdu)
{
	g_pConnState->PrintLog(5,"[TConnection::OnUnexpectedEvent] ConnectionID<%s>, State<%s>, MessageID<%s>, Status<%d>.",
		GetID().c_str(), GetStateName().c_str(), g_pConnState->GetIDName(pdu.m_MessageID).c_str(), pdu.m_Status );

	switch (pdu.m_MessageID)
	{
	case Evt_ICMU_Hangup:
		g_pConnState->ReleaseConnection( this );
		break;
	default:break;
	}
	return Smt_Success;
}

Smt_Uint TConnection::OnStateExit(Smt_Pdu &pdu)
{
	//g_pConnState->PrintLog(5,"[TConnection::OnStateExit] CurrState<%d>, MessageID<0x%x>.",
	//	GetState(), pdu.m_MessageID  );

	m_LastState = GetState();
	//m_Reason = 0;

	return Smt_Success;	
}

Smt_Uint TConnection::OnStateEnter(Smt_Pdu &pdu)
{
	//g_pConnState->PrintLog(5,"[TConnection::OnStateEnter] CurrState<%d>, MessageID<0x%x>.",
	//	GetState(), pdu.m_MessageID  );

	Smt_Uint nCurrState = GetState();
	
	switch (nCurrState)
	{
	case CONN_INITIATED:
		break;
	case CONN_ORIGINATED:
		break;
	case CONN_ALERTING:
		break;
	case CONN_QUEUED:
		break;
	case CONN_CONNECTED:
		break;
	case CONN_HOLD:
		break;
	case CONN_FAILED:
		break;
	case CONN_DISCONNECTED:
		break;
	case CONN_NULL:
		g_pConnState->ReleaseConnection( this );
		break;
	default:break;
	}
	return Smt_Success;	
}

/****************************************************************************
函 数 名: SendEvent
参    数:
返回数值: 
功能描述: 发送事件
*****************************************************************************/
Smt_Uint TConnection::SendEvent(Smt_Uint evtid )
{
	Smt_DateTime dtNow;
	Smt_Pdu evt;

	evt.m_Sender = g_pConnState->GetGOR();
	evt.m_Receiver = g_pConnState->m_CallStateGOR;	
	evt.m_MessageID = evtid;
	evt.m_Status = Smt_Success;

	evt.PutString( Key_ICMU_Channel, m_Channel );
	evt.PutString( Key_ICMU_ConnectionID, GetID() );
	evt.PutUint  ( Key_ICMU_ConnectionStatus, GetState() );
	evt.PutString( Key_ICMU_DeviceID, m_DeviceID );
	evt.PutUint  ( Key_ICMU_DeviceType, m_DeviceType );
	evt.PutString( Key_ICMU_Source, m_Source );
	evt.PutString( Key_ICMU_Destination, m_Destination );
	evt.PutString( Key_ICMU_CallerID, m_CallerID );
	evt.PutString( Key_ICMU_CallerIDName, m_CallerIDName );
	evt.PutUint  ( Key_ICMU_Reason, m_Reason );
	evt.PutString( Key_ICMU_TimeStamp, dtNow.FormatString() );
	evt.PutString( Key_ICMU_OtherConnectionID, m_OtherConnectionID );
	evt.PutString( Key_ICMU_MeetmeNum, m_MeetmeNum );
	evt.PutUint  ( Key_ICMU_Usernum, m_UserNum );
	evt.PutUint  ( Key_ICMU_LastCommand, m_LastCommand.m_MessageID );	

	if( evt.m_Receiver > 0 ) g_pConnState->PostMessage( evt );

	//*******  Print Event Logs ********//
	g_pConnState->PrintLog(5, "[CONN EVT] ======================== ");
	g_pConnState->PrintLog(5, "[CONN EVT] Channel: %s", m_Channel.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] Event: %s", g_pConnState->GetIDName(evtid).c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] ConnectionID: %s", GetID().c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] ConnectionStatus: %s", GetStateName().c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] DeviceID: %s", m_DeviceID.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] Source: %s", m_Source.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] Destination: %s", m_Destination.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] CallerID: %s", m_CallerID.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] LastCommand: 0x%x", m_LastCommand.m_MessageID );
	g_pConnState->PrintLog(5, "[CONN EVT] Reason: %d", m_Reason );
	g_pConnState->PrintLog(5, "[CONN EVT] OtherConnectionID: %s", m_OtherConnectionID.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] MeetmeNum: %s", m_MeetmeNum.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] UserNum: %d", m_UserNum );
	g_pConnState->PrintLog(5, "[CONN EVT] ======================== ");
	//*********************************//

	return Smt_Success;	
}
/****************************************************************************
函 数 名: EvtInitiated
参    数:
返回数值: 
功能描述: 发送 Connection.EvtInitiated 事件, 并获取通道的变量值
*****************************************************************************/
Smt_Uint TConnection::EvtInitiated(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Initiated );
	
	// 无法立即取到设置的数据，需要错开、定时取
	Smt_Pdu* pTemp = new Smt_Pdu();
	pTemp->PutString( Key_ICMU_ConnectionID, GetID() );
	pTemp->PutString( Key_ICMU_Channel, m_Channel );
	pTemp->PutString( Key_ICMU_DeviceID, m_DeviceID );
	
	g_pConnState->SetSingleTimer( MAX_AGIEVTTIMERLEN, Evt_ICMU_GetVarTimer, Smt_Uint(pTemp));
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtOriginated
参    数:
返回数值: 
功能描述: 发送 Connection.EvtInitiated 事件
*****************************************************************************/
Smt_Uint TConnection::EvtOriginated(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Originated );
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtAlerting
参    数:
返回数值: 
功能描述: 发送 Connection.EvtAlerting 事件
*****************************************************************************/
Smt_Uint TConnection::EvtAlerting(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Originated );
	SendEvent( Evt_ICMU_Alerting );

	TConnection * pOtherConnection = NULL;
	if( g_pConnState->m_ConnectionMgr.Lookup( m_OtherConnectionID, pOtherConnection) == Smt_Success )
	{
		pOtherConnection->SetState( CONN_ALERTING );
		pOtherConnection->SendEvent( Evt_ICMU_Alerting );		
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtInitToQueued
参    数:
返回数值: 
功能描述: 发送 Connection.(Originated+EvtQueued) 事件
*****************************************************************************/
Smt_Uint TConnection::EvtInitToQueued(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Originated );
	SendEvent( Evt_ICMU_Queued );
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtQueued
参    数:
返回数值: 
功能描述: 发送 Connection.EvtQueued 事件
*****************************************************************************/
Smt_Uint TConnection::EvtQueued(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Queued );
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtConnected
参    数:
返回数值: 
功能描述: 发送 Connection.EvtConnected 事件
*****************************************************************************/
Smt_Uint TConnection::EvtConnected(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Connected );

	TConnection * pOtherConnection = NULL;
	if( g_pConnState->m_ConnectionMgr.Lookup( m_OtherConnectionID, pOtherConnection) == Smt_Success )
	{
		pOtherConnection->SetState( CONN_CONNECTED );
		pOtherConnection->SendEvent( Evt_ICMU_Connected );		
	}
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtMeetmeConnected
参    数:
返回数值: 
功能描述: 会议通话,给所有的会话通话者发送通话事件
*****************************************************************************/
Smt_Uint TConnection::EvtMeetmeConnected(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Connected );

	TConnection* pConn = NULL;
	TConnectionMap::ITERATOR iter(g_pConnState->m_ConnectionMgr);
	for( iter = g_pConnState->m_ConnectionMgr.begin();
		 iter!= g_pConnState->m_ConnectionMgr.end();
		 ++iter )
	{
		pConn = (*iter).int_id_;

		if(pConn->GetID() == GetID())
		{
			continue;
		}

		if ( (pConn->GetState() != CONN_HOLD) &&
			 (pConn->m_MeetmeNum != "" )&& 
			 (pConn->m_MeetmeNum == m_MeetmeNum ) )
		{
			pConn->m_Reason = CauseBeMeetme;			
			pConn->SetState( CONN_CONNECTED );
			pConn->SendEvent( Evt_ICMU_Connected );
		}
	}
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtMeetmeDisconnected
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnection::EvtMeetmeDisconnected(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Disconnected );

	TConnection* pConn = NULL;
	TConnectionMap::ITERATOR iter(g_pConnState->m_ConnectionMgr);
	for( iter = g_pConnState->m_ConnectionMgr.begin();
	 	 iter!= g_pConnState->m_ConnectionMgr.end();
		 ++iter )
	{
		pConn = (*iter).int_id_;

		if(pConn->GetID() == GetID())
		{
			continue;
		}

		if ( (pConn->GetState() != CONN_HOLD) &&
			 (pConn->m_MeetmeNum != "" )&& (pConn->m_MeetmeNum == m_MeetmeNum ) )
		{
			//pConn->SetState( CONN_DISCONNECTED );   // 有问题么？
			pConn->m_Reason = CauseAtMeetLeave;	
			pConn->SendEvent( Evt_ICMU_Disconnected );
		}
	}
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtHeld
参    数:
返回数值: 
功能描述: 发送 Connection.EvtHeld 事件
*****************************************************************************/
Smt_Uint TConnection::EvtHeld(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Held );
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtRetrieved
参    数:
返回数值: 
功能描述: 发送 Connection.EvtConnected 事件
*****************************************************************************/
Smt_Uint TConnection::EvtRetrieved(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Connected );
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtDisconnected
参    数:
返回数值: 
功能描述: 发送 Connection.EvtDisconnected 事件
*****************************************************************************/
Smt_Uint TConnection::EvtDisconnected(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Disconnected );
	TConnection * pOtherConnection = NULL;
	if( g_pConnState->m_ConnectionMgr.Lookup( m_OtherConnectionID, pOtherConnection) == Smt_Success )
	{
		pOtherConnection->SetState( CONN_DISCONNECTED );
		pOtherConnection->SendEvent( Evt_ICMU_Disconnected );
	}
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtReleased
参    数:
返回数值: 
功能描述: 发送 Connection.EvtReleased 事件
*****************************************************************************/
Smt_Uint TConnection::EvtReleased(Smt_Pdu &pdu)
{
	if(m_Recording == Smt_BoolTRUE )
	{
		m_Recording = Smt_BoolFALSE;
		EvtRecord( Evt_ICMU_DeviceRecordEnd );
	}

	SendEvent( Evt_ICMU_Released );

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtInitToAlerting
参    数:
返回数值: 
功能描述: 发送 Connection.EvtOrigianted/EvtAlerting 事件
*****************************************************************************/
Smt_Uint TConnection::EvtInitToAlerting(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Originated );
	SendEvent( Evt_ICMU_Alerting );
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtRecord
参    数:
返回数值: 
功能描述: 坐席开始录音、结束录音事件
*****************************************************************************/
Smt_Uint TConnection::EvtRecord( Smt_Uint evtid )
{
	Smt_DateTime dtNow;
	Smt_Pdu evt;
	
	Smt_Uint nTimeLen = 0;
	
	if( evtid == Evt_ICMU_DeviceRecording )
	{
		m_StartRecordTime = dtNow;
	}
	else
	{
		//nTimeLen = dtNow.TimeValue().msec() - m_StartRecordTime.TimeValue().msec();
		//add by raojl 20111008 解决unsigned long 溢出问题
		nTimeLen = (dtNow.TimeValue() - m_StartRecordTime.TimeValue()).msec();
	}

	evt.m_Sender = g_pConnState->GetGOR();
	evt.m_Receiver = g_pConnState->m_DeviceStateGOR;	
	evt.m_MessageID = evtid;
	evt.m_Status = Smt_Success;
	
	evt.PutString( Key_ICMU_Channel, m_Channel );
	evt.PutString( Key_ICMU_DeviceID, m_DeviceID );
	evt.PutString( Key_ICMU_ConnectionID, GetID() );
	evt.PutString( Key_ICMU_Source, m_Source );
	evt.PutString( Key_ICMU_Destination, m_Destination );
	evt.PutString( Key_ICMU_FileName, m_RecordFileName );
	evt.PutUint  ( Key_ICMU_TimeLen, nTimeLen );
	evt.PutUint  ( Key_ICMU_Reason, m_Reason );
	evt.PutString( Key_ICMU_TimeStamp, dtNow.FormatString() );
	
	if( evt.m_Receiver > 0 ) g_pConnState->PostMessage( evt );
	
	//*******  Print Event Logs ********//
	g_pConnState->PrintLog(5, "[REC  EVT] ======================== ");
	g_pConnState->PrintLog(5, "[REC  EVT] Channel: %s", m_Channel.c_str() );
	g_pConnState->PrintLog(5, "[REC  EVT] Event: %s", g_pConnState->GetIDName(evtid).c_str() );
	g_pConnState->PrintLog(5, "[REC  EVT] DeviceID: %s", m_DeviceID.c_str() );
	g_pConnState->PrintLog(5, "[REC  EVT] ConnectionID: %s", GetID().c_str() );	
	g_pConnState->PrintLog(5, "[REC  EVT] Source: %s", m_Source.c_str() );
	g_pConnState->PrintLog(5, "[REC  EVT] Destination: %s", m_Destination.c_str() );
	g_pConnState->PrintLog(5, "[REC  EVT] FileName: %s", m_RecordFileName.c_str() );
	g_pConnState->PrintLog(5, "[REC  EVT] TimeLen: %d", nTimeLen );
	g_pConnState->PrintLog(5, "[REC  EVT] Reason: %d", m_Reason );
	g_pConnState->PrintLog(5, "[REC  EVT] ======================== ");
	//*********************************//

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtMedia
参    数:
返回数值: 
功能描述: 媒体服务事件
*****************************************************************************/
Smt_Uint TConnection::EvtMedia( Smt_Uint evtid )
{
	Smt_DateTime dtNow;
	Smt_Pdu evt;

	evt.m_Sender = g_pConnState->GetGOR();
	evt.m_Receiver = g_pConnState->m_CallStateGOR;	
	evt.m_MessageID = evtid;
	evt.m_Status = Smt_Success;
	
	evt.PutString( Key_ICMU_Channel, m_Channel );
	evt.PutString( Key_ICMU_DeviceID, m_DeviceID );
	evt.PutString( Key_ICMU_ConnectionID, GetID() );
	evt.PutString( Key_ICMU_Source, m_Source );
	evt.PutString( Key_ICMU_Destination, m_Destination );
	evt.PutString( Key_ICMU_FileName, m_PlayFileName );
	evt.PutString( Key_ICMU_DTMFDigits, m_AGIData );
	evt.PutString( Key_ICMU_Message, m_Message );
	evt.PutUint  ( Key_ICMU_TimeLen, m_AGITimeLen );
	evt.PutUint  ( Key_ICMU_Reason, m_AGIReason );
	evt.PutString( Key_ICMU_TimeStamp, dtNow.FormatString() );
	
	if( evt.m_Receiver > 0 ) g_pConnState->PostMessage( evt );
	
	//*******  Print Event Logs ********//
	g_pConnState->PrintLog(5, "[MEDIA EVT] ======================== ");
	g_pConnState->PrintLog(5, "[MEDIA EVT] Channel: %s", m_Channel.c_str() );
	g_pConnState->PrintLog(5, "[MEDIA EVT] Event: %s", g_pConnState->GetIDName(evtid).c_str() );
	g_pConnState->PrintLog(5, "[MEDIA EVT] ConnectionID: %s", GetID().c_str() );
	g_pConnState->PrintLog(5, "[MEDIA EVT] Source: %s", m_Source.c_str() );
	g_pConnState->PrintLog(5, "[MEDIA EVT] Destination: %s", m_Destination.c_str() );
	g_pConnState->PrintLog(5, "[MEDIA EVT] FileName: %s", m_PlayFileName.c_str() );
	g_pConnState->PrintLog(5, "[MEDIA EVT] DTMFDigits: %s", m_AGIData.c_str() );
	g_pConnState->PrintLog(5, "[MEDIA EVT] Message: %s", m_Message.c_str() );
	g_pConnState->PrintLog(5, "[MEDIA EVT] TimeLen: %d", m_AGITimeLen );
	g_pConnState->PrintLog(5, "[MEDIA EVT] Reason: %d", m_AGIReason );
	g_pConnState->PrintLog(5, "[MEDIA EVT] ======================== ");
	//*********************************//

	if( (evtid == Evt_ICMU_GetEnd) ||
		(evtid == Evt_ICMU_SendEnd) ||
		(evtid == Evt_ICMU_MessageSendEnd) ||
		(evtid == Evt_ICMU_RecordEnd) 
		/*(evtid == Evt_ICMU_TTSPlayEnd)*/ )
	{
		m_PlayFileName = "";
		m_AGIData = "";		
		m_AGITimeLen = 0;
		m_AGIReason = 0;
		m_Message = "";
	}
	
	if( (evtid == Evt_ICMU_PlayEnd) ||
		(evtid == Evt_ICMU_GetEnd) ||
		(evtid == Evt_ICMU_SendEnd) ||
		(evtid == Evt_ICMU_MessageSendEnd) ||
		(evtid == Evt_ICMU_RecordEnd) 
		/*(evtid == Evt_ICMU_TTSPlayEnd)*/ )
	{
		m_LastCommand.Clear();
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtOriToConnected
参    数:
返回数值: 
功能描述: 发送 Connection.EvtAlerting+EvtConnected 事件
*****************************************************************************/
Smt_Uint TConnection::EvtOriToConnected(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Alerting );
	SendEvent( Evt_ICMU_Connected );
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtDialResponse
参    数:
返回数值: 
功能描述: EvtDialResponse 事件
*****************************************************************************/
Smt_Uint TConnection::EvtDialResponse()
{
	Smt_DateTime dtNow;
	Smt_Pdu evt;	
	evt.m_Sender = g_pConnState->GetGOR();
	evt.m_Receiver = g_pConnState->m_CallStateGOR;	
	evt.m_MessageID = Evt_ICMU_DialResponse;
	evt.m_Status = Smt_Success;
	
	evt.PutString( Key_ICMU_Channel, m_Channel );
	evt.PutString( Key_ICMU_DeviceID, m_DeviceID );
	evt.PutString( Key_ICMU_ConnectionID, GetID() );
	evt.PutUint  ( Key_ICMU_Reason, m_AGIReason );
	evt.PutString( Key_ICMU_TimeStamp, dtNow.FormatString() );
	
	if( evt.m_Receiver > 0 ) g_pConnState->PostMessage( evt );
	
	//*******  Print Event Logs ********//
	g_pConnState->PrintLog(5, "[DIALRESP EVT] ======================== ");
	g_pConnState->PrintLog(5, "[DIALRESP EVT] Channel: %s", m_Channel.c_str() );
	g_pConnState->PrintLog(5, "[DIALRESP EVT] Event: %s", g_pConnState->GetIDName(Evt_ICMU_DialResponse).c_str() );
	g_pConnState->PrintLog(5, "[DIALRESP EVT] ConnectionID: %s", GetID().c_str() );
	g_pConnState->PrintLog(5, "[DIALRESP EVT] Reason: %d", m_AGIReason );
	g_pConnState->PrintLog(5, "[DIALRESP EVT] ======================== ");
	//*********************************//

	return Smt_Success;
}

/****************************************************************************
函 数 名: SendMessageEvent
参    数:
返回数值: 
功能描述: 发送 Device Message 事件
*****************************************************************************/
Smt_Uint TConnection::SendMessageEvent(Smt_Pdu &pdu)
{
	Smt_String strMessage;
	pdu.GetString( Key_ICMU_Message, &strMessage );

	Smt_DateTime dtNow;
	Smt_Pdu evt;

	evt.m_Sender = g_pConnState->GetGOR();
	evt.m_Receiver = g_pConnState->m_DeviceStateGOR;	
	evt.m_MessageID = pdu.m_MessageID;  // Evt_ICMU_MessageReceived
	evt.m_Status = Smt_Success;

	evt.PutString( Key_ICMU_Channel, m_Channel );
	evt.PutString( Key_ICMU_ConnectionID, GetID() );
	evt.PutUint  ( Key_ICMU_ConnectionStatus, GetState() );
	evt.PutString( Key_ICMU_DeviceID, m_DeviceID );
	evt.PutString( Key_ICMU_Message, strMessage );
	evt.PutUint  ( Key_ICMU_LastCommand, m_LastCommand.m_MessageID );	
	evt.PutUint  ( Key_ICMU_Reason, m_Reason );
	evt.PutString( Key_ICMU_TimeStamp, dtNow.FormatString() );
	evt.PutString( Key_ICMU_OtherConnectionID, m_OtherConnectionID );

	if( evt.m_Receiver > 0 ) g_pConnState->PostMessage( evt );

	//*******  Print Event Logs ********//
	g_pConnState->PrintLog(5, "[CONN EVT] ======================== ");
	g_pConnState->PrintLog(5, "[CONN EVT] Channel: %s", m_Channel.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] Event: %s", g_pConnState->GetIDName(pdu.m_MessageID).c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] ConnectionID: %s", GetID().c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] ConnectionStatus: %s", GetStateName().c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] DeviceID: %s", m_DeviceID.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] Message: %s", strMessage.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] LastCommand: 0x%x", m_LastCommand.m_MessageID );
	g_pConnState->PrintLog(5, "[CONN EVT] Reason: %d", m_Reason );
	g_pConnState->PrintLog(5, "[CONN EVT] OtherConnectionID: %s", m_OtherConnectionID.c_str() );
	g_pConnState->PrintLog(5, "[CONN EVT] ======================== ");
	//*********************************//

	return Smt_Success;	
}

/****************************************************************************
函 数 名: EvtFailed
参    数:
返回数值: 
功能描述: 发送 Failed 事件
*****************************************************************************/
Smt_Uint TConnection::EvtFailed(Smt_Pdu &pdu)
{
	SendEvent( Evt_ICMU_Failed );
	return Smt_Success;	
}

/****************************************************************************
函 数 名: RespSetSIPHeader
参    数:
返回数值: 
功能描述: SIP 消息头处理
*****************************************************************************/
Smt_Uint TConnection::RespSetSIPHeader(Smt_Uint reason)
{
	Smt_Pdu resppdu;
	resppdu.m_Sender = g_pConnState->GetGOR();
	resppdu.m_Receiver = g_pConnState->m_CallStateGOR;	
	resppdu.m_MessageID = Resp_ICMU_SetSIPHeader;
	resppdu.m_Status = Smt_Success;

	resppdu.PutString( Key_ICMU_ConnectionID, GetID());
	resppdu.PutUint  ( Key_ICMU_Reason, reason );

	if( resppdu.m_Receiver > 0 ) g_pConnState->PostMessage( resppdu );

	g_pConnState->PrintLog(5, "[TConnection::RespSetSIPHeader] Resp SetSIPHeader, ConnectionID<%s>, Reason<%d>.",
		GetID().c_str(), reason );

	return Smt_Success;	
}

Smt_Uint TConnection::RespGetSIPHeader(Smt_Uint reason, Smt_String headervalue)
{
	Smt_String strHeaderKey;
	Smt_String strHeaderValue;

	m_LastCommand.GetString( Key_ICMU_DataKey, &strHeaderKey );
	strHeaderValue = headervalue;

	Smt_Pdu resppdu;
	resppdu.m_Sender = g_pConnState->GetGOR();
	resppdu.m_Receiver = g_pConnState->m_CallStateGOR;	
	resppdu.m_MessageID = Resp_ICMU_GetSIPHeader;
	resppdu.m_Status = Smt_Success;
	
	resppdu.PutString( Key_ICMU_ConnectionID, GetID());
	resppdu.PutString( Key_ICMU_DataKey, strHeaderKey );
	resppdu.PutString( Key_ICMU_DataValue, strHeaderValue );
	resppdu.PutUint  ( Key_ICMU_Reason, reason );
	
	if( resppdu.m_Receiver > 0 ) g_pConnState->PostMessage( resppdu );
	
	g_pConnState->PrintLog(5, "[TConnection::RespGetSIPHeader] Resp GetSIPHeader, ConnectionID<%s>, HeaderKey<%s>, HeaderValue<%s>, Reason<%d>.",
		GetID().c_str(), strHeaderKey.c_str(), strHeaderValue.c_str(), reason );
	
	return Smt_Success;	
}

/****************************************************************************
函 数 名: SetTimer
参    数:
返回数值: 
功能描述: 设置设备时钟
*****************************************************************************/
Smt_Uint TConnection::SetTimer(Smt_Pdu &pdu)
{
	Smt_Uint nTimeLen;
	pdu.GetUint( Key_ICMU_Timeout, &nTimeLen);
	
	if( nTimeLen == 0 )
	{
		nTimeLen = DEFAULT_USEREVENTTIMELEN*2;
	}
	
	m_TimerID = g_pConnState->SetSingleTimer( nTimeLen, Evt_ICMU_UserEventTimer, (Smt_Uint)this );
 	g_pConnState->PrintLog(5,"[TConnection::SetTimer] ConnectionID<%s>,obj<%d>,TimerID<%d>,TimeLen<%d>.",
 		GetID().c_str(), (Smt_Uint)this, m_TimerID, nTimeLen );
	
	return Smt_Success;
}

Smt_Uint TConnection::ClearTimer()
{	
	g_pConnState->PrintLog(5,"[TConnection::ClearTimer] ConnectionID<%s>,TimerID<%d>.",
		GetID().c_str(), m_TimerID );

	if( m_TimerID > 0 )
		g_pConnState->ClearTimer( m_TimerID );
	m_TimerID = 0 ;
	
	return Smt_Success;
}
