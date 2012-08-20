//***************************************************************************
// TDeviceState.cpp : implementation file
//***************************************************************************

#include "TDeviceState.h"
#include "TCallState.h"
#include "TConnectionState.h"
#include "TCall.h"
#include "TConnID.h"

TDeviceState*  g_pDeviceState = NULL;

/////////////////////////////////////////////////////////////////////////////
// TDeviceState code
TDeviceState::TDeviceState( Smt_String name, 
						  Smt_Uint loglevel, Smt_String logpath, 
						  Smt_Server* pserver )
: Smt_StateService( name, loglevel, logpath, pserver )
{
	m_ConnectionStateGOR = 0;
	m_CallStateGOR = 0;
	m_MeetmeIndex = DEFAULT_MEETME_NUM;
	m_MonitorIndex = 0;
	m_RouteIndex = 0;
	m_LinkState = 0;
	m_LinkReason = 0;
}

TDeviceState::~TDeviceState()
{
	// release device
	TDevice* pDevice = NULL;
	TDeviceMap::ITERATOR iter1(m_DeviceMgr);
	for (TDeviceMap::ENTRY *entry1 = 0;
		iter1.next (entry1) != 0;
		iter1.advance ())
	{
		pDevice = entry1->int_id_;		
		if(pDevice != NULL)
		{	
			delete pDevice;
			pDevice = NULL;
		}
	}	
	
	m_DeviceMgr.RemoveAll();

	// release routeid
	TRouteID* pRouteID = NULL;
	TRouteIDMap::ITERATOR iter2(m_RouteIDMgr);
	for (TRouteIDMap::ENTRY *entry2 = 0;
	iter2.next (entry2) != 0;
	iter2.advance ())
	{
		pRouteID = entry2->int_id_;		
		if(pRouteID != NULL)
		{	
			delete pRouteID;
			pRouteID = NULL;
		}
	}	
	
	m_RouteIDMgr.RemoveAll();
}

Smt_Uint TDeviceState::OnUserOnline(Smt_Uint sender, Smt_String sendername)
{
	PrintLog(4,"[TDeviceState::OnUserOnline ] sender<0x%x>, sendername<%s>.", sender, sendername.c_str());
	
	if( Smt_String(CMU_CONNECTIONSTATE) == sendername )
	{
		m_ConnectionStateGOR = sender;
	}

	if( Smt_String(CMU_CALLSTATENAME) == sendername )
	{
		m_CallStateGOR = sender;
	}

	return Smt_Success;
}

Smt_Uint TDeviceState::OnUserOffline(Smt_Uint sender, Smt_String sendername)
{
	PrintLog(4,"[TDeviceState::OnUserOffline] sender<0x%x>, sendername<%s>.", sender, sendername.c_str());
	
	if( Smt_String(CMU_CONNECTIONSTATE) == sendername )
	{
		m_ConnectionStateGOR = 0;
	}

	if( Smt_String(CMU_CALLSTATENAME) == sendername )
	{
		m_CallStateGOR = 0;
	}

	// remove monitor info
	TDevice* pDevice = NULL;
	for( TDeviceMap::ITERATOR 
		iter = m_DeviceMgr.begin();
		iter != m_DeviceMgr.end(); 
		iter++ )
	{
		pDevice = (*iter).int_id_;
		pDevice->RemoveMonitorByGOR( sender );
	}

	return Smt_Success;
}

Smt_Uint TDeviceState::OnTimer(Smt_Uint& timerid, Smt_Uint& messageid, Smt_Uint& senderobj)
{
	switch (messageid)
	{
	case Evt_ICMU_DeviceTimer:
		OnEvtDeviceTimer( senderobj );
		break;
	}
	return Smt_Success;
}

Smt_Uint TDeviceState::InitStates()
{
	m_ShiftTable		
	+ new Smt_ScriptState(DST_IDLE, "DST_IDLE") 
		+ new Smt_ScriptRule(Evt_ICMU_Initiated,-1,DST_OFFHOOK, NULL )
		+ new Smt_ScriptRule(Evt_ICMU_Released,-1,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )//add by caoyj 20111101

	+ new Smt_ScriptState(DST_OFFHOOK, "DST_OFFHOOK") 
		+ new Smt_ScriptRule(Evt_ICMU_Originated,CauseNotKnown,DST_OFFHOOK, NULL )
		+ new Smt_ScriptRule(Evt_ICMU_Originated,CauseOriginated,DST_OFFHOOK, (ST_ACTION)&TDevice::EvtOffHook )
		+ new Smt_ScriptRule(Evt_ICMU_Alerting,CauseDestSeized,DST_DESTSEIZED, (ST_ACTION)&TDevice::EvtDestSeized )
		+ new Smt_ScriptRule(Evt_ICMU_Alerting,CauseInboundCall,DST_INBOUNDCALL, (ST_ACTION)&TDevice::EvtInboundCall )
		+ new Smt_ScriptRule(Evt_ICMU_ChannelDataReached,CauseOpConsulted,DST_OFFHOOK, (ST_ACTION)&TDevice::SetTimer )
		+ new Smt_ScriptRule(Evt_ICMU_DeviceTimerExpired,CauseOpConsulted,DST_CONSULT_DESTSEIZED, (ST_ACTION)&TDevice::EvtConsultDestSeized )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNotKnown,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseDestBusy,DST_IDLE, (ST_ACTION)&TDevice::EvtDestBusy_RetrieveCall )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseDestInvalid,DST_IDLE, (ST_ACTION)&TDevice::EvtDestInvalid_RetrieveCall )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseListening,DST_ANSWERED, (ST_ACTION)&TDevice::EvtTp_OpConferenced )
		+ new Smt_ScriptRule(Evt_ICMU_Queued,CauseNotKnown,DST_QUEUED, (ST_ACTION)&TDevice::EvtQueued_RouteRequest )
		+ new Smt_ScriptRule(Evt_ICMU_ChannelDataReached,CauseSingleStepTransferred,DST_OFFHOOK, (ST_ACTION)&TDevice::SetTimer )
		+ new Smt_ScriptRule(Evt_ICMU_DeviceTimerExpired,CauseSingleStepTransferred,DST_INBOUNDCALL, (ST_ACTION)&TDevice::EvtTransferDestSeized )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpAnswered,DST_ANSWERED, (ST_ACTION)&TDevice::EvtOpAnswered )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseCallRejected,DST_IDLE, (ST_ACTION)&TDevice::EvtDestInvalid_RetrieveCall )

	+ new Smt_ScriptState(DST_DESTSEIZED, "DST_DESTSEIZED") 
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpAnswered,DST_ANSWERED, (ST_ACTION)&TDevice::EvtOpAnswered )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseNotKnown,DST_ANSWERED, (ST_ACTION)&TDevice::EvtOpAnswered )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseDestBusy,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNoAnswer,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseCallRejected,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Alerting,CauseDestSeized,DST_DESTSEIZED, (ST_ACTION)&TDevice::EvtDestSeized )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNotKnown,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseTpDisconnected,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )//add by caoyj 20111124
		

	+ new Smt_ScriptState(DST_INBOUNDCALL, "DST_INBOUNDCALL") 
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseTpAnswered,DST_ANSWERED, (ST_ACTION)&TDevice::EvtTpAnswered )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseConsultTpAnswered,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtTpAnswered )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseCallRejected,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected_RetrieveCall )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNoAnswer,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected_RetrieveCall )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseDestBusy,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected_RetrieveCall )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseTpDisconnected,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseSingleStepTransferred,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )

	+ new Smt_ScriptState(DST_ANSWERED, "DST_ANSWERED") 
		+ new Smt_ScriptRule(Evt_ICMU_Disconnected,-1,DST_ANSWERED, NULL )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNotKnown,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseTpDisconnected,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected_RetrieveCall )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseTpHeld,DST_MEETCONNECTED, (ST_ACTION)&TDevice::ActHoldOtherParty )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseNotKnown,DST_MEETCONNECTED, NULL )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpTransferred,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtOpTransferred )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpConferenced,DST_CONFERENCED, (ST_ACTION)&TDevice::EvtOpConferenced )
		+ new Smt_ScriptRule(Evt_ICMU_Held,-1,DST_BEHELD, (ST_ACTION)&TDevice::EvtHeld )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseSingleStepTransferred,DST_IDLE, (ST_ACTION)&TDevice::EvtTp_OpTransferred )
		+ new Smt_ScriptRule(Evt_ICMU_Alerting,CauseDestSeized,DST_DESTSEIZED, (ST_ACTION)&TDevice::EvtDestSeized )
		+ new Smt_ScriptRule(Evt_ICMU_Queued,CauseNotKnown,DST_QUEUED, (ST_ACTION)&TDevice::EvtQueued_RouteRequest )

	+ new Smt_ScriptState(DST_MEETCONNECTED, "DST_MEETCONNECTED") 
		+ new Smt_ScriptRule(Evt_ICMU_Disconnected,-1,DST_MEETCONNECTED, NULL )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNotKnown,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseTpDisconnected,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected_RetrieveCall )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseNotKnown,DST_MEETCONNECTED, NULL )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseTpHeld,DST_MEETCONNECTED, (ST_ACTION)&TDevice::ActHoldOtherParty )
		+ new Smt_ScriptRule(Evt_ICMU_Held,-1,DST_BEHELD, (ST_ACTION)&TDevice::EvtHeld )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseTpRetrieved,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtTpRetrieved )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpTransferred,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtOpTransferred )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseSingleStepTransferred,DST_IDLE, (ST_ACTION)&TDevice::EvtTp_OpTransferred )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpConferenced,DST_CONFERENCED, (ST_ACTION)&TDevice::EvtOpConferenced )
		//add by caoyj 20110921 HMP时 坐席保持取保持后转IVR Queue设备，来这个事件
		+ new Smt_ScriptRule(Evt_ICMU_Queued,CauseNotKnown,DST_QUEUED, (ST_ACTION)&TDevice::EvtQueued_RouteRequest )
		//add by caoyj 20120328 
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseOpDisconnected,DST_IDLE, (ST_ACTION)&TDevice::EvtTp_OpDisconnected )

	+ new Smt_ScriptState(DST_DISCONNECTED, "DST_DISCONNECTED")  
		// no roule
	+ new Smt_ScriptState(DST_HELD, "DST_HELD") 
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseTpRetrieved,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtTpRetrieved )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseTpDisconnected,DST_IDLE, (ST_ACTION)&TDevice::EvtTp_OpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNotKnown,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Failed,CauseNotKnown,DST_HELD, (ST_ACTION)&TDevice::EvtDestFail_RetrieveCall )

	+ new Smt_ScriptState(DST_BEHELD, "DST_BEHELD") 
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseMeetme,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtOpRetrieved )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpAnswered,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtOpRetrieved )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseNotKnown,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtOpRetrieved )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpTransferred,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtOpTransferred )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpConferenced,DST_CONFERENCED, (ST_ACTION)&TDevice::EvtOpConferenced )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseTpDisconnected,DST_IDLE, (ST_ACTION)&TDevice::EvtTp_OpDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseNotKnown,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected )

	+ new Smt_ScriptState(DST_CONSULT_DESTSEIZED, "DST_CONSULT_DESTSEIZED") 
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseConsultOpAnswered,DST_CONSULT_ANSWERED, (ST_ACTION)&TDevice::EvtOpAnswered )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseConsultTpAnswered,DST_CONSULT_ANSWERED, (ST_ACTION)&TDevice::EvtTpAnswered )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseTpRetrieved,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtTpRetrieved )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpAnswered,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtOpAnswered )

	+ new Smt_ScriptState(DST_CONSULT_ANSWERED, "DST_CONSULT_ANSWERED") 
		+ new Smt_ScriptRule(Evt_ICMU_Disconnected,-1,DST_CONSULT_ANSWERED, NULL )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseTpDisconnected,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected_RetrieveCall )
		+ new Smt_ScriptRule(Evt_ICMU_Released,CauseTpTransferred,DST_IDLE, (ST_ACTION)&TDevice::EvtTpTransferred )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseOpAnswered,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtTpRetrieved )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseTpConferenced,DST_CONFERENCED, (ST_ACTION)&TDevice::EvtTpConferenced )
		+ new Smt_ScriptRule(Evt_ICMU_Connected,CauseTpRetrieved,DST_MEETCONNECTED, (ST_ACTION)&TDevice::EvtTpRetrieved )

	+ new Smt_ScriptState(DST_CONFERENCED, "DST_CONFERENCED") 
		+ new Smt_ScriptRule(Evt_ICMU_Disconnected,-1,DST_CONFERENCED, NULL )
		+ new Smt_ScriptRule(Evt_ICMU_Released,-1,DST_IDLE, (ST_ACTION)&TDevice::EvtConfTpDisconnected )
	
	+ new Smt_ScriptState(DST_QUEUED, "DST_QUEUED") 
		+ new Smt_ScriptRule(Evt_ICMU_Alerting,CauseDestSeized,DST_DESTSEIZED, (ST_ACTION)&TDevice::EvtDestSeized_RouteEnd )
		+ new Smt_ScriptRule(Evt_ICMU_Released,-1,DST_IDLE, (ST_ACTION)&TDevice::EvtTpDisconnected_RouteEnd )

	+ new Smt_ScriptState(DST_FAILED, "DST_FAILED") 
		//+ new Smt_ScriptRule(Evt_ICMU_Originated,-1,CALL_INITIATED, (ST_ACTION)&TCall::EvtCallInitiated )
	;
	return Smt_Success;
}

