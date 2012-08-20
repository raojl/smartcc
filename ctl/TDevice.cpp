
//***************************************************************************
// TDevice.cpp : implementation file
//***************************************************************************

#include "TDevice.h"
#include "TDeviceState.h"
#include "TCall.h"
#include "TCallState.h"
#include "TConnID.h"

/////////////////////////////////////////////////////////////////////////////
// TDevice code

TDevice::TDevice( Smt_String objid, Smt_Uint type, Smt_Uint initstate, TDeviceState* powner )
:Smt_StateObject( objid, initstate, powner )
{
	m_DeviceID = objid;
	m_DeviceType = type;

	InitData();

	PrintLog(5,"[TDevice::TDevice] Device Created, DeviceID<%s>.", objid.c_str() );
}
	
TDevice::~TDevice()
{
	// release monitor
	TDeviceMonitor* pMonitor = NULL;
	TDeviceMonitorMap::ITERATOR iter1(m_MonitorMgr);
	for (TDeviceMonitorMap::ENTRY *entry1 = 0;
		iter1.next (entry1) != 0;
		iter1.advance ())
	{
		pMonitor = entry1->int_id_;		
		if(pMonitor != NULL)
		{	
			delete pMonitor;
			pMonitor = NULL;
		}
	}	
	
	m_MonitorMgr.RemoveAll();

	PrintLog(5,"[TDevice::~TDevice] Device Released, DeviceID<%s>.", GetID().c_str() );
}

Smt_Uint TDevice::InitData()
{
	m_CallID = 0;
	m_OldCallID = 0;
	m_SecOldCallID = 0;
	m_OtherParty = "";
	m_CallingParty = "";
	m_CalledParty = "";
	m_ThirdParty = "";
	m_Reason = 0;
	m_Variable = "";
	m_TimerID = 0;
	m_MeetmeNum = "";
	m_LastCommand.Clear();
	m_TimerReason = 0;
	m_CallerID = "";
	return Smt_Success;
}

Smt_Uint TDevice::OnUnexpectedEvent(Smt_Pdu &pdu)
{
	PrintLog(5,"[TDevice::OnUnexpectedEvent] DeviceID<%s>, State<%s>, MessageID<%s>, Status<%d>.",
		GetID().c_str(), GetStateName().c_str(), g_pDeviceState->GetIDName(pdu.m_MessageID).c_str(), pdu.m_Status  );
	
	switch (pdu.m_MessageID)
	{
	case Evt_ICMU_Released:
		if( GetState() != DST_IDLE )
		{
			SetState( DST_IDLE );
			EvtTpDisconnected( pdu );    // 保护作用
		}

		InitData();
		
		PrintLog(3,"[TDevice::OnUnexpectedEvent] Set Device<%s> Idle, Event<%s>.", 
			GetID().c_str(), g_pDeviceState->GetIDName(pdu.m_MessageID).c_str() );
		break;
	default:break;
	}

	return Smt_Success;
}

Smt_Uint TDevice::OnStateExit(Smt_Pdu &pdu)
{
	//PrintLog(5,"[TDevice::OnStateExit] DeviceID<%s>, State<%d>, MessageID<0x%x>.",
	//	GetID().c_str(), GetState(), pdu.m_MessageID  );

	return Smt_Success;
}

Smt_Uint TDevice::OnStateEnter(Smt_Pdu &pdu)
{
	//PrintLog(5,"[TDevice::OnStateEnter] DeviceID<%s>, State<%d>, MessageID<0x%x>.",
	//	GetID().c_str(), GetState(), pdu.m_MessageID  );
	switch (GetState())
	{
	case DST_IDLE:
		InitData();
		break;
	default:
		break;
	}

	return Smt_Success;
}

void TDevice::PrintLog(Smt_Uint loglevel, const char* fmt, ...)
{
	char buf[Smt_LOGBUFF_LEN];
	va_list list;
	va_start(list,fmt);
	ACE_OS::vsprintf(buf, fmt, list);
	va_end(list);	
	
	g_pDeviceState->PrintLog(loglevel, buf);
}

