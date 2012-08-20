
//***************************************************************************
// TCall.cpp : implementation file
//***************************************************************************

#include "TCall.h"
#include "TCallState.h"

/////////////////////////////////////////////////////////////////////////////
// TCall code

TCall::TCall( Smt_String objid, Smt_Uint initstate, TCallState* powner )
:Smt_StateObject( objid, initstate, powner )
{
	m_CallID = ACE_OS::atoi( objid.c_str() );
	m_CallingParty = "";
	m_CalledParty = "";
	m_InitiatedParty = "";
	m_AlertingParty = "";
	m_AnsweringParty = "";
	m_HoldingParty = "";
	m_RetrievingParty = "";
	m_ConsultingParty = "";
	m_TransferringParty = "";
	m_ConferencingParty = "";
	m_ReleaseingParty = "";

	PrintLog(5,"[TCall::TCall] Call Created, CallID<%d>.", m_CallID );
}
	
TCall::~TCall()
{
	m_ConnIDList.RemoveAll();
	m_PrivateData.RemoveAll();
	PrintLog(5,"[TCall::~TCall] Call Released, CallID<%d>.", m_CallID );
}

Smt_Uint TCall::OnUnexpectedEvent(Smt_Pdu &pdu)
{
	PrintLog(5,"[TCall::OnUnexpectedEvent] CallID<%s>, State<%s>, MessageID<0x%x>, Status<%d>.",
		GetID().c_str(), GetStateName().c_str(), pdu.m_MessageID, pdu.m_Status  );

	return Smt_Success;
}

Smt_Uint TCall::OnStateExit(Smt_Pdu &pdu)
{
	//PrintLog(5,"[TCall::OnStateExit] CallID<%s>, State<%d>, MessageID<0x%x>.",
	//	GetID().c_str(), GetState(), pdu.m_MessageID  );

	return Smt_Success;
}

Smt_Uint TCall::OnStateEnter(Smt_Pdu &pdu)
{
	//PrintLog(5,"[TCall::OnStateEnter] CallID<%s>, State<%d>, MessageID<0x%x>.",
	//	GetID().c_str(), GetState(), pdu.m_MessageID  );

	if( GetState() == CALL_NULL )
	{
		g_pCallState->ReleaseCall( m_CallID );
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtCallOriginated
参    数:
返回数值: 
功能描述: 初始化 Call 信息
*****************************************************************************/
Smt_Uint TCall::EvtCallInitToOriginated(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallInitiated, pdu.m_Status );
	SendCallEvent(Evt_ICMU_CallOriginated, pdu.m_Status );
	return Smt_Success;
}

Smt_Uint TCall::EvtCallOriginated(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallOriginated, pdu.m_Status );
	return Smt_Success;
}

Smt_Uint TCall::EvtCallDelivered(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallDelivered, pdu.m_Status );
	return Smt_Success;
}

Smt_Uint TCall::EvtCallConnected(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallConnected, pdu.m_Status );
	return Smt_Success;
}

Smt_Uint TCall::EvtCallHeld(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallHeld, pdu.m_Status );
	return Smt_Success;
}

Smt_Uint TCall::EvtCallCleared(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallCleared, pdu.m_Status );
	return Smt_Success;
}

Smt_Uint TCall::EvtCallQueued(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallQueued, pdu.m_Status );
	return Smt_Success;
}

Smt_Uint TCall::EvtCallFailed(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallFailed, pdu.m_Status );
	return Smt_Success;
}

/****************************************************************************
函 数 名: AddConnID
参    数:
返回数值: 
功能描述: ConnIDList 操作函数
*****************************************************************************/
Smt_Uint TCall::AddConnID(TConnID* pconnid)
{
	m_ConnIDList.SetAt( pconnid->m_ConnID, pconnid );
	return Smt_Success;
}

Smt_Uint TCall::RemoveConnID(TConnID* pconnid)
{
	if( m_ConnIDList.FindByKey( pconnid->m_ConnID ) == Smt_Success )
	{
		m_ConnIDList.Remove( pconnid->m_ConnID );
		return Smt_Success;
	}
	else
		return Smt_Fail;
}

Smt_Uint TCall::RemoveAllConnID()
{
	m_ConnIDList.RemoveAll();
	return Smt_Success;
}

Smt_Uint TCall::GetConnIDCount()
{
	return m_ConnIDList.GetCount();
}