Smt_StateObject* TDeviceState::PrehandleMessage(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	switch ( pdu.m_MessageID )
	{
	// Cmd/Resp
	case Cmd_ICMU_Assign:
		OnCmdAssign( pdu );
		break;
	case Resp_ICMU_Assign:
		OnRespAssign( pdu );
		break;
	case Cmd_ICMU_Deassign:
		OnCmdDeassign( pdu );
		break;
	case Cmd_ICMU_MakeCall:
		OnCmdMakeCall( pdu );
		break;
	case Cmd_ICMU_HangupCall:
		OnCmdHangupCall( pdu );
		break;
	case Cmd_ICMU_HoldCall:
		OnCmdHoldCall( pdu );
		break;
	case Cmd_ICMU_RetrieveCall:	
		OnCmdRetrieveCall( pdu );
		break;
	case Cmd_ICMU_ConsultationCall:
		OnCmdConsultationCall( pdu );
		break;
	case Cmd_ICMU_TransferCall:
		OnCmdTransferCall( pdu );
		break;	
	case Cmd_ICMU_ConferenceCall:
		OnCmdConferenceCall( pdu );
		break;
	case Cmd_ICMU_ReconnectCall:
		OnCmdReconnectCall( pdu );
		break;	
	case Cmd_ICMU_SingleStepTransfer:
		OnCmdSingleStepTransfer( pdu );
		break;
	case Cmd_ICMU_SingleStepConference:
		OnCmdSingleStepConference( pdu );
		break;
/*	case Cmd_ICMU_DeflectCall:
		OnCmdDeflectCall( pdu );
		break;
	case Cmd_ICMU_PickupCall:
		OnCmdPickupCall( pdu );
		break;
*/

	case Cmd_ICMU_RouteSelected:
		OnCmdRouteSelected( pdu );
		break;
	case Cmd_ICMU_SendDTMF:
		OnCmdSendDTMF( pdu );
		break;
	case Cmd_ICMU_StartRecord:
	case Cmd_ICMU_StopRecord:
		OnCmdRecord( pdu );
		break;
	case Cmd_ICMU_DeviceSnapshot:
		OnCmdDeviceSnapshot(pdu);
		break;
	case Cmd_ICMU_SendMessage:
		OnCmdSendMessage(pdu);
		break;
	case Cmd_ICMU_SetData:
	case Cmd_ICMU_GetData:
	case Cmd_ICMU_ClearCall:
		OnCmd_To_CallState( pdu );
		break;
	
	// Connection Event
	case Evt_ICMU_Initiated:
		pDevice = OnEvtInitiated(pdu);
		break;
	case Evt_ICMU_Originated:
		pDevice = OnEvtOriginated(pdu);
		break;
	case Evt_ICMU_Alerting:
		pDevice = OnEvtAlerting(pdu);
		break;
	case Evt_ICMU_Connected:
		pDevice = OnEvtConnected(pdu);
		break;
	case Evt_ICMU_Disconnected:
		pDevice = OnEvtDisconnected(pdu);
		break;
	case Evt_ICMU_Released:
		pDevice = OnEvtReleased(pdu);
		break;				
	case Evt_ICMU_Held:
		pDevice = OnEvtHeld(pdu);
		break;
	case Evt_ICMU_Queued:
		pDevice = OnEvtQueued(pdu);
		break;
	case Evt_ICMU_Failed:
		pDevice = OnEvtFailed(pdu);
		break;
	case Evt_ICMU_ChannelDataReached:
		pDevice = OnEvtChannelDataReached(pdu);
		break;
	case Evt_ICMU_DeviceTimerExpired:
		pDevice = OnEvtDeviceTimerExpired(pdu);
		break;

	case Evt_ICMU_DeviceRecording:
	case Evt_ICMU_DeviceRecordEnd:
	case Evt_ICMU_BackInService:
	case Evt_ICMU_OutOfService:
		OnEvtOtherDeviceEvent(pdu);
		break;	

	case Evt_ICMU_MessageReceived:
		OnEvtMessageReceived(pdu);
		break;

	case Evt_ICMU_LinkUp:
	case Evt_ICMU_LinkDown:
		OnEvtLinkEvent(pdu);
		break;

	case Cmd_ICMU_QueryLinkState:
		OnCmdQueryLinkState(pdu);
		break;

	default:
		PrintLog(3, "[TDeviceState::PrehandleMessage] Receive Unknown Message, PduInf %s .",
			pdu.GetInfo().c_str() );
		break;
	}

	return pDevice;
}