/****************************************************************************
函 数 名: SetCallData
参    数:
返回数值: 
功能描述: 设置呼叫事件数据
*****************************************************************************/
Smt_Uint TDevice::SetCallData(Smt_Pdu pdu)
{
	pdu.GetUint  ( Key_ICMU_CallID, &m_CallID );
 	pdu.GetUint  ( Key_ICMU_OldCallID, &m_OldCallID );
 	pdu.GetUint  ( Key_ICMU_SecOldCallID, &m_SecOldCallID );
	pdu.GetString( Key_ICMU_CallingParty, &m_CallingParty );
	pdu.GetString( Key_ICMU_CalledParty, &m_CalledParty );

	PrintLog(5,"[TDevice::SetCallData] CallID<%d>, OldCallID<%d>, SecOldCallID<%d>, CallingParty<%s>, CalledParty<%s>.",
		m_CallID, m_OldCallID, m_SecOldCallID, m_CallingParty.c_str(), m_CalledParty.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: AddMonitor
参    数:
返回数值: 
功能描述: 增加监视信息
*****************************************************************************/
Smt_Uint TDevice::AddMonitor(Smt_Uint monitorgor)
{
	Smt_Uint nDeviceRefID = 0;
	TDeviceMonitor* pMonitor = NULL;
	TDeviceMonitor* pTmpMonitor = NULL;

	for(TDeviceMonitorMap::ITERATOR 
		iter = m_MonitorMgr.begin(); 
		iter != m_MonitorMgr.end(); 
		iter++)
	{
		pTmpMonitor = (*iter).int_id_;
		if(pTmpMonitor->m_MonitorGOR == monitorgor)
		{
			pMonitor = pTmpMonitor;
			break;
		}
	}

	if( pMonitor == NULL )
	{
		nDeviceRefID = g_pDeviceState->AllocateDeviceRefID();
		pMonitor = new TDeviceMonitor();
		pMonitor->m_DeviceRefID = nDeviceRefID;
		pMonitor->m_MonitorGOR = monitorgor;

		m_MonitorMgr.SetAt(nDeviceRefID, pMonitor);
	}
	else
	{
		nDeviceRefID = pMonitor->m_DeviceRefID;

		PrintLog(5,"[TDevice::AddMonitor] Device ReMonitor, DeviceRefID<%d>, DeviceID<%s>, MonitorGOR<0x%x>.",
			nDeviceRefID, GetID().c_str(), monitorgor );
	}

	PrintLog(5,"[TDevice::AddMonitor] DeviceRefID<%d>, DeviceID<%s>, MonitorGOR<0x%x>.",
		nDeviceRefID, GetID().c_str(), monitorgor );

	return nDeviceRefID;
}

Smt_Uint TDevice::RemoveMonitor(Smt_Uint devicerefid)
{
	TDeviceMonitor* pMonitor = NULL;
	if( m_MonitorMgr.Lookup(devicerefid, pMonitor) == Smt_Success )
	{
		PrintLog(5,"[TDevice::RemoveMonitor] DeviceRefID<%d>, DeviceID<%s>.",
			devicerefid, GetID().c_str() );

		m_MonitorMgr.Remove( devicerefid );

		delete pMonitor;
		pMonitor = NULL;
	}

	return Smt_Success;
}

Smt_Uint TDevice::RemoveMonitorByGOR(Smt_Uint monitorgor)
{
	Smt_Uint nDeviceRefID;
	TDeviceMonitor* pMonitor = NULL;

	TDeviceMonitorMap::ITERATOR iter(m_MonitorMgr);       
	for( TDeviceMonitorMap::ENTRY* entry = 0;
		iter.next(entry) != 0; )
	{
		pMonitor = entry->int_id_;
		iter.advance();
		
		if( pMonitor->m_MonitorGOR == monitorgor )
		{
			nDeviceRefID = pMonitor->m_DeviceRefID;
			
			PrintLog(5,"[TDevice::RemoveMonitorByGOR] DeviceRefID<%d>, DeviceID<%s>.",
				nDeviceRefID, GetID().c_str() );

			m_MonitorMgr.Remove( nDeviceRefID );

			delete pMonitor; 
			pMonitor = NULL;
		}
	}
	
	return Smt_Success;
}

Smt_Bool TDevice::IsMonitored(Smt_Uint devicerefid)
{
	TDeviceMonitor* pMonitor = NULL;
	if(m_MonitorMgr.Lookup(devicerefid, pMonitor) == Smt_Success)
	{
		return Smt_BoolTRUE;	
	}
	else
		return Smt_BoolFALSE;
}


/****************************************************************************
函 数 名: SendDeviceEvent
参    数:
返回数值: 
功能描述: 发送设备事件，并转换 CTC 的相应数据
*****************************************************************************/
Smt_Uint TDevice::SendDeviceEvent(Smt_Uint evtid, Smt_Uint reason)
{
	//IVR路由点设备因有中继虚拟设备事件，不需要发IVR路由点设备事件给TS  add by caoyj 20120112
	if(m_DeviceType == DTYPE_IVR_OUTBOUND || m_DeviceType == DTYPE_IVR_INBOUND)
		return Smt_Fail;
    ///////////////////////////////////////////////////////////////////////////

	Smt_DateTime dtNow;
	Smt_String strTimeStamp = dtNow.FormatString();

	// convert state
	Smt_Uint nDeviceState = GetState();
	nDeviceState = ConvState( nDeviceState );

	// listen or force-insert	
	if( GetState() == DST_ANSWERED &&
		m_LastCommand.m_MessageID == Cmd_ICMU_SingleStepConference )
	{	
		Smt_Uint nJoinMode;
		m_LastCommand.GetUint( Key_ICMU_JoinMode, &nJoinMode);
		if( nJoinMode == JOIN_SILENT )
		{
			nDeviceState = St_SilentState;
		}
		m_LastCommand.Clear();
	}	
    /////////////////////////////////////////////////////////////////////

	// convert otherparty when otherparty's type is trunk
	Smt_String strOtherParty;
	strOtherParty = ConvOtherParty( m_OtherParty );

	// send event
	Smt_Pdu devEvt;
	devEvt.m_Sender = g_pDeviceState->GetGOR();
	devEvt.m_MessageID = evtid;
	devEvt.m_Status = Smt_Success;

	devEvt.PutString( Key_ICMU_DeviceID, GetID() );
	devEvt.PutUint  ( Key_ICMU_DeviceType, m_DeviceType );
	devEvt.PutUint  ( Key_ICMU_DeviceState, nDeviceState );
	devEvt.PutUint  ( Key_ICMU_CallID, m_CallID );
	devEvt.PutUint  ( Key_ICMU_OldCallID, m_OldCallID );
	devEvt.PutUint  ( Key_ICMU_SecOldCallID, m_SecOldCallID );
	devEvt.PutString( Key_ICMU_OtherParty, strOtherParty );
	devEvt.PutString( Key_ICMU_CallingParty, m_CallingParty );
	devEvt.PutString( Key_ICMU_CalledParty, m_CalledParty);
	devEvt.PutString( Key_ICMU_ThirdParty, m_ThirdParty );
	devEvt.PutUint  ( Key_ICMU_Reason, reason );
	devEvt.PutString( Key_ICMU_TimeStamp, strTimeStamp );
	
	// send event to monitor
	Smt_Uint nSendCount = 0;
	TDeviceMonitor* pMonitor = NULL;
	for(TDeviceMonitorMap::ITERATOR 
		iter = m_MonitorMgr.begin(); 
		iter != m_MonitorMgr.end(); 
		iter++)
	{
		pMonitor = (*iter).int_id_;
		
		devEvt.m_Receiver = pMonitor->m_MonitorGOR;
		devEvt.PutUint( Key_ICMU_DeviceRefID, pMonitor->m_DeviceRefID );
		if( devEvt.m_Receiver > 0 ) g_pDeviceState->PostMessage( devEvt );
		nSendCount++;
	}

	// Print device-event logs
	PrintLog(5, "[DEVICE EVT] ======================== ");
	PrintLog(5, "[DEVICE EVT] Event: %s, MonitorParty<%s>, CallID<%d>", g_pDeviceState->GetIDName(evtid).c_str(), GetID().c_str(), m_CallID );
	PrintLog(5, "[DEVICE EVT] DeviceState : %s", GetStateName().c_str() );
	PrintLog(5, "[DEVICE EVT] CallID      : %d", m_CallID );
	PrintLog(5, "[DEVICE EVT] OldCallID   : %d", m_OldCallID );
	PrintLog(5, "[DEVICE EVT] SecOldCallID: %d", m_SecOldCallID );
	PrintLog(5, "[DEVICE EVT] OtherParty  : %s", m_OtherParty.c_str() );
	PrintLog(5, "[DEVICE EVT] CallingParty: %s", m_CallingParty.c_str() );
	PrintLog(5, "[DEVICE EVT] CalledParty : %s", m_CalledParty.c_str() );
	PrintLog(5, "[DEVICE EVT] ThirdParty  : %s", m_ThirdParty.c_str() );
	PrintLog(5, "[DEVICE EVT] Reason      : %d", reason );
	PrintLog(5, "[DEVICE EVT] TimeStamp: %s", strTimeStamp.c_str() );
	PrintLog(5, "[DEVICE EVT] ======================== <%d>", nSendCount );
	
	return Smt_Success;
}

Smt_Uint TDevice::SendOtherEvent(Smt_Pdu& pdu)
{
	Smt_DateTime dtNow;
	Smt_String strTimeStamp = dtNow.FormatString();

	Smt_Pdu devEvt = pdu;
	devEvt.m_Sender = g_pDeviceState->GetGOR();
	devEvt.m_Status = Smt_Success;

	devEvt.PutUint  ( Key_ICMU_CallID, m_CallID );
	devEvt.PutString( Key_ICMU_OtherParty, m_OtherParty );
	devEvt.PutString( Key_ICMU_CallingParty, m_CallingParty );
	devEvt.PutString( Key_ICMU_CalledParty, m_CalledParty );
	devEvt.PutString( Key_ICMU_TimeStamp, strTimeStamp );

	// send event to monitor
	TDeviceMonitor* pMonitor = NULL;
	for(TDeviceMonitorMap::ITERATOR 
		iter = m_MonitorMgr.begin(); 
		iter != m_MonitorMgr.end(); 
		iter++)
	{
		pMonitor = (*iter).int_id_;
		
		devEvt.m_Receiver = pMonitor->m_MonitorGOR;
		devEvt.PutUint( Key_ICMU_DeviceRefID, pMonitor->m_DeviceRefID );
		if( devEvt.m_Receiver > 0 ) g_pDeviceState->PostMessage( devEvt );
	}

	PrintLog(5, "[DEVICE EVT] ======================== ");
	PrintLog(5, "[DEVICE EVT] Event: %s, MonitorParty<%s>, CallID<%d>", g_pDeviceState->GetIDName(devEvt.m_MessageID).c_str(), GetID().c_str(), m_CallID );
	PrintLog(5, "[DEVICE EVT] TimeStamp: %s", strTimeStamp.c_str() );
	PrintLog(5, "[DEVICE EVT] ======================== " );

	return Smt_Success;
}

Smt_Uint TDevice::ConvState(Smt_Uint state)
{
	Smt_Uint nRetState;
	switch (state)
	{
	case DST_IDLE:
		nRetState = St_NullState;
		break;
	case DST_OFFHOOK:
		nRetState = St_InitiateState;
		break;
	case DST_DESTSEIZED:
		nRetState = St_DeliverState;
		break;
	case DST_INBOUNDCALL:
		nRetState = St_ReceiveState;
		break;
	case DST_ANSWERED:
		nRetState = St_ActiveState;
		break;
	case DST_HELD:
		nRetState = St_HoldState;
		break;
	case DST_BEHELD:
		nRetState = St_HoldState;
		break;
	case DST_CONSULT_DESTSEIZED:
		nRetState = St_DeliverState;
		break;
	case DST_CONSULT_ANSWERED:
		nRetState = St_ActiveState;
		break;
	case DST_CONFERENCED:
		nRetState = St_ActiveState;
		break;
	case DST_QUEUED:
		nRetState = St_QueueState;
		break;
	case DST_DISCONNECTED:
		nRetState = St_ActiveState;
		break;
	case DST_MEETCONNECTED:
		nRetState = St_ActiveState;
		break;
	default:
		nRetState = St_UnavailableState;
		break;
	}
	return nRetState;
}

/****************************************************************************
函 数 名: ConvOtherParty
参    数:
返回数值: 
功能描述: convert otherparty when otherparty's type is trunk
*****************************************************************************/
Smt_String TDevice::ConvOtherParty(Smt_String otherparty)
{
	Smt_String strOtherParty = otherparty;               
	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(strOtherParty, pOtherDevice) == Smt_Success )
	{
		if( pOtherDevice->m_DeviceType == DTYPE_TRUNK_SIP ||
			pOtherDevice->m_DeviceType == DTYPE_TRUNK_DAHDI )
		{
			strOtherParty = pOtherDevice->m_CallerID;
		}
	}

	return strOtherParty;
}

/****************************************************************************
函 数 名: EvtOffHook
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDevice::EvtOffHook(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_OffHook, CauseInitiated);  
	SendDeviceEvent(Evt_ICMU_OffHook, CauseOriginated); 
	return Smt_Success;
}

Smt_Uint TDevice::EvtInboundCall(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_InboundCall, pdu.m_Status );  
	return Smt_Success;
}

Smt_Uint TDevice::EvtDestSeized(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_DestSeized, pdu.m_Status );  
	return Smt_Success;
}

Smt_Uint TDevice::EvtTpAnswered(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_TpAnswered, pdu.m_Status ); 
	return Smt_Success;
}

Smt_Uint TDevice::EvtOpAnswered(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_OpAnswered, pdu.m_Status ); 
	return Smt_Success;
}

Smt_Uint TDevice::EvtTpDisconnected_RetrieveCall(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_TpDisconnected, pdu.m_Status );

/*	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		//pOtherDevice->SendDeviceEvent(Evt_ICMU_OpDisconnected, pdu.m_Status );

		// 如果挂机方是被咨询方，自动接回
		if( pOtherDevice->m_OldCallID != 0 )
		{
			ActRetrieveCall( pOtherDevice );
		}	
	}
*/
	//add by caoyj 20111124  解决otherparty不是对方设备上的，而是本方号码
	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		//需要给对方补发Evt_ICMU_OpDisconnected,并将该事件的OtherParty改为监听强插方
		Smt_DateTime dtNow;
		Smt_String strTimeStamp = dtNow.FormatString();
		
		// convert otherparty's state
		Smt_Uint nDeviceState = pOtherDevice->GetState();
		nDeviceState = ConvState( nDeviceState );
					
		// convert otherparty's otherparty when otherparty's type is trunk
		Smt_String strOtherParty;
		strOtherParty = ConvOtherParty( GetID() );//注意这里是本方号码
		
		// send event
		Smt_Pdu devEvt;
		devEvt.m_Sender = g_pDeviceState->GetGOR();
		devEvt.m_MessageID = Evt_ICMU_OpDisconnected;
		devEvt.m_Status = Smt_Success;
		
		devEvt.PutString( Key_ICMU_DeviceID, pOtherDevice->GetID() );
		devEvt.PutUint  ( Key_ICMU_DeviceType, pOtherDevice->m_DeviceType );
		devEvt.PutUint  ( Key_ICMU_DeviceState, nDeviceState );
		devEvt.PutUint  ( Key_ICMU_CallID, pOtherDevice->m_CallID );
		devEvt.PutUint  ( Key_ICMU_OldCallID, pOtherDevice->m_OldCallID );
		devEvt.PutUint  ( Key_ICMU_SecOldCallID, pOtherDevice->m_SecOldCallID );
		devEvt.PutString( Key_ICMU_OtherParty, strOtherParty );
		devEvt.PutString( Key_ICMU_CallingParty, pOtherDevice->m_CallingParty );
		devEvt.PutString( Key_ICMU_CalledParty, pOtherDevice->m_CalledParty);
		devEvt.PutString( Key_ICMU_ThirdParty, pOtherDevice->m_ThirdParty );
		devEvt.PutUint  ( Key_ICMU_Reason, pdu.m_Status );
		devEvt.PutString( Key_ICMU_TimeStamp, strTimeStamp );
		
		// send event to monitor
		Smt_Uint nSendCount = 0;
		TDeviceMonitor* pMonitor = NULL;
		for(TDeviceMonitorMap::ITERATOR 
			iter = m_MonitorMgr.begin(); 
		iter != m_MonitorMgr.end(); 
		iter++)
		{
			pMonitor = (*iter).int_id_;
			
			devEvt.m_Receiver = pMonitor->m_MonitorGOR;
			devEvt.PutUint( Key_ICMU_DeviceRefID, pMonitor->m_DeviceRefID );
			if( devEvt.m_Receiver > 0 ) g_pDeviceState->PostMessage( devEvt );
			nSendCount++;
		}
		
		// Print device-event logs
		PrintLog(5, "[DEVICE EVT] ==========caoyj============== ");
		PrintLog(5, "[DEVICE EVT] Event: %s, MonitorParty<%s>, CallID<%d>", g_pDeviceState->GetIDName(devEvt.m_MessageID).c_str(), pOtherDevice->GetID().c_str(), pOtherDevice->m_CallID );
		PrintLog(5, "[DEVICE EVT] DeviceState : %s", pOtherDevice->GetStateName().c_str() );
		PrintLog(5, "[DEVICE EVT] CallID      : %d", pOtherDevice->m_CallID );
		PrintLog(5, "[DEVICE EVT] OldCallID   : %d", pOtherDevice->m_OldCallID );
		PrintLog(5, "[DEVICE EVT] SecOldCallID: %d", pOtherDevice->m_SecOldCallID );
		PrintLog(5, "[DEVICE EVT] OtherParty  : %s", strOtherParty.c_str() );
		PrintLog(5, "[DEVICE EVT] CallingParty: %s", pOtherDevice->m_CallingParty.c_str() );
		PrintLog(5, "[DEVICE EVT] CalledParty : %s", pOtherDevice->m_CalledParty.c_str() );
		PrintLog(5, "[DEVICE EVT] ThirdParty  : %s", pOtherDevice->m_ThirdParty.c_str() );
		PrintLog(5, "[DEVICE EVT] Reason      : %d", pdu.m_Status );
		PrintLog(5, "[DEVICE EVT] TimeStamp: %s", strTimeStamp.c_str() );
		PrintLog(5, "[DEVICE EVT] ======================== <%d>", nSendCount );
	
		// 如果挂机方是被咨询方，自动接回
		if( pOtherDevice->m_OldCallID != 0 )
		{
			ActRetrieveCall( pOtherDevice );
		}
	}
	///////////////////////////////////////////////////////////////////////

	return Smt_Success;
}

