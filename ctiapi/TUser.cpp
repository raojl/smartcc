//***************************************************************************
// TUser.cpp : implementation file
//***************************************************************************
#include "TUser.h"
#include "TConfig.h"

/////////////////////////////////////////////////////////////////////////////
// TUser code
TUser* g_pCMUDLLUser = NULL;
TUser::TUser( Smt_String name, 
			  Smt_Uint loglevel, Smt_String logpath, 
			  Smt_Server* pserver )
: Smt_User(name, loglevel, logpath, pserver )
{
	m_QueryLinkTimer = 0;
	m_DeviceStateName = CMU_DEVICESTATENAME;
	m_DeviceStateGOR = 0;
	m_CallStateName = CMU_CALLSTATENAME;
	m_CallStateGOR = 0;
	m_LastLinkState = 0;

	//add by caoyj 20111025
	m_EventQueue.low_water_mark(1024 * 1024);
	m_EventQueue.high_water_mark(1024 * 1024);
}

TUser::~TUser()
{
	ClearTimer( m_QueryLinkTimer );

	Smt_TimeValue tmVal;
	tmVal.msec( 1 );
	
	Smt_Pdu* pPdu = NULL;
	Smt_Int nRet = 0;
	while(true)	
	{
		nRet = m_EventQueue.dequeue_head( pPdu, &tmVal );
		
		if (nRet == -1 && errno == EWOULDBLOCK)
			break;
		
		if( pPdu != NULL )
		{
			delete pPdu;
			pPdu = NULL;
		}
	}
}

Smt_Uint TUser::OnUserOnline(Smt_Uint sender, Smt_String sendername)
{
	PrintLog(3,"[TUser::OnUserOnline ] sender<0x%x>, sendername<%s>.", sender, sendername.c_str());
	if( m_CallStateName == sendername )
	{
		m_CallStateGOR = sender;

//		if( m_QueryLinkTimer == 0 )
//			m_QueryLinkTimer = SetTimer(MAX_QUERYLINK_TIMERLEN, Evt_ICMU_QueryLinkTimer);
	}

	if( m_DeviceStateName == sendername )
	{
		m_DeviceStateGOR = sender;
		if( m_QueryLinkTimer == 0 )
			m_QueryLinkTimer = SetTimer(MAX_QUERYLINK_TIMERLEN, Evt_ICMU_QueryLinkTimer);
	}

	return Smt_Success;
}

Smt_Uint TUser::OnUserOffline(Smt_Uint sender, Smt_String sendername)
{
	PrintLog(3,"[TUser::OnUserOffline] sender<0x%x>, sendername<%s>.", sender, sendername.c_str());

	if( m_CallStateName == sendername )
	{
		m_CallStateGOR = 0;
/*		Smt_DateTime dtNow;
		Smt_Pdu pdu;
		pdu.m_MessageID = Evt_ICMU_LinkDown;
		pdu.PutUint  (Key_ICMU_Reason, CauseDLLNetWorkError);
		pdu.PutString(Key_ICMU_TimeStamp, dtNow.FormatString());
		OnEvtLinkDown( pdu );
		
		m_LastLinkState = St_LinkDownState;
*/	}

	if( m_DeviceStateName == sendername )
	{
		m_DeviceStateGOR = 0;

		ClearTimer( m_QueryLinkTimer );
		m_QueryLinkTimer = 0;
		
		Smt_DateTime dtNow;
		Smt_Pdu pdu;
		pdu.m_MessageID = Evt_ICMU_LinkDown;
		pdu.PutUint  (Key_ICMU_Reason, CauseDLLNetWorkError);
		pdu.PutString(Key_ICMU_TimeStamp, dtNow.FormatString());
		OnEvtLinkDown( pdu );
		
		m_LastLinkState = St_LinkDownState;
	}

	return Smt_Success;
}

Smt_Uint TUser::OnTimer(Smt_Uint& timerid, Smt_Uint& messageid, Smt_Uint& senderobj)
{
	switch (messageid)
	{
	case Evt_ICMU_QueryLinkTimer:
		OnEvtQueryLinkTimer();
		break;
	default:
		break;
	}
	return Smt_Success;
}

Smt_Uint TUser::HandleMessage(Smt_Pdu& pdu)
{
	switch(pdu.m_MessageID)
	{
	// resp event
	case Resp_ICMU_Assign:
		OnRespAssign( pdu );
		break;
	case Resp_ICMU_Deassign:
	case Resp_ICMU_MakeCall:	
	case Resp_ICMU_HangupCall:
	case Resp_ICMU_ClearCall:
	case Resp_ICMU_HoldCall:
	case Resp_ICMU_RetrieveCall:
	case Resp_ICMU_ConsultationCall:
	case Resp_ICMU_ConferenceCall:
	case Resp_ICMU_TransferCall:
	case Resp_ICMU_ReconnectCall:
	case Resp_ICMU_SingleStepTransfer:	
	case Resp_ICMU_SingleStepConference:
	case Resp_ICMU_SendDTMF:
	case Resp_ICMU_RouteSelected:
	case Resp_ICMU_StartRecord:
	case Resp_ICMU_StopRecord:
	case Resp_ICMU_SetData:
	case Resp_ICMU_SendMessage:
		OnRespCtcResponse( pdu );
		break;
	case Resp_ICMU_GetData:
		OnRespGetData( pdu );
		break;
	case Resp_ICMU_DeviceSnapshot:
		OnRespDeviceSnapshot( pdu );
		break;    

	case Resp_ICMU_AssignEx:
		OnRespAssignEx( pdu );
		break;
	case Resp_ICMU_DeassignEx:
	case Resp_ICMU_AnswerCallEx:
	case Resp_ICMU_HangupCallEx:
	case Resp_ICMU_SingleStepTransferEx:
	case Resp_ICMU_Dial:
	case Resp_ICMU_PlayFile:
	case Resp_ICMU_PlayFileList:
	case Resp_ICMU_GetDigits:
	case Resp_ICMU_RecordEx:
	case Resp_ICMU_SendDTMFEx:
	case Resp_ICMU_SendFax:
	case Resp_ICMU_ReceiveFax:	
	case Resp_ICMU_SubscribeCallEvent:
	case Resp_ICMU_UnsubscribeCallEvent:
	case Resp_ICMU_SendMessageEx:
	case Resp_ICMU_SetSIPHeader:
		OnRespGcResponse( pdu );
		break; 	
	case Resp_ICMU_MakeCallEx:
		OnRespMakeCallEx( pdu );
		break;	
	case Resp_ICMU_GetSIPHeader:
		OnRespGetSIPHeader( pdu );
		break;	

	// call event
	case Evt_ICMU_CallInitiated:
	case Evt_ICMU_CallDelivered  :
	case Evt_ICMU_CallConnected  :
	case Evt_ICMU_CallHeld       :
	case Evt_ICMU_CallRetrieved  :
	case Evt_ICMU_CallConferenced:
	case Evt_ICMU_CallQueued     :
	case Evt_ICMU_CallTransferred:
	case Evt_ICMU_CallCleared    :
	case Evt_ICMU_CallFailed     :
		OnEvtCallEvent( pdu );
		break;

	// device event
	case Evt_ICMU_BackInService:
	case Evt_ICMU_OutOfService:
		OnEvtDeviceBackIn_OutOfService( pdu );
		break;
	case Evt_ICMU_OffHook:
	case Evt_ICMU_InboundCall:
	case Evt_ICMU_DestSeized:
	case Evt_ICMU_TpAnswered:
	case Evt_ICMU_OpAnswered:
	case Evt_ICMU_TpSuspended:
	case Evt_ICMU_OpHeld:
	case Evt_ICMU_TpRetrieved:
	case Evt_ICMU_OpRetrieved:
	case Evt_ICMU_TpDisconnected:
	case Evt_ICMU_OpDisconnected:
	case Evt_ICMU_TpTransferred:
	case Evt_ICMU_OpTransferred:
	case Evt_ICMU_TpConferenced:
	case Evt_ICMU_OpConferenced:
	case Evt_ICMU_DestBusy:
	case Evt_ICMU_DestInvalid:
	case Evt_ICMU_Queued:
		OnEvtDeviceEvent( pdu );
		break;

	case Evt_ICMU_RouteRequest:
	case Evt_ICMU_RouteEnd:
		OnEvtRouteEvent( pdu );
		break;
	case Evt_ICMU_Playing  :
	case Evt_ICMU_PlayEnd  :
	case Evt_ICMU_Sending  :
	case Evt_ICMU_SendEnd  :
	case Evt_ICMU_Geting   :
	case Evt_ICMU_GetEnd   :
	case Evt_ICMU_Recording:
	case Evt_ICMU_RecordEnd:
	case Evt_ICMU_MessageSending:
	case Evt_ICMU_MessageSendEnd:
		OnEvtMediaEvent( pdu );
		break;
	case Evt_ICMU_DeviceRecording:
	case Evt_ICMU_DeviceRecordEnd:
		OnEvtDeviceRecordEvent( pdu );
		break;

	case Evt_ICMU_MessageReceived:
		OnEvtMessageReceived( pdu );
		break;

	case Evt_ICMU_LinkUp:
		OnEvtLinkUp( pdu );
		break;

	case Evt_ICMU_LinkDown:
		OnEvtLinkDown( pdu );
		break;

// 	case Evt_ICMU_TTSPlayEnd:
// 		OnEvtTTSPlayEnd(pdu);
// 		break;

	case Resp_ICMU_QueryLinkState:
		OnRespQueryLinkState( pdu );
		break;
		
	default:
		PrintLog(3, "[TUser::HandleMessage ] Receive Unknown Message, %s.",
			pdu.GetInfo().c_str() );
		break;
	}
	
	return Smt_Success;
}