/****************************************************************************
函 数 名: OnEvtInitiated
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TDevice* TDeviceState::OnEvtInitiated(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceType;
	Smt_String strConnectionID;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_DeviceType, &nDeviceType );
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );

	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		pDevice = new TDevice(strDeviceID, nDeviceType, DST_IDLE, this );
		m_DeviceMgr.SetAt(strDeviceID, pDevice );
	}

	pDevice->m_DeviceType = nDeviceType;  
	pDevice->m_ConnID = strConnectionID;
	pDevice->SetCallData( pdu );	

	PrintLog(5,"[TDeviceState::OnEvtInitiated] DeviceID<%s>, DeviceType<%d>, ConnectionID<%s>.", 
		strDeviceID.c_str(), nDeviceType, strConnectionID.c_str() );
	
	return pDevice;
}

/****************************************************************************
函 数 名: OnEvtOriginated
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TDevice* TDeviceState::OnEvtOriginated(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	Smt_String strSource;
	Smt_String strDestination;
	Smt_String strCallerID;
	Smt_Uint   nReason;
	
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetString( Key_ICMU_Destination, &strDestination );
	pdu.GetString( Key_ICMU_CallerID, &strCallerID );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );

	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtOriginated] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}

	pDevice->m_CallerID = strCallerID;
	pDevice->SetCallData( pdu );

	do 
	{
		if( nReason == CauseListening)
		{
			// do nothing
			break;
		}

		if( strSource == "" && strDeviceID == strCallerID )
		//if( strSource == "" && strDestination == "" &&
		//	pDevice->m_LastCommand.m_MessageID == Cmd_ICMU_MakeCall ) // 可能有问题
		{
			//pDevice->m_LastCommand.Clear();

			pdu.m_Status = CauseOriginated;
			break;
		}

		if( strDeviceID == strSource )
		{
			pdu.m_Status = CauseOriginated;
			break;
		}
	} while (0);

	PrintLog(5,"[TDeviceState::OnEvtOriginated] DeviceID<%s>.", 
		strDeviceID.c_str() );

	return pDevice;
}

/****************************************************************************
函 数 名: OnEvtAlerting
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TDevice* TDeviceState::OnEvtAlerting(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	Smt_String strSource;
	Smt_String strDestination;
	Smt_String strOtherParty;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetString( Key_ICMU_Destination, &strDestination );

	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtAlerting] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}
	
	pDevice->SetCallData( pdu );
	strOtherParty = LookupOtherPartyByCallID(pDevice->m_DeviceID, pDevice->m_CallID );
	if( strOtherParty != "")
		pDevice->m_OtherParty = strOtherParty;

	if( strDeviceID == strSource )
	{
		pdu.m_Status = CauseDestSeized;
	}
	else
	{
		pdu.m_Status = CauseInboundCall;
	}

	// dial 时修正 otherparty
	if( strOtherParty == "" )
	{
		if( pdu.m_Status == CauseDestSeized )
		{
			pDevice->m_OtherParty = strDestination;
		} 
		else // pdu.m_Status == CauseInboundCall
		{
			pDevice->m_OtherParty = strSource;
		}
	}
/*
//////////////////////////////////////////////////////////////
//add bu caoyj 20110917 解决保持取回后坐席转ivr后，MeetmeNum清空，否则ivr再转回到坐席时会有问题
//注：	太啰嗦，放到TDeviceState::OnEvtConnected里面情况
	// 设备和IVR通话， MeetmeNum 赋值为空
	TDevice* pOtherDevice = NULL;
	if(m_DeviceMgr.Lookup(pDevice->m_OtherParty, pOtherDevice) == Smt_Success )
	{
		if( pOtherDevice->m_DeviceType == DTYPE_IVR_INBOUND || pOtherDevice->m_DeviceType == DTYPE_ROUTE)
		{
			pDevice->m_MeetmeNum = "";
		}
	}
/////////////////////////////////////////////////////////////
*/
	PrintLog(5,"[TDeviceState::OnEvtAlerting] DeviceID<%s>.", 
		strDeviceID.c_str() );
	
	return pDevice;
}

/****************************************************************************
函 数 名: OnEvtConnected
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TDevice* TDeviceState::OnEvtConnected(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	Smt_String strSource;
	Smt_String strDestination;
	Smt_Uint   nReason;
	Smt_String strMeetmeNum;
	Smt_String strConsultingParty;
	Smt_String strTransferringParty;
	Smt_String strConferencingParty;
	Smt_String strOtherConnectionID;
	Smt_String strOtherParty;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetString( Key_ICMU_Destination, &strDestination );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	pdu.GetString( Key_ICMU_MeetmeNum, &strMeetmeNum );
	pdu.GetString( Key_ICMU_ConsultingParty, &strConsultingParty );
	pdu.GetString( Key_ICMU_TransferringParty, &strTransferringParty );
	pdu.GetString( Key_ICMU_ConferencingParty, &strConferencingParty );
	pdu.GetString( Key_ICMU_OtherConnectionID, &strOtherConnectionID );

	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtConnected] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}
	
	pDevice->SetCallData( pdu );
	strOtherParty = LookupOtherPartyByCallID(pDevice->m_DeviceID, pDevice->m_CallID );
	if( strOtherParty != "" )
		pDevice->m_OtherParty = strOtherParty;

	do 
	{
		if( nReason == CauseMeetme || nReason == CauseBeMeetme )
		{
			//pDevice->m_MeetmeNum = strMeetmeNum;
			//add by caoyj 20111124  解决呼入通话咨询转时，为"h"导致后续错误的处理
			if(strMeetmeNum!="h" && strMeetmeNum!="")
				pDevice->m_MeetmeNum = strMeetmeNum;

			if(strConsultingParty != "")        // 咨询通话
			{	
				if(strDeviceID == strConsultingParty )
				{
					pdu.m_Status = CauseConsultOpAnswered; // CauseOpAnswered
				}	
				else
				{
					pdu.m_Status = CauseConsultTpAnswered; // CauseTpAnswered
				}			
				break;
			}
			
			if( strTransferringParty != "" )    // 话路转移
			{
				pdu.m_Status = CauseOpTransferred;
				break;
			}

			if( strConferencingParty != "" )    // 话路会议
			{
				if( strDeviceID == strConferencingParty )
				{
					pdu.m_Status = CauseTpConferenced;
				}
				else
				{
					pdu.m_Status = CauseOpConferenced;
				}				
				break;
			}

			if( pDevice->GetState() == DST_CONSULT_DESTSEIZED ) // 咨询振铃中，保持方挂机后，被咨询方应答
			{
				pdu.m_Status = CauseOpAnswered;
				break;
			}
			
			if( pDevice->GetState() == DST_INBOUNDCALL )       
			{			
				pdu.m_Status = CauseTpAnswered;
				break;				
			}

			if( pDevice->m_LastCommand.m_MessageID == Cmd_ICMU_HoldCall ||
				pDevice->m_LastCommand.m_MessageID == Cmd_ICMU_ConsultationCall )
			{
				pDevice->m_LastCommand.m_MessageID = 0;
				pdu.m_Status = CauseTpHeld;
				break;
			}
			else if( pDevice->m_LastCommand.m_MessageID == Cmd_ICMU_RetrieveCall ||
				     pDevice->m_LastCommand.m_MessageID == Cmd_ICMU_ReconnectCall )
			{
				pDevice->m_LastCommand.m_MessageID = 0;
				pdu.m_Status = CauseTpRetrieved;
				break;
			}
			else
			{
				pdu.m_Status = CauseNotKnown;
				break;
			}
			break;  // 无论什么条件都跳出
		}

		if(nReason == CauseListening )
		{
			Smt_String strOtherDeviceID = LookupDeviceIDByConnID(strOtherConnectionID);
			pDevice->m_OtherParty = strOtherDeviceID;
			pdu.m_Status = CauseListening;
			break;
		}

		if( strDeviceID == strSource )
		{
			pdu.m_Status = CauseOpAnswered;
		}
		else
		{
			pdu.m_Status = CauseTpAnswered;
		}

		//add bu caoyj 20110917 解决保持取回后坐席转ivr后，MeetmeNum清空，否则ivr再转回到坐席时会有问题
		pDevice->m_MeetmeNum = ""; 

	} while (0);
	
	PrintLog(5,"[TDeviceState::OnEvtConnected] DeviceID<%s>,nReason<%d>,pduStatus<%d>.", 
		strDeviceID.c_str(),nReason,pdu.m_Status );
	
	return pDevice;
}

/****************************************************************************
函 数 名: OnEvtDisconnected
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TDevice* TDeviceState::OnEvtDisconnected(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	TDevice* pOtherDevice = NULL;
	Smt_String strDeviceID;
	Smt_Uint   nReason;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	
	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtDisconnected] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}

	pDevice->SetCallData( pdu );
	
	PrintLog(5,"[TDeviceState::OnEvtDisconnected] DeviceID<%s>.", 
		strDeviceID.c_str() );
	
	return pDevice;
}

/****************************************************************************
函 数 名: OnEvtReleased
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TDevice* TDeviceState::OnEvtReleased(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	TDevice* pOtherDevice = NULL;
	Smt_String strDeviceID;
	Smt_String strTransferringParty;
	Smt_Uint   nReason;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_TransferringParty, &strTransferringParty );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );

	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtReleased] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}

	pDevice->SetCallData( pdu );
	
	do 
	{
		if( strTransferringParty != "" && pDevice->m_DeviceID == strTransferringParty )
		{
			pdu.m_Status = CauseTpTransferred;
			break;
		}

		if( nReason == CauseDestBusy )
		{
			pdu.m_Status = CauseDestBusy;
			break;
		}

		if( nReason == CauseDestInvalid )
		{
			pdu.m_Status = CauseDestInvalid;
			break;
		}

		if( nReason == CauseNoAnswer )
		{
			pdu.m_Status = CauseNoAnswer;
			break;
		}

		if( nReason == CauseCallRejected )
		{
			pdu.m_Status = CauseCallRejected;
			break;
		}

		if( nReason == CauseSingleStepTransferred ||
			nReason == CauseSingleStepTransferredMeetme )
		{
			pdu.m_Status = CauseSingleStepTransferred;
			break;
		}

		// IVR 外拨时，发生话路转接，需要修正被转接方的 OtherParty
		if(pDevice->m_DeviceType == DTYPE_IVR_OUTBOUND)
		{
			Smt_String strTmpParty;
			TDevice* pTmpDevice = NULL;
			if( m_DeviceMgr.Lookup(pDevice->m_OtherParty, pOtherDevice) == Smt_Success )
			{
				strTmpParty = LookupOtherPartyByCallID(pDevice->m_OtherParty, pDevice->m_CallID);
				pOtherDevice->m_OtherParty = strTmpParty;
				
				if(m_DeviceMgr.Lookup(strTmpParty, pTmpDevice) == Smt_Success)
				{
					pTmpDevice->m_OtherParty = pOtherDevice->m_DeviceID;

					PrintLog(5,"[TDeviceState::OnEvtReleased] DeviceID<%s>, OtherParty<%s>, Relation-OtherParty<%s> .", 
						pDevice->m_DeviceID.c_str(), pOtherDevice->m_DeviceID.c_str(), pTmpDevice->m_DeviceID.c_str() );
				}
			}

			pdu.m_Status = CauseTpDisconnected;
			break;
		}

		if( m_DeviceMgr.Lookup(pDevice->m_OtherParty, pOtherDevice) == Smt_Success )
		{
			if( pOtherDevice->GetState() == DST_CONSULT_DESTSEIZED )
			{
				if( pDevice->GetState() == DST_BEHELD )//原始话路为内线时，咨询振铃时原始内线挂机，内线状态为DST_BEHELD收到EvtReleased
				{
					pdu.m_Status = CauseTpDisconnected;
					break;
				}
				//add by caoyj 20120328,原始话路为外线时，咨询振铃时外线挂机，外中继状态会先从DST_BEHELD改变到DST_MEETCONNECTED后收到EvtReleased
				else if( pDevice->GetState() == DST_MEETCONNECTED)
				{
					pdu.m_Status = CauseOpDisconnected;
					break;
				}
				//////////////////////////////////////////////////////////////////////////////////
				else
				{
					pdu.m_Status = CauseNoAnswer;
					break;
				}		
			}			

			if( pOtherDevice->GetState() > DST_IDLE)
			{
				pdu.m_Status = CauseTpDisconnected;
				break;
			}
		}
	} while (0);

	PrintLog(5,"[TDeviceState::OnEvtReleased] DeviceID<%s>.", 
		strDeviceID.c_str() );
	
	return pDevice;
}

/****************************************************************************
函 数 名: OnEvtHeld
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TDevice* TDeviceState::OnEvtHeld(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	
	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtHeld] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}

	pDevice->SetCallData( pdu );
		
	PrintLog(5,"[TDeviceState::OnEvtHeld] DeviceID<%s>.", 
		strDeviceID.c_str() );
	
	return pDevice;
}

/****************************************************************************
函 数 名: OnEvtQueued
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TDevice* TDeviceState::OnEvtQueued(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	Smt_String strSource;
	Smt_String strDestination;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetString( Key_ICMU_Destination, &strDestination );

	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtQueued] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}

	pDevice->SetCallData( pdu );
	pDevice->m_OtherParty = strDestination;
	
	PrintLog(5,"[TDeviceState::OnEvtQueued] DeviceID<%s>.", 
		strDeviceID.c_str() );
	
	return pDevice;
}

/****************************************************************************
函 数 名: OnEvtFailed
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TDevice* TDeviceState::OnEvtFailed(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	
	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtFailed] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}

	pDevice->SetCallData( pdu );
		
	PrintLog(5,"[TDeviceState::OnEvtFailed] DeviceID<%s>.", 
		strDeviceID.c_str() );
	
	return pDevice;
}

/****************************************************************************
函 数 名: AllocateDeviceRefID
参    数:
返回数值: 
功能描述: 生成新的监视ID
*****************************************************************************/
Smt_Uint TDeviceState::AllocateDeviceRefID()
{
	Smt_SingleLock sLock(m_Lock);
	
	m_MonitorIndex++;
	if( m_MonitorIndex > 0x7FFFFFFF )
	{
		m_MonitorIndex = 1;
	}
	
	return m_MonitorIndex;
}