Smt_Uint TDevice::EvtTpDisconnected(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_TpDisconnected, pdu.m_Status );
	return Smt_Success;
}

Smt_Uint TDevice::EvtOpDisconnected(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_OpDisconnected, pdu.m_Status );
	return Smt_Success;
}

/****************************************************************************
函 数 名: ActHoldOtherParty
参    数:
返回数值: 
功能描述: 把对方转移到 Hold 设备
*****************************************************************************/
Smt_Uint TDevice::ActHoldOtherParty(Smt_Pdu &pdu)
{
	Smt_String strOtherConnID;
	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		strOtherConnID = pOtherDevice->m_ConnID;
	}

	Smt_Pdu sendpdu;
	sendpdu.m_Sender = g_pDeviceState->GetGOR();
	sendpdu.m_Receiver = g_pDeviceState->m_ConnectionStateGOR;
	sendpdu.m_MessageID = Cmd_ICMU_SingleStepTransfer;

	sendpdu.PutString( Key_ICMU_DeviceID, m_DeviceID );
	sendpdu.PutString( Key_ICMU_Context, AMI_CONTEXT_CUSTOM_MEETME );
	sendpdu.PutString( Key_ICMU_ConnectionID, strOtherConnID );
	sendpdu.PutString( Key_ICMU_CalledNum, HLFormatStr("%d",DEFAULT_HOLDDEVICE_NUM) );
	
	if( sendpdu.m_Receiver > 0 ) g_pDeviceState->PostMessage( sendpdu );

	PrintLog(5,"[TDevice::ActHoldOtherParty] DeviceID<%s>, OtherParty<%s>.",
		GetID().c_str(), m_OtherParty.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtHeld
参    数:
返回数值: 
功能描述: 发送双方的保持事件
          如果是咨询请求，发起一个呼叫
*****************************************************************************/
Smt_Uint TDevice::EvtHeld(Smt_Pdu &pdu)
{
	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		pOtherDevice->SetState( DST_HELD );
		pOtherDevice->SendDeviceEvent(Evt_ICMU_TpSuspended, pdu.m_Status );
	}

	SendDeviceEvent(Evt_ICMU_OpHeld, pdu.m_Status ); 

	// make call
	if( pOtherDevice != NULL )
	{
		Smt_String strDeviceID;
		Smt_String strCallerNum;
		Smt_String strCalledNum;
		Smt_Uint   nTimeLen;
		Smt_String strVariable;
		Smt_String strConnectionID;
		Smt_Uint   nLastCommand;

		pOtherDevice->m_LastCommand.GetUint  ( Key_ICMU_LastCommand, &nLastCommand );

		if(nLastCommand == Cmd_ICMU_ConsultationCall)
		{
			pOtherDevice->m_LastCommand.GetString( Key_ICMU_CallerNum, &strCallerNum );
			pOtherDevice->m_LastCommand.GetString( Key_ICMU_CalledNum, &strCalledNum );
			pOtherDevice->m_LastCommand.GetUint  ( Key_ICMU_Timeout, &nTimeLen );

			strConnectionID = pOtherDevice->m_ConnID;
			strDeviceID = strCalledNum;
			strCalledNum = pOtherDevice->m_MeetmeNum;
			strVariable = HLFormatStr("%s=%s,%s<%s>,%s<%s>",
				AMI_CHANNEL_VARIABLE_VAR1, AMI_CONSULTATIONCALL,
				AMI_CONSULTINGPARTY, pOtherDevice->m_DeviceID.c_str(),
				AMI_CONSULTEDPARTY, strDeviceID.c_str() );

			Smt_Pdu sendpdu;
			sendpdu.m_Sender = g_pDeviceState->GetGOR();
			sendpdu.m_Receiver = g_pDeviceState->m_ConnectionStateGOR;
			sendpdu.m_MessageID = Cmd_ICMU_MakeCall;
			
			sendpdu.PutString( Key_ICMU_DeviceID, strDeviceID );
			sendpdu.PutUint  ( Key_ICMU_DeviceType, DTYPE_EXTENSION );
			sendpdu.PutString( Key_ICMU_CallerNum, strCallerNum );
			sendpdu.PutString( Key_ICMU_CalledNum, strCalledNum );
			sendpdu.PutUint  ( Key_ICMU_Timeout, nTimeLen );
			sendpdu.PutString( Key_ICMU_Variable, strVariable );
			sendpdu.PutString( Key_ICMU_Context, AMI_CONTEXT_CUSTOM_MEETME );
			sendpdu.PutString( Key_ICMU_ConnectionID, strConnectionID );

			if( sendpdu.m_Receiver > 0 ) g_pDeviceState->PostMessage( sendpdu );

			PrintLog(5,"[TDevice::EvtHeld] ConsultationCall, CallerNum<%s>, CalledNum<%s>, MeetmeNum<%s>.",
				strCallerNum.c_str(), strDeviceID.c_str(), pOtherDevice->m_MeetmeNum.c_str() );
		}
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtRetrieved
参    数:
返回数值: 
功能描述: 发送双方的取回事件
*****************************************************************************/
Smt_Uint TDevice::EvtTpRetrieved(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_TpRetrieved, pdu.m_Status );
	return Smt_Success;
}

Smt_Uint TDevice::EvtOpRetrieved(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_OpRetrieved, pdu.m_Status );
	return Smt_Success;
}