Smt_Uint TUser::PutEventQueue(Csta_Event* pevt)
{
	Smt_Int nRet = Smt_Success;

	Smt_Pdu* pPdu = new Smt_Pdu();
	pPdu->PutByteArray(Key_ICMU_CstaEventBuffer, (unsigned char*)pevt, sizeof(Csta_Event) );
	
	Smt_TimeValue tmVal;
	tmVal.msec( 10 );

	nRet = m_EventQueue.enqueue_tail( pPdu, &tmVal );
	if( nRet == -1)
	{
		if(pPdu != NULL)
		{
			delete pPdu;
			pPdu = NULL;
		}
		
		PrintLog(3,"[TUser::PutEventQueue] putq fail<%d>,Count<%d>,size<%d>.", nRet,m_EventQueue.message_count(),m_EventQueue.message_bytes() );

		nRet = Smt_Fail;	
	}

	return nRet;
}

Smt_Uint TUser::GetEventQueue(Csta_Event* pevt, Smt_Uint timeout )
{
	Smt_Int nRet = Smt_Success;

	Smt_Pdu* pPdu = NULL;
	Smt_DateTime dtNow;
	Smt_TimeValue tmVal;

	tmVal.msec( (Smt_Int)timeout );
	tmVal = dtNow.TimeValue() + tmVal;
		
	do 
	{
		nRet = m_EventQueue.dequeue_head( pPdu, &tmVal);
		if ( nRet == -1 )
		{
			nRet = Err_ICMU_Timeout;
			break;
		}

		Smt_Uint nByteArrayLen;	
		Smt_ByteArray cByteArray;
		
		pPdu->GetByteArray(Key_ICMU_CstaEventBuffer, &cByteArray, &nByteArrayLen );

		try
		{
			ACE_OS::memcpy( pevt, cByteArray, sizeof(Csta_Event) );	
		}
		catch (...)
		{
			nRet = Err_ICMU_MemoryException;
			break;	
		}

	} while (0);

	if(pPdu != NULL )
	{
		delete pPdu;
		pPdu = NULL;
	}

	return nRet;
}

/****************************************************************************
函 数 名: GetEvtType
参    数:
返回数值: 
功能描述: 转换事件类型
*****************************************************************************/
Smt_Uint TUser::GetEvtType(Smt_Uint messageid)
{
	Smt_Uint nRet;
	switch (messageid)
	{
	// command resp
	case Resp_ICMU_Assign:
		nRet = CONF_CTCASSIGN;
		break;
	case Resp_ICMU_Deassign:
		nRet = CONF_CTCDEASSIGN;
		break;
	case Resp_ICMU_MakeCall:	
		nRet = CONF_CTCMAKECALL;
		break;
	case Resp_ICMU_HangupCall:	
		nRet = CONF_CTCHANGUPCALL;
		break;
	case Resp_ICMU_HoldCall:	
		nRet = CONF_CTCHOLDCALL;
		break;
	case Resp_ICMU_RetrieveCall:	
		nRet = CONF_CTCRETRIEVECALL;
		break;
	case Resp_ICMU_ConsultationCall:	
		nRet = CONF_CTCCONSULTATIONCALL;
		break;
	case Resp_ICMU_ConferenceCall:	
		nRet = CONF_CTCCONFERENCECALL;
		break;
	case Resp_ICMU_TransferCall:	
		nRet = CONF_CTCTRANSFERCALL;
		break;
	case Resp_ICMU_ReconnectCall:	
		nRet = CONF_CTCRECONNECTCALL;
		break;
	case Resp_ICMU_SingleStepTransfer:	
		nRet = CONF_CTCSINGLESTEPTRANSFER;
		break;
	case Resp_ICMU_SingleStepConference:
		nRet = CONF_CTCSINGLESTEPCONFERENCE;
		break;
	case Resp_ICMU_SendDTMF:	
		nRet = CONF_CTCSENDDTMF;
		break;
	case Resp_ICMU_RouteSelected:
		nRet = CONF_CTCROUTESELECTED;
		break;
	case Resp_ICMU_StartRecord:
		nRet = CONF_CTCSTARTRECORD;
		break;
	case Resp_ICMU_StopRecord:
		nRet = CONF_CTCSTOPRECORD;
		break;
	case Resp_ICMU_SetData:
		nRet = CONF_CTCSETDATAVALUE;
		break;
	case Resp_ICMU_GetData:
		nRet = CONF_CTCGETDATAVALUE;
		break;
	case Resp_ICMU_DeviceSnapshot:
		nRet = CONF_CTCSNAPSHOT;
		break;
	case Resp_ICMU_SendMessage:
		nRet = CONF_CTCSENDMESSAGE;
		break;

	case Resp_ICMU_AssignEx:
		nRet = CONF_GC_OPEN;
		break;
	case Resp_ICMU_DeassignEx:
		nRet = CONF_GC_CLOSE;
		break;
	case Resp_ICMU_MakeCallEx:		
		nRet = CONF_GC_MAKECALL;
		break;
	case Resp_ICMU_AnswerCallEx:
		nRet = CONF_GC_ANSWERCALL;
		break;
	case Resp_ICMU_HangupCallEx:
		nRet = CONF_GC_HANGUPCALL;
		break;
	case Resp_ICMU_SingleStepTransferEx:
		nRet = CONF_GC_BLINDTRANSFER;
		break;
	case Resp_ICMU_Dial:
		nRet = CONF_GC_DIAL;
		break;
	case Resp_ICMU_PlayFile:
		nRet = CONF_DX_PLAY;
		break;
	case Resp_ICMU_PlayFileList:
		nRet = CONF_DX_PLAYIOTTDATA;
		break;
	case Resp_ICMU_GetDigits:
		nRet = CONF_DX_GETDIG;
		break;
	case Resp_ICMU_RecordEx:
		nRet = CONF_DX_REC;
		break;
	case Resp_ICMU_SendDTMFEx:
		nRet = CONF_DX_DIAL;
		break;
	case Resp_ICMU_SendFax:
		nRet = CONF_FX_SENDFAX;
		break;
	case Resp_ICMU_ReceiveFax:	
		nRet = CONF_FX_RCVFAX;
		break;
	case Resp_ICMU_SubscribeCallEvent:
		nRet = CONF_SUBSCRIBECALLEVENT;
		break;
	case Resp_ICMU_UnsubscribeCallEvent:
		nRet = CONF_UNSUBSCRIBECALLEVENT;
		break;
	case Resp_ICMU_SendMessageEx:
		nRet = CONF_DX_SENDMESSAGE;
		break;
	case Resp_ICMU_SetSIPHeader:
		nRet = CONF_DX_SETSIPHEADER;
		break;

	// event
	case Evt_ICMU_CallInitiated  :
		nRet = CSTA_CALLINITIATED;
		break;
	case Evt_ICMU_CallDelivered  :
		nRet = CSTA_CALLDELIVERED;
		break;
	case Evt_ICMU_CallConnected  :
		nRet = CSTA_CALLCONNECTED;
		break;
	case Evt_ICMU_CallHeld       :
		nRet = CSTA_CALLHELD;
		break;
	case Evt_ICMU_CallRetrieved  :
		nRet = CSTA_CALLRETRIEVED;
		break;
	case Evt_ICMU_CallConferenced:
		nRet = CSTA_CALLCONFERENCED;
		break;
	case Evt_ICMU_CallQueued     :
		nRet = CSTA_CALLQUEUED;
		break;
	case Evt_ICMU_CallTransferred:
		nRet = CSTA_CALLTRANSFERRED;
		break;
	case Evt_ICMU_CallCleared    :
		nRet = CSTA_CALLCLEARED;
		break;
	case Evt_ICMU_CallFailed     :
		nRet = CSTA_CALLFAILED;
		break;
	case Evt_ICMU_BackInService  :
		nRet = CSTA_DEVICE_BACKINSERVICE;
		break;
	case Evt_ICMU_OutOfService   : 
		nRet = CSTA_DEVICE_OUTOFSERVICE;
		break;
	case Evt_ICMU_OffHook        :
		nRet = CSTA_DEVICE_OFFHOOK;
		break;
	case Evt_ICMU_InboundCall    :
		nRet = CSTA_DEVICE_INBOUNDCALL;
		break;
	case Evt_ICMU_DestSeized     :
		nRet = CSTA_DEVICE_DESTSEIZED;
		break;
	case Evt_ICMU_TpAnswered     :
		nRet = CSTA_DEVICE_TPANSWERED;
		break;
	case Evt_ICMU_OpAnswered     :
		nRet = CSTA_DEVICE_OPANSWERED;
		break;
	case Evt_ICMU_TpSuspended    :
		nRet = CSTA_DEVICE_TPSUSPENDED;
		break;
	case Evt_ICMU_OpHeld         :
		nRet = CSTA_DEVICE_OPHELD;
		break;
	case Evt_ICMU_TpRetrieved    :
		nRet = CSTA_DEVICE_TPRETRIEVED;
		break;
	case Evt_ICMU_OpRetrieved    :
		nRet = CSTA_DEVICE_OPRETRIEVED;
		break;
	case Evt_ICMU_TpDisconnected :
		nRet = CSTA_DEVICE_TPDISCONNECTED;
		break;
	case Evt_ICMU_OpDisconnected :
		nRet = CSTA_DEVICE_OPDISCONNECTED;
		break;
	case Evt_ICMU_TpTransferred  :
		nRet = CSTA_DEVICE_TPTRANSFERRED;
		break;
	case Evt_ICMU_OpTransferred  :
		nRet = CSTA_DEVICE_OPTRANSFERRED;
		break;
	case Evt_ICMU_TpConferenced  :
		nRet = CSTA_DEVICE_TPCONFERENCED;
		break;
	case Evt_ICMU_OpConferenced  :
		nRet = CSTA_DEVICE_OPCONFERENCED;
		break;
	case Evt_ICMU_DestBusy       :
		nRet = CSTA_DEVICE_DESTBUSY;
		break;
	case Evt_ICMU_DestInvalid    :
		nRet = CSTA_DEVICE_DESTINVALID;
		break;
	case Evt_ICMU_Queued    :
		nRet = CSTA_DEVICE_QUEUED;
		break;
	
	case Evt_ICMU_RouteRequest   :
		nRet = CSTA_DEVICE_ROUTEREQUEST;
		break;
	case Evt_ICMU_RouteEnd       :
		nRet = CSTA_DEVICE_ROUTEEND;
		break;
	case Evt_ICMU_DeviceRecording:
		nRet = CSTA_DEVICE_RECORDING;
		break;
	case Evt_ICMU_DeviceRecordEnd:
		nRet = CSTA_DEVICE_RECORDEND;
		break;
	case Evt_ICMU_Recording      :
		nRet = CSTA_MEDIA_RECORDING;
		break;
	case Evt_ICMU_RecordEnd      :
		nRet = CSTA_MEDIA_RECORDEND;
		break;
	case Evt_ICMU_Playing        :
		nRet = CSTA_MEDIA_PLAYING;
		break;
	case Evt_ICMU_PlayEnd        :
		nRet = CSTA_MEDIA_PLAYEND;
		break;
	case Evt_ICMU_Sending        :
		nRet = CSTA_MEDIA_SENDING;
		break;
	case Evt_ICMU_SendEnd        :
		nRet = CSTA_MEDIA_SENDEND;
		break;
	case Evt_ICMU_Geting         :
		nRet = CSTA_MEDIA_GETING;
		break;
	case Evt_ICMU_GetEnd         :
		nRet = CSTA_MEDIA_GETEND;
		break;
	case Evt_ICMU_LinkDown         :
		nRet = CSTA_LINKDOWN;
		break;
	case Evt_ICMU_LinkUp         :
		nRet = CSTA_LINKUP;
		break;
	case Evt_ICMU_MessageReceived:
		nRet = CSTA_DEVICE_MESSAGE;
		break;
	case Evt_ICMU_MessageSending:
		nRet = CSTA_MEDIA_MESSAGESENDING;
		break;
	case Evt_ICMU_MessageSendEnd:
		nRet = CSTA_MEDIA_MESSAGESENDEND;
		break;
// 	case Evt_ICMU_TTSPlayEnd:
// 		nRet = CSTA_TTSPLAYEND;
// 		break;
	default:
		nRet = 0;
		break;
	}
	return nRet;
}