/****************************************************************************
函 数 名: AllocateMeetmeNum
参    数:
返回数值: 
功能描述: 可能会重复分配？？？
*****************************************************************************/
Smt_String TDeviceState::AllocateMeetmeNum()
{
	Smt_SingleLock sLock(m_Lock);

	m_MeetmeIndex++;
	if( m_MeetmeIndex >= DEFAULT_MAXMEETME_NUM )
	{
		m_MeetmeIndex = DEFAULT_MEETME_NUM;
	}
	
	return HLFormatStr("%d", m_MeetmeIndex );
}

/****************************************************************************
函 数 名: AllocateRouteID
参    数:
返回数值: 
功能描述: 生成新的RouteID
*****************************************************************************/
Smt_Uint TDeviceState::AllocateRouteID()
{
	Smt_SingleLock sLock(m_Lock);
	
	m_RouteIndex++;
	if( m_RouteIndex > 0xFFFF )
	{
		m_RouteIndex = 1;
	}
	
	return m_RouteIndex;
}

/****************************************************************************
函 数 名: LookupDeviceByDeviceRefID
参    数: 
返回数值: TDevice*
功能描述: 根据监视ID查找设备对象.
*****************************************************************************/
TDevice* TDeviceState::LookupDeviceByDeviceRefID( Smt_Uint devicerefid )
{	
	TDevice* pDevice = NULL;
	for( TDeviceMap::ITERATOR 
		iter = m_DeviceMgr.begin();
		iter != m_DeviceMgr.end(); 
		iter++ )
	{
		pDevice = (*iter).int_id_;
		if( pDevice->IsMonitored(devicerefid) == Smt_BoolTRUE )
		{
			return pDevice;
		}
	}
	
	return NULL;
}

/****************************************************************************
函 数 名: LookupDeviceByCallID
参    数: 
返回数值: TDevice*
功能描述: 根据 CallID 查找设备对象,并排除 otherdevice
*****************************************************************************/
TDevice* TDeviceState::LookupDeviceByCallID( Smt_Uint callid, Smt_String otherdeviceid )
{
	TDevice* pDevice = NULL;
	for( TDeviceMap::ITERATOR 
		iter = m_DeviceMgr.begin();
		iter != m_DeviceMgr.end(); 
		iter++ )
	{
		pDevice = (*iter).int_id_;
		if( (pDevice->m_CallID == callid) && 
			(pDevice->m_DeviceID != otherdeviceid) )
		{
			return pDevice;
		}
	}
	
	return NULL;
}

TRouteID* TDeviceState::LookupRouteIDByCallID(Smt_Uint callid)
{
	TRouteID* pRouteID = NULL;
	for( TRouteIDMap::ITERATOR 
		iter = m_RouteIDMgr.begin();
		iter != m_RouteIDMgr.end(); 
		iter++ )
	{
		pRouteID = (*iter).int_id_;
		if( pRouteID->m_CallID == callid)
		{
			return pRouteID;
		}
	}
	
	return NULL;
}

Smt_String TDeviceState::GetIDName(Smt_Uint msgid)
{
	return g_pConnState->GetIDName( msgid );
}

Smt_String TDeviceState::GetStateName(Smt_Uint stateid)
{
	switch( stateid )
	{
		CASE_STR(DST_IDLE              )
		CASE_STR(DST_OFFHOOK           )
		CASE_STR(DST_DESTSEIZED        )
		CASE_STR(DST_INBOUNDCALL       )
		CASE_STR(DST_ANSWERED          )
		CASE_STR(DST_HELD              )
		CASE_STR(DST_BEHELD            )
		CASE_STR(DST_CONSULT_DESTSEIZED)
		CASE_STR(DST_CONSULT_ANSWERED  )
		CASE_STR(DST_CONFERENCED       )
		CASE_STR(DST_QUEUED            )
		CASE_STR(DST_DISCONNECTED      )
		CASE_STR(DST_MEETCONNECTED     )
		default: 
		{
			Smt_String strTemp = HLFormatStr( "?(%#x)", stateid );	
			return strTemp;
		}
	}
}