/****************************************************************************
函 数 名: SetTimer
参    数:
返回数值: 
功能描述: 设置设备时钟
*****************************************************************************/
Smt_Uint TDevice::SetTimer(Smt_Pdu &pdu)
{
	Smt_Uint nTimeLen;
	pdu.GetUint( Key_ICMU_Timeout, &nTimeLen);

	if( nTimeLen == 0 )
	{
		nTimeLen = DEFAULT_CMUTIMELEN;
	}

	m_TimerReason = pdu.m_Status;
	m_TimerID = g_pDeviceState->SetSingleTimer( nTimeLen, Evt_ICMU_DeviceTimer, (Smt_Uint)this );

	PrintLog(5,"[TDevice::SetTimer] DeviceID<%s>, TimerID<%d>,nTimeLen<%d>,TimerReason<%d>.",
		GetID().c_str(), m_TimerID,nTimeLen,m_TimerReason );

	return Smt_Success;
}

Smt_Uint TDevice::ClearTimer()
{

	PrintLog(5,"[TDevice::ClearTimer] DeviceID<%s>, TimerID<%d>.",
		GetID().c_str(), m_TimerID );

	if( m_TimerID > 0 )
		g_pDeviceState->ClearTimer( m_TimerID );
	m_TimerID = 0;

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtConsultDestSeized
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDevice::EvtConsultDestSeized(Smt_Pdu &pdu)
{
	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		pOtherDevice->m_OtherParty = g_pDeviceState->LookupOtherPartyByCallID(pOtherDevice->m_DeviceID, m_CallID );
		pOtherDevice->SetState( DST_OFFHOOK );
		pOtherDevice->SendDeviceEvent(Evt_ICMU_OffHook, CauseInitiated);  
		pOtherDevice->SendDeviceEvent(Evt_ICMU_OffHook, CauseOriginated); 
	}

	SetState( DST_INBOUNDCALL );
	SendDeviceEvent(Evt_ICMU_InboundCall, pdu.m_Status);  

	if( pOtherDevice!= NULL )
	{
		pOtherDevice->SetState( DST_CONSULT_DESTSEIZED );
		pOtherDevice->SendDeviceEvent(Evt_ICMU_DestSeized, pdu.m_Status);  
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtTpTransferred
参    数:
返回数值: 
功能描述: 发布本方转移事件，并把保持方转移到通话
*****************************************************************************/
Smt_Uint TDevice::EvtTpTransferred(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_TpTransferred, pdu.m_Status);  
	SendDeviceEvent(Evt_ICMU_TpDisconnected, pdu.m_Status);  

	Smt_String strHoldParty;
	Smt_String strHoldConnID;
	TCall* pCall = NULL;
	
	if( g_pCallState->m_CallMgr.Lookup(m_CallID, pCall) == Smt_Success )
	{
		TConnID* pTmpConnID = NULL;
		for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
			iter!= pCall->m_ConnIDList.end(); 
			iter++)
		{
			pTmpConnID = (*iter).int_id_;
			if(pTmpConnID->m_ConnState == CONN_HOLD)
			{
				break;
			}
		}
		
		strHoldParty = pTmpConnID->m_DeviceID;
		strHoldConnID = pTmpConnID->m_ConnID;
	}

	Smt_Pdu sendpdu;
	sendpdu.m_Sender = g_pDeviceState->GetGOR();
	sendpdu.m_Receiver = g_pDeviceState->m_ConnectionStateGOR;
	sendpdu.m_MessageID = Cmd_ICMU_SingleStepTransfer;
	
	sendpdu.PutString( Key_ICMU_DeviceID, m_DeviceID );
	sendpdu.PutString( Key_ICMU_ConnectionID, strHoldConnID );
	sendpdu.PutString( Key_ICMU_CalledNum, m_MeetmeNum );
	sendpdu.PutString( Key_ICMU_Context, AMI_CONTEXT_CUSTOM_MEETME );
	
	if( sendpdu.m_Receiver > 0 ) g_pDeviceState->PostMessage( sendpdu );
	
	PrintLog(5,"[TDevice::EvtTpTransferred] DeviceID<%s>, HoldParty<%s>.",
		GetID().c_str(), strHoldParty.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtOpTransferred
参    数:
返回数值: 
功能描述: 发送 OpTransferred 事件，并修正 OtherParty
*****************************************************************************/
Smt_Uint TDevice::EvtOpTransferred(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_OpTransferred, pdu.m_Status); 

	TCall* pCall = NULL;
	if( g_pCallState->m_CallMgr.Lookup(m_CallID, pCall) == Smt_Success )
	{
		TConnID* pTmpConnID = NULL;
		for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
			iter!= pCall->m_ConnIDList.end(); 
			iter++)
		{
			pTmpConnID = (*iter).int_id_;
			if(pTmpConnID->m_DeviceID != m_DeviceID)
			{
				m_OtherParty = pTmpConnID->m_DeviceID;
				break;
			}
		}
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtTpConferenced
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDevice::EvtTpConferenced(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_TpConferenced, pdu.m_Status); 
	return Smt_Success;
}

Smt_Uint TDevice::EvtOpConferenced(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_OpConferenced, pdu.m_Status); 
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtConfTpDisconnected
参    数:
返回数值: 
功能描述: 会议时一方挂机，其他双方发送 OpDisconnected 事件，并修正 OtherParty
*****************************************************************************/
Smt_Uint TDevice::EvtConfTpDisconnected(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_TpDisconnected, pdu.m_Status); 

	TCall* pCall = NULL;
	TDevice* pOtherDevice = NULL;
	if( g_pCallState->m_CallMgr.Lookup(m_CallID, pCall) == Smt_Success )
	{
		TConnID* pTmpConnID = NULL;
		for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
			iter!= pCall->m_ConnIDList.end(); 
			iter++)
		{
			pTmpConnID = (*iter).int_id_;
			if(pTmpConnID->m_DeviceID != m_DeviceID)  // if-1
			{
				if(g_pDeviceState->m_DeviceMgr.Lookup(pTmpConnID->m_DeviceID, pOtherDevice) == Smt_Success )
				{
					pOtherDevice->SetState( DST_MEETCONNECTED );
					pOtherDevice->EvtOpDisconnected( pdu );
					pOtherDevice->m_OtherParty = g_pDeviceState->LookupOtherPartyByCallID(pOtherDevice->m_DeviceID, pOtherDevice->m_CallID );
				}
			} // end-if-1
		} // end-for
	}
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtTp_OpDisconnected
参    数:
返回数值: 
功能描述:
*****************************************************************************/
Smt_Uint TDevice::EvtTp_OpDisconnected(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_TpDisconnected, pdu.m_Status); 

	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		// 如果挂机方是咨询通话时的被保持方，修改呼叫数据/状态
		if( pOtherDevice->GetState() == DST_CONSULT_DESTSEIZED || 
			pOtherDevice->GetState() == DST_CONSULT_ANSWERED )
		{
			Smt_Uint nSecOldCallID = pOtherDevice->m_CallID;
			pOtherDevice->m_CallID = m_CallID;
			pOtherDevice->m_OldCallID = 0;
			pOtherDevice->m_SecOldCallID = nSecOldCallID; 
			pOtherDevice->m_OtherParty = m_DeviceID;
			pOtherDevice->SendDeviceEvent(Evt_ICMU_OpDisconnected, pdu.m_Status );
			
			pOtherDevice->m_CallID = nSecOldCallID;
			pOtherDevice->m_OldCallID = 0;
			pOtherDevice->m_SecOldCallID = 0;
			pOtherDevice->m_OtherParty = g_pDeviceState->LookupOtherPartyByCallID(pOtherDevice->m_DeviceID, nSecOldCallID);
			
			if( pOtherDevice->GetState() == DST_CONSULT_ANSWERED )
			{
				pOtherDevice->SetState( DST_MEETCONNECTED );
			}
			//else if( pOtherDevice->GetState() == DST_CONSULT_DESTSEIZED)
			//{
			//	pOtherDevice->SetState( DST_DESTSEIZED );
			//}
		}
		else // DST_HELD/ DST_BEHELD
			pOtherDevice->SendDeviceEvent(Evt_ICMU_OpDisconnected, pdu.m_Status );
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtDestBusy_RetrieveCall
参    数:
返回数值: 
功能描述: 咨询时目标忙，接回话路，从来不发生么？
*****************************************************************************/
Smt_Uint TDevice::EvtDestBusy_RetrieveCall(Smt_Pdu &pdu)
{	
	SendDeviceEvent(Evt_ICMU_TpDisconnected, pdu.m_Status); 

	ClearTimer();

	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		pOtherDevice->SendDeviceEvent(Evt_ICMU_DestBusy, pdu.m_Status );

		if( pOtherDevice->GetState() == DST_HELD)
			ActRetrieveCall( pOtherDevice );
	}

	return Smt_Success;
}	