/****************************************************************************
函 数 名: SendConnectionEvent
参    数:
返回数值: 
功能描述: 发送消息到 DeviceState
*****************************************************************************/
Smt_Uint TCall::SendConnectionEvent(Smt_Pdu& pdu)
{
	Smt_String strConnectionID;
	Smt_String strDeviceID;	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );

	pdu.m_Sender = g_pCallState->GetGOR();
	pdu.m_Receiver = g_pCallState->m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	
	pdu.PutUint  ( Key_ICMU_CallID, m_CallID );
	pdu.PutUint  ( Key_ICMU_CallState, GetState() );
	pdu.PutString( Key_ICMU_CallingParty, m_CallingParty );
	pdu.PutString( Key_ICMU_CalledParty, m_CalledParty );
	pdu.PutString( Key_ICMU_InitiatedParty, m_InitiatedParty );
	pdu.PutString( Key_ICMU_AlertingParty, m_AlertingParty );
	pdu.PutString( Key_ICMU_AnsweringParty, m_AnsweringParty );
	pdu.PutString( Key_ICMU_HoldingParty, m_HoldingParty );
	pdu.PutString( Key_ICMU_RetrievingParty, m_RetrievingParty );
	pdu.PutString( Key_ICMU_ConsultingParty, m_ConsultingParty );
	pdu.PutString( Key_ICMU_TransferringParty, m_TransferringParty );
	pdu.PutString( Key_ICMU_ConferencingParty, m_ConferencingParty );

	TConnID* pConnID = NULL;
	if( g_pCallState->m_ConnIDMgr.Lookup(strConnectionID, pConnID) == Smt_Success )
	{
		pdu.PutUint( Key_ICMU_CallID, pConnID->m_CallID);
		pdu.PutUint( Key_ICMU_OldCallID, pConnID->m_OldCallID);
		pdu.PutUint( Key_ICMU_SecOldCallID, pConnID->m_SecOldCallID);
	}

	g_pCallState->SendMessage( pdu );

	PrintLog(5,"[TCall::SendConnectionEvent] CallID<%d>, DeviceID<%s>, ConnectionID<%s>,Msg<%s>.", 
		m_CallID, strDeviceID.c_str(), strConnectionID.c_str(),g_pCallState->GetIDName(pdu.m_MessageID).c_str());
	return Smt_Success;
}

/****************************************************************************
函 数 名: SendCallEvent
参    数:
返回数值: 
功能描述: 发送消息到 DeviceState
*****************************************************************************/
Smt_Uint TCall::SendCallEvent(Smt_Uint evtid, Smt_Uint reason)
{
	Smt_DateTime dtNow;
	Smt_String strTimeStamp = dtNow.FormatString();

	Smt_Pdu callEvt;
	callEvt.m_MessageID = evtid;
	
	callEvt.PutUint  ( Key_ICMU_CallID, m_CallID );
	callEvt.PutUint  ( Key_ICMU_CallState, GetState() );
	callEvt.PutString( Key_ICMU_CallingParty, m_CallingParty );
	callEvt.PutString( Key_ICMU_CalledParty, m_CalledParty );
	callEvt.PutString( Key_ICMU_InitiatedParty, m_InitiatedParty );
	callEvt.PutString( Key_ICMU_AlertingParty, m_AlertingParty );
	callEvt.PutString( Key_ICMU_AnsweringParty, m_AnsweringParty );
	callEvt.PutString( Key_ICMU_HoldingParty, m_HoldingParty );
	callEvt.PutString( Key_ICMU_RetrievingParty, m_RetrievingParty );
	callEvt.PutString( Key_ICMU_ConsultingParty, m_ConsultingParty );
	callEvt.PutString( Key_ICMU_TransferringParty, m_TransferringParty );
	callEvt.PutString( Key_ICMU_ConferencingParty, m_ConferencingParty );
	callEvt.PutUint  ( Key_ICMU_Reason, reason );
	callEvt.PutString( Key_ICMU_TimeStamp, strTimeStamp );
	
	g_pCallState->PublishEvent( callEvt );
	
	// Print call-event logs
	PrintLog(5, "[CALL EVT] ======================== ");
	PrintLog(5, "[CALL EVT] Event: %s, CallID<%d>", g_pCallState->GetIDName(evtid).c_str(), m_CallID );
	PrintLog(5, "[CALL EVT] CallState: %s", GetStateName().c_str() );
	PrintLog(5, "[CALL EVT] CallingParty: %s", m_CallingParty.c_str() );
	PrintLog(5, "[CALL EVT] CalledParty : %s", m_CalledParty.c_str() );
	PrintLog(5, "[CALL EVT] InitiatedParty: %s", m_InitiatedParty.c_str() );
	PrintLog(5, "[CALL EVT] AlertingParty : %s", m_AlertingParty.c_str() );
	PrintLog(5, "[CALL EVT] AnsweringParty : %s", m_AnsweringParty.c_str() );
	PrintLog(5, "[CALL EVT] HoldingParty   : %s", m_HoldingParty.c_str() );
	PrintLog(5, "[CALL EVT] RetrievingParty: %s", m_RetrievingParty.c_str() );
	PrintLog(5, "[CALL EVT] ConsultingParty: %s", m_ConsultingParty.c_str() );
	PrintLog(5, "[CALL EVT] TransferringParty: %s", m_TransferringParty.c_str() );
	PrintLog(5, "[CALL EVT] ConferencingParty: %s", m_ConferencingParty.c_str() );
	PrintLog(5, "[CALL EVT] Reason: %d", reason );
	PrintLog(5, "[CALL EVT] TimeStamp: %s", strTimeStamp.c_str() );
	PrintLog(5, "[CALL EVT] ======================== ");

	return Smt_Success;
}