/****************************************************************************
函 数 名: OnEvtCallEvent
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_String TUser::GetEvtName(Smt_Uint evttype)
{
	switch(evttype)
	{
		CASE_STR( CONF_CTCASSIGN               )
		CASE_STR( CONF_CTCDEASSIGN             )
		CASE_STR( CONF_CTCMAKECALL             )
		CASE_STR( CONF_CTCHANGUPCALL           )
		CASE_STR( CONF_CTCSINGLESTEPTRANSFER   )
		CASE_STR( CONF_CTCSINGLESTEPCONFERENCE )
		CASE_STR( CONF_CTCROUTESELECTED  )
		CASE_STR( CONF_CTCSENDDTMF             )
		CASE_STR( CONF_CTCSTARTRECORD          )
		CASE_STR( CONF_CTCSTOPRECORD           )
		CASE_STR( CONF_CTCSETDATAVALUE         )
		CASE_STR( CONF_CTCGETDATAVALUE         )
		CASE_STR( CONF_GC_OPEN                 )
		CASE_STR( CONF_GC_CLOSE                )
		CASE_STR( CONF_GC_MAKECALL             )
		CASE_STR( CONF_GC_ANSWERCALL           )
		CASE_STR( CONF_GC_HANGUPCALL           )
		CASE_STR( CONF_GC_BLINDTRANSFER        )
		CASE_STR( CONF_DX_PLAY                 )
		CASE_STR( CONF_DX_ADDIOTTDATA          )
		CASE_STR( CONF_DX_PLAYIOTTDATA         )
		CASE_STR( CONF_DX_GETDIG               )
		CASE_STR( CONF_DX_DIAL                 )
		CASE_STR( CONF_DX_REC                  )
		CASE_STR( CONF_FX_SENDFAX              )
		CASE_STR( CONF_FX_RCVFAX               )
		CASE_STR( CONF_SUBSCRIBECALLEVENT      )
		CASE_STR( CONF_UNSUBSCRIBECALLEVENT    )
		CASE_STR( CONF_CTCSNAPSHOT             )
		CASE_STR( CONF_GC_DIAL                 )
		CASE_STR( CONF_CTCSENDMESSAGE          )
		CASE_STR( CONF_DX_SENDMESSAGE          )

		// CSTA_CALLEVENT            
		CASE_STR( CSTA_CALLINITIATED           )
		CASE_STR( CSTA_CALLDELIVERED           )
		CASE_STR( CSTA_CALLCONNECTED           )
		CASE_STR( CSTA_CALLHELD                )
		CASE_STR( CSTA_CALLRETRIEVED           )
		CASE_STR( CSTA_CALLCONFERENCED         )
		CASE_STR( CSTA_CALLQUEUED              )
		CASE_STR( CSTA_CALLTRANSFERRED         )
		CASE_STR( CSTA_CALLCLEARED             )
		CASE_STR( CSTA_CALLFAILED              )

		// CSTA_DEVICEEVENT          
		CASE_STR( CSTA_DEVICE_BACKINSERVICE    )
		CASE_STR( CSTA_DEVICE_OUTOFSERVICE     )
		CASE_STR( CSTA_DEVICE_OFFHOOK          )
		CASE_STR( CSTA_DEVICE_INBOUNDCALL      )
		CASE_STR( CSTA_DEVICE_DESTSEIZED       )
		CASE_STR( CSTA_DEVICE_TPANSWERED       )
		CASE_STR( CSTA_DEVICE_OPANSWERED       )
		CASE_STR( CSTA_DEVICE_TPSUSPENDED      )
		CASE_STR( CSTA_DEVICE_OPHELD           )
		CASE_STR( CSTA_DEVICE_TPRETRIEVED      )
		CASE_STR( CSTA_DEVICE_OPRETRIEVED      )
		CASE_STR( CSTA_DEVICE_TPDISCONNECTED   )
		CASE_STR( CSTA_DEVICE_OPDISCONNECTED   )
		CASE_STR( CSTA_DEVICE_TPTRANSFERRED    )
		CASE_STR( CSTA_DEVICE_OPTRANSFERRED    )
		CASE_STR( CSTA_DEVICE_TPCONFERENCED    )
		CASE_STR( CSTA_DEVICE_OPCONFERENCED    )
		CASE_STR( CSTA_DEVICE_DESTBUSY         )
		CASE_STR( CSTA_DEVICE_DESTINVALID      )
		CASE_STR( CSTA_DEVICE_ROUTEREQUEST     )
		CASE_STR( CSTA_DEVICE_ROUTEEND         )
		CASE_STR( CSTA_DEVICE_QUEUED           )

		// CSTA_MEDIAEVENT          
		CASE_STR( CSTA_MEDIA_PLAYING           )
		CASE_STR( CSTA_MEDIA_PLAYEND           )
		CASE_STR( CSTA_MEDIA_SENDING           )
		CASE_STR( CSTA_MEDIA_SENDEND           )
		CASE_STR( CSTA_MEDIA_GETING            )
		CASE_STR( CSTA_MEDIA_GETEND            )
		CASE_STR( CSTA_MEDIA_RECORDING         )
		CASE_STR( CSTA_MEDIA_RECORDEND         )

		// CSTA_DEVICERECORDEVENT   
		CASE_STR( CSTA_DEVICE_RECORDING        )
		CASE_STR( CSTA_DEVICE_RECORDEND        )
		CASE_STR( CSTA_DEVICE_MESSAGE        )
		            
		// CSTA_SYSTEMEVENT         
		CASE_STR( CSTA_LINKDOWN                )
		CASE_STR( CSTA_LINKUP                  )

	default:
		return HLFormatStr("_?(%d)", evttype );
	}
	return "";
}

/****************************************************************************
函 数 名: OnEvtCallEvent
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnEvtCallEvent(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;

	Smt_Uint   nCallID;
	Smt_Uint   nCallState;
	Smt_String strCallingParty;
	Smt_String strCalledParty;
	Smt_String strInitiatedParty;
	Smt_String strAnsweringParty;
	Smt_String strAlertingParty;
	Smt_String strHoldingParty;
	Smt_String strRetrievingParty;
	Smt_String strConsultingParty;
	Smt_String strTransferringParty;
	Smt_String strConferencingParty;
	Smt_Uint   nReason;
	Smt_String strTimeStamp;

	pdu.GetUint  (Key_ICMU_CallID, &nCallID );
	pdu.GetUint  (Key_ICMU_CallState, &nCallState );
	pdu.GetString(Key_ICMU_CallingParty, &strCallingParty );
	pdu.GetString(Key_ICMU_CalledParty, &strCalledParty );
	pdu.GetString(Key_ICMU_InitiatedParty, &strInitiatedParty );
	pdu.GetString(Key_ICMU_AnsweringParty, &strAnsweringParty );
	pdu.GetString(Key_ICMU_AlertingParty, &strAlertingParty );
	pdu.GetString(Key_ICMU_HoldingParty, &strHoldingParty );
	pdu.GetString(Key_ICMU_RetrievingParty, &strRetrievingParty );
	pdu.GetString(Key_ICMU_ConsultingParty, &strConsultingParty );
	pdu.GetString(Key_ICMU_TransferringParty, &strTransferringParty );
	pdu.GetString(Key_ICMU_ConferencingParty, &strConferencingParty );
	pdu.GetUint  (Key_ICMU_Reason, &nReason );
	pdu.GetString(Key_ICMU_TimeStamp, &strTimeStamp );
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_CALLEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );
	
	evtTemp.u.evtcall.CallID = nCallID;
	evtTemp.u.evtcall.CallState = nCallState;
	StrAssign(evtTemp.u.evtcall.CallingParty, strCallingParty.c_str() );
	StrAssign(evtTemp.u.evtcall.CalledParty, strCalledParty.c_str() );
	StrAssign(evtTemp.u.evtcall.InitiatedParty, strInitiatedParty.c_str() );
	StrAssign(evtTemp.u.evtcall.AnsweringParty, strAnsweringParty.c_str() );
	StrAssign(evtTemp.u.evtcall.AlertingParty, strAlertingParty.c_str() );
	StrAssign(evtTemp.u.evtcall.HoldingParty, strHoldingParty.c_str() );
	StrAssign(evtTemp.u.evtcall.RetrievingParty, strRetrievingParty.c_str() );
	StrAssign(evtTemp.u.evtcall.ConsultingParty, strConsultingParty.c_str() );
	StrAssign(evtTemp.u.evtcall.TransferringParty, strTransferringParty.c_str() );
	StrAssign(evtTemp.u.evtcall.ConferencingParty, strConferencingParty.c_str() );
	evtTemp.u.evtcall.Reason = nReason;
	StrAssign(evtTemp.u.evtcall.TimeStamp, strTimeStamp.c_str() );
	
	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnEvtCallEvent] evttype<%s>, nCallID<%d>, nCallState<%d>.",
		GetEvtName(evtTemp.evttype).c_str(), nCallID, nCallState );

	// delete playfile list info
	if( pdu.m_MessageID == Evt_ICMU_CallCleared )
	{
		TPlayList* pPlayList = NULL;
		if( m_PlayList.Lookup(nCallID, pPlayList ) == Smt_Success )
		{
			m_PlayList.Remove( nCallID );
			delete pPlayList;
			pPlayList = NULL;
		}
	}

	return nRet;
}

/****************************************************************************
函 数 名: OnEvtDeviceBackIn_OutOfService
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnEvtDeviceBackIn_OutOfService(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Uint   nDeviceRefID;
	Smt_String strDeviceID;
	Smt_String strTimeStamp;
	Smt_Uint   nReason;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	pdu.GetString( Key_ICMU_TimeStamp, &strTimeStamp );
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_DEVICEEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );
	
	evtTemp.u.evtdevice.ChannelID = nDeviceRefID;
	StrAssign(evtTemp.u.evtdevice.MonitorParty, strDeviceID.c_str() );
	evtTemp.u.evtdevice.Reason = nReason;
	StrAssign(evtTemp.u.evtdevice.TimeStamp, strTimeStamp.c_str() );

	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnEvtDeviceBackIn_OutOfService] evttype<%s>, nChannelID<%d>, strDeviceID<%s>, nReason<%d>.",
		GetEvtName(evtTemp.evttype).c_str(), nDeviceRefID, strDeviceID.c_str(), nReason );

	return nRet;
}

/****************************************************************************
函 数 名: OnEvtDeviceEvent
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnEvtDeviceEvent(Smt_Pdu& pdu)
{
	Smt_Uint   nRet = Smt_Success;
	Smt_Uint   nDeviceRefID;
	Smt_String strDeviceID;           
	Smt_Uint   nDeviceType;           
	Smt_Uint   nDeviceState;           
	Smt_Uint   nCallID;           
	Smt_Uint   nOldCallID;          
	Smt_Uint   nSecOldCallID;          
	Smt_String strOtherParty;
	Smt_String strCallingParty;  
	Smt_String strCalledParty;         
	Smt_String strThirdParty;        
	Smt_Uint   nReason;           
	Smt_String strTimeStamp;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_DeviceType, &nDeviceType );
	pdu.GetUint  ( Key_ICMU_DeviceState, &nDeviceState );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetUint  ( Key_ICMU_OldCallID, &nOldCallID );
	pdu.GetUint  ( Key_ICMU_SecOldCallID, &nSecOldCallID );
	pdu.GetString( Key_ICMU_OtherParty, &strOtherParty );
	pdu.GetString( Key_ICMU_CallingParty, &strCallingParty );
	pdu.GetString( Key_ICMU_CalledParty, &strCalledParty );
	pdu.GetString( Key_ICMU_ThirdParty, &strThirdParty );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	pdu.GetString( Key_ICMU_TimeStamp, &strTimeStamp );
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_DEVICEEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );
	
	evtTemp.u.evtdevice.ChannelID = nDeviceRefID;
	StrAssign(evtTemp.u.evtdevice.MonitorParty, strDeviceID.c_str() );
	evtTemp.u.evtdevice.DeviceType = nDeviceType;
	evtTemp.u.evtdevice.DeviceState = nDeviceState;
	evtTemp.u.evtdevice.CallID = nCallID;
	evtTemp.u.evtdevice.OldCallID = nOldCallID;
	evtTemp.u.evtdevice.SecOldCallID = nSecOldCallID;
	StrAssign(evtTemp.u.evtdevice.OtherParty, strOtherParty.c_str() );
	StrAssign(evtTemp.u.evtdevice.CallingParty, strCallingParty.c_str() );
	StrAssign(evtTemp.u.evtdevice.CalledParty, strCalledParty.c_str() );
	StrAssign(evtTemp.u.evtdevice.ThirdParty, strThirdParty.c_str() );
	evtTemp.u.evtdevice.Reason = nReason;
	StrAssign(evtTemp.u.evtdevice.TimeStamp, strTimeStamp.c_str() );

	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnEvtDeviceEvent] evttype<%s>, nChannelID<%d>, strDeviceID<%s>, nCallID<%d>.",
		GetEvtName(evtTemp.evttype).c_str(), nDeviceRefID, strDeviceID.c_str(), nCallID );

	return nRet;
}

/****************************************************************************
函 数 名: OnEvtRouteEvent
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnEvtRouteEvent(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Uint   nDeviceRefID;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceState;
	Smt_Uint   nRouteID;
	Smt_Uint   nCallID;
	Smt_Uint   nOldCallID;
	Smt_String strOtherParty;
	Smt_String strCallingParty;
	Smt_String strCalledParty;
	Smt_Uint   nReason;
	Smt_String strTimeStamp;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_DeviceState, &nDeviceState );
	pdu.GetUint  ( Key_ICMU_RouteID, &nRouteID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetUint  ( Key_ICMU_OldCallID, &nOldCallID );
	pdu.GetString( Key_ICMU_OtherParty, &strOtherParty );
	pdu.GetString( Key_ICMU_CallingParty, &strCallingParty );
	pdu.GetString( Key_ICMU_CalledParty, &strCalledParty );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	pdu.GetString( Key_ICMU_TimeStamp, &strTimeStamp );

	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_ROUTEEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );
	
	evtTemp.u.evtroute.ChannelID = nDeviceRefID;
	StrAssign(evtTemp.u.evtroute.DeviceID, strDeviceID.c_str() );
	evtTemp.u.evtroute.RouteID = nRouteID;
	evtTemp.u.evtroute.CallID = nCallID;
	evtTemp.u.evtroute.OldCallID = nOldCallID;
	StrAssign(evtTemp.u.evtroute.OtherParty, strOtherParty.c_str() );
	StrAssign(evtTemp.u.evtroute.CallingParty, strCallingParty.c_str() );
	StrAssign(evtTemp.u.evtroute.CalledParty, strCalledParty.c_str() );
	evtTemp.u.evtroute.Reason = nReason;
	StrAssign(evtTemp.u.evtroute.TimeStamp, strTimeStamp.c_str() );

	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnEvtRouteEvent] evttype<%s>, nChannelID<%d>, strDeviceID<%s>, nRouteID<%d>, nCallID<%d>, nReason<%d>.",
		GetEvtName(evtTemp.evttype).c_str(), nDeviceRefID, strDeviceID.c_str(), nRouteID, nCallID, nReason );

	return nRet;
}

/****************************************************************************
函 数 名: OnEvtMediaEvent
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnEvtMediaEvent(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Uint   nCallID;
	Smt_String strSource;
	Smt_String strDestination;
	Smt_String strFileName;
	Smt_String strDTMFDigits;
	Smt_String strMessage;
	Smt_Uint   nTimeLen;
	Smt_Uint   nReason;
	Smt_String strTimeStamp;

	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetString( Key_ICMU_Destination, &strDestination );
	pdu.GetString( Key_ICMU_FileName, &strFileName );
	pdu.GetString( Key_ICMU_DTMFDigits, &strDTMFDigits );
	pdu.GetString( Key_ICMU_Message, &strMessage );
	pdu.GetUint  ( Key_ICMU_TimeLen, &nTimeLen );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	pdu.GetString( Key_ICMU_TimeStamp, &strTimeStamp );

	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_MEDIAEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );

	evtTemp.u.evtmedia.CallID = nCallID;
	StrAssign(evtTemp.u.evtmedia.CallingParty, strSource.c_str() );
	StrAssign(evtTemp.u.evtmedia.CalledParty, strDestination.c_str() );
	LStrAssign(evtTemp.u.evtmedia.FileName, strFileName.c_str() );
	LStrAssign(evtTemp.u.evtmedia.DTMFDigits, strDTMFDigits.c_str() );
	LStrAssign(evtTemp.u.evtmedia.Message, strMessage.c_str() );
	evtTemp.u.evtmedia.TimeLen = nTimeLen;
	evtTemp.u.evtmedia.Reason = nReason;
	StrAssign(evtTemp.u.evtmedia.TimeStamp, strTimeStamp.c_str() );

	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnEvtMediaEvent] evttype<%s>, nCallID<%d>, strFileName<%s>, strDTMFDigits<%s>, nReason<%d>.",
		GetEvtName(evtTemp.evttype).c_str(), nCallID, strFileName.c_str(), strDTMFDigits.c_str(), nReason );

	return nRet;
}

/****************************************************************************
函 数 名: OnEvtDeviceRecordEvent
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnEvtDeviceRecordEvent(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Uint   nDeviceRefID;
	Smt_String strDeviceID;
	Smt_Uint   nCallID;           
	Smt_String strOtherParty;
	Smt_String strCallingParty;  
	Smt_String strCalledParty;         
	Smt_String strFileName;
	Smt_Uint   nTimeLen;
	Smt_Uint   nReason;
	Smt_String strTimeStamp;

	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetString( Key_ICMU_OtherParty, &strOtherParty );
	pdu.GetString( Key_ICMU_CallingParty, &strCallingParty );
	pdu.GetString( Key_ICMU_CalledParty, &strCalledParty );
	pdu.GetString( Key_ICMU_FileName, &strFileName );
	pdu.GetUint  ( Key_ICMU_TimeLen, &nTimeLen );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	pdu.GetString( Key_ICMU_TimeStamp, &strTimeStamp );	

	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_DEVICERECORDEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );

	evtTemp.u.evtdevicerecord.ChannelID = nDeviceRefID;
	evtTemp.u.evtdevicerecord.CallID = nCallID;
	StrAssign(evtTemp.u.evtdevicerecord.OtherParty, strOtherParty.c_str() );
	StrAssign(evtTemp.u.evtdevicerecord.CallingParty, strCallingParty.c_str() );
	StrAssign(evtTemp.u.evtdevicerecord.CalledParty, strCalledParty.c_str() );
	LStrAssign(evtTemp.u.evtdevicerecord.FileName, strFileName.c_str() );
	evtTemp.u.evtdevicerecord.TimeLen = nTimeLen;
	evtTemp.u.evtdevicerecord.Reason = nReason;
	StrAssign(evtTemp.u.evtdevicerecord.TimeStamp, strTimeStamp.c_str() );

	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnEvtDeviceRecordEvent] evttype<%s>, nChannelID<%d>, strDeviceID<%s>, nCallID<%d>, strOtherParty<%s>, strFileName<%s>, nTimeLen<%d>.",
		GetEvtName(evtTemp.evttype).c_str(),nDeviceRefID, strDeviceID.c_str(), nCallID, strOtherParty.c_str(), strFileName.c_str(), nTimeLen );

	return nRet;
}

Smt_Uint TUser::OnEvtMessageReceived(Smt_Pdu& pdu)
{
	Smt_Uint   nRet = Smt_Success;
	Smt_Uint   nDeviceRefID;
	Smt_String strDeviceID;
	Smt_Uint   nCallID;      
	Smt_String strMessage;
	Smt_Uint   nReason;
	Smt_String strTimeStamp;
	
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_CallID, &nCallID );
	pdu.GetString( Key_ICMU_Message, &strMessage );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	pdu.GetString( Key_ICMU_TimeStamp, &strTimeStamp );	
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_DEVICEMESSAGEEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );
	
	evtTemp.u.evtdevicemessage.ChannelID = nDeviceRefID;
	evtTemp.u.evtdevicemessage.CallID = nCallID;
	StrAssign(evtTemp.u.evtdevicemessage.MonitorParty, strDeviceID.c_str() );
	LStrAssign(evtTemp.u.evtdevicemessage.Message, strMessage.c_str() );
	evtTemp.u.evtdevicemessage.Reason = nReason;
	StrAssign(evtTemp.u.evtdevicemessage.TimeStamp, strTimeStamp.c_str() );
	
	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnEvtMessageReceived] evttype<%s>, nChannelID<%d>, strDeviceID<%s>, nCallID<%d>, Message<%s>.",
		GetEvtName(evtTemp.evttype).c_str(),nDeviceRefID, strDeviceID.c_str(), nCallID, strMessage.c_str() );
	
	return nRet;
}

/****************************************************************************
函 数 名: OnCmdAssign
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdAssign(Smt_Uint invokeid, Smt_String deviceid, Smt_Uint devicetype )
{
	Smt_Uint nRet = Smt_Success;

	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_Assign;
	
	pdu.PutUint  ( Key_ICMU_InvokeID, invokeid );
	pdu.PutString( Key_ICMU_DeviceID, deviceid );
	pdu.PutUint  ( Key_ICMU_DeviceType, devicetype );

	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );

	PrintLog(5,"[TUser::CmdAssign] InvokeID<%d>, DeviceID<%s>, DeviceType<%d>, nRet<0x%x>.",
		invokeid, deviceid.c_str(), devicetype, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: OnRespAssign
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnRespAssign(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;

	Smt_Uint   nInvokeID;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nDeviceStatus;
	Smt_String strDeviceIP;
	Smt_Uint   nDevicePort;
	Smt_Uint   nReason;

	pdu.GetUint  ( Key_ICMU_InvokeID, &nInvokeID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_DeviceStatus, &nDeviceStatus );
	pdu.GetString( Key_ICMU_DeviceIP, &strDeviceIP );
	pdu.GetUint  ( Key_ICMU_DevicePort, &nDevicePort );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );

	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_CONFEVENT;
	evtTemp.evttype = CONF_CTCASSIGN;
	evtTemp.u.evtconf.u.confAssign.ChannelID = nDeviceRefID;
	StrAssign(evtTemp.u.evtconf.u.confAssign.DeviceID, strDeviceID.c_str() );
	evtTemp.u.evtconf.u.confAssign.InvokeID = nInvokeID;
//	evtTemp.u.evtconf.u.confAssign.DeviceStatus = nDeviceStatus;
	//2011-06-18 add by caoyj 
	if(nDeviceStatus == CauseRegistered)
	{
		evtTemp.u.evtconf.u.confAssign.DeviceStatus = cmuK_Registered;
	}
	else
	{
		evtTemp.u.evtconf.u.confAssign.DeviceStatus = cmuK_Unregistered;
	}
	StrAssign(evtTemp.u.evtconf.u.confAssign.DeviceIP, strDeviceIP.c_str() );
	evtTemp.u.evtconf.u.confAssign.DevicePort = nDevicePort;
	evtTemp.u.evtconf.u.confAssign.Reason = nReason;

	PutEventQueue( &evtTemp );

	PrintLog(5,"[TUser::OnRespAssign] nInvokeID<%d>, strDeviceID<%s>, nChannelID<%d>, nDeviceStatus<%d>, strDeviceIP<%s>, nDevicePort<%d>, nReason<%d>.",
		nInvokeID, strDeviceID.c_str(), nDeviceRefID, nDeviceStatus, strDeviceIP.c_str(), nDevicePort, nReason );

	return nRet;
}

/****************************************************************************
函 数 名: CmdDeassign
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdDeassign(Smt_Uint channelid)
{
	Smt_Uint nRet = Smt_Success;

	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_Deassign;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );

	PrintLog(5,"[TUser::CmdDeassign] ChannelID<%d>, nRet<0x%x>.",
		channelid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: OnRespCtcResponse
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnRespCtcResponse(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;

	Smt_Uint nDeviceRefID;
	Smt_Uint nReason;
	Smt_Uint nCallID;
	Smt_Uint nRouteID;

	pdu.GetUint(Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint(Key_ICMU_Reason, &nReason );
	pdu.GetUint(Key_ICMU_CallID, &nCallID );
	pdu.GetUint(Key_ICMU_RouteID, &nRouteID );

	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_CONFEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );
	evtTemp.u.evtconf.u.confctcResponse.ChannelID = nDeviceRefID;
	evtTemp.u.evtconf.u.confctcResponse.Reason = nReason;
	evtTemp.u.evtconf.u.confctcResponse.CallID = nCallID;
	evtTemp.u.evtconf.u.confctcResponse.RouteID = nRouteID;

	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnRespCtcResponse] nChannelID<%d>, nReason<%d>, CallID<%d>, RouteID<%d>.",
		nDeviceRefID, nReason, nCallID, nRouteID );

	return nRet;
}

/****************************************************************************
函 数 名: CmdMakeCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdMakeCall(Smt_Uint channelid, Smt_String callednum, Smt_String callernum )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_MakeCall;

	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutString( Key_ICMU_CalledNum, callednum );
	pdu.PutString( Key_ICMU_CallerNum, callernum );
	pdu.PutUint  ( Key_ICMU_Timeout, DEFAULT_MAKECALLTIMEOUT );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdMakeCall] ChannelID<%d>, CalledNum<%s>, CallerNum<%s>, nRet<0x%x>.",
		channelid, callednum.c_str(), callernum.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdHangupCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdHangupCall(Smt_Uint channelid, Smt_Uint callid )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_HangupCall;
	
	pdu.PutUint( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint( Key_ICMU_CallID, callid );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdHangupCall] ChannelID<%d>, CallID<%d>, nRet<0x%x>.",
		channelid, callid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdClearCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdClearCall(Smt_Uint channelid, Smt_Uint callid )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_ClearCall;
	
	pdu.PutUint( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint( Key_ICMU_CallID, callid );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdClearCall] ChannelID<%d>, CallID<%d>, nRet<0x%x>.",
		channelid, callid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdHoldCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdHoldCall(Smt_Uint channelid, Smt_Uint callid )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_HoldCall;
	
	pdu.PutUint( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint( Key_ICMU_CallID, callid );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdHoldCall] ChannelID<%d>, CallID<%d>, nRet<0x%x>.",
		channelid, callid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdRetrieveCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdRetrieveCall(Smt_Uint channelid, Smt_Uint callid )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_RetrieveCall;
	
	pdu.PutUint( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint( Key_ICMU_CallID, callid );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdRetrieveCall] ChannelID<%d>, CallID<%d>, nRet<0x%x>.",
		channelid, callid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdConsultationCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdConsultationCall(Smt_Uint channelid, Smt_Uint callid, Smt_String callednum, Smt_String callernum, Smt_Uint timeout)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_ConsultationCall;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_CallerNum, callernum );
	pdu.PutString( Key_ICMU_CalledNum, callednum );
	pdu.PutUint  ( Key_ICMU_Timeout, timeout );

	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdConsultationCall] ChannelID<%d>, CallID<%d>, CalledNum<%s>, CallerNum<%s>, nRet<0x%x>.",
		channelid, callid, callednum.c_str(), callernum.c_str(), nRet );

	return nRet;
}
/****************************************************************************
函 数 名: CmdConferenceCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdConferenceCall(Smt_Uint channelid, Smt_Uint holdcallid, Smt_Uint activecallid )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_ConferenceCall;
	
	pdu.PutUint( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint( Key_ICMU_CallID, activecallid );
	pdu.PutUint( Key_ICMU_OldCallID, holdcallid );	
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdConferenceCall] ChannelID<%d>, HoldCallID<%d>, ActiveCallID<%d>, nRet<0x%x>.",
		channelid, holdcallid, activecallid, nRet );

	return nRet;
}
/****************************************************************************
函 数 名: CmdTransferCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdTransferCall(Smt_Uint channelid, Smt_Uint holdcallid, Smt_Uint activecallid )
{
	Smt_Uint nRet = Smt_Success;
	
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_TransferCall;
	
	pdu.PutUint( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint( Key_ICMU_CallID, activecallid );
	pdu.PutUint( Key_ICMU_OldCallID, holdcallid );	
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdTransferCall] ChannelID<%d>, HoldCallID<%d>, ActiveCallID<%d>, nRet<0x%x>.",
		channelid, holdcallid, activecallid, nRet );

	return nRet;
}
/****************************************************************************
函 数 名: CmdReconnectCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdReconnectCall(Smt_Uint channelid, Smt_Uint holdcallid, Smt_Uint activecallid )
{
	Smt_Uint nRet = Smt_Success;

	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_ReconnectCall;
	
	pdu.PutUint( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint( Key_ICMU_CallID, activecallid );
	pdu.PutUint( Key_ICMU_OldCallID, holdcallid );	
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdReconnectCall] ChannelID<%d>, HoldCallID<%d>, ActiveCallID<%d>, nRet<0x%x>.",
		channelid, holdcallid, activecallid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdSingleStepTransfer
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdSingleStepTransfer(Smt_Uint channelid, Smt_String callednum, Smt_Uint callid )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_SingleStepTransfer;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_CalledNum, callednum );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdSingleStepTransfer] ChannelID<%d>, CalledNum<%s>, CallID<%d>, nRet<0x%x>.",
		channelid, callednum.c_str(), callid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdSingleStepConference
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdSingleStepConference(Smt_Uint channelid, Smt_String callednum, Smt_Uint callid, Smt_Uint mode )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_SingleStepConference;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutString( Key_ICMU_CalledNum, callednum );
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutUint  ( Key_ICMU_JoinMode, mode );

	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdSingleStepConference] ChannelID<%d>, CalledNum<%s>, CallID<%d>, JoinMode<%d>, nRet<0x%x>.",
		channelid, callednum.c_str(), callid, mode, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdRouteSelected
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdRouteSelected(Smt_Uint channelid, Smt_Uint routeid, Smt_String callednum, Smt_String agentid )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_RouteSelected;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint  ( Key_ICMU_RouteID, routeid );
	pdu.PutString( Key_ICMU_CalledNum, callednum );
	pdu.PutString( Key_ICMU_AgentID, agentid );	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdRouteSelected] ChannelID<%d>, RouteID<%d>, CalledNum<%s>, AgentID<%s>, nRet<0x%x>.",
		channelid, routeid, callednum.c_str(), agentid.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdSendDTMF
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdSendDTMF(Smt_Uint channelid, Smt_Uint callid, Smt_String digits )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_SendDTMF;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_DTMFDigits, digits );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdSendDTMF] ChannelID<%d>, CallerID<%d>, DTMFDigits<%s>, nRet<0x%x>.",
		channelid, callid, digits.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdStartRecord
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdStartRecord(Smt_Uint channelid, Smt_Uint callid, Smt_String filename )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_StartRecord;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_FileName, filename );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdStartRecord] ChannelID<%d>, CallerID<%d>, FileName<%s>, nRet<0x%x>.",
		channelid, callid, filename.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdStopRecord
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdStopRecord(Smt_Uint channelid, Smt_Uint callid )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_StopRecord;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdStopRecord] ChannelID<%d>, CallerID<%d>, nRet<0x%x>.",
		channelid, callid, nRet );

	return nRet;
}

Smt_Uint TUser::CmdSendMessage(Smt_Uint channelid, Smt_Uint callid, Smt_String message )
{
	Smt_Uint nRet = Smt_Success;
	
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_SendMessage;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_Message, message );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdSendMessage] ChannelID<%d>, CallerID<%d>, Message<%s>, nRet<0x%x>.",
		channelid, callid, message.c_str(), nRet );
	
	return nRet;
}

/****************************************************************************
函 数 名: CmdDeviceSnapshot
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdDeviceSnapshot(Smt_Uint channelid)
{
	Smt_Uint nRet = Smt_Success;

	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_DeviceSnapshot;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );

	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdDeviceSnapshot] ChannelID<%d>, nRet<0x%x>.",
		channelid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: OnRespDeviceSnapshot
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnRespDeviceSnapshot(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Uint nDeviceRefID;
	Smt_Uint nReason;
	Smt_Uint nCallID;
	Smt_Uint nCallState;
	Smt_Kvset *pKvs = new Smt_Kvset[MAX_CALLID_ARRAY];
	Smt_Uint   nCount = 0;

	pdu.GetUint(Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint(Key_ICMU_Reason, &nReason );
    pdu.GetKvsetArray( Key_ICMU_CallIDArray, &pKvs, &nCount );

	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_CONFEVENT;
	evtTemp.evttype = CONF_CTCSNAPSHOT;
	
 	evtTemp.u.evtconf.u.confSnapshot.ChannelID = nDeviceRefID;
	evtTemp.u.evtconf.u.confSnapshot.Reason = nReason;
	evtTemp.u.evtconf.u.confSnapshot.SnapshotData.Count = nCount;

	for(Smt_Uint i = 0; i<nCount; i++)
	{
		pKvs[i].GetUint(Key_ICMU_CallID, &nCallID);
		pKvs[i].GetUint(Key_ICMU_CallState, &nCallState);

		evtTemp.u.evtconf.u.confSnapshot.SnapshotData.Info[i].CallID = nCallID;
		evtTemp.u.evtconf.u.confSnapshot.SnapshotData.Info[i].CallState = nCallState;
	}
	
	PutEventQueue( &evtTemp );
	
 	PrintLog(5,"[TUser::OnRespDeviceSnapshot] nChannelID<%d>, nCount<%d>, nReason<%d>.",
 		nDeviceRefID, nCount, nReason );

	if(pKvs != NULL)
	{
		delete [] pKvs;
		pKvs = NULL;
	}

	return nRet;
}

/****************************************************************************
函 数 名: CmdSetData
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdSetData(Smt_Uint channelid, Smt_Uint callid, Smt_String key, Smt_String value )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_SetData;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_DataKey, key );
	pdu.PutString( Key_ICMU_DataValue, value );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdSetData] ChannelID<%d>, CallerID<%d>, DataKey<%s>, DataValue<%s>, nRet<0x%x>.",
		channelid, callid, key.c_str(), value.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdGetData
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdGetData(Smt_Uint channelid, Smt_Uint callid, Smt_String key )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_DeviceStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_GetData;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_DataKey, key );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdGetData] ChannelID<%d>, CallerID<%d>, DataKey<%s>, nRet<0x%x>.",
		channelid, callid, key.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: OnRespGetData
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnRespGetData(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Uint nDeviceRefID;
	Smt_String strDataValue;
	Smt_Uint nReason;
	
	pdu.GetUint  (Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetString(Key_ICMU_DataValue, &strDataValue );
	pdu.GetUint  (Key_ICMU_Reason, &nReason );
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_CONFEVENT;
	evtTemp.evttype = CONF_CTCGETDATAVALUE;
	
	evtTemp.u.evtconf.u.confGetDataValue.ChannelID = nDeviceRefID;
	LStrAssign(evtTemp.u.evtconf.u.confGetDataValue.DataValue, strDataValue.c_str() );
	evtTemp.u.evtconf.u.confGetDataValue.Reason = nReason;
	
	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnRespGetData] nChannelID<%d>, DataValue<%s>, nReason<%d>.",
		nDeviceRefID, strDataValue.c_str(), nReason );

	return nRet;
}

/****************************************************************************
函 数 名: CmdAssignEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdAssignEx(Smt_Uint invokeid, Smt_String deviceid, Smt_Uint devicetype )
{
	Smt_Uint nRet = Smt_Success;

	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_AssignEx;
	
	pdu.PutUint  ( Key_ICMU_InvokeID, invokeid );
	pdu.PutString( Key_ICMU_DeviceID, deviceid );
	pdu.PutUint  ( Key_ICMU_DeviceType, devicetype );

	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );

	PrintLog(5,"[TUser::CmdAssignEx] InvokeID<%d>, DeviceID<%s>, DeviceType<%d>, nRet<0x%x>.",
		invokeid, deviceid.c_str(), devicetype, nRet );

	return nRet;
}
/****************************************************************************
函 数 名: OnRespAssign
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnRespAssignEx(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;

	Smt_Uint   nInvokeID;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceRefID;
	Smt_Uint   nReason;

	pdu.GetUint  ( Key_ICMU_InvokeID, &nInvokeID );
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_DeviceRefID, &nDeviceRefID );
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );

	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_CONFEVENT;
	evtTemp.evttype = CONF_GC_OPEN;

	evtTemp.u.evtconf.u.confgc_open.ChannelID = nDeviceRefID;
	evtTemp.u.evtconf.u.confgc_open.InvokeID = nInvokeID;
	evtTemp.u.evtconf.u.confgc_open.Reason = nReason;

	PutEventQueue( &evtTemp );

	PrintLog(5,"[TUser::OnRespAssignEx] nInvokeID<%d>, strDeviceID<%s>, nChannelID<%d>, nReason<%d>.",
		nInvokeID, strDeviceID.c_str(), nDeviceRefID, nReason );

	return nRet;
}

/****************************************************************************
函 数 名: CmdDeassignEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdDeassignEx(Smt_Uint channelid)
{
	Smt_Uint nRet = Smt_Success;

	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_DeassignEx;
	
	pdu.PutUint  ( Key_ICMU_DeviceRefID, channelid );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );

	PrintLog(5,"[TUser::CmdDeassignEx] ChannelID<%d>, nRet<0x%x>.",
		channelid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: OnRespGcResponse
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnRespGcResponse(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;

	Smt_Uint nCallID;
	Smt_Uint nReason;

	pdu.GetUint(Key_ICMU_CallID, &nCallID );
	pdu.GetUint(Key_ICMU_Reason, &nReason );
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_CONFEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );
	evtTemp.u.evtconf.u.confgcResponse.CallID = nCallID;
	evtTemp.u.evtconf.u.confgcResponse.Reason = nReason;
	
	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnRespGcResponse] CallID<%d>, nReason<%d>.",
		nCallID, nReason );

	return nRet;
}

/****************************************************************************
函 数 名: CmdMakeCallEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdMakeCallEx(Smt_Uint invokeid, Smt_String deviceid, Smt_String callednum, Smt_String callernum, Smt_Uint timeout )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_MakeCallEx;
	
	pdu.PutUint  ( Key_ICMU_InvokeID, invokeid );
	pdu.PutString( Key_ICMU_DeviceID, deviceid );
	pdu.PutString( Key_ICMU_CalledNum, callednum );
	pdu.PutString( Key_ICMU_CallerNum, callernum );
	pdu.PutUint  ( Key_ICMU_Timeout, timeout );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdMakeCallEx] gc_makecall, InvokeID<%d>, DeviceID<%s>, CalledNum<%s>, CallerNum<%s>, nRet<0x%x>.",
		invokeid, deviceid.c_str(), callednum.c_str(), callernum.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: OnRespMakeCallEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnRespMakeCallEx(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Uint nInvokeID;
	Smt_Uint nReason;
	Smt_Uint nCallID;
	
	pdu.GetUint(Key_ICMU_InvokeID, &nInvokeID );
	pdu.GetUint(Key_ICMU_Reason, &nReason );
	pdu.GetUint(Key_ICMU_CallID, &nCallID );
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_CONFEVENT;
	evtTemp.evttype = CONF_GC_MAKECALL;
	
	evtTemp.u.evtconf.u.confgc_makecall.InvokeID = nInvokeID;
	evtTemp.u.evtconf.u.confgc_makecall.Reason = nReason;
	evtTemp.u.evtconf.u.confgc_makecall.CallID = nCallID;
	
	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnRespMakeCallEx] nInvokeID<%d>, nCallID<%d>, nReason<%d>.",
			nInvokeID, nCallID, nReason );

	return nRet;
}

/****************************************************************************
函 数 名: CmdAnswerCallEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdAnswerCallEx(Smt_Uint callid )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_AnswerCallEx;
	
	pdu.PutUint( Key_ICMU_CallID, callid );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdAnswerCallEx] gc_answercall, CallID<%d>, nRet<0x%x>.",
		callid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdHangupCallEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdHangupCallEx(Smt_Uint callid )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_HangupCallEx;
	
	pdu.PutUint( Key_ICMU_CallID, callid );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdHangupCallEx] gc_hangupcall, CallID<%d>, nRet<0x%x>.",
		callid, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdSingleStepTransferEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdSingleStepTransferEx(Smt_Uint callid, Smt_String callednum )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_SingleStepTransferEx;
	
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_CalledNum, callednum );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdSingleStepTransferEx] gc_blindtransfer, CallID<%d>, CalledNum<%s>, nRet<0x%x>.",
		callid, callednum.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdPlayFile
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdPlayFile(Smt_Uint callid, Smt_String filename, Smt_String escapedigits )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_PlayFile;
	
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_FileName, filename );
	pdu.PutString( Key_ICMU_EscapeDigit, escapedigits );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdPlayFile] dx_play, CallID<%d>, FileName<%s>, EscapeDigit<%s>, nRet<0x%x>.",
		callid, filename.c_str(), escapedigits.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdAddFileList
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdAddFileList(Smt_Uint callid, Smt_String filename )
{
	Smt_Uint nRet = Smt_Success;
	TPlayList* pPlayList = NULL;
	if( m_PlayList.Lookup(callid, pPlayList) != Smt_Success )
	{
		pPlayList = new TPlayList( callid );
		pPlayList->AddFile( filename );
		m_PlayList.SetAt( callid, pPlayList );
	}
	else
	{
		pPlayList->AddFile( filename );
	}

	PrintLog(5,"[TUser::CmdAddFileList] nCallID<%d>, FileName<%s>.",
		callid, filename.c_str() );

	return nRet;
}

/****************************************************************************
函 数 名: CmdPlayFileList
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdPlayFileList(Smt_Uint callid, Smt_String escapedigits )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_PlayFileList;
	
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_EscapeDigit, escapedigits );

	TPlayList* pPlayList = NULL;
	Smt_Uint nCount = 0;
	Smt_Kvset kvs[MAX_FILENAME_ARRAY];

	if( m_PlayList.Lookup(callid, pPlayList ) == Smt_Success )
	{
		for(nCount=0; nCount<pPlayList->m_PlayIndex; nCount++)
		{
			kvs[nCount].PutString( Key_ICMU_FileName, pPlayList->m_PlayList[nCount] );
		}

		pdu.PutKvsetArray( Key_ICMU_FileNameArray, kvs, pPlayList->m_PlayIndex );

		pPlayList->Clear();
	}

	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdPlayFileList] dx_playiottdata, CallID<%d>, EscapeDigit<%s>, FileCount<%d>, nRet<0x%x>.",
		callid, escapedigits.c_str(), nCount, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdGetDigits
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdGetDigits(Smt_Uint callid, Smt_Uint timeout, Smt_Uint maxcount )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_GetDigits;
	
	pdu.PutUint( Key_ICMU_CallID, callid );
	pdu.PutUint( Key_ICMU_Timeout, timeout );
	pdu.PutUint( Key_ICMU_MaxDigitsCount, maxcount );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdGetDigits] dx_getdig, CallID<%d>, Timeout<%d>, MaxDigitsCount<%d>, nRet<0x%x>.",
		callid, timeout, maxcount, nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdSendDTMFEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdSendDTMFEx(Smt_Uint callid, Smt_String digits )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_SendDTMFEx;
	
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_DTMFDigits, digits );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdSendDTMFEx] dx_dial, CallID<%d>, DTMFDigits<%s>, nRet<0x%x>.",
		callid, digits.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdRecordEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdRecordEx(Smt_Uint callid, Smt_String filename, Smt_String escapedigits, Smt_Uint timeout, Smt_Uint silence )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_RecordEx;
	
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_FileName, filename );
	pdu.PutString( Key_ICMU_EscapeDigit, escapedigits );
	pdu.PutUint  ( Key_ICMU_Timeout, timeout );
	pdu.PutUint  ( Key_ICMU_Silence, silence );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdRecordEx] dx_rec, CallID<%d>, FileName<%s>, nRet<0x%x>.",
		callid, filename.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdSendFax
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdSendFax(Smt_Uint channelid,Smt_Uint callid,Smt_String message)
{
	Smt_Uint nRet = Smt_Success;
	return nRet;
}

/****************************************************************************
函 数 名: CmdReceiveFax
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdReceiveFax(Smt_Uint callid )
{
	Smt_Uint nRet = Smt_Success;
	return nRet;
}

/****************************************************************************
函 数 名: CmdSubscribeCallEvent
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdSubscribeCallEvent( )
{
	Smt_Uint nRet = Smt_Success;
	
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_SubscribeCallEvent;
	
	pdu.PutString( Key_ICMU_SubscriberName, g_pCMUDLLCfg->m_ServerName );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdSubscribeCallEvent] SubscriberName<%s>, nRet<0x%x>.", 
		g_pCMUDLLCfg->m_ServerName.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: CmdUnsubscribeCallEvent
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::CmdUnsubscribeCallEvent( )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_UnsubscribeCallEvent;
	
	pdu.PutString( Key_ICMU_SubscriberName, g_pCMUDLLCfg->m_ServerName );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdUnsubscribeCallEvent] SubscriberName<%s>, nRet<0x%x>.", 
		g_pCMUDLLCfg->m_ServerName.c_str(), nRet );

	return nRet;
}

/****************************************************************************
函 数 名: OnEvtLinkUp
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnEvtLinkUp(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Uint   nReason;
	Smt_String strTimeStamp;

	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	pdu.GetString( Key_ICMU_TimeStamp, &strTimeStamp );
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_SYSTEMEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );
	
	evtTemp.u.evtsystem.u.evtLinkUp.Reason = nReason;
	StrAssign(evtTemp.u.evtsystem.u.evtLinkUp.TimeStamp, strTimeStamp.c_str() );
	
	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnEvtLinkUp] nReason<%d>.",	nReason );

	return nRet;
}

/****************************************************************************
函 数 名: OnEvtLinkDown
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnEvtLinkDown(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Uint   nReason;
	Smt_String strTimeStamp;
	
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	pdu.GetString( Key_ICMU_TimeStamp, &strTimeStamp );
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_SYSTEMEVENT;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );
	
	evtTemp.u.evtsystem.u.evtLinkDown.Reason = nReason;
	StrAssign(evtTemp.u.evtsystem.u.evtLinkDown.TimeStamp, strTimeStamp.c_str() );
	
	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnEvtLinkDown] nReason<%d>.",	nReason );

	return nRet;
}

/****************************************************************************
函 数 名: OnEvtQueryLinkTimer
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TUser::OnEvtQueryLinkTimer()
{
	CmdQueryLinkState();
	return Smt_Success;
}

Smt_Uint TUser::OnEvtTTSPlayEnd(Smt_Pdu& pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Uint   nReason;
	Smt_String strTimeStamp;
	
	pdu.GetUint  ( Key_ICMU_Reason, &nReason );
	pdu.GetString( Key_ICMU_TimeStamp, &strTimeStamp );
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_TTSPLAYEND;
	evtTemp.evttype = GetEvtType( pdu.m_MessageID );
	
	evtTemp.u.evtsystem.u.evtLinkDown.Reason = nReason;
	StrAssign(evtTemp.u.evtsystem.u.evtLinkDown.TimeStamp, strTimeStamp.c_str() );
	
	PutEventQueue( &evtTemp );
	PrintLog(3,"[TUser::OnEvtTTSPlayEnd] OnEvtTTSPlayEnd." );
	return Smt_Success;
}

Smt_Uint TUser::CmdQueryLinkState( )
{
	if(m_DeviceStateGOR>0)
	{
		Smt_Pdu pdu;
		pdu.m_Sender = GetGOR();
		pdu.m_Receiver = m_DeviceStateGOR;
		pdu.m_Status = Smt_Success;
		pdu.m_MessageID = Cmd_ICMU_QueryLinkState;
		
		if( pdu.m_Receiver > 0 ) PostMessage( pdu );
		
		//PrintLog(3,"[TUser::CmdQueryLinkState] Cmd Query Link State." );
	}
	return Smt_Success;
}

Smt_Uint TUser::OnRespQueryLinkState(Smt_Pdu& pdu)
{
	Smt_DateTime dtNow;
	Smt_Uint nReason;
	Smt_Uint nLinkState;

	pdu.GetUint( Key_ICMU_Reason, &nReason );
	pdu.GetUint( Key_ICMU_LinkState, &nLinkState);
	
//	PrintLog(5, "[TUser::OnRespQueryLinkState] Resp Query Link LastLinkState<%d>,CurLinkState<%d>.", m_LastLinkState ,nLinkState);

	if( nLinkState != m_LastLinkState )
	{
		m_LastLinkState = nLinkState;

		if( nLinkState == St_LinkUpState )
		{
			pdu.m_MessageID = Evt_ICMU_LinkUp;
			pdu.PutUint  ( Key_ICMU_Reason, nReason );
			pdu.PutString( Key_ICMU_TimeStamp, dtNow.FormatString());
			OnEvtLinkUp( pdu );
		}
		else // nLinkState == St_LinkDownState
		{
			pdu.m_MessageID = Evt_ICMU_LinkDown;
			pdu.PutUint  ( Key_ICMU_Reason, nReason );
			pdu.PutString( Key_ICMU_TimeStamp, dtNow.FormatString());
			OnEvtLinkDown( pdu );
		}

		PrintLog(5, "[TUser::OnRespQueryLinkState] Resp Query Link State<%d>.", nLinkState );

	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: CmdDial
参    数:
返回数值: 
功能描述: 共振
*****************************************************************************/
Smt_Uint TUser::CmdDial(Smt_Uint callid, Smt_String callednum, Smt_Uint timeout )
{
	Smt_Uint nRet = Smt_Success;

	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_Dial;
	
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_CalledNum, callednum );
	pdu.PutUint  ( Key_ICMU_Timeout, timeout );

	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdDial] gc_dial, CallID<%d>, CalledNum<%s>, Timeout<%d>, nRet<0x%x>.",
		callid, callednum.c_str(), timeout, nRet );

	return nRet;
}