Smt_Uint TDevice::EvtDestInvalid_RetrieveCall(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_TpDisconnected, pdu.m_Status); 
	
	ClearTimer();
	
	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		pOtherDevice->SendDeviceEvent(Evt_ICMU_DestInvalid, pdu.m_Status );
		
		if( pOtherDevice->GetState() == DST_HELD)
			ActRetrieveCall( pOtherDevice );
	}

	return Smt_Success;
}

Smt_Uint TDevice::EvtDestFail_RetrieveCall(Smt_Pdu &pdu)
{	
	SendDeviceEvent(Evt_ICMU_DestInvalid, pdu.m_Status );

	if( GetState() == DST_HELD)
		ActRetrieveCall( this );

	return Smt_Success;
}

Smt_Uint TDevice::ActRetrieveCall(TDevice* pdevice)
{
	if( pdevice == NULL)
	{
		return Smt_Fail;
	}

	TDevice*     pDevice = pdevice;
	TDevice*     pHoldDevice = NULL;
	Smt_Uint   nHoldCallID; 
	Smt_String strDeviceID; 
	Smt_String strHoldDeviceID;
	Smt_String strHoldConnID;
	Smt_String strMeetmeNum;	

	if( pDevice->m_OldCallID != 0 )
	{
		nHoldCallID = pDevice->m_OldCallID;
	}
	else
		nHoldCallID = pDevice->m_CallID;
	
	strDeviceID = pDevice->m_DeviceID;

	strHoldDeviceID = g_pDeviceState->LookupOtherPartyByCallID(strDeviceID, nHoldCallID );
	if(g_pDeviceState->m_DeviceMgr.Lookup(strHoldDeviceID, pHoldDevice) == Smt_Success )
	{
		strHoldConnID = pHoldDevice->m_ConnID;
	}

	pDevice->m_CallID = nHoldCallID;
	pDevice->m_OldCallID = 0;
	pDevice->m_OtherParty = strHoldDeviceID;
	pDevice->m_LastCommand.m_MessageID = Cmd_ICMU_RetrieveCall;
	strMeetmeNum = pDevice->m_MeetmeNum;
	
	Smt_Pdu sendpdu;
	sendpdu.m_Sender = g_pDeviceState->GetGOR();
	sendpdu.m_Receiver = g_pDeviceState->m_ConnectionStateGOR;
	sendpdu.m_MessageID = Cmd_ICMU_SingleStepTransfer;
	
	sendpdu.PutString( Key_ICMU_DeviceID, strDeviceID );
	sendpdu.PutString( Key_ICMU_ConnectionID, strHoldConnID );
	sendpdu.PutString( Key_ICMU_CalledNum, strMeetmeNum );
	sendpdu.PutString( Key_ICMU_Context, AMI_CONTEXT_CUSTOM_MEETME );
	
	if( sendpdu.m_Receiver > 0 ) g_pDeviceState->PostMessage( sendpdu );
	
	PrintLog(5,"[TDevice::ActRetrieveCall] HoldCallID<%d>, DeviceID<%s>.",
		nHoldCallID, strDeviceID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtTp_OpTransferred
参    数:
返回数值: 
功能描述: 单步转移发送双方的转移事件
*****************************************************************************/
Smt_Uint TDevice::EvtTp_OpTransferred(Smt_Pdu &pdu)
{
	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		pOtherDevice->SetCallData( pdu );
		pOtherDevice->SendDeviceEvent( Evt_ICMU_OpTransferred, pdu.m_Status );

		if(g_pDeviceState->m_DeviceMgr.Lookup(pOtherDevice->m_OtherParty, pOtherDevice) == Smt_Success )
		{
			pOtherDevice->SetCallData( pdu );
			pOtherDevice->SendDeviceEvent( Evt_ICMU_OpTransferred, pdu.m_Status );
		}
	}

	SendDeviceEvent(Evt_ICMU_TpTransferred, pdu.m_Status); 
	SendDeviceEvent(Evt_ICMU_TpDisconnected, pdu.m_Status);  // 补挂机事件

	return Smt_Success;
}

Smt_Uint TDevice::EvtDestBusy(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_DestBusy, pdu.m_Status); 
	return Smt_Success;
}