void TCall::PrintLog(Smt_Uint loglevel, const char* fmt, ...)
{
	char buf[Smt_LOGBUFF_LEN];
	va_list list;
	va_start(list,fmt);
	ACE_OS::vsprintf(buf, fmt, list);
	va_end(list);	
	
	g_pCallState->PrintLog(loglevel, buf);
}

/****************************************************************************
函 数 名: EvtCallClearedEx
参    数:
返回数值: 
功能描述: 呼叫转移
*****************************************************************************/
Smt_Uint TCall::EvtCallClearedEx(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallInitiated, pdu.m_Status );

	Smt_Uint nCallID, nOldCallID;
	TCall* pCall = NULL;
	TCall* pOldCall = NULL;

	pdu.GetUint( Key_ICMU_NoUseCallID, &nCallID);
	pdu.GetUint( Key_ICMU_NoUseOldCallID, &nOldCallID);

	if( g_pCallState->m_CallMgr.Lookup(nCallID, pCall) == Smt_Success )
	{
		pCall->EvtCallCleared( pdu );
		g_pCallState->ReleaseCall( nCallID );
	}

	if( g_pCallState->m_CallMgr.Lookup(nOldCallID, pOldCall) == Smt_Success )
	{
		pOldCall->EvtCallCleared( pdu );
		g_pCallState->ReleaseCall( nOldCallID );
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtCallConferenced
参    数:
返回数值: 
功能描述: 呼叫会议
*****************************************************************************/
Smt_Uint TCall::EvtCallConferenced(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallConferenced, pdu.m_Status );
	
	Smt_Uint nCallID, nOldCallID;
	TCall* pCall = NULL;
	TCall* pOldCall = NULL;
	
	pdu.GetUint( Key_ICMU_NoUseCallID, &nCallID);
	pdu.GetUint( Key_ICMU_NoUseOldCallID, &nOldCallID);
	
	if( g_pCallState->m_CallMgr.Lookup(nCallID, pCall) == Smt_Success )
	{
		pCall->EvtCallCleared( pdu );
		g_pCallState->ReleaseCall( nCallID );
	}
	
	if( g_pCallState->m_CallMgr.Lookup(nOldCallID, pOldCall) == Smt_Success )
	{
		pOldCall->EvtCallCleared( pdu );
		g_pCallState->ReleaseCall( nOldCallID );
	}

	return Smt_Success;
}

Smt_Uint TCall::EvtCallTransferredToOriginated(Smt_Pdu &pdu)
{
	SendCallEvent(Evt_ICMU_CallTransferred, pdu.m_Status );
	SendCallEvent(Evt_ICMU_CallOriginated, pdu.m_Status );
	return Smt_Success;
}
/****************************************************************************
函 数 名: LookupOtherParty
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_String TCall::LookupOtherParty(Smt_String localconnid)
{
	Smt_String strOtherParty = "";
	TConnID* pTmpConnID = NULL;

	for(TConnIDMap::ITERATOR 
		iter = m_ConnIDList.begin(); 
		iter!= m_ConnIDList.end(); 
		iter++)
	{
		pTmpConnID = (*iter).int_id_;
		if(pTmpConnID->m_ConnID != localconnid)
		{
			strOtherParty = pTmpConnID->m_DeviceID;
			break;
		}
	}

	PrintLog(5,"[TCall::LookupOtherParty] CallID<%d>, LocalConnID<%s>, OtherParty<%s>.", 
		m_CallID, localconnid.c_str(),strOtherParty.c_str() );

	return strOtherParty;
}

Smt_Uint TCall::SetData(Smt_String key, Smt_String val )
{
	Smt_String strTemp;
	if( key != "" )
		m_PrivateData.SetAtEx(key, val, strTemp);
	return Smt_Success;
}

Smt_String TCall::GetData(Smt_String key )
{
	Smt_String strRetVal = "";	
	m_PrivateData.Lookup(key, strRetVal);
	return strRetVal;
}