Smt_Uint TUser::CmdSendMessageEx(Smt_Uint callid, Smt_String message )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_SendMessageEx;
	
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_Message, message );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdSendMessageEx] dx_sendmessage, CallID<%d>, Message<%s>, nRet<0x%x>.",
		callid, message.c_str(), nRet );

	return nRet;
}

Smt_Uint TUser::CmdSetSIPHeader(Smt_Uint callid, Smt_String headerkey, Smt_String headervalue )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_SetSIPHeader;
	
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_DataKey, headerkey );
	pdu.PutString( Key_ICMU_DataValue, headervalue );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdSetSIPHeader] dx_setsipheader, CallID<%d>, HeaderKey<%s>, HeaderValue<%s>, nRet<0x%x>.",
		callid, headerkey.c_str(), headervalue.c_str(), nRet );
	
	return nRet;
}

Smt_Uint TUser::CmdGetSIPHeader(Smt_Uint callid, Smt_String headerkey )
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_GetSIPHeader;
	
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_DataKey, headerkey );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdGetSIPHeader] dx_getsipheader, CallID<%d>, HeaderKey<%s>, nRet<0x%x>.",
		callid, headerkey.c_str(), nRet );
	
	return nRet;
}

Smt_Uint TUser::OnRespGetSIPHeader(Smt_Pdu& pdu)
{
	Smt_Uint   nRet = Smt_Success;
	Smt_Uint   nCallID;
	Smt_String strDataKey;
	Smt_String strDataValue;
	Smt_Uint   nReason;

	pdu.GetUint  (Key_ICMU_CallID, &nCallID );
	pdu.GetString(Key_ICMU_DataKey, &strDataKey );
	pdu.GetString(Key_ICMU_DataValue, &strDataValue );
	pdu.GetUint  (Key_ICMU_Reason, &nReason );
	
	// convert to Csta_Event
	Csta_Event evtTemp;
	ACE_OS::memset( &evtTemp, '\0', sizeof(Csta_Event) );
	
	evtTemp.evtclass = CSTA_CONFEVENT;
	evtTemp.evttype = CONF_DX_GETSIPHEADER;
	evtTemp.u.evtconf.u.confdx_getsipheader.CallID = nCallID;
	LStrAssign(evtTemp.u.evtconf.u.confdx_getsipheader.HeaderKey, strDataKey.c_str() );
	LStrAssign(evtTemp.u.evtconf.u.confdx_getsipheader.HeaderValue, strDataValue.c_str() );
	evtTemp.u.evtconf.u.confdx_getsipheader.Reason = nReason;
	
	PutEventQueue( &evtTemp );
	
	PrintLog(5,"[TUser::OnRespGetSIPHeader] nCallID<%d>, strDataKey<%s>, strDataValue<%s>, nReason<%d>.",
		nCallID, strDataKey.c_str(), strDataValue.c_str(), nReason );
	
	return nRet;
}

Smt_Uint TUser::CmdTTSPlay(Smt_Uint callid,Smt_String message)
{
	Smt_Uint nRet = Smt_Success;
	Smt_Pdu pdu;
	pdu.m_Sender = GetGOR();
	pdu.m_Receiver = m_CallStateGOR;
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Cmd_ICMU_TTSPlay;
	
	pdu.PutUint  ( Key_ICMU_CallID, callid );
	pdu.PutString( Key_ICMU_ApplicationOptions, message );
	
	if( pdu.m_Receiver > 0 ) nRet = PostMessage( pdu );
	
	PrintLog(5,"[TUser::CmdTTSPlay] CallerID<%d>, Options<%s>, nRet<0x%x>.", callid, message.c_str(), nRet );
	
	return nRet;
}

// #end file