Smt_Uint TDevice::EvtDestBusy_TpDisconnected(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_DestBusy, pdu.m_Status); 
	SendDeviceEvent(Evt_ICMU_TpDisconnected, pdu.m_Status); 
	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtTp_OpConferenced
参    数:
返回数值: 
功能描述: 单步会议，发送三方会议事件
*****************************************************************************/
Smt_Uint TDevice::EvtTp_OpConferenced(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_TpConferenced, pdu.m_Status); 
	
	TCall* pCall = NULL;
	TDevice* pOtherDevice = NULL;
	if( g_pCallState->m_CallMgr.Lookup(m_CallID, pCall) == Smt_Success )
	{
		TConnID* pTmpConnID = NULL;
		for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
			iter!= pCall->m_ConnIDList.end(); 
			iter++)
		{
			pTmpConnID = (*iter).int_id_;
			if(pTmpConnID->m_DeviceID != m_DeviceID)  // if-1
			{
				if(g_pDeviceState->m_DeviceMgr.Lookup(pTmpConnID->m_DeviceID, pOtherDevice) == Smt_Success )
				{
					Smt_String strTemp = pOtherDevice->m_OtherParty ;
					pOtherDevice->m_OtherParty = m_DeviceID;
					pOtherDevice->EvtOpConferenced( pdu );
					pOtherDevice->m_OtherParty = strTemp;
				}
			} // end-if-1
		} // end-for
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtRouteRequest
参    数:
返回数值: 
功能描述: 排队路由请求事件
*****************************************************************************/
Smt_Uint TDevice::EvtRouteRequest(Smt_Pdu &pdu)
{
	Smt_DateTime dtNow;
	Smt_String strTimeStamp;
	Smt_Uint   nRouteID;
	Smt_Uint   nCallID;
	Smt_Uint   nOldCallID;
	Smt_String strCallingParty;
	Smt_String strCalledParty;
	Smt_String strOtherParty;
	Smt_String strOtherConnectionID;
	Smt_Uint   nReason;

	pdu.GetString( Key_ICMU_DeviceID, &strOtherParty );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetUint  ( Key_ICMU_OldCallID, &nOldCallID );
	pdu.GetString( Key_ICMU_CallingParty, &strCallingParty );
	pdu.GetString( Key_ICMU_CalledParty, &strCalledParty );
	pdu.GetString( Key_ICMU_ConnectionID, &strOtherConnectionID );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );

	strTimeStamp = dtNow.FormatString();
	nRouteID = g_pDeviceState->AllocateRouteID();

	// 修正 OtherParty
	strOtherParty = ConvOtherParty( strOtherParty );

	// create routeid
	TRouteID* pRouteID = new TRouteID();
	pRouteID->m_CallID = nCallID;
	pRouteID->m_OldCallID = nOldCallID;
	pRouteID->m_RouteID = nRouteID;
	pRouteID->m_RouteDN = GetID();
	pRouteID->m_OtherParty = strOtherParty;
	pRouteID->m_OtherConnID = strOtherConnectionID;
	pRouteID->m_CallingParty = strCallingParty;
	pRouteID->m_CalledParty = strCalledParty;

	g_pDeviceState->m_RouteIDMgr.SetAt( nRouteID, pRouteID );

	PrintLog(5, "[TDevice::EvtRouteRequest] Create RouteID Ok, RouteDN<%s>, RouteID<%d>, CallID<%d>.",
		GetID().c_str(), nRouteID, nCallID );

	// send route event
	Smt_Pdu devEvt;
	devEvt.m_Sender = g_pDeviceState->GetGOR();
	devEvt.m_MessageID = Evt_ICMU_RouteRequest;
	devEvt.m_Status = Smt_Success;
	
	devEvt.PutString( Key_ICMU_DeviceID, GetID() );
	devEvt.PutUint  ( Key_ICMU_CallID, nCallID );
	devEvt.PutUint  ( Key_ICMU_OldCallID, nOldCallID );
	devEvt.PutUint  ( Key_ICMU_RouteID, nRouteID );
	devEvt.PutString( Key_ICMU_OtherParty, strOtherParty );
	devEvt.PutString( Key_ICMU_CallingParty, strCallingParty );
	devEvt.PutString( Key_ICMU_CalledParty, strCalledParty);
	devEvt.PutUint  ( Key_ICMU_Reason, nReason );
	devEvt.PutString( Key_ICMU_TimeStamp, strTimeStamp );
	
	// send event to monitor
	Smt_Uint nSendCount = 0;
	TDeviceMonitor* pMonitor = NULL;
	for(TDeviceMonitorMap::ITERATOR 
		iter = m_MonitorMgr.begin(); 
		iter != m_MonitorMgr.end(); 
		iter++)
	{
		pMonitor = (*iter).int_id_;
		
		devEvt.m_Receiver = pMonitor->m_MonitorGOR;
		devEvt.PutUint( Key_ICMU_DeviceRefID, pMonitor->m_DeviceRefID );
		if( devEvt.m_Receiver > 0 ) g_pDeviceState->PostMessage( devEvt );
		nSendCount++;
	}
	
	// Print device-event logs
	PrintLog(5, "[ROUTE EVT] ======================== ");
	PrintLog(5, "[ROUTE EVT] Event: %s, MonitorParty<%s>, CallID<%d>, RouteID<%d>", g_pDeviceState->GetIDName(devEvt.m_MessageID).c_str(), GetID().c_str(), nCallID, nRouteID );
	PrintLog(5, "[ROUTE EVT] CallID      : %d", nCallID );
	PrintLog(5, "[ROUTE EVT] OldCallID   : %d", nOldCallID );
	PrintLog(5, "[ROUTE EVT] RouteID     : %d", nRouteID );
	PrintLog(5, "[ROUTE EVT] OtherParty  : %s", strOtherParty.c_str() );
	PrintLog(5, "[ROUTE EVT] CallingParty: %s", strCallingParty.c_str() );
	PrintLog(5, "[ROUTE EVT] CalledParty : %s", strCalledParty.c_str() );
	PrintLog(5, "[ROUTE EVT] Reason      : %d", nReason );
	PrintLog(5, "[ROUTE EVT] TimeStamp   : %s", strTimeStamp.c_str() );
	PrintLog(5, "[ROUTE EVT] ======================== <%d>", nSendCount );
	
	return Smt_Success;
}