/****************************************************************************
函 数 名: OnCmdAssign
参    数:
返回数值: 
功能描述: 打开设备监视
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdAssign(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceType;
	Smt_Uint   nDeviceRefID;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_DeviceType, &nDeviceType );

	if( m_DeviceMgr.Lookup(strDeviceID, pDevice) != Smt_Success )
	{
		pDevice = new TDevice(strDeviceID, nDeviceType, DST_IDLE, this );
		m_DeviceMgr.SetAt(strDeviceID, pDevice );
	}

	nDeviceRefID = pDevice->AddMonitor( pdu.m_Sender );
	pDevice->m_LastCommand = pdu;

	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_ConnectionStateGOR;
	pdu.m_MessageID = Cmd_ICMU_Assign;
	pdu.m_Status  = Smt_Success;	

	pdu.PutUint( Key_ICMU_DeviceRefID, nDeviceRefID );
	
	if( pdu.m_Receiver > 0 ) PostMessage( pdu );

	PrintLog(5, "[TDeviceState::OnCmdAssign] DeviceRefID<%d>,DeviceID<%s>,DeviceType<%d>.", 
		nDeviceRefID, strDeviceID.c_str(), nDeviceType );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnRespAssign
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnRespAssign( Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_String strDeviceID;
	Smt_Uint   nOriSender;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	
	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnRespAssign] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}

	nOriSender = pDevice->m_LastCommand.m_Sender;

	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = nOriSender;
	pdu.m_MessageID = Resp_ICMU_Assign;
	pdu.m_Status = Smt_Success;

	if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	
	PrintLog(5, "[TDeviceState::OnRespAssign] DeviceRefID<%d>, DeviceID<%s>.",
		nDeviceRefID, strDeviceID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdDeassign
参    数:
返回数值: 
功能描述: 取消设备监视
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdDeassign(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceRefID;

	pdu.GetUint( Key_ICMU_DeviceRefID, &nDeviceRefID );

	pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
	if( pDevice == NULL )
	{
		RespFailure( Resp_ICMU_Deassign, CauseInvChannelID, pdu );
		return Smt_Fail;
	}

	pDevice->RemoveMonitor( nDeviceRefID );

	RespCmd(Resp_ICMU_Deassign, pdu);

	PrintLog(5, "[TTDeviceState::OnCmdDeassign] DeviceRefID<%d>, DeviceID<%s>.",
		nDeviceRefID, pDevice->GetID().c_str() );

	return Smt_Success;
}

Smt_Uint TDeviceState::RespCmd(Smt_Uint evtid, Smt_Pdu pdu)
{
	Smt_Uint nOriSender;
	Smt_Uint nDeviceRefID;
	Smt_Uint nCallID;
		
	pdu.GetUint( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint( Key_ICMU_CallID, &nCallID );
	pdu.PutUint( Key_ICMU_Reason, CauseSuccess );

	nOriSender = pdu.m_Sender;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = nOriSender;
	pdu.m_MessageID = evtid;
	pdu.m_Status = Smt_Success;
	
	if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	
	PrintLog(5, "[TDeviceState::RespCmd] Resp<%s>, DeviceRefID<%d>, CallID<%d>.", 
		GetIDName(evtid).c_str(), nDeviceRefID, nCallID );
	
	return Smt_Success;
}

Smt_Uint TDeviceState::RespFailure(Smt_Uint evtid, Smt_Uint error, Smt_Pdu pdu)
{
	Smt_Uint nOriSender;
	Smt_Uint nDeviceRefID;
	Smt_Uint nCallID;

	pdu.GetUint( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint( Key_ICMU_CallID, &nCallID );
	pdu.PutUint( Key_ICMU_Reason, error );
	
	nOriSender = pdu.m_Sender;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = nOriSender;
	pdu.m_MessageID = evtid;
	pdu.m_Status = Smt_Success;
	
	if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	
	PrintLog(5, "[TDeviceState::RespFailure] RespFail<%s>, DeviceRefID<%d>, CallID<%d>, Reason<%d>.", 
		GetIDName(evtid).c_str(), nDeviceRefID, nCallID, error);
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdMakeCall
参    数:
返回数值: 
功能描述: 发起呼叫
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdMakeCall(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceType;
	Smt_String strCallerNum;
	Smt_String strCalledNum;
	
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetString( Key_ICMU_CallerNum, &strCallerNum );
	pdu.GetString( Key_ICMU_CalledNum, &strCalledNum );
	
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_MakeCall, CauseInvChannelID, pdu );
			break;
		}

		strDeviceID = pDevice->m_DeviceID;

		if( pDevice->GetState() != DST_IDLE )
		{
			RespFailure( Resp_ICMU_MakeCall, CauseInvDeviceState, pdu );
			break;
		}

		RespCmd(Resp_ICMU_MakeCall, pdu);		
		
		if( strCallerNum == "" )
		{
			strCallerNum = strDeviceID;
		}

		nDeviceType = pDevice->m_DeviceType;
		pDevice->m_LastCommand = pdu;

		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_MakeCall;

		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutUint  ( Key_ICMU_DeviceType, nDeviceType );
		pdu.PutString( Key_ICMU_CallerNum, strCallerNum );
		
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdMakeCall] DeviceRefID<%d>, DeviceID<%s>, DeviceType<%d>, CallerNum<%s>, CalledNum<%s>.",
		nDeviceRefID, strDeviceID.c_str(), nDeviceType, strCallerNum.c_str(), strCalledNum.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdHangupCall
参    数:
返回数值: 
功能描述: 挂断呼叫
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdHangupCall(Smt_Pdu& pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strConnID;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_HangupCall, CauseInvChannelID, pdu );
			break;
		}

		strDeviceID = pDevice->m_DeviceID;

		if( pDevice->m_CallID != nCallID )
		{
			RespFailure( Resp_ICMU_HangupCall, CauseInvCallID, pdu );
			break;
		}

		if( pDevice->GetState() == DST_IDLE )
		{
			RespFailure( Resp_ICMU_HangupCall, CauseInvDeviceState, pdu );
			break;
		}

		RespCmd(Resp_ICMU_HangupCall, pdu);
	
		strConnID = pDevice->m_ConnID;
		pDevice->m_LastCommand = pdu;

		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_HangupCall;
		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strConnID );
		
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdHangupCall] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdHoldCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdHoldCall(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strConnID;
	Smt_String strOtherConnID;
	Smt_String strMeetmeNum;
	Smt_String strOtherMeetmeNum;
	TDevice* pOtherDevice = NULL;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	
	Smt_Uint nReason = Smt_Success;
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_HoldCall, CauseInvChannelID, pdu );
			break;
		}

		strDeviceID = pDevice->m_DeviceID;
		
		if( pDevice->m_CallID != nCallID )
		{
			RespFailure( Resp_ICMU_HoldCall, CauseInvCallID, pdu );
			break;
		}

		if( (pDevice->GetState() != DST_ANSWERED) &&
			(pDevice->GetState() != DST_MEETCONNECTED) )
		{
			RespFailure( Resp_ICMU_HoldCall, CauseInvDeviceState, pdu );
			break;
		}

		RespCmd(Resp_ICMU_HoldCall, pdu);

		if( m_DeviceMgr.Lookup(pDevice->m_OtherParty, pOtherDevice) == Smt_Success )
		{
			strOtherConnID = pOtherDevice->m_ConnID;
			strOtherMeetmeNum = pOtherDevice->m_MeetmeNum;
		}		

		strConnID = pDevice->m_ConnID;
		pDevice->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_SingleStepTransfer;
		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_Context, AMI_CONTEXT_CUSTOM_MEETME );

		if( pDevice->m_MeetmeNum == "" && strOtherMeetmeNum == "" )
		{
			strMeetmeNum = AllocateMeetmeNum();
			pdu.PutString( Key_ICMU_ConnectionID, strConnID );
			pdu.PutString( Key_ICMU_OtherConnection, strOtherConnID );
			pdu.PutString( Key_ICMU_CalledNum, strMeetmeNum );
		}
		else // DST_MEETCONNECTED
		{
			pdu.PutString( Key_ICMU_ConnectionID, strOtherConnID );
			pdu.PutString( Key_ICMU_CalledNum, HLFormatStr("%d",DEFAULT_HOLDDEVICE_NUM) );
		}

		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	} while (0);

	PrintLog(5, "[TDeviceState::OnCmdHoldCall] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdRetrieveCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdRetrieveCall(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strConnID;
	
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_RetrieveCall, CauseInvChannelID, pdu );
			break;
		}
		
		strDeviceID = pDevice->m_DeviceID;

		if( pDevice->m_CallID != nCallID )
		{
			RespFailure( Resp_ICMU_RetrieveCall, CauseInvCallID, pdu );
			break;
		}

		if( pDevice->GetState() != DST_HELD )
		{
			RespFailure( Resp_ICMU_RetrieveCall, CauseInvDeviceState, pdu );
			break;
		}

		RespCmd(Resp_ICMU_RetrieveCall, pdu);

		Smt_String strMeetmeNum;
		Smt_String strOtherConnID;
		TDevice* pOtherDevice = NULL;
		if(m_DeviceMgr.Lookup(pDevice->m_OtherParty, pOtherDevice) == Smt_Success )
		{
			strOtherConnID = pOtherDevice->m_ConnID;
		}

		strMeetmeNum = pDevice->m_MeetmeNum;
		pDevice->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_SingleStepTransfer;
		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strOtherConnID );
		pdu.PutString( Key_ICMU_CalledNum, strMeetmeNum );
		pdu.PutString( Key_ICMU_Context, AMI_CONTEXT_CUSTOM_MEETME );

		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdRetrieveCall] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdConsultationCall
参    数:
返回数值: 
功能描述: 咨询先保持呼叫，再外呼一个分机, 在函数 TDevice::EvtHeld 处发起外呼
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdConsultationCall(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strConnID;
	Smt_String strCalledNum;
	Smt_String strCallerNum;
	TDevice*  pCalledDevice = NULL;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetString( Key_ICMU_CalledNum, &strCalledNum );
	pdu.GetString( Key_ICMU_CallerNum, &strCallerNum );

	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_ConsultationCall, CauseInvChannelID, pdu );
			break;
		}

		strDeviceID = pDevice->m_DeviceID;
		
		if( pDevice->m_CallID != nCallID )
		{
			RespFailure( Resp_ICMU_ConsultationCall, CauseInvCallID, pdu );
			break;
		}
	
		if( m_DeviceMgr.Lookup(strCalledNum, pCalledDevice) == Smt_Success )
		{
			if( pCalledDevice->GetState() != DST_IDLE && pCalledDevice->m_DeviceType == DTYPE_EXTENSION )
			{
				RespFailure( Resp_ICMU_ConsultationCall, CauseDestBusy, pdu );
				pDevice->SendDeviceEvent( Evt_ICMU_DestBusy, CauseDestBusy );
				break;
			}
		}

		if( strCallerNum == strCalledNum )
		{
			RespFailure( Resp_ICMU_ConsultationCall, CauseNotKnown, pdu );
			break;
		}

		RespCmd(Resp_ICMU_ConsultationCall, pdu);

		if( strCallerNum == "" )
		{
			strCallerNum = strDeviceID;
		}

		pdu.PutString( Key_ICMU_CalledNum, strCalledNum );
		pdu.PutString( Key_ICMU_CallerNum, strCallerNum );
		pdu.PutUint  ( Key_ICMU_LastCommand, Cmd_ICMU_ConsultationCall );
		pDevice->m_LastCommand = pdu;

		OnCmdHoldCall( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdConsultationCall] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>, CalledNum<%s>, CallerNum<%s>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID, strCalledNum.c_str(), strCallerNum.c_str() );

	return Smt_Success;
}
/****************************************************************************
函 数 名: OnCmdTransferCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdTransferCall(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_Uint   nOldCallID;
	Smt_String strDeviceID;
	Smt_String strConnID;
	
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetUint  ( Key_ICMU_OldCallID, &nOldCallID );
	
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_TransferCall, CauseInvChannelID, pdu );
			break;
		}
		
		strDeviceID = pDevice->m_DeviceID;

		if( pDevice->m_CallID != nCallID || 
			pDevice->m_OldCallID != nOldCallID )
		{
			RespFailure( Resp_ICMU_TransferCall, CauseInvCallID, pdu );
			break;
		}

		if( pDevice->GetState() != DST_CONSULT_ANSWERED )   // 仅支持咨询通话转
		{
			RespFailure( Resp_ICMU_TransferCall, CauseInvDeviceState, pdu );
			break;
		}

		RespCmd(Resp_ICMU_TransferCall, pdu);

		strConnID = pDevice->m_ConnID;
		pDevice->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_HangupCall;
		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strConnID );
		
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdTransferCall] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>, OldCallID<%d>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID, nOldCallID );

	return Smt_Success;
}
/****************************************************************************
函 数 名: OnCmdConferenceCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdConferenceCall(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_Uint   nOldCallID;
	Smt_String strDeviceID;
	Smt_String strHoldConnID;
	Smt_String strMeetmeNum;
	
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetUint  ( Key_ICMU_OldCallID, &nOldCallID );
	
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_ConferenceCall, CauseInvChannelID, pdu );
			break;
		}
		
		strDeviceID = pDevice->m_DeviceID;

		if( pDevice->m_CallID != nCallID || 
			pDevice->m_OldCallID != nOldCallID )
		{
			RespFailure( Resp_ICMU_ConferenceCall, CauseInvCallID, pdu );
			break;
		}

		if( pDevice->GetState() != DST_CONSULT_ANSWERED )   // 仅支持咨询通话会议
		{
			RespFailure( Resp_ICMU_ConferenceCall, CauseInvDeviceState, pdu );
			break;
		}

		RespCmd(Resp_ICMU_ConferenceCall, pdu);

		TDevice* pHoldDevice = LookupDeviceByCallID( nOldCallID, pDevice->m_DeviceID );
		if( pHoldDevice != NULL )
		{
			strHoldConnID = pHoldDevice->m_ConnID;
		}
		
		pDevice->m_LastCommand = pdu;
		strMeetmeNum = pDevice->m_MeetmeNum;

		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_SingleStepTransfer;
		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strHoldConnID );
		pdu.PutString( Key_ICMU_Context, AMI_CONTEXT_CUSTOM_MEETME );
		pdu.PutString( Key_ICMU_CalledNum, strMeetmeNum );

		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdConferenceCall] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>, OldCallID<%d>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID, nOldCallID );

	return Smt_Success;
}
/****************************************************************************
函 数 名: OnCmdReconnectCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdReconnectCall(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_Uint   nOldCallID;
	Smt_String strDeviceID;
	Smt_String strOtherConnID;
	
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetUint  ( Key_ICMU_OldCallID, &nOldCallID );
	
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_ReconnectCall, CauseInvChannelID, pdu );
			break;
		}
		
		strDeviceID = pDevice->m_DeviceID;

		if( pDevice->m_CallID != nCallID || 
			pDevice->m_OldCallID != nOldCallID )
		{
			RespFailure( Resp_ICMU_ReconnectCall, CauseInvCallID, pdu );
			break;
		}

		RespCmd(Resp_ICMU_ReconnectCall, pdu);

		TDevice* pOtherDevice = NULL;
		if(m_DeviceMgr.Lookup(pDevice->m_OtherParty, pOtherDevice) == Smt_Success )
		{
			strOtherConnID = pOtherDevice->m_ConnID;
		}

		pDevice->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_HangupCall;
		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strOtherConnID );
		
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdReconnectCall] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>, OldCallID<%d>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID, nOldCallID );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdSingleStepTransfer
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdSingleStepTransfer(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strConnID;
	Smt_String strCalledNum;
	Smt_String strOtherConnID;
	TDevice* pOtherDevice = NULL;
	TDevice* pCalledDevice = NULL;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetString( Key_ICMU_CalledNum, &strCalledNum );
	
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_SingleStepTransfer, CauseInvChannelID, pdu );
			break;
		}
		
		strDeviceID = pDevice->m_DeviceID;
		
		if( pDevice->m_CallID != nCallID )
		{
			RespFailure( Resp_ICMU_SingleStepTransfer, CauseInvCallID, pdu );
			break;
		}
		
		if( m_DeviceMgr.Lookup(strCalledNum, pCalledDevice) == Smt_Success )
		{
			if( pCalledDevice->GetState() != DST_IDLE && pCalledDevice->m_DeviceType == DTYPE_EXTENSION )
			{
				RespFailure( Resp_ICMU_SingleStepTransfer, CauseDestBusy, pdu );
				pDevice->SendDeviceEvent( Evt_ICMU_DestBusy, CauseDestBusy );
				break;
			}
		}

		RespCmd(Resp_ICMU_SingleStepTransfer, pdu);

		if( pDevice->m_MeetmeNum != "" )
		{
			pdu.PutUint  ( Key_ICMU_LastCommand, Cmd_ICMU_SingleStepTransfer );
			pdu.PutUint  ( Key_ICMU_Reason, CauseMeetme );

			strConnID = pDevice->m_ConnID;
			pDevice->m_LastCommand = pdu;
			
			pdu.m_Sender = GetGOR();
			pdu.m_Receiver = m_ConnectionStateGOR;
			pdu.m_MessageID = Cmd_ICMU_HangupCall;			
			pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
			pdu.PutString( Key_ICMU_ConnectionID, strConnID );			
			if( pdu.m_Receiver > 0 ) PostMessage( pdu );			
			break;
		}

		if( m_DeviceMgr.Lookup(pDevice->m_OtherParty, pOtherDevice) == Smt_Success )
		{
			strOtherConnID = pOtherDevice->m_ConnID;
		}		

		pdu.PutUint  ( Key_ICMU_LastCommand, Cmd_ICMU_SingleStepTransfer );
		pdu.PutUint  ( Key_ICMU_Reason, CauseNotKnown );

		pDevice->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_SingleStepTransfer;		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strOtherConnID );
		pdu.PutString( Key_ICMU_CalledNum, strCalledNum );	
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdSingleStepTransfer] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>, CalledNum<%s>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID, strCalledNum.c_str() );

	return Smt_Success;
}
/****************************************************************************
函 数 名: OnCmdSingleStepConference
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdSingleStepConference(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strCalledNum;
	Smt_String strCalledConnID;
	TDevice* pCalledDevice = NULL;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetString( Key_ICMU_CalledNum, &strCalledNum );

	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_SingleStepConference, CauseInvChannelID, pdu );
			break;
		}
		//add by caoyj 20111124
		//屏蔽同一个分机进行多次单步会议
		if(pDevice->GetState()!=DST_IDLE)
		{
			RespFailure( Resp_ICMU_SingleStepConference, CauseFail, pdu );
			break;
		}
		//////////////////////////////////////////////////////////////////////////


		strDeviceID = pDevice->m_DeviceID;
		
		if( m_DeviceMgr.Lookup(strCalledNum, pCalledDevice) != Smt_Success )
		{
			RespFailure( Resp_ICMU_SingleStepConference, CauseInvDeviceID, pdu );
			break;
		}	
	
		if( pCalledDevice->m_CallID != nCallID )
		{
			RespFailure( Resp_ICMU_SingleStepConference, CauseInvCallID, pdu );
			break;
		}

		RespCmd(Resp_ICMU_SingleStepConference, pdu);

		strCalledConnID = pCalledDevice->m_ConnID;
		pDevice->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_SingleStepConference;
		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strCalledConnID );
		
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdSingleStepConference] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>, CalledNum<%s>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID, strCalledNum.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdRouteSelected
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdRouteSelected(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_Uint   nRouteID;
	Smt_String strDeviceID;
	Smt_String strCalledNum;
	Smt_String strOtherConnID;
	TRouteID* pRouteID = NULL;
	TDevice* pCalledDevice = NULL;
	TDevice* pRouteDevice = NULL;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_RouteID, &nRouteID );
	pdu.GetString( Key_ICMU_CalledNum, &strCalledNum );
	
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_RouteSelected, CauseInvChannelID, pdu );
			break;
		}
		
		strDeviceID = pDevice->m_DeviceID;
		
		if( m_RouteIDMgr.Lookup( nRouteID, pRouteID) != Smt_Success )
		{
			RespFailure( Resp_ICMU_RouteSelected, CauseInvRouteID, pdu );
			break;
		}

		if( m_DeviceMgr.Lookup(strCalledNum, pCalledDevice) == Smt_Success )
		{
			if( pCalledDevice->GetState() != DST_IDLE && pCalledDevice->m_DeviceType == DTYPE_EXTENSION )
			{
				RespFailure( Resp_ICMU_RouteSelected, CauseDestBusy, pdu );

				if( m_DeviceMgr.Lookup(pRouteID->m_RouteDN, pRouteDevice) == Smt_Success )
				{
					pRouteDevice->EvtReRoute( pRouteID, CauseDestBusy );
				}
				break;
			}
		}

		RespCmd(Resp_ICMU_RouteSelected, pdu);

		nCallID = pRouteID->m_CallID;
		strOtherConnID = pRouteID->m_OtherConnID;
		pDevice->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_RouteSelected;

		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strOtherConnID );
		pdu.PutString( Key_ICMU_CalledNum, strCalledNum );
		
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdRouteSelected] DeviceRefID<%d>, DeviceID<%s>, RouteID<%d>, CallID<%d>, CalledNum<%s>.",
		nDeviceRefID, strDeviceID.c_str(), nRouteID, nCallID, strCalledNum.c_str() );

	return Smt_Success;
}

Smt_Uint TDeviceState::OnCmdSendDTMF(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strOtherConnID;
	TDevice* pOtherDevice = NULL;
	
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_SendDTMF, CauseInvChannelID, pdu );
			break;
		}
		
		strDeviceID = pDevice->m_DeviceID;
		
		if( pDevice->m_CallID != nCallID )
		{
			RespFailure( Resp_ICMU_SendDTMF, CauseInvCallID, pdu );
			break;
		}
	
		RespCmd(Resp_ICMU_SendDTMF, pdu);

		if( m_DeviceMgr.Lookup(pDevice->m_OtherParty, pOtherDevice) == Smt_Success )
		{
			strOtherConnID = pOtherDevice->m_ConnID;
		}
			
		pDevice->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_SendDTMF;
		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strOtherConnID );
		
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdSendDTMF] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID );

	return Smt_Success;
}

Smt_Uint TDeviceState::OnCmdRecord(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strConnID;
	Smt_Uint   nRespMsg;
	
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	
	if( pdu.m_MessageID == Cmd_ICMU_StartRecord)
	{
		nRespMsg = Resp_ICMU_StartRecord;
	}
	else
	{
		nRespMsg = Resp_ICMU_StopRecord;
	}

	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( nRespMsg, CauseInvChannelID, pdu );
			break;
		}
		
		strDeviceID = pDevice->m_DeviceID;
		
		if( pDevice->m_CallID != nCallID )
		{
			RespFailure( nRespMsg, CauseInvCallID, pdu );
			break;
		}

		RespCmd(nRespMsg, pdu);
		
		strConnID = pDevice->m_ConnID;		
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strConnID );
		
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 ); 

	PrintLog(5, "[TDeviceState::OnCmdRecord] Message<%s>, DeviceRefID<%d>, DeviceID<%s>, CallID<%d>.",
		GetIDName(pdu.m_MessageID).c_str() ,nDeviceRefID, strDeviceID.c_str(), nCallID );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdDeviceSnapshot
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdSendMessage(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nCallID;
	Smt_String strDeviceID;
	Smt_String strOtherConnID;
	TDevice* pOtherDevice = NULL;
	
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_SendMessage, CauseInvChannelID, pdu );
			break;
		}
		
		strDeviceID = pDevice->m_DeviceID;
		
		if( pDevice->m_CallID != nCallID )
		{
			RespFailure( Resp_ICMU_SendMessage, CauseInvCallID, pdu );
			break;
		}

		RespCmd(Resp_ICMU_SendMessage, pdu);

		if( m_DeviceMgr.Lookup(pDevice->m_OtherParty, pOtherDevice) == Smt_Success )
		{
			strOtherConnID = pOtherDevice->m_ConnID;
		}

		pDevice->m_LastCommand = pdu;
		
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_ConnectionStateGOR;
		pdu.m_MessageID = Cmd_ICMU_SendMessage;
		
		pdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		pdu.PutString( Key_ICMU_ConnectionID, strOtherConnID );
		
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	}
	while( 0 );

	PrintLog(5, "[TDeviceState::OnCmdSendMessage] DeviceRefID<%d>, DeviceID<%s>, CallID<%d>.",
		nDeviceRefID, strDeviceID.c_str(), nCallID );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdDeviceSnapshot
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TDeviceState::OnCmdDeviceSnapshot(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strConnectionID;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nOrigSender;

	pdu.GetUint( Key_ICMU_DeviceRefID, &nDeviceRefID );
		
	do 
	{
		pDevice = LookupDeviceByDeviceRefID(nDeviceRefID);
		if( pDevice == NULL )
		{
			RespFailure( Resp_ICMU_DeviceSnapshot, CauseInvChannelID, pdu );
			break;
		}

		strDeviceID = pDevice->m_DeviceID;
		
		Smt_Uint nCallID;
		TCall* pCall = NULL;	
		Smt_Kvset CallIDArray[MAX_CALLID_ARRAY];
		Smt_Uint  nCount = 0;

		nCallID = pDevice->m_CallID;
		if( g_pCallState->m_CallMgr.Lookup( nCallID, pCall) == Smt_Success )
		{
			CallIDArray[nCount].PutUint( Key_ICMU_CallID, pCall->m_CallID );
			CallIDArray[nCount].PutUint( Key_ICMU_CallState, pCall->GetState() );
			nCount++;
		}

		nCallID = pDevice->m_OldCallID;
		if( g_pCallState->m_CallMgr.Lookup( nCallID, pCall) == Smt_Success )
		{
			CallIDArray[nCount].PutUint( Key_ICMU_CallID, pCall->m_CallID );
			CallIDArray[nCount].PutUint( Key_ICMU_CallState, pCall->GetState() );
			nCount++;
		}

		nCallID = pDevice->m_SecOldCallID;
		if( g_pCallState->m_CallMgr.Lookup( nCallID, pCall) == Smt_Success )
		{
			CallIDArray[nCount].PutUint( Key_ICMU_CallID, pCall->m_CallID );
			CallIDArray[nCount].PutUint( Key_ICMU_CallState, pCall->GetState() );
			nCount++;
		}
		
		pdu.PutKvsetArray( Key_ICMU_CallIDArray, CallIDArray, nCount );
		pdu.PutUint( Key_ICMU_Reason, CauseSuccess );
		
		nOrigSender = pdu.m_Sender;
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = nOrigSender;
		pdu.m_MessageID = Resp_ICMU_DeviceSnapshot;
		pdu.m_Status    = Smt_Success;
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );	
		
		PrintLog(5, "[TDeviceState::OnCmdDeviceSnapshot] ChannelID<%d>, DeviceID<%s>.", 
			nDeviceRefID, strDeviceID.c_str() );
	}while( 0 );
	return Smt_Success;
}

TDevice* TDeviceState::OnEvtChannelDataReached(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	Smt_String strVariable;
	Smt_String strConsultingParty;
	Smt_String strConsultedParty;
	Smt_String strTransferredParty;
	Smt_Uint nCallID;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_Variable, &strVariable );
	pdu.GetString( Key_ICMU_ConsultingParty, &strConsultingParty );
	pdu.GetString( Key_ICMU_ConsultedParty, &strConsultedParty );
	pdu.GetString( Key_ICMU_TransferredParty, &strTransferredParty );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );

	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtChannelDataReached] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}

	pDevice->m_Variable = strVariable;
	if( strConsultingParty != "" )
	{
		pDevice->SetCallData( pdu );
		pDevice->m_OtherParty = strConsultingParty;
		pDevice->m_CallerID = strConsultedParty;      // 咨询时需要修正 Device 的 CallerID 值
		pdu.m_Status = CauseOpConsulted;

		// set other device callinfo
		TDevice* pOtherDevice = NULL;
		if( m_DeviceMgr.Lookup(strConsultingParty,pOtherDevice) == Smt_Success )
		{
			pOtherDevice->m_OldCallID = pOtherDevice->m_CallID;
			pOtherDevice->m_CallID = pDevice->m_CallID;
			pOtherDevice->m_OtherParty = pDevice->m_DeviceID;
	
			pdu.GetString( Key_ICMU_CallingParty, &pOtherDevice->m_CallingParty );
			pdu.GetString( Key_ICMU_CalledParty, &pOtherDevice->m_CalledParty );
		}
	}

	if( strTransferredParty != "" )
	{
		pDevice->SetCallData( pdu );
		pDevice->m_OtherParty = strTransferredParty;
		pdu.m_Status = CauseSingleStepTransferred;

		// set other device callinfo
		TDevice* pOtherDevice = NULL;
		if( m_DeviceMgr.Lookup(strTransferredParty,pOtherDevice) == Smt_Success )
		{
			Smt_Uint nOldCallID;
			nOldCallID = pOtherDevice->m_CallID;
			pDevice->m_OldCallID = nOldCallID;

			pOtherDevice->m_CallID = pDevice->m_CallID;
			pOtherDevice->m_OtherParty = pDevice->m_DeviceID;
			pdu.GetString( Key_ICMU_CallingParty, &pOtherDevice->m_CallingParty );
			pdu.GetString( Key_ICMU_CalledParty, &pOtherDevice->m_CalledParty );
		}
	}

	pdu.PutUint( Key_ICMU_Timeout, DEFAULT_SHORTTIMERLEN );

	PrintLog(5,"[TDeviceState::OnEvtChannelDataReached] DeviceID<%s>, ConsultingParty<%s>, TransferredParty<%s>, Variable<%s>.", 
		strDeviceID.c_str(), strConsultingParty.c_str(), strTransferredParty.c_str(), strVariable.c_str() );

	return pDevice;
}

Smt_Uint TDeviceState::OnEvtDeviceTimer(Smt_Uint senderobj)
{
	Smt_String strDeviceID;
	TDevice* pDevice = NULL;
	TDevice* pTmpDevice = NULL;
	
	for( TDeviceMap::ITERATOR 
		iter = m_DeviceMgr.begin();
		iter != m_DeviceMgr.end(); 
		iter++ )
	{
		pTmpDevice = (*iter).int_id_;
		if( Smt_Uint(pTmpDevice) == senderobj )
		{
			pDevice = pTmpDevice;
			break;
		}
	}

	if( pDevice != NULL )
	{
		strDeviceID = pDevice->m_DeviceID;

		Smt_Pdu sendpdu;
		sendpdu.m_Sender = GetGOR();
		sendpdu.m_Receiver = GetGOR();
		sendpdu.m_MessageID = Evt_ICMU_DeviceTimerExpired;
		sendpdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );
	}

	return Smt_Success;
}

TDevice* TDeviceState::OnEvtDeviceTimerExpired(Smt_Pdu &pdu)
{
	Smt_String strDeviceID;
	TDevice* pDevice = NULL;
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	
	if( m_DeviceMgr.Lookup(strDeviceID,pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtDeviceTimerExpired] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}

	pdu.m_Status = pDevice->m_TimerReason;

	PrintLog(5,"[TDeviceState::OnEvtDeviceTimerExpired] DeviceID<%s>,TimerReason<%d>.", strDeviceID.c_str(),pDevice->m_TimerReason );

	return pDevice;
}

/****************************************************************************
函 数 名: LookupOtherPartyByCallID
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_String TDeviceState::LookupOtherPartyByCallID(Smt_String deviceid, Smt_Uint callid)
{
	Smt_String strOtherParty = "";
	TCall* pCall = NULL;
	TConnID* pTmpConnID = NULL;

	if( (g_pCallState->m_CallMgr.Lookup(callid, pCall) == Smt_Success) &&
		(pCall->m_ConnIDList.GetCount() <= 2) )
	{	
		for(TConnIDMap::ITERATOR iter = pCall->m_ConnIDList.begin(); 
			iter!= pCall->m_ConnIDList.end(); 
			iter++)
		{
			pTmpConnID = (*iter).int_id_;
			if(pTmpConnID->m_DeviceID != deviceid)
			{
				strOtherParty = pTmpConnID->m_DeviceID;
				break;
			}
		}		
	}

	PrintLog(5,"[TDeviceState::LookupOtherPartyByCallID] CallID<%d>, DeviceID<%s>, OtherParty<%s>.", 
		callid, deviceid.c_str(),strOtherParty.c_str() );

	return strOtherParty;
}

Smt_String TDeviceState::LookupDeviceIDByConnID(Smt_String connid)
{
	Smt_String strDeviceID = "";
	
	TDevice* pTmpDevice = NULL;
	for( TDeviceMap::ITERATOR 
		iter = m_DeviceMgr.begin();
		iter != m_DeviceMgr.end(); 
		iter++ )
	{
		pTmpDevice = (*iter).int_id_;
		if( pTmpDevice->m_ConnID == connid )
		{
			strDeviceID = pTmpDevice->m_DeviceID;
			break;
		}
	}
	PrintLog(5,"[TDeviceState::LookupDeviceIDByConnID] ConnID<%s>, DeviceID<%s>.", 
		connid.c_str(), strDeviceID.c_str() );
	
	return strDeviceID;
}

Smt_Uint TDeviceState::OnCmdQueryLinkState(Smt_Pdu &pdu)
{
	Smt_Uint nOrigSender = pdu.m_Sender;
	pdu.m_Sender = this->GetGOR();
	pdu.m_Receiver = nOrigSender;
	pdu.m_MessageID = Resp_ICMU_QueryLinkState;
	pdu.m_Status = Smt_Success;

	pdu.PutUint( Key_ICMU_Reason, m_LinkReason );
	pdu.PutUint( Key_ICMU_LinkState, m_LinkState );

	if( pdu.m_Receiver > 0 ) PostMessage( pdu );
	
	//PrintLog( 5, "[TCallState::RespQueryLinkState] LinkState<%d>.", m_nLinkState );

	return Smt_Success;
}

Smt_Uint TDeviceState::OnEvtOtherDeviceEvent(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	Smt_String strDeviceID;
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	
	if( m_DeviceMgr.Lookup(strDeviceID, pDevice) != Smt_Success )
	{
		PrintLog(5,"[TDeviceState::OnEvtOtherDeviceEvent] Lookup Device Fail, DeviceID<%s>.", 
			strDeviceID.c_str() );
		return NULL;
	}

	pDevice->SendOtherEvent( pdu );

	return Smt_Success;
}

Smt_Uint TDeviceState::OnEvtMessageReceived(Smt_Pdu &pdu)
{
	TDevice* pDevice = NULL;
	TDevice* pOtherDevice = NULL;
	Smt_String strDeviceID = "";
	Smt_String strOtherParty = "";

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	
	do 
	{
		if( m_DeviceMgr.Lookup(strDeviceID, pDevice) != Smt_Success )
		{
			break;
		}
		
		strOtherParty = pDevice->m_OtherParty;
		
		if( m_DeviceMgr.Lookup(strOtherParty, pOtherDevice) != Smt_Success )
		{
			break;
		}
		
		pOtherDevice->SendOtherEvent( pdu );

	} while (0);

	PrintLog(5,"[TDeviceState::OnEvtMessageReceived] Message DeviceID<%s> To OtherParty<%s>.", 
		strDeviceID.c_str(), strOtherParty.c_str() );
	
	return Smt_Success;
}

Smt_Uint TDeviceState::OnEvtLinkEvent(Smt_Pdu &pdu)
{
	pdu.GetUint( Key_ICMU_Reason, &m_LinkReason );

	if( pdu.m_MessageID == Evt_ICMU_LinkUp)
	{
		m_LinkState = St_LinkUpState;
	}
	else // Evt_ICMU_LinkDown
	{
		m_LinkState = St_LinkDownState;
	}
	     
	PrintLog(3, "[TDeviceState::OnEvtLinkEvent] MessageID<%s>, LinkState<%d>.", 
		GetIDName( pdu.m_MessageID).c_str(), m_LinkState );

	return Smt_Success;
}

Smt_Uint TDeviceState::OnCmd_To_CallState(Smt_Pdu &pdu)
{
	Smt_Uint   nDeviceRefID;
	pdu.GetUint( Key_ICMU_DeviceRefID, &nDeviceRefID );

	pdu.m_Receiver = m_CallStateGOR;
	if( pdu.m_Receiver > 0 ) PostMessage( pdu );

	PrintLog(5, "[TDeviceState::OnCmd_To_CallState] DeviceRefID<%d>, MessageID<%s>.",
		nDeviceRefID, GetIDName( pdu.m_MessageID).c_str() );

	return Smt_Success;
}