Smt_Uint TDevice::EvtRouteEnd(Smt_Pdu &pdu)
{
	Smt_Uint nCallID;
	Smt_Uint nReason;
	pdu.GetUint( Key_ICMU_CallID, &nCallID );
	pdu.GetUint( Key_ICMU_Reason, &nReason );

	TRouteID* pRouteID = g_pDeviceState->LookupRouteIDByCallID( nCallID);
	if( pRouteID == NULL )
	{
		PrintLog(5, "[TDevice::EvtRouteEnd] Lookup RouteID Fail, CallID<%d>", nCallID );
		return Smt_Fail;
	}

	// send route event
	Smt_DateTime dtNow;
	Smt_String strTimeStamp;
	Smt_Uint   nOldCallID;
	Smt_Uint   nRouteID;
	Smt_String strOtherParty;
	Smt_String strRouteDN;

	nOldCallID = pRouteID->m_OldCallID;
	nRouteID = pRouteID->m_RouteID;
	strOtherParty = pRouteID->m_OtherParty;
	strTimeStamp = dtNow.FormatString();
	strRouteDN = pRouteID->m_RouteDN;

	Smt_Pdu devEvt;
	devEvt.m_Sender = g_pDeviceState->GetGOR();
	devEvt.m_MessageID = Evt_ICMU_RouteEnd;
	devEvt.m_Status = Smt_Success;
	
	devEvt.PutString( Key_ICMU_DeviceID, GetID() );
	devEvt.PutUint  ( Key_ICMU_CallID, nCallID );
	devEvt.PutUint  ( Key_ICMU_OldCallID, nOldCallID );
	devEvt.PutUint  ( Key_ICMU_RouteID, nRouteID );
	devEvt.PutString( Key_ICMU_OtherParty, strOtherParty );
	devEvt.PutUint  ( Key_ICMU_Reason, nReason );
	devEvt.PutString( Key_ICMU_TimeStamp, strTimeStamp );
	
	// send event to monitor
	Smt_Uint nSendCount = 0;
	TDeviceMonitor* pMonitor = NULL;
	for(TDeviceMonitorMap::ITERATOR 
		iter = m_MonitorMgr.begin(); 
		iter != m_MonitorMgr.end(); 
		iter++)
	{
		pMonitor = (*iter).int_id_;
		
		devEvt.m_Receiver = pMonitor->m_MonitorGOR;
		devEvt.PutUint( Key_ICMU_DeviceRefID, pMonitor->m_DeviceRefID );
		if( devEvt.m_Receiver > 0 ) g_pDeviceState->PostMessage( devEvt );
		nSendCount++;
	}
	
	// Print device-event logs
	PrintLog(5, "[ROUTE EVT] ======================== ");
	PrintLog(5, "[ROUTE EVT] Event: %s, MonitorParty<%s>, CallID<%d>, RouteID<%d>", g_pDeviceState->GetIDName(devEvt.m_MessageID).c_str(), GetID().c_str(), nCallID, nRouteID );
	PrintLog(5, "[ROUTE EVT] CallID      : %d", nCallID );
	PrintLog(5, "[ROUTE EVT] OldCallID   : %d", nOldCallID );
	PrintLog(5, "[ROUTE EVT] RouteID     : %d", nRouteID );
	PrintLog(5, "[ROUTE EVT] OtherParty  : %s", strOtherParty.c_str() );
	PrintLog(5, "[ROUTE EVT] Reason      : %d", nReason );
	PrintLog(5, "[ROUTE EVT] TimeStamp   : %s", strTimeStamp.c_str() );
	PrintLog(5, "[ROUTE EVT] ======================== <%d>", nSendCount );
	
	// release routeid
	g_pDeviceState->m_RouteIDMgr.Remove( nRouteID );
	if( pRouteID != NULL )
	{
		delete pRouteID;
		pRouteID = NULL;
	}

	PrintLog(5, "[TDevice::EvtRouteEnd] Delete RouteID Ok, RouteDN<%s>, RouteID<%d>, CallID<%d>.",
		strRouteDN.c_str(), nRouteID, nCallID );

	return Smt_Success;
}

Smt_Uint TDevice::EvtReRoute(TRouteID* prouteid, Smt_Uint reason)
{
	TRouteID* pRouteID = prouteid;
	Smt_DateTime dtNow;
	Smt_String strTimeStamp;
	Smt_Uint   nCallID;
	Smt_Uint   nRouteID;
	Smt_Uint   nReason;
	Smt_String strRouteDN;
	Smt_String strOtherParty;
	Smt_String strCallingParty;
	Smt_String strCalledParty;

	strRouteDN = pRouteID->m_RouteDN;
	nCallID = pRouteID->m_CallID;
	nRouteID = pRouteID->m_RouteID;
	strOtherParty= pRouteID->m_OtherParty;
	strCallingParty = pRouteID->m_CallingParty;
	strCalledParty = pRouteID->m_CalledParty;
	nReason = reason;
	strTimeStamp = dtNow.FormatString();

	Smt_Pdu devEvt;
	devEvt.m_Sender = g_pDeviceState->GetGOR();
	devEvt.m_MessageID = Evt_ICMU_ReRoute;
	devEvt.m_Status = Smt_Success;

	devEvt.PutString( Key_ICMU_DeviceID, GetID() );
	devEvt.PutUint  ( Key_ICMU_CallID, nCallID );
	devEvt.PutUint  ( Key_ICMU_RouteID, nRouteID );
	devEvt.PutString( Key_ICMU_OtherParty, strOtherParty );
	devEvt.PutString( Key_ICMU_CallingParty, strCallingParty );
	devEvt.PutString( Key_ICMU_CalledParty, strCalledParty);
	devEvt.PutUint  ( Key_ICMU_Reason, nReason );
	devEvt.PutString( Key_ICMU_TimeStamp, strTimeStamp );
	
	// send event to monitor
	Smt_Uint nSendCount = 0;
	TDeviceMonitor* pMonitor = NULL;
	for(TDeviceMonitorMap::ITERATOR 
		iter = m_MonitorMgr.begin(); 
		iter != m_MonitorMgr.end(); 
		iter++)
	{
		pMonitor = (*iter).int_id_;
		
		devEvt.m_Receiver = pMonitor->m_MonitorGOR;
		devEvt.PutUint( Key_ICMU_DeviceRefID, pMonitor->m_DeviceRefID );
		if( devEvt.m_Receiver > 0 ) g_pDeviceState->PostMessage( devEvt );
		nSendCount++;
	}
	
	// Print device-event logs
	PrintLog(5, "[ROUTE EVT] ======================== ");
	PrintLog(5, "[ROUTE EVT] Event: %s, MonitorParty<%s>, CallID<%d>, RouteID<%d>", g_pDeviceState->GetIDName(devEvt.m_MessageID).c_str(), GetID().c_str(), nCallID, nRouteID );
	PrintLog(5, "[ROUTE EVT] CallID      : %d", nCallID );
	PrintLog(5, "[ROUTE EVT] RouteID     : %d", nRouteID );
	PrintLog(5, "[ROUTE EVT] OtherParty  : %s", strOtherParty.c_str() );
	PrintLog(5, "[ROUTE EVT] CallingParty: %s", strCallingParty.c_str() );
	PrintLog(5, "[ROUTE EVT] CalledParty : %s", strCalledParty.c_str() );
	PrintLog(5, "[ROUTE EVT] Reason      : %d", nReason );
	PrintLog(5, "[ROUTE EVT] TimeStamp   : %s", strTimeStamp.c_str() );
	PrintLog(5, "[ROUTE EVT] ======================== <%d>", nSendCount );

	return Smt_Success;
}

Smt_Uint TDevice::EvtQueued_RouteRequest(Smt_Pdu &pdu)
{
	SendDeviceEvent(Evt_ICMU_Queued, pdu.m_Status); 

	TDevice* pOtherDevice = NULL;
	if( g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		if( pOtherDevice->m_DeviceType == DTYPE_ROUTE )
		{
			pOtherDevice->EvtRouteRequest( pdu );
		}
	}
	return Smt_Success;
}

Smt_Uint TDevice::EvtDestSeized_RouteEnd(Smt_Pdu &pdu)
{	
	SendDeviceEvent(Evt_ICMU_DestSeized, pdu.m_Status); 

	TDevice* pRouteDevcie = NULL;
	TRouteID* pRouteID = g_pDeviceState->LookupRouteIDByCallID( m_CallID );
	if( pRouteID != NULL )
	{
		if(g_pDeviceState->m_DeviceMgr.Lookup(pRouteID->m_RouteDN, pRouteDevcie) == Smt_Success )
		{
			pRouteDevcie->EvtRouteEnd( pdu );
		}
	}
	
	return Smt_Success;
}

Smt_Uint TDevice::EvtTpDisconnected_RouteEnd(Smt_Pdu &pdu)
{
	TDevice* pRouteDevcie = NULL;
	TRouteID* pRouteID = g_pDeviceState->LookupRouteIDByCallID( m_CallID );
	if( pRouteID != NULL )
	{
		if(g_pDeviceState->m_DeviceMgr.Lookup(pRouteID->m_RouteDN, pRouteDevcie) == Smt_Success )
		{
			pRouteDevcie->EvtRouteEnd( pdu );
		}
	}

	SendDeviceEvent(Evt_ICMU_TpDisconnected, pdu.m_Status); 

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtTransferDestSeized
参    数:
返回数值: 
功能描述: 单步转移后，超时引起的振铃事件
*****************************************************************************/
Smt_Uint TDevice::EvtTransferDestSeized(Smt_Pdu &pdu)
{
	SetState( DST_INBOUNDCALL );
	SendDeviceEvent(Evt_ICMU_InboundCall, pdu.m_Status);  
	
	TDevice* pOtherDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		pOtherDevice->SetState( DST_DESTSEIZED );
		pOtherDevice->SendDeviceEvent(Evt_ICMU_DestSeized, pdu.m_Status);  
	}
	return Smt_Success;
}

Smt_Uint TDevice::ActSingleStepTransfer()
{
	TDevice* pOtherDevice = NULL;
/*	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		Smt_String strDeviceID;
		Smt_String strCallerNum;
		Smt_String strCalledNum;
		Smt_String strVariable;
		
		m_LastCommand.GetString( Key_ICMU_CalledNum, &strCalledNum );
		strDeviceID = strCalledNum;	
		strCallerNum = pOtherDevice->m_CallerID; // pOtherDevice->m_DeviceID;
		strCalledNum = m_MeetmeNum;
		
		strVariable = HLFormatStr("%s=%s,%s<%s>",
			AMI_CHANNEL_VARIABLE_VAR1, AMI_SINGLESTEPTRANSFER, AMI_TRANSFERREDPARTY, pOtherDevice->m_DeviceID.c_str() );
		
		Smt_Pdu sendpdu;
		sendpdu.m_Sender = g_pDeviceState->GetGOR();
		sendpdu.m_Receiver = g_pDeviceState->m_ConnectionStateGOR;
		sendpdu.m_MessageID = Cmd_ICMU_MakeCall;
		
		sendpdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		sendpdu.PutUint  ( Key_ICMU_DeviceType, DTYPE_EXTENSION );
		sendpdu.PutString( Key_ICMU_CallerNum, strCallerNum );
		sendpdu.PutString( Key_ICMU_CalledNum, strCalledNum );
		sendpdu.PutString( Key_ICMU_Variable, strVariable );
		sendpdu.PutString( Key_ICMU_Context, AMI_CONTEXT_CUSTOM_MEETME );
		
		if( sendpdu.m_Receiver > 0 ) g_pDeviceState->PostMessage( sendpdu );
		
		PrintLog(5,"[TDevice::ActSingleStepTransfer] SingleStepTransfer, CalledNum<%s>, MeetmeNum<%s>.",
			strDeviceID.c_str(), pOtherDevice->m_MeetmeNum.c_str() );			
	}
*/
	//add by caoyj 20110917
	TDevice* pCalledDevice = NULL;
	if(g_pDeviceState->m_DeviceMgr.Lookup(m_OtherParty, pOtherDevice) == Smt_Success )
	{
		Smt_String strDeviceID;
		Smt_String strCallerNum;
		Smt_String strCalledNum;
		Smt_String strVariable;
		Smt_String strOtherConnID;
		Smt_Uint   nCalledDeviceType = 0;
		
		m_LastCommand.GetString( Key_ICMU_CalledNum, &strCalledNum );
		
		if( g_pDeviceState->m_DeviceMgr.Lookup(strCalledNum, pCalledDevice) == Smt_Success )
		{
			nCalledDeviceType = pCalledDevice->m_DeviceType;
		}
		/*add by caoyj 20110921增加HMP时DTYPE_ROUTE类型处理*/
		if( nCalledDeviceType == DTYPE_IVR_INBOUND  || nCalledDeviceType == DTYPE_ROUTE)
		{
			strDeviceID = GetID();	
			strOtherConnID = pOtherDevice->m_ConnID;	
			
			Smt_Pdu sendpdu;
			sendpdu.m_Sender = g_pDeviceState->GetGOR();
			sendpdu.m_Receiver = g_pDeviceState->m_ConnectionStateGOR;
			sendpdu.m_MessageID = Cmd_ICMU_SingleStepTransfer;		
			
			sendpdu.PutString( Key_ICMU_DeviceID, strDeviceID );
			sendpdu.PutString( Key_ICMU_ConnectionID, strOtherConnID );
			sendpdu.PutString( Key_ICMU_CalledNum, strCalledNum );	
			if( sendpdu.m_Receiver > 0 ) g_pDeviceState->PostMessage( sendpdu );
			
			PrintLog(5,"[TDevice::ActSingleStepTransfer] SingleStepTransfer, DeviceID<%s>, OtherParty<%s>, CalledNum<%s>,ConnectionID<%s>.",
				strDeviceID.c_str(),m_OtherParty.c_str(),strCalledNum.c_str(),strOtherConnID.c_str() );
		}
		else // other device
		{
			strDeviceID = strCalledNum;	
			strCallerNum = pOtherDevice->m_CallerID; // pOtherDevice->m_DeviceID;
			strCalledNum = m_MeetmeNum;
			strOtherConnID = pOtherDevice->m_ConnID;
			
			strVariable = HLFormatStr("%s=%s,%s<%s>",
				AMI_CHANNEL_VARIABLE_VAR1, AMI_SINGLESTEPTRANSFER,
				AMI_TRANSFERREDPARTY, pOtherDevice->m_DeviceID.c_str());
			
			Smt_Pdu sendpdu;
			sendpdu.m_Sender = g_pDeviceState->GetGOR();
			sendpdu.m_Receiver = g_pDeviceState->m_ConnectionStateGOR;
			sendpdu.m_MessageID = Cmd_ICMU_MakeCall;
			
			sendpdu.PutString( Key_ICMU_DeviceID, strDeviceID );
			sendpdu.PutUint  ( Key_ICMU_DeviceType, DTYPE_EXTENSION );
			sendpdu.PutString( Key_ICMU_CallerNum, strCallerNum );
			sendpdu.PutString( Key_ICMU_CalledNum, strCalledNum );
			sendpdu.PutString( Key_ICMU_Variable, strVariable );
			sendpdu.PutString( Key_ICMU_Context, AMI_CONTEXT_CUSTOM_MEETME );
			sendpdu.PutString( Key_ICMU_ConnectionID, strOtherConnID );
			
			if( sendpdu.m_Receiver > 0 ) g_pDeviceState->PostMessage( sendpdu );
			
			PrintLog(5,"[TDevice::ActSingleStepTransfer] SingleStepTransfer,DeviceID<%s>,CallerNum<%s>,OtherParty<%s>,ConnectionID<%s>,CalledDeviceType<%d>.",
				strDeviceID.c_str(),strCallerNum.c_str(),m_OtherParty.c_str(),strOtherConnID.c_str(),nCalledDeviceType );
		}
	}
	else
	{
		PrintLog(5,"[TDevice::ActSingleStepTransfer] SingleStepTransfer Fail, OtherParty<%s>.",
			m_OtherParty.c_str() );
	}
	return Smt_Success;		
}
