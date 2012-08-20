//***************************************************************************
// TConnectionState.cpp : implementation file
//***************************************************************************

#include "TConnectionState.h"
#include "TASTConnector.h"
#include "TConfig.h"
#include "asterisk-causes.h"
#include "Smt_XML.h"

#define SIP_MESSAGE_BUFF_LEN 500

TConnectionState*  g_pConnState = NULL;

/////////////////////////////////////////////////////////////////////////////
// TTrunkGroup code
TConnectionState::TTrunkGroup::TTrunkGroup()
{
	m_TrunkID="";
	m_TrunkName="";
	m_TrunkType = 0;
	m_TACCode = "";
	m_CircuitCode = "";
	m_ExtnDN = "";
	m_CurrID = 0;
};

TConnectionState::TTrunkGroup::~TTrunkGroup()
{
	TTrunkDN* pTrunkDN = NULL;
	while( !m_TrunkDNMgr.IsEmpty() )
	{
		pTrunkDN = m_TrunkDNMgr.GetTail();
		if(pTrunkDN != NULL)
		{	
			delete pTrunkDN;
			pTrunkDN = NULL;
		}
	}
}

Smt_String TConnectionState::TTrunkGroup::GetDahdiTrunkDn(Smt_Uint circuitcode)
{
	Smt_String strRetDN = "";
	TTrunkDN* pTrunkDN = NULL;

	TTrunkDNList::Smt_Iter iter(m_TrunkDNMgr);
	while (!iter.done ())
	{
		pTrunkDN = iter.next();
		if ( pTrunkDN->m_CircuitCode == circuitcode )
		{
			strRetDN = pTrunkDN->m_ExtenDN;
			pTrunkDN->m_State = ST_TRUNKDN_BUSY;

			if( pTrunkDN->m_TimerID != 0 )
			{
				g_pConnState->ClearTimer( pTrunkDN->m_TimerID );
				pTrunkDN->m_TimerID = 0;
			}
			
			break;
		}
		iter++;
	} 

	g_pConnState->PrintLog(5,"[TTrunkGroup::GetDahdiTrunkDn] DeviceID<%s>, TrunkID<%s>, TrunkName<%s>, CircuitCode<%d>.",
		strRetDN.c_str(), m_TrunkID.c_str(), m_TrunkName.c_str(), circuitcode );

	return strRetDN;
}

Smt_String TConnectionState::TTrunkGroup::AllocateDahdiTrunk()
{
	Smt_String strRetCode = "";
	Smt_String strRetDN = "";
	TTrunkDN* pTrunkDN = NULL;

	Smt_Bool bOk = Smt_BoolFALSE;
	
	TTrunkDNList::Smt_Iter iter(m_TrunkDNMgr);
	while (!iter.done ())
	{
		pTrunkDN = iter.next();
		
		if( m_CurrID >= ACE_OS::atoi(pTrunkDN->m_ExtenDN.c_str()) )
		{
			iter++;
			continue;
		}
		
		if( pTrunkDN->m_State == ST_TRUNKDN_BUSY )
		{
			iter++;
			continue;
		}
		
		strRetDN = pTrunkDN->m_ExtenDN;
		strRetCode = HLFormatStr("%d",pTrunkDN->m_CircuitCode);
		m_CurrID = ACE_OS::atoi(strRetDN.c_str());
		pTrunkDN->m_State = ST_TRUNKDN_LOCKED;
		pTrunkDN->m_TimerID = g_pConnState->SetSingleTimer(DEFAULT_TRUNKLOCKTIMELEN, Evt_ICMU_TrunkDnLockedTimer, m_CurrID );
		bOk = Smt_BoolTRUE;			
		break;
		
		iter++;
	} 
	
	if( bOk == Smt_BoolFALSE )  // 重新开始分配
	{
		m_CurrID = 0;
		
		TTrunkDNList::Smt_Iter iter1(m_TrunkDNMgr);
		while (!iter1.done ())
		{
			pTrunkDN = iter1.next();
			
			if( pTrunkDN->m_State == ST_TRUNKDN_BUSY )
			{
				iter1++;
				continue;
			}
			
			strRetDN = pTrunkDN->m_ExtenDN;
			strRetCode = HLFormatStr("%d",pTrunkDN->m_CircuitCode);
			m_CurrID = ACE_OS::atoi(strRetDN.c_str());
			pTrunkDN->m_State = ST_TRUNKDN_LOCKED;	
			pTrunkDN->m_TimerID = g_pConnState->SetSingleTimer(MAX_ACTIONTIMERLEN, Evt_ICMU_TrunkDnLockedTimer, m_CurrID );
			break;
			
			iter1++;
		} 
	}
	
	g_pConnState->PrintLog(5,"[TTrunkGroup::AllocateDahdiTrunk] DeviceID<%s>, CircuitCode<%s>, TrunkID<%s>, TrunkName<%s>.",
		strRetDN.c_str(), strRetCode.c_str(), m_TrunkID.c_str(), m_TrunkName.c_str() );

	return strRetCode;
}

Smt_String TConnectionState::TTrunkGroup::AllocateSipTrunk()
{
	Smt_String strRetDN = "";
	TTrunkDN* pTrunkDN = NULL;
	Smt_Bool bOk = Smt_BoolFALSE;
	
	TTrunkDNList::Smt_Iter iter(m_TrunkDNMgr);
	while (!iter.done ())
	{
		pTrunkDN = iter.next();
		
		if( m_CurrID >= ACE_OS::atoi(pTrunkDN->m_ExtenDN.c_str()) )
		{
			iter++;
			continue;
		}
		
		if( pTrunkDN->m_State == ST_TRUNKDN_BUSY )
		{
			iter++;
			continue;
		}
		
		strRetDN = pTrunkDN->m_ExtenDN;
		m_CurrID = ACE_OS::atoi(strRetDN.c_str());
		pTrunkDN->m_State = ST_TRUNKDN_BUSY;
		bOk = Smt_BoolTRUE;			
		break;
		
		iter++;
	} 
	
	if( bOk == Smt_BoolFALSE )  // 重新开始分配
	{
		m_CurrID = 0;
		
		TTrunkDNList::Smt_Iter iter1(m_TrunkDNMgr);
		while (!iter1.done ())
		{
			pTrunkDN = iter1.next();
			
			if( pTrunkDN->m_State == ST_TRUNKDN_BUSY )
			{
				iter1++;
				continue;
			}
			
			strRetDN = pTrunkDN->m_ExtenDN;
			m_CurrID = ACE_OS::atoi(strRetDN.c_str());
			pTrunkDN->m_State = ST_TRUNKDN_BUSY;
			break;
			
			iter1++;
		} 
	}

	g_pConnState->PrintLog(5,"[TTrunkGroup::AllocateSipTrunk] DeviceID<%s>, TrunkID<%s>, TrunkName<%s>.",
		strRetDN.c_str(), m_TrunkID.c_str(), m_TrunkName.c_str() );
	return strRetDN;
}

Smt_Uint TConnectionState::TTrunkGroup::FreeDevice(Smt_String dn)
{
	Smt_Uint nRetVal = Smt_Fail;
	TTrunkDN* pTrunkDN = NULL;

	TTrunkDNList::Smt_Iter iter(m_TrunkDNMgr);
	while (!iter.done ())
	{
		pTrunkDN = iter.next();
		if ( pTrunkDN->m_ExtenDN == dn )
		{
			pTrunkDN->m_State = ST_TRUNKDN_IDLE;
			nRetVal = Smt_Success;
			break;
		}
		iter++;
	} 

	g_pConnState->PrintLog(5,"[TTrunkGroup::FreeDevice] DeviceID<%s>, TrunkID<%s>, TrunkName<%s>, nRetVal<%d>.",
		dn.c_str(), m_TrunkID.c_str(), m_TrunkName.c_str(), nRetVal );

	return nRetVal;
}

Smt_Uint TConnectionState::TTrunkGroup::FindDevice(Smt_String dn)
{
	Smt_Uint nRetVal = Smt_Fail;
	TTrunkDN* pTrunkDN = NULL;
	
	TTrunkDNList::Smt_Iter iter(m_TrunkDNMgr);
	while (!iter.done ())
	{
		pTrunkDN = iter.next();
		if ( pTrunkDN->m_ExtenDN == dn )
		{
			nRetVal = Smt_Success;
			break;
		}
		iter++;
	} 
	
	return nRetVal;
}

Smt_Uint TConnectionState::TTrunkGroup::GetDeviceState(Smt_String dn)
{
	Smt_Uint nRetVal = ST_TRUNKDN_IDLE;
	TTrunkDN* pTrunkDN = NULL;
	
	TTrunkDNList::Smt_Iter iter(m_TrunkDNMgr);
	while (!iter.done ())
	{
		pTrunkDN = iter.next();
		if ( pTrunkDN->m_ExtenDN == dn )
		{
			nRetVal = pTrunkDN->m_State;
			break;
		}
		iter++;
	} 
	
	return nRetVal;
}

/////////////////////////////////////////////////////////////////////////////
// TConnectionState code
TConnectionState::TConnectionState( Smt_String name, 
						  Smt_Uint loglevel, Smt_String logpath, 
						  Smt_Server* pserver )
: Smt_StateService( name, loglevel, logpath, pserver )
{	
	m_UniqueIndex = 0;
	m_ActionIndex = 0;
	m_AmiHandle = 0;
	m_CallStateGOR = 0;
	m_DeviceStateGOR = 0;
}

TConnectionState::~TConnectionState()
{
	// Destroy ast-connector
	if( g_pAstConnector != NULL )
	{
		delete g_pAstConnector;
		g_pAstConnector = NULL;
	}

	ClearMapInfo();

	// TDeviceMap
	TAssignedDevice* pDevice = NULL;
	TDeviceMap::ITERATOR iter2( m_DeviceMgr );
	for (TDeviceMap::ENTRY *entry2 = 0;
		iter2.next (entry2) != 0;
		iter2.advance ())
	{
		pDevice = entry2->int_id_;
		
		if(pDevice != NULL)
		{	
			delete pDevice;
			pDevice = NULL;
		}
	}	
	m_DeviceMgr.RemoveAll();
	
	// TTrunkGroupMap
	TTrunkGroup* pTrunkGroup = NULL;
	TTrunkGroupMap::ITERATOR iter3( m_TrunkGroupMgr );
	for (TTrunkGroupMap::ENTRY *entry3 = 0;
	iter3.next (entry3) != 0;
	iter3.advance ())
	{
		pTrunkGroup = entry3->int_id_;
		
		if(pTrunkGroup != NULL)
		{	
			delete pTrunkGroup;
			pTrunkGroup = NULL;
		}
	}	
	m_TrunkGroupMgr.RemoveAll();

}

Smt_Uint TConnectionState::ClearMapInfo()
{
	// TConnectionMap
	TConnection* pConnection = NULL;
	TConnectionMap::ITERATOR iter0( m_ConnectionMgr );
	for (TConnectionMap::ENTRY *entry0 = 0;
		iter0.next (entry0) != 0;
		iter0.advance ())
	{
		pConnection = entry0->int_id_;
		
		if(pConnection != NULL)
		{	
			delete pConnection;
			pConnection = NULL;
		}
	}	
	m_ConnectionMgr.RemoveAll();
	
	// TActionMap
	TActionPdu* pActPdu = NULL;
	TActionMap::ITERATOR iter1( m_ActMgr );
	for (TActionMap::ENTRY *entry1 = 0;
		iter1.next (entry1) != 0;
		iter1.advance ())
	{
		pActPdu = entry1->int_id_;
		
		if(pActPdu != NULL)
		{	
			delete pActPdu;
			pActPdu = NULL;
		}
	}	
	m_ActMgr.RemoveAll();

	return Smt_Success;
}

Smt_Uint TConnectionState::OnUserOnline(Smt_Uint sender, Smt_String sendername)
{
	PrintLog(5,"[TConnectionState::OnUserOnline ] sender<0x%x>, sendername<%s>.", sender, sendername.c_str());

	if( sendername == Smt_String(CMU_CALLSTATENAME) )
	{
		m_CallStateGOR = sender;

		if( g_pAstConnector == NULL )
		{
			// Init Data
			ReadXMLConfig();

			// Create ast-connector
			Smt_String strCurrPath = HLGetModuleFilePath();
			g_pCfg->m_ASTConnLog = strCurrPath + g_pCfg->m_ASTConnLog;
			g_pAstConnector = new TASTConnector(
				g_pCfg->m_ASTConnLogLevel, g_pCfg->m_ASTConnLog, 
				g_pCfg->m_AGIIP, g_pCfg->m_AGIPort, Smt_BoolTRUE );

			g_pAstConnector->AddRemoteServer( g_pCfg->m_ASTIP, g_pCfg->m_AMIPort );
			g_pAstConnector->Run();
		}
	}

	if( sendername == Smt_String(CMU_DEVICESTATENAME) )
	{
		m_DeviceStateGOR = sender;
	}
	
	return Smt_Success;
}

Smt_Uint TConnectionState::OnUserOffline(Smt_Uint sender, Smt_String sendername)
{
	PrintLog(5,"[TConnectionState::OnUserOffline] sender<0x%x>, sendername<%s>.", sender, sendername.c_str());

	if( sendername == Smt_String(CMU_CALLSTATENAME) )
	{
		m_CallStateGOR = 0;
	}

	if( sendername == Smt_String(CMU_DEVICESTATENAME) )
	{
		m_DeviceStateGOR = 0;
	}
	
	return Smt_Success;
}

Smt_Uint TConnectionState::OnTimer(Smt_Uint& timerid, Smt_Uint& messageid, Smt_Uint& senderobj)
{
	switch (messageid)
	{
	case Evt_ICMU_ActionTimer:
		OnEvtActionTimer( senderobj );
		break;
	case Evt_ICMU_AGIEvtTimer:
		OnEvtAGIEvtTimer( senderobj );
		break;				
	case Evt_ICMU_TrunkDnLockedTimer:
		OnEvtTrunkDnLockedTimer( senderobj );
		break;	
	case Evt_ICMU_GetVarTimer:
		OnEvtGetVarTimer( senderobj );
		break;
	case Evt_ICMU_UserEventTimer://add by caoyj 20120315
		OnEvtUserEventTimer(timerid, senderobj );
		break;
	default:break;
	}

	return Smt_Success;
}

Smt_Uint TConnectionState::InitStates()
{
	m_ShiftTable		
	+ new Smt_ScriptState(CONN_NULL, "CONN_NULL") 
		+ new Smt_ScriptRule(Evt_ICMU_Newchannel,-1,CONN_INITIATED, (ST_ACTION)&TConnection::EvtInitiated )
	
	+ new Smt_ScriptState(CONN_INITIATED, "CONN_INITIATED") 
		+ new Smt_ScriptRule(Evt_ICMU_Newcallerid,CauseInitiated,CONN_ORIGINATED, (ST_ACTION)&TConnection::EvtOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_Newcallerid,CauseAlerting,CONN_ALERTING, (ST_ACTION)&TConnection::EvtAlerting )
		+ new Smt_ScriptRule(Evt_ICMU_Newcallerid,CauseConnected,CONN_ALERTING, (ST_ACTION)&TConnection::EvtAlerting )
		+ new Smt_ScriptRule(Evt_ICMU_Newcallerid,CauseListening,CONN_ORIGINATED, (ST_ACTION)&TConnection::EvtOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseRouteRequest,CONN_QUEUED, (ST_ACTION)&TConnection::EvtInitToQueued )
//		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseIVRRequest,CONN_ALERTING, (ST_ACTION)&TConnection::EvtInitToAlerting )
//		+ new Smt_ScriptRule(Evt_ICMU_AGIRequest,-1,CONN_ALERTING, (ST_ACTION)&TConnection::EvtInitToAlerting ) 
//add by caoyj 20120315
		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseIVRRequest,CONN_INITIATED, (ST_ACTION)&TConnection::SetTimer )
		+ new Smt_ScriptRule(Evt_ICMU_UserEventTimerExpired,CauseIVRRequest,CONN_ALERTING, (ST_ACTION)&TConnection::EvtInitToAlerting )
		+ new Smt_ScriptRule(Evt_ICMU_UserEventTimerExpired,CauseAGIInvalid,CONN_INITIATED, (ST_ACTION)&TConnection::SetTimer )
///////////////////////
		+ new Smt_ScriptRule(Evt_ICMU_Dial,-1,CONN_ALERTING, (ST_ACTION)&TConnection::EvtOriginated )
        + new Smt_ScriptRule(Evt_ICMU_Hangup,-1,CONN_NULL, (ST_ACTION)&TConnection::EvtReleased )

	+ new Smt_ScriptState(CONN_ORIGINATED, "CONN_ORIGINATED") 
		+ new Smt_ScriptRule(Evt_ICMU_Dial,-1,CONN_ALERTING, NULL )
		+ new Smt_ScriptRule(Evt_ICMU_Hangup,-1,CONN_NULL, (ST_ACTION)&TConnection::EvtReleased )
//		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseIVRRequest,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtOriToConnected )           // 分机 makecall IVR号码
//add by caoyj 20120315
		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseIVRRequest,CONN_ORIGINATED, (ST_ACTION)&TConnection::SetTimer )           // 分机 makecall IVR号码
		+ new Smt_ScriptRule(Evt_ICMU_UserEventTimerExpired,CauseIVRRequest,CONN_ALERTING, (ST_ACTION)&TConnection::EvtAlerting )
		+ new Smt_ScriptRule(Evt_ICMU_UserEventTimerExpired,CauseAGIInvalid,CONN_ORIGINATED, (ST_ACTION)&TConnection::SetTimer )
////////////////////////
		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseRouteRequest,CONN_QUEUED, (ST_ACTION)&TConnection::EvtQueued )
		+ new Smt_ScriptRule(Evt_ICMU_MeetmeJoin,-1,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtMeetmeConnected )
		+ new Smt_ScriptRule(Evt_ICMU_OriginateResponse,CauseListening,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtConnected )
		+ new Smt_ScriptRule(Evt_ICMU_Unlink,-1,CONN_DISCONNECTED, (ST_ACTION)&TConnection::EvtDisconnected )

	+ new Smt_ScriptState(CONN_ALERTING, "CONN_ALERTING") 
		+ new Smt_ScriptRule(Evt_ICMU_Link,-1,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtConnected )
		+ new Smt_ScriptRule(Evt_ICMU_Hangup,-1,CONN_NULL, (ST_ACTION)&TConnection::EvtReleased )
		+ new Smt_ScriptRule(Evt_ICMU_Newcallerid,-1,CONN_ORIGINATED, (ST_ACTION)&TConnection::EvtOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_AGIResponse,CauseConnected,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtConnected )
		+ new Smt_ScriptRule(Evt_ICMU_Unlink,-1,CONN_DISCONNECTED, (ST_ACTION)&TConnection::EvtDisconnected )
//add by caoyj 20120315
		+ new Smt_ScriptRule(Evt_ICMU_Dial,-1,CONN_ALERTING, (ST_ACTION)&TConnection::EvtOriginated )
////////////////////////

	+ new Smt_ScriptState(CONN_CONNECTED, "CONN_CONNECTED") 
		+ new Smt_ScriptRule(Evt_ICMU_Unlink,-1,CONN_DISCONNECTED, (ST_ACTION)&TConnection::EvtDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Hold,-1,CONN_HOLD, (ST_ACTION)&TConnection::EvtHeld )
		+ new Smt_ScriptRule(Evt_ICMU_Hangup,-1,CONN_NULL, (ST_ACTION)&TConnection::EvtReleased )
		+ new Smt_ScriptRule(Evt_ICMU_Newcallerid,CauseInitiated,CONN_ORIGINATED, (ST_ACTION)&TConnection::EvtOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_Unhold,-1,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtRetrieved )
		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseRouteRequest,CONN_QUEUED, (ST_ACTION)&TConnection::EvtInitToQueued )
		+ new Smt_ScriptRule(Evt_ICMU_MeetmeJoin,-1,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtMeetmeConnected )
		+ new Smt_ScriptRule(Evt_ICMU_MeetmeLeave,-1,CONN_DISCONNECTED, (ST_ACTION)&TConnection::EvtMeetmeDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_OriginateResponse,CauseFail,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtFailed )
		+ new Smt_ScriptRule(Evt_ICMU_Link,-1,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtConnected )

	+ new Smt_ScriptState(CONN_HOLD, "CONN_HOLD") 
		+ new Smt_ScriptRule(Evt_ICMU_Unhold,-1,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtRetrieved )
		+ new Smt_ScriptRule(Evt_ICMU_Unlink,-1,CONN_DISCONNECTED, (ST_ACTION)&TConnection::EvtDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Newcallerid,CauseInitiated,CONN_ORIGINATED, (ST_ACTION)&TConnection::EvtOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_MeetmeJoin,-1,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtMeetmeConnected )
		+ new Smt_ScriptRule(Evt_ICMU_MeetmeLeave,-1,CONN_DISCONNECTED, (ST_ACTION)&TConnection::EvtMeetmeDisconnected )
		+ new Smt_ScriptRule(Evt_ICMU_Hangup,-1,CONN_NULL, (ST_ACTION)&TConnection::EvtReleased )

	+ new Smt_ScriptState(CONN_DISCONNECTED, "CONN_DISCONNECTED") 
		+ new Smt_ScriptRule(Evt_ICMU_Hangup,-1,CONN_NULL, (ST_ACTION)&TConnection::EvtReleased )
		+ new Smt_ScriptRule(Evt_ICMU_Newcallerid,-1,CONN_ORIGINATED, (ST_ACTION)&TConnection::EvtOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_Link,-1,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtConnected )
		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseRouteRequest,CONN_QUEUED, (ST_ACTION)&TConnection::EvtInitToQueued )
		+ new Smt_ScriptRule(Evt_ICMU_Dial,-1,CONN_ALERTING, (ST_ACTION)&TConnection::EvtOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseHoldRequest,CONN_HOLD, (ST_ACTION)&TConnection::EvtHeld )
		+ new Smt_ScriptRule(Evt_ICMU_MeetmeJoin,-1,CONN_CONNECTED, (ST_ACTION)&TConnection::EvtMeetmeConnected )
		+ new Smt_ScriptRule(Evt_ICMU_MeetmeLeave,-1,CONN_DISCONNECTED, (ST_ACTION)&TConnection::EvtMeetmeDisconnected )

		//+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseIVRRequest,CONN_ALERTING, (ST_ACTION)&TConnection::EvtInitToAlerting )
		//add by caoyj 20110805
//		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseIVRRequest,CONN_DISCONNECTED, NULL )
//      + new Smt_ScriptRule(Evt_ICMU_AGIRequest,-1,CONN_ALERTING, (ST_ACTION)&TConnection::EvtInitToAlerting ) 
//add by caoyj 20120315
		+ new Smt_ScriptRule(Evt_ICMU_UserEvent,CauseIVRRequest,CONN_DISCONNECTED, (ST_ACTION)&TConnection::SetTimer )
		+ new Smt_ScriptRule(Evt_ICMU_UserEventTimerExpired,CauseIVRRequest,CONN_ALERTING, (ST_ACTION)&TConnection::EvtInitToAlerting )
		+ new Smt_ScriptRule(Evt_ICMU_UserEventTimerExpired,CauseAGIInvalid,CONN_DISCONNECTED, (ST_ACTION)&TConnection::SetTimer )
///////////////////////////

	+ new Smt_ScriptState(CONN_QUEUED, "CONN_QUEUED") 
		+ new Smt_ScriptRule(Evt_ICMU_Hangup,-1,CONN_NULL, (ST_ACTION)&TConnection::EvtReleased )
		+ new Smt_ScriptRule(Evt_ICMU_Newcallerid,-1,CONN_ORIGINATED, (ST_ACTION)&TConnection::EvtOriginated )
		+ new Smt_ScriptRule(Evt_ICMU_Dial,-1,CONN_ALERTING, NULL )
	;
	return Smt_Success;
}

Smt_StateObject* TConnectionState::PrehandleMessage(Smt_Pdu &pdu)
{
	if(pdu.m_MessageID == Evt_ICMU_AGIRequest)  //add by caoyj 20120106
	   PrintLog(5,"[TConnectionState::PrehandleMessage] Recv Evt_ICMU_AGIRequest PduInfo %s.", pdu.GetInfo().c_str() );

	TConnection * pConnection = NULL;
	switch (pdu.m_MessageID)
	{
	// App Command
	case Cmd_ICMU_Assign:  
		OnCmdAssign( pdu );
		break;
	case Cmd_ICMU_MakeCall:  
		OnCmdMakeCall( pdu );      
		break;
	case Cmd_ICMU_HangupCall:  
		OnCmdHangupCall( pdu );
		break;
	case Cmd_ICMU_RouteSelected:
		OnCmdRouteSelected( pdu ); 
		break;
	case Cmd_ICMU_SingleStepTransfer:  
		OnCmdSingleStepTransfer( pdu ); 
		break;
	case Cmd_ICMU_SingleStepConference:  
		OnCmdSingleStepConference( pdu ); 
		break;
	case Cmd_ICMU_SendDTMF:  
		OnCmdSendDTMF( pdu );
		break;
	case Cmd_ICMU_StartRecord:  
		OnCmdStartRecord( pdu );
		break;
	case Cmd_ICMU_StopRecord:  
		OnCmdStopRecord( pdu );
		break;
	case Cmd_ICMU_AnswerCallEx:  
		OnCmdAnswerCallEx( pdu );
		break;
	case Cmd_ICMU_HangupCallEx:  
		OnCmdHangupCallEx( pdu );
		break;
	case Cmd_ICMU_PlayFile:  
		OnCmdPlayFile( pdu );
		break;
	case Cmd_ICMU_GetDigits:  
		OnCmdGetDigits( pdu );
		break;
	case Cmd_ICMU_RecordEx:  
		OnCmdRecordEx( pdu );
		break;	
	case Cmd_ICMU_SendDTMFEx:  
		OnCmdSendDTMFEx( pdu );
		break;	
	case Cmd_ICMU_PlayFileList:  
		OnCmdPlayFileList( pdu );
		break;		
	case Cmd_ICMU_ExecAMI:  
		OnCmdExecAMI( pdu );
		break;	
	case Cmd_ICMU_ExecAGI:  
		OnCmdExecAGI( pdu );
		break;
	case Evt_ICMU_LinkDown:  
		OnEvtLinkDown( pdu );
		break;	
	case Cmd_ICMU_Dial:  
		OnCmdDial( pdu );
		break;
	case Cmd_ICMU_SendMessage:  
		OnCmdSendMessage( pdu );
		break;	
	case Cmd_ICMU_SendMessageEx:  
		OnCmdSendMessageEx( pdu );
		break;	
	case Cmd_ICMU_SetSIPHeader:  
		OnCmdSetSIPHeader( pdu );
		break;	
	case Cmd_ICMU_GetSIPHeader:  
		OnCmdGetSIPHeader( pdu );
		break;
// 	case Cmd_ICMU_TTSPlay:
// 		OnCmdPlayText(pdu);
// 		break;

		// Asterisk Event
	case Evt_ICMU_Newchannel:
		pConnection = OnEvtNewchannel( pdu );
		break;
	case Evt_ICMU_Newcallerid:  
		pConnection = OnEvtNewcallerid( pdu );
		break;
	case Evt_ICMU_Dial:  
		pConnection = OnEvtDial( pdu );
		break;
	case Evt_ICMU_Link:  
		pConnection = OnEvtLink( pdu );
		break;
	case Evt_ICMU_Unlink:  
		pConnection = OnEvtUnlink( pdu );
		break;
	case Evt_ICMU_Hangup:  
		pConnection = OnEvtHangup( pdu );
		break;
	case Evt_ICMU_Hold:  
		pConnection = OnEvtHold( pdu );
		break;
	case Evt_ICMU_Unhold:
		pConnection = OnEvtUnhold( pdu );
		break;
	case Evt_ICMU_UserEvent:
		pConnection = OnEvtUserEvent( pdu );
		break;
	case Evt_ICMU_MeetmeJoin:  
		pConnection = OnEvtMeetmeJoin( pdu );
		break;
	case Evt_ICMU_MeetmeLeave:  
		pConnection = OnEvtMeetmeLeave( pdu );
		break;
	case Evt_ICMU_PeerStatus:  
		OnEvtPeerStatus( pdu );
		break;		
	case Evt_ICMU_AMIResponse:  
		OnEvtAMIResponse( pdu );
		break;	
	case Evt_ICMU_AGIRequest:  
		OnEvtAGIRequest( pdu );
//		pConnection = OnEvtAGIRequest( pdu );
		break;
//add by caoyj 20120315
	case Evt_ICMU_AGIHandleOffline:  
		OnEvtAGIHandleOffline( pdu );
		break;
	case Evt_ICMU_UserEventTimerExpired:  
		pConnection = OnEvtUserEventTimerExpired( pdu );
		break;
	case Evt_ICMU_Newexten:  
		OnEvtNewexten( pdu );
		break;
///////////////////////////
	case Evt_ICMU_AGIResponse:  
		pConnection = OnEvtAGIResponse( pdu );
		break;
	case Evt_ICMU_OriginateResponse:  
		pConnection = OnEvtOriginateResponse( pdu );
		break;
	case Evt_ICMU_DTMFReceived:  
		OnEvtDTMFReceived( pdu );
		break;	
	case Evt_ICMU_MessageReceived:  
		OnEvtMessageReceived( pdu );
		break;
// 	case Evt_ICMU_TTSPlayEnd:
// 		OnEvtPlayTextEnd(pdu);
		break;
	default:break;
	}
      
	return pConnection;
}

/****************************************************************************
函 数 名: OnEvtNewchannel
参    数:
返回数值: 
功能描述: 新建 Connection
*****************************************************************************/
TConnection* TConnectionState::OnEvtNewchannel(Smt_Pdu &pdu)
{
	TConnection * pConnection = NULL;
	Smt_Uint   nHandle;
	Smt_String strChannel;
	Smt_String strCallerIDNum;
	Smt_String strCallerIDName;
	Smt_String strUniqueid;
	Smt_String strConnectionID;
		  
	pdu.GetString( Key_ICMU_Channel, &strChannel );
	pdu.GetUint  ( Key_ICMU_Handle, &nHandle );
	pdu.GetString( Key_ICMU_CallerIDNum, &strCallerIDNum );
	pdu.GetString( Key_ICMU_CallerIDName, &strCallerIDName );
	pdu.GetString( Key_ICMU_Uniqueid, &strUniqueid );

	if( strChannel == "")
	{
		return NULL;
	}

	pConnection = LookupByChannel( strChannel );

	if( pConnection == NULL )
	{
		pConnection = LookupByDeadChannel( strChannel ); // lookup connection by dead channel again
	}
/*
	if( pConnection == NULL )
	{
		Smt_String strDeviceID;
		Smt_Uint   nDeviceType;
		Smt_String strTACCode;

		// 获取设备DN，对中继设备的分配解锁
		GetDeviceID( strChannel, strDeviceID, nDeviceType, strTACCode );

		strConnectionID = GenConnectionID();

		pConnection = new TConnection(strConnectionID, (Smt_Uint)CONN_NULL, this);
		pConnection->m_Channel = strChannel;
		pConnection->m_AMIHandle = nHandle;
		pConnection->m_Uniqueid = strUniqueid;
		pConnection->m_DeviceID = strDeviceID;
		pConnection->m_DeviceType = nDeviceType;
		pConnection->m_TACCode = strTACCode;
		pConnection->m_FullDeviceID = strChannel;	
	
		m_ConnectionMgr.SetAt( strConnectionID, pConnection );
	}
*/
//add by caoyj 20120315
	Smt_String strDeviceID;
	Smt_Uint   nDeviceType;
	Smt_String strTACCode;
	
	do 
	{
		if( pConnection != NULL )
		{
			PrintLog(3, "[TConnectionState::OnEvtNewchannel] Connection Not Null, Channel<%s>.", 
				strChannel.c_str() );
			break;
		}
		
		// 获取设备DN，对中继设备的分配解锁
		GetDeviceID( strChannel, strDeviceID, nDeviceType, strTACCode );
		
		// 判断设备是否已经在处理话路, DTYPE_EXTENSION 设备仅支持一通话路，仅支持一个 Connection
		if( nDeviceType == DTYPE_EXTENSION )
		{
			pConnection = LookupByDeviceID( strDeviceID );
			
			if( pConnection != NULL )
			{
				PrintLog(3, "[TConnectionState::OnEvtNewchannel] Connection Not Null, Channel<%s>, DeviceID<%s>.", 
					strChannel.c_str(), strDeviceID.c_str() );
				break;
			}
		}
		
		strConnectionID = GenConnectionID();
		
		pConnection = new TConnection(strConnectionID, (Smt_Uint)CONN_NULL, this);
		pConnection->m_Channel = strChannel;
		pConnection->m_AMIHandle = nHandle;
		pConnection->m_Uniqueid = strUniqueid;
		pConnection->m_DeviceID = strDeviceID;
		pConnection->m_DeviceType = nDeviceType;
		pConnection->m_TACCode = strTACCode;
		pConnection->m_FullDeviceID = strChannel;	
		
		m_ConnectionMgr.SetAt( strConnectionID, pConnection );
		
	} while (0);
///////////////////////////////////////////////////////////////////
	PrintLog(5,"[TConnectionState::OnEvtNewchannel] ConnectionID<%s>, Channel<%s>, CallerID<%s>.",
		strConnectionID.c_str(), strChannel.c_str(), strCallerIDNum.c_str() );

	return pConnection;
}

/****************************************************************************
函 数 名: OnEvtNewcallerid
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TConnection* TConnectionState::OnEvtNewcallerid(Smt_Pdu &pdu)   
{
	Smt_Uint nRet = Smt_Success;
	TConnection * pConnection = NULL;
	TConnection * pOtherConnection = NULL;
	Smt_String strConnectionID;
	Smt_String strChannel;
	Smt_String strCallerID;
	Smt_String strCallerIDName;	
	Smt_Uint nPduStatus = 0;
	
	pdu.GetString( Key_ICMU_Channel, &strChannel );
	pdu.GetString( Key_ICMU_CallerID, &strCallerID );
	pdu.GetString( Key_ICMU_CallerIDName, &strCallerIDName );

	do 
	{
		pConnection = LookupByChannel( strChannel );
		if( pConnection == NULL )
		{
			nRet = Err_ICMU_NoConnection;
			break;
		}

		strConnectionID = pConnection->GetID();
		pConnection->m_CallerID = ChangeCallerIDByTAC(strCallerID, pConnection);
		pConnection->m_CallerIDName = strCallerIDName;
	
		// ?????
		// 监听/强插时的 CallerID 是被监听方的 ConnectionID
		pOtherConnection = LookupByDeviceID( strCallerID );
		if( pOtherConnection != NULL && 
			pOtherConnection->m_LastCommand.m_MessageID == Cmd_ICMU_SingleStepConference )
		{
			pOtherConnection->m_LastCommand.Clear();

			nPduStatus = CauseListening;
			pConnection->m_CallerID = pOtherConnection->m_CallerID;
			pConnection->m_Source = pOtherConnection->m_Source;
			pConnection->m_Destination = pOtherConnection->m_Destination;
			pConnection->m_OtherConnectionID = pOtherConnection->GetID();
			pConnection->m_Reason = CauseListening;
			break;
		}
	
		// Dial 事件和 Newcallerid 事件时序调整
		if( m_ConnectionMgr.Lookup( pConnection->m_OtherConnectionID, pOtherConnection) == Smt_Success)
		{
			if( pOtherConnection->GetState() == CONN_ALERTING )
			{
				nPduStatus = CauseAlerting;
				break;
			}
			
			if( pOtherConnection->GetState() == CONN_CONNECTED )
			{
				nPduStatus = CauseConnected;
				break;
			}
		}

		nPduStatus = CauseInitiated;

	} while (0);

	pdu.m_Status = nPduStatus;

	PrintLog(5,"[TConnectionState::OnEvtNewcallerid] ConnectionID<%s>, Channel<%s>, CallerID<%s>.",
		strConnectionID.c_str(), strChannel.c_str(), strCallerID.c_str() );

	return pConnection;
}
 
/****************************************************************************
函 数 名: OnEvtDial
参    数:
返回数值: 
功能描述: 仅查找 Channel1 的 Connection；
          在发送 Alerting 事件时，给 Channel2 的Connection 也发送 Alerting 事件
*****************************************************************************/
TConnection* TConnectionState::OnEvtDial(Smt_Pdu &pdu)   
{
	Smt_Uint nRet = Smt_Success;
	TConnection * pConnection1 = NULL;
	TConnection * pConnection2 = NULL;
	Smt_String strSource;
	Smt_String strDestination;
	Smt_String strCallerID;
	Smt_String strCallerIDName;
	Smt_String strConnectionID1="", strConnectionID2="";

	pdu.GetString( Key_ICMU_Source, &strSource );
	pdu.GetString( Key_ICMU_Destination, &strDestination );
	pdu.GetString( Key_ICMU_CallerID, &strCallerID );
	pdu.GetString( Key_ICMU_CallerIDName, &strCallerIDName );

	do 
	{
		pConnection1 = LookupByChannel( strSource );
		if( pConnection1 == NULL )
		{
			pConnection1 = LookupByDeadChannel( strSource ); 
		}

		if( pConnection1 == NULL )
		{
			nRet = Err_ICMU_NoConnection1;
			break;		
		}

		strConnectionID1 = pConnection1->GetID();
		pConnection1->m_ChannelType = CHTYPE_SRC;
		pConnection1->m_CallerID = ChangeCallerIDByTAC(strCallerID, pConnection1);
		pConnection1->m_CallerIDName = strCallerIDName;
	
		pConnection2 = LookupByChannel( strDestination );
		if( pConnection2 == NULL )
		{
			nRet = Err_ICMU_NoConnection2;
			break;		
		}

		strConnectionID2 = pConnection2->GetID();
		pConnection2->m_ChannelType = CHTYPE_DST;
	
		// set src/dest
		pConnection1->m_OtherConnectionID = strConnectionID2;
		pConnection2->m_OtherConnectionID = strConnectionID1;

		Smt_String strTmpSource, strTmpDestination;
		strTmpSource = pConnection1->m_DeviceID;
		strTmpDestination = pConnection2->m_DeviceID;

		pConnection1->m_Source = strTmpSource;
		pConnection1->m_Destination = strTmpDestination;
		pConnection2->m_Source = strTmpSource;
		pConnection2->m_Destination = strTmpDestination;
	
	} while (0);

	PrintLog(5,"[TConnectionState::OnEvtDial] ConnectionID1<%s>, ConnectionID2<%s>, nRet<0x%x>.",
		strConnectionID1.c_str(), strConnectionID2.c_str(), nRet );

	return pConnection1;
}

/****************************************************************************
函 数 名: OnEvtLink
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TConnection* TConnectionState::OnEvtLink(Smt_Pdu &pdu)
{
	TConnection * pConnection1 = NULL;
	TConnection * pConnection2 = NULL;
	Smt_String strChannel1, strChannel2;
	Smt_String strConnectionID1="", strConnectionID2="";

	pdu.GetString( Key_ICMU_Channel1, &strChannel1 );
	pdu.GetString( Key_ICMU_Channel2, &strChannel2 );

	pConnection1 = LookupByChannel( strChannel1 );
	if( pConnection1 == NULL )
	{
		pConnection1 = LookupByDeadChannel( strChannel1 ); 
	}

	if( pConnection1 != NULL )
	{
		strConnectionID1 = pConnection1->GetID();
		pConnection1->m_Reason = CauseNotKnown;
	}

	pConnection2 = LookupByChannel( strChannel2 );
	if( pConnection2 != NULL )
	{
		strConnectionID2 = pConnection2->GetID();
		pConnection2->m_Reason = CauseNotKnown;
	}

	// OtherConnectionID 在 OnEvtDial 事件赋值作用是用于发送 Alerting 事件；
	// 在这里赋值是处理一个 Channel 产生多个 Dial 事件后，通道关系改变了，在这里进行修正

	if((pConnection1!= NULL) && (pConnection2!=NULL) )
	{
		pConnection1->m_OtherConnectionID = strConnectionID2;
		pConnection1->m_Source = pConnection1->m_DeviceID;
		pConnection1->m_Destination = pConnection2->m_DeviceID;
		pConnection2->m_OtherConnectionID = strConnectionID1;
		pConnection2->m_Source = pConnection1->m_DeviceID;
		pConnection2->m_Destination = pConnection2->m_DeviceID;
	}

	PrintLog(5,"[TConnectionState::OnEvtLink] ConnectionID1<%s>, Channel1<%s>, ConnectionID2<%s>, Channel2<%s>.",
		strConnectionID1.c_str(), strChannel1.c_str(), strConnectionID2.c_str(), strChannel2.c_str() );

	return pConnection1;
}

/****************************************************************************
函 数 名: OnEvtUnlink
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TConnection* TConnectionState::OnEvtUnlink(Smt_Pdu &pdu)   
{
	TConnection * pConnection1 = NULL;
	TConnection * pConnection2 = NULL;
	Smt_String strChannel1, strChannel2;
	Smt_String strConnectionID1="", strConnectionID2="";
	
	pdu.GetString( Key_ICMU_Channel1, &strChannel1 );
	pdu.GetString( Key_ICMU_Channel2, &strChannel2 );
	
	pConnection1 = LookupByChannel( strChannel1 );
	if( pConnection1 == NULL )
	{
		pConnection1 = LookupByDeadChannel( strChannel1 ); 
	}
	if( pConnection1 != NULL )
	{
		strConnectionID1 = pConnection1->GetID();
		pConnection1->m_Reason = CauseNotKnown;
	}
	
	pConnection2 = LookupByChannel( strChannel2 );
	if( pConnection2 != NULL )
	{
		strConnectionID2 = pConnection2->GetID();
		pConnection2->m_Reason = CauseNotKnown;
	}

	// IVR 外呼场景
	if((pConnection1!= NULL) && (pConnection2!=NULL) )
	{
		pConnection1->m_OtherConnectionID = strConnectionID2;
		pConnection1->m_Source = pConnection1->m_DeviceID;
		pConnection1->m_Destination = pConnection2->m_DeviceID;
		pConnection2->m_OtherConnectionID = strConnectionID1;
		pConnection2->m_Source = pConnection1->m_DeviceID;
		pConnection2->m_Destination = pConnection2->m_DeviceID;
	}
	
	PrintLog(5,"[TConnectionState::OnEvtUnlink] ConnectionID1<%s>, Channel1<%s>, ConnectionID2<%s>, Channel2<%s>.",
		strConnectionID1.c_str(), strChannel1.c_str(), strConnectionID2.c_str(), strChannel2.c_str() );
	
	return pConnection1;
}

/****************************************************************************
函 数 名: OnEvtHangup
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TConnection* TConnectionState::OnEvtHangup(Smt_Pdu &pdu)      
{
	TConnection * pConnection = NULL;
	Smt_String strChannel;
	Smt_String strCause;
	Smt_String strCauseTxt;
	Smt_String strConnectionID="";
	Smt_String strDeviceID;
		  
	pdu.GetString( Key_ICMU_Channel, &strChannel );
	pdu.GetString( Key_ICMU_Cause, &strCause );
	pdu.GetString( Key_ICMU_Causetxt, &strCauseTxt );

	pConnection = LookupByChannel( strChannel );

	if( pConnection == NULL )
	{
		pConnection = LookupByDeadChannel( strChannel ); 
	}

	if( pConnection != NULL )
	{
		strConnectionID = pConnection->GetID();
		strDeviceID = pConnection->m_DeviceID;

		if( ACE_OS::atoi(strCause.c_str()) == AST_CAUSE_NORMAL_CLEARING ||
			ACE_OS::atoi(strCause.c_str()) == AST_CAUSE_NOTDEFINED)
		{
			pConnection->m_Reason = CauseNormalReleased;
		}
		else if( ACE_OS::atoi(strCause.c_str()) == AST_CAUSE_USER_BUSY )
		{
			pConnection->m_Reason = CauseDestBusy;
		}
		else if(  ACE_OS::atoi(strCause.c_str()) == AST_CAUSE_CALL_REJECTED)
		{
			pConnection->m_Reason = CauseCallRejected;
		}
		else if(  ACE_OS::atoi(strCause.c_str()) == AST_CAUSE_NO_ANSWER)
		{
			pConnection->m_Reason = CauseNoAnswer;
		}
		else if(  ACE_OS::atoi(strCause.c_str()) == AST_CAUSE_UNALLOCATED)
		{
			pConnection->m_Reason = CauseDestInvalid;
		}
//add by caoyj 20120315
		else if(  ACE_OS::atoi(strCause.c_str()) == AST_CAUSE_NETWORK_OUT_OF_ORDER)
		{
			pConnection->m_Reason = CauseDestNotInService;
		}
//////////////////////////////

		// set singlesteptransfer reason
		do 
		{
			Smt_Uint nLastCommand;
			Smt_Uint nReason;
			pConnection->m_LastCommand.GetUint(Key_ICMU_LastCommand, &nLastCommand );
			pConnection->m_LastCommand.GetUint(Key_ICMU_Reason, &nReason );
			if( nLastCommand == Cmd_ICMU_SingleStepTransfer && nReason == CauseMeetme )
			{
				pConnection->m_Reason = CauseSingleStepTransferredMeetme;
				pConnection->m_LastCommand.Clear();
				break;
			}
			
			TConnection * pOtherConnection = NULL;
			if(m_ConnectionMgr.Lookup(pConnection->m_OtherConnectionID, pOtherConnection) == Smt_Success )
			{
				pOtherConnection->m_LastCommand.GetUint(Key_ICMU_LastCommand, &nLastCommand );
				pConnection->m_LastCommand.GetUint(Key_ICMU_Reason, &nReason );
				if( nLastCommand == Cmd_ICMU_SingleStepTransfer )
				{
					pConnection->m_Reason = CauseSingleStepTransferred;
					pOtherConnection->m_LastCommand.Clear();
					break;
				}
			}
		} while (0);

		// release TrunkDevice
		TTrunkGroup* pTrunkGroup = NULL;
		TTrunkGroupMap::ITERATOR iter(m_TrunkGroupMgr);
		for( iter = m_TrunkGroupMgr.begin();
			iter != m_TrunkGroupMgr.end();
			++iter )
		{
			pTrunkGroup = (*iter).int_id_;
			if ( pTrunkGroup->FindDevice( strDeviceID ) == Smt_Success )
			{
				pTrunkGroup->FreeDevice( strDeviceID );
				break;
			}
		}
	}

	PrintLog(5,"[TConnectionState::OnEvtHangup] ConnectionID<%s>, Channel<%s>, Cause<%s>, CauseTxt<%s>.",
		strConnectionID.c_str(), strChannel.c_str(), strCause.c_str(), strCauseTxt.c_str() );

	return pConnection;
}

TConnection* TConnectionState::OnEvtHold(Smt_Pdu &pdu)      
{
	TConnection * pConnection = NULL;
	Smt_String strChannel;
	Smt_String strConnectionID="";

	pdu.GetString( Key_ICMU_Channel, &strChannel );

	pConnection = LookupByChannel( strChannel );
	if( pConnection != NULL )
	{
		strConnectionID = pConnection->GetID();
	}
	
	PrintLog(5,"[TConnectionState::OnEvtHold] ConnectionID<%s>, Channel<%s>.",
		strConnectionID.c_str(), strChannel.c_str());

	return pConnection;
}

TConnection* TConnectionState::OnEvtUnhold(Smt_Pdu &pdu)         
{
	TConnection * pConnection = NULL;
	Smt_String strChannel;
	Smt_String strConnectionID="";
	
	pdu.GetString( Key_ICMU_Channel, &strChannel );
	
	pConnection = LookupByChannel( strChannel );
	if( pConnection != NULL )
	{
		strConnectionID = pConnection->GetID();
	}
	
	PrintLog(5,"[TConnectionState::OnEvtUnhold] ConnectionID<%s>, Channel<%s>.",
		strConnectionID.c_str(), strChannel.c_str());

	return pConnection;
}

/****************************************************************************
函 数 名: OnEvtUserEvent
参    数:
返回数值: 
功能描述: 用户自定义事件
*****************************************************************************/
TConnection* TConnectionState::OnEvtUserEvent(Smt_Pdu &pdu)
{
	TConnection * pConnection = NULL;
	Smt_String strChannel;
	Smt_String strUserEvent;
	Smt_String strCallerID;
	Smt_String strExtension;
	Smt_String strConnectionID="";
	Smt_Uint   nPduStatus = 0;
	Smt_Uint uReason=0;

	pdu.GetString( Key_ICMU_Channel, &strChannel );
	pdu.GetString( Key_ICMU_UserEvent, &strUserEvent );
	pdu.GetString( Key_ICMU_CallerID, &strCallerID );
	pdu.GetString( Key_ICMU_Extension, &strExtension );
	
	pConnection = LookupByChannel( strChannel );
	if( pConnection != NULL )
	{
		strConnectionID = pConnection->GetID();
		             
		if( strUserEvent == USER_EVTNT_RouteRequest )
		{
			pdu.m_Status = CauseRouteRequest;
			pConnection->m_Reason = CauseRouteRequest;
			pConnection->m_Source = pConnection->m_DeviceID;
			pConnection->m_Destination = strExtension;	
			pConnection->m_CallerID = ChangeCallerIDByTAC(strCallerID, pConnection);			
		}
		
		else if( strUserEvent == USER_EVTNT_IVRRequest )
		{
			pdu.m_Status = CauseIVRRequest;
// 			pConnection->m_Reason = CauseIVRRequest;
// 			pConnection->m_Source = pConnection->m_DeviceID;
// 			pConnection->m_Destination = strExtension;	
// 			pConnection->m_CallerID = ChangeCallerIDByTAC(strCallerID, pConnection);		
			//add by caoyj 20120106 增加逻辑判断  //add by caoyj 20120221修改
			pConnection->m_Reason = CauseIVRRequest;
			pConnection->m_Source = pConnection->m_DeviceID;
			pConnection->m_Destination = strExtension;	
			if(pConnection->m_CallerID == "")
			{
				pConnection->m_CallerID = ChangeCallerIDByTAC(strCallerID, pConnection);		
			}
			//////////////////////////////////////////////////////
		}
		else if( strUserEvent == USER_EVTNT_HoldRequest )    // 保持音时，不需要号码信息
		{
			pdu.m_Status = CauseHoldRequest;
			pConnection->m_Reason = CauseHoldRequest;
		}
		uReason = pConnection->m_Reason;
	}

	PrintLog(5,"[TConnectionState::OnEvtUserEvent] ConnectionID<%s>,Channel<%s>,CallerID<%s>,Extension<%s>,Reason<%d>,pdu.Status<%d>.",
		strConnectionID.c_str(), strChannel.c_str(), strCallerID.c_str(), strExtension.c_str(), uReason, pdu.m_Status );

	return pConnection;
}

/****************************************************************************
函 数 名: GenConnectionID
参    数:
返回数值: 
功能描述: 获取唯一流水号，格式：YYYYMMDDHHMMSSMSC_index
*****************************************************************************/
Smt_String TConnectionState::GenConnectionID()
{
	Smt_SingleLock sLock(m_Lock);

	m_UniqueIndex++;
	if( m_UniqueIndex > 999 )
	{
		m_UniqueIndex = 0;
	}

	Smt_String strTemp;
	Smt_DateTime dtNow;
	strTemp = HLFormatStr("%s_%03d", 
		dtNow.FormatString(DATETIMEFORMAT0).c_str(), m_UniqueIndex );
	
	return strTemp;
}

/****************************************************************************
函 数 名: GenActionID
参    数:
返回数值: 
功能描述: 获取唯一流水号，格式：YYYYMMDDHHMMSSMSC_index
*****************************************************************************/
Smt_String TConnectionState::GenActionID(Smt_Pdu &pdu)
{
	Smt_SingleLock sLock(m_Lock);

	m_ActionIndex++;
	if( m_ActionIndex > 999999 )
	{
		m_ActionIndex = 1;
	}

	Smt_String strActID;
	strActID = HLFormatStr("%06d", m_ActionIndex );

	// create act pdu
	TActionPdu* pAct = new TActionPdu();
	pAct->m_Pdu = pdu;
	pAct->m_ActionID = strActID;
	pAct->m_TimerID = SetSingleTimer(MAX_ACTIONTIMERLEN, Evt_ICMU_ActionTimer, (Smt_Uint)pAct );
	
	m_ActMgr.SetAt( strActID, pAct );

	return strActID;
}

/****************************************************************************
函 数 名: LookupByChannel
参    数:
返回数值: 
功能描述: 根据通道查找 Connection 对象
*****************************************************************************/
TConnection* TConnectionState::LookupByChannel(Smt_String channel)
{
	TConnection* pConn = NULL;
	TConnection* pTmpConn = NULL;
	TConnectionMap::ITERATOR iter(m_ConnectionMgr);
	for( iter = m_ConnectionMgr.begin();
		 iter != m_ConnectionMgr.end();
		 ++iter )
	{
		pTmpConn = (*iter).int_id_;
		if ( pTmpConn->m_Channel == channel )
		{
		  pConn = pTmpConn;
		  break;
		}
	}

	return pConn;
}

/****************************************************************************
函 数 名: LookupByDeadChannel
参    数:
返回数值: 
功能描述: 根据通道查找 Connection 对象
          有时 Asterisk 会出现 DEAD 或 ZOMBIE 的通道，格式"xxxxx,x"
		       AsyncGoto/SIP/3003-09a60158<ZOMBIE>
*****************************************************************************/
TConnection* TConnectionState::LookupByDeadChannel(Smt_String channel)
{
	TConnection* pConn = NULL;
	TConnection* pTmpConn = NULL;

	Smt_String strChannel = channel;

	// 格式："xxxxx,x"
	Smt_String strTmp1, strTmp2;
	Smt_Int    nPos = strChannel.find(",");
	if( nPos>0 )
	{
		strTmp1 = strChannel.substr( 0, nPos );
		
		TConnectionMap::ITERATOR iter(m_ConnectionMgr);
		for( iter = m_ConnectionMgr.begin();
			iter != m_ConnectionMgr.end();
			++iter )
		{
			pTmpConn = (*iter).int_id_;
			
			nPos = pTmpConn->m_Channel.find(",");
			
			if( nPos>0 )
				strTmp2 = pTmpConn->m_Channel.substr( 0, nPos );
			else
				strTmp2 = pTmpConn->m_Channel;
			
			if ( strTmp1 == strTmp2 )
			{
				pConn = pTmpConn;
				break;
			}
		}// for
	}// if

	// 格式：xxxxxx<ZOMBIE>
	if( pConn == NULL )
	{
		nPos = strChannel.find("<ZOMBIE>");
		if( nPos>0 )
		{
			strTmp1 = strChannel.substr( 0, nPos );
			
			TConnectionMap::ITERATOR iter(m_ConnectionMgr);
			for( iter = m_ConnectionMgr.begin();
				 iter != m_ConnectionMgr.end();
				 ++iter )
			{
				pTmpConn = (*iter).int_id_;
						
				if ( pTmpConn->m_Channel == strTmp1 )
				{
					pConn = pTmpConn;
					break;
				}
			}// for
		}// if
	}	
	
	return pConn;
}

/****************************************************************************
函 数 名: LookupByHandle
参    数:
返回数值: 
功能描述: 根据 AGI SocketHandle 查找 Connection 对象
*****************************************************************************/
TConnection* TConnectionState::LookupByHandle(Smt_Uint handle)
{
	TConnection* pConn = NULL;
	TConnection* pTmpConn = NULL;
	TConnectionMap::ITERATOR iter(m_ConnectionMgr);
	for( iter = m_ConnectionMgr.begin();
		iter != m_ConnectionMgr.end();
		++iter )
	{
		pTmpConn = (*iter).int_id_;
		if ( pTmpConn->m_AGIHandle == handle )
		{
			pConn = pTmpConn;
			break;
		}
	}
	return pConn;
}

/****************************************************************************
函 数 名: LookupByDeviceID
参    数:
返回数值: 
功能描述: 根据 DeviceID 查找 Connection 对象
*****************************************************************************/
TConnection* TConnectionState::LookupByDeviceID(Smt_String deviceid)
{
	TConnection* pConn = NULL;
	TConnection* pTmpConn = NULL;
	TConnectionMap::ITERATOR iter(m_ConnectionMgr);
	for( iter = m_ConnectionMgr.begin();
		iter != m_ConnectionMgr.end();
		++iter )
	{
		pTmpConn = (*iter).int_id_;
		if ( pTmpConn->m_DeviceID == deviceid )
		{
			pConn = pTmpConn;
			break;
		}
	}
	return pConn;
}

/****************************************************************************
函 数 名: GetDeviceID
参    数:
返回数值: 
功能描述: 根据 Channel 分析 DeviceID,
1) SIP/3002-0976e790                       DTYPE_EXTENSION
2) Local/6000@from-internal-custom-0868    DTYPE_IVR_OUTBOUND
3) DAHDI/1-1                               DTYPE_TRUNK_DAHDI
4) SIP/SIPTo189-09799390                   DTYPE_TRUNK_SIP
5) SIP/192.168.1.189-09799390              DTYPE_TRUNK_SIP
*****************************************************************************/
Smt_Uint TConnectionState::GetDeviceID(Smt_String channel, Smt_String& deviceid, Smt_Uint& devicetype, Smt_String& taccode)
{
	Smt_String strDeviceID = "";
	Smt_Uint   nDeviceType=0;
	Smt_String strTACCode="";
	Smt_Int    nPosSlash;
	Smt_Int    nPosBar;
	Smt_Int    nPosAt; 
	Smt_Int    nPosDahdi;
	Smt_Int    nPosSIP;
	Smt_Int    nPosLocal;
	Smt_String strCircuit;
	Smt_String strTrunkName;
	Smt_Bool bOk = Smt_BoolFALSE;
	TTrunkGroup* pTrunkGroup = NULL;

	nPosSlash = channel.find("/");
	nPosBar = channel.find("-");
	nPosAt = channel.find("@");
	nPosSIP = channel.find( CHANNELTYPE_SIP );	
	nPosDahdi = channel.find( CHANNELTYPE_DAHDI );	
	nPosLocal = channel.find( CHANNELTYPE_LOCAL );
	
	PrintLog(5, "[TConnectionState::GetDeviceID] nPosSlash<%d>, nPosBar<%d>, nPosAt<%d>, nPosSIP<%d>, nPosDahdi<%d>, nPosLocal<%d>.",
		nPosSlash, nPosBar, nPosAt, nPosSIP, nPosDahdi, nPosLocal );
    do 
	{
		if( nPosSlash == -1 )
		{
			strDeviceID = "";
			break;
		}

		// SIP 中继设备, 如: SIP/SIPTo208-09799390
		if( nPosSIP>=0 )
		{
			strTrunkName = channel.substr(nPosSlash+1, nPosBar-nPosSlash-1 );
			
			TTrunkGroupMap::ITERATOR iter(m_TrunkGroupMgr);
			for( iter = m_TrunkGroupMgr.begin();
				iter != m_TrunkGroupMgr.end();
				++iter )
			{
				pTrunkGroup = (*iter).int_id_;
				if ( pTrunkGroup->m_TrunkName == strTrunkName )
				{
					strDeviceID = pTrunkGroup->AllocateSipTrunk();
					nDeviceType = pTrunkGroup->m_TrunkType; // DTYPE_TRUNK_SIP;
					strTACCode = pTrunkGroup->m_TACCode;
					bOk = Smt_BoolTRUE;
					break;
				}
			}
		}		

		if( bOk == Smt_BoolTRUE ) break;

		// DAHDI 中继设备, 如: DAHDI/1-1	
		if( nPosDahdi >= 0 )  // if-1
		{
			strCircuit = channel.substr(nPosSlash+1, nPosBar-nPosSlash-1 );
			strTrunkName = channel.substr(nPosBar+1, channel.length()-1 );

			TTrunkGroupMap::ITERATOR iter(m_TrunkGroupMgr);
			for( iter = m_TrunkGroupMgr.begin();
				 iter != m_TrunkGroupMgr.end();
				 ++iter )
			{
				pTrunkGroup = (*iter).int_id_;
				if ( pTrunkGroup->m_TrunkName == strTrunkName )
				{
					strDeviceID = pTrunkGroup->GetDahdiTrunkDn( ACE_OS::atoi(strCircuit.c_str()) );
					
					if( strDeviceID != "")
					{
						nDeviceType = pTrunkGroup->m_TrunkType; // DTYPE_TRUNK_DAHDI;
						strTACCode = pTrunkGroup->m_TACCode;
						bOk = Smt_BoolTRUE;
						break;
					}
				}
			}
		} // end if-1

		if( bOk == Smt_BoolTRUE ) break;

		// 带有 @ 的内部通道处理, 如: Local/6000@from-internal-custom-0868
		TAssignedDevice* pDevice = NULL;
		if( nPosAt != -1 )         
		{
			strDeviceID = channel.substr(nPosSlash+1, nPosAt-nPosSlash-1 );
			if(m_DeviceMgr.Lookup(strDeviceID, pDevice) == Smt_Success)
			{
				nDeviceType = pDevice->m_DeviceType;
			}
			else
			{
				nDeviceType = DTYPE_EXTENSION;
			}
			break;
		}
		
		// 内部分机, 如:SIP/3002-0976e790
		strDeviceID = channel.substr(nPosSlash+1, nPosBar-nPosSlash-1 );
		if(m_DeviceMgr.Lookup(strDeviceID, pDevice) == Smt_Success)
		{
			nDeviceType = pDevice->m_DeviceType;
		}
		else
		{
			nDeviceType = DTYPE_EXTENSION;
		}
	
	} while (0);

	deviceid = strDeviceID;
	devicetype = nDeviceType;
	taccode = strTACCode;

	if( strDeviceID == "" )
	{
		PrintLog(3, "[TConnectionState::GetDeviceID] Get DeviceID Fail, Channel<%s>.",
			channel.c_str() );
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: GetIDName
参    数:
返回数值: 
功能描述: ID 转换成 String
*****************************************************************************/
Smt_String TConnectionState::GetIDName(Smt_Uint nid)
{
	switch(nid)
	{
		CASE_STR(Cmd_ICMU_Assign                    ) 
		CASE_STR(Resp_ICMU_Assign                   )
		CASE_STR(Cmd_ICMU_Deassign                  ) 
		CASE_STR(Resp_ICMU_Deassign                 )
		CASE_STR(Cmd_ICMU_MakeCall                  ) 
		CASE_STR(Resp_ICMU_MakeCall                 )
		CASE_STR(Cmd_ICMU_AnswerCall                ) 
		CASE_STR(Resp_ICMU_AnswerCall               )
		CASE_STR(Cmd_ICMU_HangupCall                ) 
		CASE_STR(Resp_ICMU_HangupCall               )
		CASE_STR(Cmd_ICMU_SingleStepTransfer        ) 
		CASE_STR(Resp_ICMU_SingleStepTransfer       )
		CASE_STR(Cmd_ICMU_SingleStepConference      ) 
		CASE_STR(Resp_ICMU_SingleStepConference     )
		CASE_STR(Cmd_ICMU_SendDTMF                  ) 
		CASE_STR(Resp_ICMU_SendDTMF                 )
		CASE_STR(Cmd_ICMU_RouteSelected             ) 
		CASE_STR(Resp_ICMU_RouteSelected            )
		CASE_STR(Cmd_ICMU_StartRecord               ) 
		CASE_STR(Resp_ICMU_StartRecord              )
		CASE_STR(Cmd_ICMU_StopRecord                ) 
		CASE_STR(Resp_ICMU_StopRecord               )
		CASE_STR(Cmd_ICMU_SetData                   ) 
		CASE_STR(Resp_ICMU_SetData                  )
		CASE_STR(Cmd_ICMU_GetData                   ) 
		CASE_STR(Resp_ICMU_GetData                  )
		CASE_STR(Cmd_ICMU_MakeCallEx                ) 
		CASE_STR(Resp_ICMU_MakeCallEx               )
		CASE_STR(Cmd_ICMU_AnswerCallEx              ) 
		CASE_STR(Resp_ICMU_AnswerCallEx             )
		CASE_STR(Cmd_ICMU_HangupCallEx              ) 
		CASE_STR(Resp_ICMU_HangupCallEx             )
		CASE_STR(Cmd_ICMU_SingleStepTransferEx      ) 
		CASE_STR(Resp_ICMU_SingleStepTransferEx     )
		CASE_STR(Cmd_ICMU_PlayFile                  ) 
		CASE_STR(Resp_ICMU_PlayFile                 )
		CASE_STR(Cmd_ICMU_GetDigits                 ) 
		CASE_STR(Resp_ICMU_GetDigits                )
		CASE_STR(Cmd_ICMU_RecordEx                  ) 
		CASE_STR(Resp_ICMU_RecordEx                 )
		CASE_STR(Cmd_ICMU_SendDTMFEx                ) 
		CASE_STR(Resp_ICMU_SendDTMFEx               )
		CASE_STR(Cmd_ICMU_SendFax                   ) 
		CASE_STR(Resp_ICMU_SendFax                  )
		CASE_STR(Cmd_ICMU_ReceiveFax                ) 
		CASE_STR(Resp_ICMU_ReceiveFax               )
		CASE_STR(Cmd_ICMU_ExecAMI                   ) 
		CASE_STR(Resp_ICMU_ExecAMI                  )
		CASE_STR(Cmd_ICMU_ExecAGI                   ) 
		CASE_STR(Resp_ICMU_ExecAGI                  )
		CASE_STR(Cmd_ICMU_SubscribeCallEvent        ) 
		CASE_STR(Resp_ICMU_SubscribeCallEvent       )
		CASE_STR(Cmd_ICMU_UnsubscribeCallEvent      ) 
		CASE_STR(Resp_ICMU_UnsubscribeCallEvent     )
		CASE_STR(Cmd_ICMU_PlayFileList              ) 
		CASE_STR(Resp_ICMU_PlayFileList             )
		CASE_STR(Cmd_ICMU_MemorySnapShot            )
		CASE_STR(Resp_ICMU_MemorySnapShot           )
		CASE_STR(Cmd_ICMU_DeviceSnapshot            )
		CASE_STR(Resp_ICMU_DeviceSnapshot           )
		CASE_STR(Cmd_ICMU_QueryLinkState            )
		CASE_STR(Resp_ICMU_QueryLinkState           )
		CASE_STR(Cmd_ICMU_Dial                      )
		CASE_STR(Resp_ICMU_Dial                     )
		CASE_STR(Cmd_ICMU_RetryDial                 )
		CASE_STR(Resp_ICMU_RetryDial                )
		CASE_STR(Cmd_ICMU_SendMessage               ) 
		CASE_STR(Resp_ICMU_SendMessage              )
		CASE_STR(Cmd_ICMU_SendMessageEx             ) 
		CASE_STR(Resp_ICMU_SendMessageEx            )
		CASE_STR(Cmd_ICMU_HoldCall                  )
		CASE_STR(Resp_ICMU_HoldCall                 )
		CASE_STR(Cmd_ICMU_RetrieveCall              )
		CASE_STR(Resp_ICMU_RetrieveCall             )
		CASE_STR(Cmd_ICMU_ConsultationCall          )
		CASE_STR(Resp_ICMU_ConsultationCall         )
		CASE_STR(Cmd_ICMU_TransferCall              )
		CASE_STR(Resp_ICMU_TransferCall             )
		CASE_STR(Cmd_ICMU_ConferenceCall            )
		CASE_STR(Resp_ICMU_ConferenceCall           )
		CASE_STR(Cmd_ICMU_ReconnectCall             )
		CASE_STR(Resp_ICMU_ReconnectCall            )
		CASE_STR(Cmd_ICMU_DeflectCall               )
		CASE_STR(Resp_ICMU_DeflectCall              )
		CASE_STR(Cmd_ICMU_PickupCall                )
		CASE_STR(Resp_ICMU_PickupCall               )
		CASE_STR(Cmd_ICMU_GetVariable               )
		CASE_STR(Resp_ICMU_GetVariable              )
		CASE_STR(Cmd_ICMU_AssignEx                  ) 
		CASE_STR(Resp_ICMU_AssignEx                 )
		CASE_STR(Cmd_ICMU_DeassignEx                ) 
		CASE_STR(Resp_ICMU_DeassignEx               )
		CASE_STR(Cmd_ICMU_ClearCall                 ) 
		CASE_STR(Resp_ICMU_ClearCall                )
		CASE_STR(Cmd_ICMU_SetSIPHeader              ) 
		CASE_STR(Resp_ICMU_SetSIPHeader             )
		CASE_STR(Cmd_ICMU_GetSIPHeader              ) 
		CASE_STR(Resp_ICMU_GetSIPHeader             )

		CASE_STR(Evt_ICMU_Newchannel                )
		CASE_STR(Evt_ICMU_Newcallerid               )
		CASE_STR(Evt_ICMU_Dial                      )
		CASE_STR(Evt_ICMU_Link                      )
		CASE_STR(Evt_ICMU_Unlink                    )
		CASE_STR(Evt_ICMU_Hangup                    )
		CASE_STR(Evt_ICMU_Hold                      )
		CASE_STR(Evt_ICMU_Unhold                    )
		CASE_STR(Evt_ICMU_Join                      )
		CASE_STR(Evt_ICMU_Leave                     )
		CASE_STR(Evt_ICMU_ParkedCall                )
		CASE_STR(Evt_ICMU_UnParkedCall              )
		CASE_STR(Evt_ICMU_ParkedCallGiveUp          )
		CASE_STR(Evt_ICMU_AMIResponse               )
		CASE_STR(Evt_ICMU_UserEvent                 )
		CASE_STR(Evt_ICMU_PeerStatus                )
		CASE_STR(Evt_ICMU_OriginateResponse         )
		CASE_STR(Evt_ICMU_DTMFReceived              )
		CASE_STR(Evt_ICMU_MessageReceived           )
		CASE_STR(Evt_ICMU_MeetmeJoin                )
		CASE_STR(Evt_ICMU_MeetmeLeave               )
		CASE_STR(Evt_ICMU_Initiated                 )
		CASE_STR(Evt_ICMU_Originated                )
		CASE_STR(Evt_ICMU_Alerting                  )
		CASE_STR(Evt_ICMU_Queued                    )
		CASE_STR(Evt_ICMU_Connected                 )
		CASE_STR(Evt_ICMU_Held                      )
		CASE_STR(Evt_ICMU_Retrieved                 )
		CASE_STR(Evt_ICMU_Failed                    )
		CASE_STR(Evt_ICMU_Disconnected              )
		CASE_STR(Evt_ICMU_Released                  )
		CASE_STR(Evt_ICMU_BackInService             )
		CASE_STR(Evt_ICMU_OutOfService              )
		CASE_STR(Evt_ICMU_ActionTimer               )
		CASE_STR(Evt_ICMU_Recording                 )
		CASE_STR(Evt_ICMU_RecordEnd                 )
		CASE_STR(Evt_ICMU_Playing                   )
		CASE_STR(Evt_ICMU_PlayEnd                   )
		CASE_STR(Evt_ICMU_Sending                   )
		CASE_STR(Evt_ICMU_SendEnd                   )
		CASE_STR(Evt_ICMU_AGIRequest                )
		CASE_STR(Evt_ICMU_AGIResponse               )
		CASE_STR(Evt_ICMU_AGIEvtTimer               )
		CASE_STR(Evt_ICMU_Geting                    )
		CASE_STR(Evt_ICMU_GetEnd                    )
		CASE_STR(Evt_ICMU_CallInitiated             )
		CASE_STR(Evt_ICMU_CallDelivered             )
		CASE_STR(Evt_ICMU_CallConnected             )
		CASE_STR(Evt_ICMU_CallHeld                  )
		CASE_STR(Evt_ICMU_CallRetrieved             )
		CASE_STR(Evt_ICMU_CallConferenced           )
		CASE_STR(Evt_ICMU_CallQueued                )
		CASE_STR(Evt_ICMU_CallTransferred           )
		CASE_STR(Evt_ICMU_CallCleared               )
		CASE_STR(Evt_ICMU_CallFailed                )
		CASE_STR(Evt_ICMU_RouteRequest              )
		CASE_STR(Evt_ICMU_RouteEnd                  )
		CASE_STR(Evt_ICMU_OffHook                   )
		CASE_STR(Evt_ICMU_InboundCall               )
		CASE_STR(Evt_ICMU_DestSeized                )
		CASE_STR(Evt_ICMU_TpAnswered                )
		CASE_STR(Evt_ICMU_OpAnswered                )
		CASE_STR(Evt_ICMU_TpSuspended               )
		CASE_STR(Evt_ICMU_OpHeld                    )
		CASE_STR(Evt_ICMU_TpRetrieved               )
		CASE_STR(Evt_ICMU_OpRetrieved               )
		CASE_STR(Evt_ICMU_TpDisconnected            )
		CASE_STR(Evt_ICMU_OpDisconnected            )
		CASE_STR(Evt_ICMU_TpTransferred             )
		CASE_STR(Evt_ICMU_OpTransferred             )
		CASE_STR(Evt_ICMU_TpConferenced             )
		CASE_STR(Evt_ICMU_OpConferenced             )
		CASE_STR(Evt_ICMU_DestBusy                  )
		CASE_STR(Evt_ICMU_DestInvalid               )
		CASE_STR(Evt_ICMU_DeviceRecording           )
		CASE_STR(Evt_ICMU_DeviceRecordEnd           )
		CASE_STR(Evt_ICMU_LinkDown                  )
		CASE_STR(Evt_ICMU_LinkUp                    )
		CASE_STR(Evt_ICMU_QueryLinkTimer            )
		CASE_STR(Evt_ICMU_DialResponse              )
		CASE_STR(Evt_ICMU_TrunkDnLockedTimer        )
		CASE_STR(Evt_ICMU_MessageSending            )
		CASE_STR(Evt_ICMU_MessageSendEnd            )
		CASE_STR(Evt_ICMU_CallOriginated            )
		CASE_STR(Evt_ICMU_ChannelDataReached        )
		CASE_STR(Evt_ICMU_DeviceTimer               )
		CASE_STR(Evt_ICMU_DeviceTimerExpired        )
		CASE_STR(Evt_ICMU_ReRoute                   )
	default:
	       return HLFormatStr("_?(%d)", nid);
	}
	return "";
}

/****************************************************************************
函 数 名: ReleaseConnection
参    数:
返回数值: 
功能描述: 释放 Connection
*****************************************************************************/
Smt_Uint TConnectionState::ReleaseConnection( TConnection* pconn )
{
	Smt_Uint nRet = Smt_Success;
	Smt_String strConnectionID;

	if( pconn != NULL )
	{
		strConnectionID = pconn->GetID();
		if( m_ConnectionMgr.FindByKey(strConnectionID) == Smt_Success )
		{
			m_ConnectionMgr.Remove( strConnectionID );
		}	

		delete pconn;
		pconn = NULL;
	}

	PrintLog(5,"[TConnectionState::ReleaseConnection] ConnectionID<%s>.",
		strConnectionID.c_str() );

	return nRet;
}

/****************************************************************************
函 数 名: OnEvtPeerStatus
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtPeerStatus(Smt_Pdu &pdu)
{
	Smt_String strPeer;
	Smt_String strPeerStatus;
	Smt_String strDeviceID;

	pdu.GetString( Key_ICMU_Peer, &strPeer );
	pdu.GetString( Key_ICMU_PeerStatus, &strPeerStatus );

	Smt_Int nPos = strPeer.find("/");
	if( nPos > 0 )
	{
		strDeviceID = strPeer.substr( nPos + 1, strPeer.length() - (nPos+1) );
	}

	Smt_Pdu sendpdu;
	sendpdu.m_Receiver = m_DeviceStateGOR;
	sendpdu.m_Sender = GetGOR();
	sendpdu.m_Status = Smt_Success;

	sendpdu.PutString( Key_ICMU_DeviceID, strDeviceID );

//	if( strPeerStatus == PEER_STATUS_Registered )
	if( strPeerStatus == PEER_STATUS_Registered || strPeerStatus == PEER_STATUS_Reachable)//add by caoyj 20111221
	{
		sendpdu.m_MessageID = Evt_ICMU_BackInService;
		sendpdu.PutUint  ( Key_ICMU_Reason, St_ActiveState );//2011-06-18 add by caoyj 
		if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );
	}
	else if( strPeerStatus == PEER_STATUS_Unregistered  )
	{
		sendpdu.m_MessageID = Evt_ICMU_OutOfService;
		sendpdu.PutUint  ( Key_ICMU_Reason, St_UnavailableState );//2011-06-18 add by caoyj 
		if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );
	}
	else if( strPeerStatus == PEER_STATUS_Unreachable  )  //2011-06-18 add by caoyj 
	{
		sendpdu.m_MessageID = Evt_ICMU_OutOfService;
		sendpdu.PutUint  ( Key_ICMU_Reason, St_FailState );
		if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );
	}
	else
	{
		// do nothing
	}

	PrintLog(5,"[TConnectionState::OnEvtPeerStatus] DeviceID<%s>, PeerStatus<%s>.",
		strDeviceID.c_str(), strPeerStatus.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtAMIResponse
参    数:
返回数值: 
功能描述: LinkUp 时，发送LinkUp Event、InitDialplan
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtAMIResponse(Smt_Pdu &pdu)
{
	Smt_Uint nRet = Smt_Success;
	Smt_String strActID;
	Smt_String strResult;

	pdu.GetString( Key_ICMU_ActionID, &strActID );

	if( strActID == ACTION_LINKCONNECTED)
	{
		Smt_String strMsg;
		pdu.GetString( Key_ICMU_Message, &strMsg );
		if( strMsg == AMI_RESPONSE_ACCEPTED )
		{
			EvtLinkUp( CauseSuccess );
			// Dialplan_AddRouteSelected();
		}		
	}

	TActionPdu* pAct = NULL;
	Smt_String strConnectionID="";
	if(m_ActMgr.Lookup( strActID, pAct) == Smt_Success )
	{
		pAct->m_Pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );

		switch ( pAct->m_Pdu.m_MessageID )
		{
		case Cmd_ICMU_Assign:
			RespAssign( pdu, pAct->m_Pdu );
			break;
		case Cmd_ICMU_MakeCall: // do noting, waiting for Event: OriginateResponse
		case Cmd_ICMU_SingleStepConference:
			return Smt_Success;
		case Cmd_ICMU_HangupCall:
		case Cmd_ICMU_SingleStepTransfer:		
		case Cmd_ICMU_SendDTMF:
		case Cmd_ICMU_RouteSelected:
		case Cmd_ICMU_SendMessage:
			Resp_AmiCmd( pdu, pAct->m_Pdu );
			break;
		case Cmd_ICMU_StartRecord:
			RespStartRecord( pdu, pAct->m_Pdu );
			break;
		case Cmd_ICMU_StopRecord:
			RespStopRecord( pdu, pAct->m_Pdu );
			break;
		case Cmd_ICMU_GetVariable:
			OnRespGetVariable( pdu, pAct->m_Pdu );
			break;
		default:
			PrintLog(3,"[TConnectionState::OnEvtAMIResponse] Can not find Original Action, ActionID<%s>.",
				strActID.c_str() );
			break;
		}

		ClearTimer( pAct->m_TimerID );

		m_ActMgr.Remove( pAct->m_ActionID );

		if( pAct != NULL )
		{
			delete pAct;
			pAct = NULL;
		}
	}

	PrintLog(5,"[TConnectionState::OnEvtAMIResponse] ConnectionID<%s>,ActionID<%s>.",
		strConnectionID.c_str(),strActID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdAssign
参    数:
返回数值: 
功能描述: 打开设备监视
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdAssign(Smt_Pdu &pdu)
{
	Smt_String strActID = "";
	Smt_String strDeviceID;
	Smt_Uint   nDeviceType;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_DeviceType, &nDeviceType );

	// add device to map
	TAssignedDevice* pDevice = NULL;
	if(m_DeviceMgr.Lookup(strDeviceID, pDevice) != Smt_Success )
	{
		pDevice = new TAssignedDevice();
		pDevice->m_DeviceType = nDeviceType;
		pDevice->m_DeviceID = strDeviceID;
		m_DeviceMgr.SetAt(strDeviceID, pDevice);
	}

	if( nDeviceType == DTYPE_EXTENSION )
	{
		strActID = GenActionID( pdu );
		g_pAstConnector->Ami_SIPShowPeer( m_AmiHandle, strActID, strDeviceID );
	}
	else //if( nDeviceType == DTYPE_ROUTE )
	{
		Smt_Pdu newpdu;
		newpdu.PutString( Key_ICMU_ResponseResult, RESPONSE_EVENT_SUCCESS );
		newpdu.PutString( Key_ICMU_Status, PEER_STATUS_OK );

		RespAssign( newpdu, pdu );
		                        
		if( (nDeviceType == DTYPE_ROUTE) ||
			(nDeviceType == DTYPE_IVR_INBOUND) ||
			(nDeviceType == DTYPE_IVR_OUTBOUND) )
		{
			Dialplan_AddDevice( pDevice );
		}
	}
	
	PrintLog(5,"[TConnectionState::OnCmdAssign] DeviceID<%s>, ActionID<%s>.",
		strDeviceID.c_str(), strActID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: RespAssign
参    数:
返回数值: 
功能描述: 返回 CmdAssign 命令
*****************************************************************************/
Smt_Uint TConnectionState::RespAssign(Smt_Pdu &newpdu, Smt_Pdu &oripdu)
{
	Smt_Uint   nStatus;
	Smt_String strResult;
	Smt_String strDeviceStatus;
	Smt_String strAddressIP;
	Smt_String strAddressPort;
	Smt_Uint   nDeviceStatus;

	newpdu.GetString( Key_ICMU_ResponseResult, &strResult );
	newpdu.GetString( Key_ICMU_Status, &strDeviceStatus );
	newpdu.GetString( Key_ICMU_AddressIP, &strAddressIP );
	newpdu.GetString( Key_ICMU_AddressPort, &strAddressPort );
	
	if( strResult == RESPONSE_EVENT_SUCCESS )
	{
		nStatus = Smt_Success;
	}
	else
		nStatus = Smt_Fail;
/*
	if( ACE_OS::strcasecmp(strDeviceStatus.c_str(), PEER_STATUS_UNKNOWN ) == 0 )
	{
		nDeviceStatus = CauseUnregistered;
	}
	else // PEER_STATUS_OK
	{
		nDeviceStatus = CauseRegistered;
	}
*/  //2011-06-18 add by caoyj 
	if( ACE_OS::strcasecmp(strDeviceStatus.substring(0,2).c_str(), PEER_STATUS_OK ) == 0 )
	{// PEER_STATUS_OK
		nDeviceStatus = CauseRegistered;
	}
	else 
	{
		nDeviceStatus = CauseUnregistered;
	}


	Smt_String strDeviceID;
	oripdu.GetString( Key_ICMU_DeviceID, &strDeviceID );

	// resp action
	Smt_Pdu sendpdu;
	sendpdu = oripdu;

	sendpdu.m_Sender = oripdu.m_Receiver;
	sendpdu.m_Receiver = oripdu.m_Sender;
	sendpdu.m_MessageID = Resp_ICMU_Assign;
	sendpdu.m_Status = nStatus;

	sendpdu.PutUint  ( Key_ICMU_DeviceStatus, nDeviceStatus ); 
	sendpdu.PutString( Key_ICMU_DeviceIP, strAddressIP );
	sendpdu.PutUint  ( Key_ICMU_DevicePort, ACE_OS::atoi(strAddressPort.c_str()) );

	if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );

	PrintLog(5,"[TConnectionState::RespAssign] DeviceID<%s>, DeviceStatus<%d:%s>.",
		strDeviceID.c_str(), nDeviceStatus,strDeviceStatus.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdMakeCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdMakeCall(Smt_Pdu &pdu)
{
	Smt_String strActID;
	Smt_String strCallerNum;
	Smt_String strCalledNum;
	Smt_String strDeviceID;
	Smt_Uint   nDeviceType;
	Smt_Uint   nTimeout;
	Smt_String strVariable;
	Smt_String strContext;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetUint  ( Key_ICMU_DeviceType, &nDeviceType );
	pdu.GetString( Key_ICMU_CallerNum, &strCallerNum );
	pdu.GetString( Key_ICMU_CalledNum, &strCalledNum );
	pdu.GetUint  ( Key_ICMU_Timeout, &nTimeout );          // < MAX_ACTIONTIMERLEN
	pdu.GetString( Key_ICMU_Variable, &strVariable );
	pdu.GetString( Key_ICMU_Context, &strContext );

	if( nTimeout == 0 )
	{
		nTimeout = DEFAULT_MAKECALLTIMEOUT;		
	}

	if( strContext == "" )
	{
		strContext = AMI_CONTEXT_INTERNAL;
	}

	if( nDeviceType == DTYPE_EXTENSION )
	{
		strActID = GenActionID( pdu );
		
		//Smt_String strChannel = HLFormatStr("%s/%s", CHANNELTYPE_SIP, strDeviceID.c_str() );
		Smt_String strChannel = ParseCalledNum( strDeviceID );
		
		g_pAstConnector->Ami_Originate( 
			m_AmiHandle, strActID, strChannel, 
			strCalledNum, strCallerNum, strVariable,
			HLFormatStr("%d", nTimeout ), strContext, AMI_PRIORITY );
	}
	else if( nDeviceType == DTYPE_IVR_OUTBOUND )
	{
		strActID = GenActionID( pdu );
		
		strVariable = HLFormatStr("%s=%s",
			AMI_CHANNEL_VARIABLE_VAR1, AMI_IVR_MAKECALL );

		Smt_String strChannel = HLFormatStr("%s/%s@%s", CHANNELTYPE_LOCAL, strDeviceID.c_str(), AMI_CONTEXT_INTERNAL );
		
		g_pAstConnector->Ami_Originate( 
			m_AmiHandle, strActID, strChannel, 
			strCalledNum, strCallerNum, strVariable, 
			HLFormatStr("%d", nTimeout ), strContext, AMI_PRIORITY );	
	}
	
	PrintLog(5,"[TConnectionState::OnCmdMakeCall] DeviceID<%s>, DeviceType<%d>, ActionID<%s>, CallerNum<%s>, strCalledNum<%s>, Timeout<%d>.",
		strDeviceID.c_str(), nDeviceType, strActID.c_str(), strCallerNum.c_str(), strCalledNum.c_str(), nTimeout );
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdHangupCall
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdHangupCall(Smt_Pdu &pdu)
{
	Smt_String strActID;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	TConnection* pConnection = NULL;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	
	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
// 		pConnection->m_LastCommand = pdu;
// 		strActID = GenActionID( pdu );
// 		
// 		g_pAstConnector->Ami_Hangup( m_AmiHandle, strActID, pConnection->m_Channel );
//add by caoyj 20120315
		if( pConnection->m_CurrContext == AMI_CONTEXT_INTERNAL_XFER )
		{
			pConnection->m_CurrContext = "";
			
			PrintLog(3,"[TConnectionState::OnCmdHangupCall] Hi, DTMF BlindTransfer, Do Not HangupCall, DeviceID<%s>, ConnectionID<%s>.",
				strDeviceID.c_str(), strConnectionID.c_str());
		}
		else // hangup call
		{
			pConnection->m_LastCommand = pdu;
			strActID = GenActionID( pdu );
			
			g_pAstConnector->Ami_Hangup( m_AmiHandle, strActID, pConnection->m_Channel );
		}
////////////////////////////
	}

	PrintLog(5,"[TConnectionState::OnCmdHangupCall] DeviceID<%s>, ActionID<%s>, ConnectionID<%s>.",
		strDeviceID.c_str(), strActID.c_str(), strConnectionID.c_str());
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdRouteSelected
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdRouteSelected(Smt_Pdu &pdu)
{
	Smt_String strActID;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	Smt_String strCalledNum;
	Smt_String strAgentID;
	TConnection* pConnection = NULL;
	Smt_String strContext;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_CalledNum, &strCalledNum );
	pdu.GetString( Key_ICMU_AgentID, &strAgentID );
	
	// 如果是内部分机，直接转自定义拨号方案，可以提高转接成功率: from-custom-dial
	strContext = AMI_CONTEXT_INTERNAL;
	
	TAssignedDevice* pDevice = NULL;
	if( m_DeviceMgr.Lookup( strCalledNum, pDevice ) == Smt_Success )
	{
		if( pDevice->m_DeviceType == DTYPE_EXTENSION )
		{
			strContext = AMI_CONTEXT_CUSTOM_DIAL;
		}
	}			

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		pConnection->m_LastCommand = pdu;
		strActID = GenActionID( pdu );
		
		g_pAstConnector->Ami_Redirect( 
			m_AmiHandle, strActID, pConnection->m_Channel, "", strCalledNum, 
			strContext, AMI_PRIORITY ); //AMI_CONTEXT_CUSTOM_DIAL
	}
	
	PrintLog(5,"[TConnectionState::OnCmdRouteSelected] DeviceID<%s>, ActionID<%s>, ConnectionID<%s>, CalledNum<%s>, AgentID<%s>.",
		strDeviceID.c_str(), strActID.c_str(), strConnectionID.c_str(), strCalledNum.c_str(), strAgentID.c_str() );
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdSingleStepTransfer
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdSingleStepTransfer(Smt_Pdu &pdu)
{	
	Smt_String strActID;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	Smt_String strOtherConnectionID;
	Smt_String strCalledNum;
	Smt_String strContext;
	Smt_String strChannelID = "";
	Smt_String strOtherChannelID = "";
	TConnection* pConnection = NULL;
	TConnection* pOtherConnection = NULL;

	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_OtherConnection, &strOtherConnectionID );
	pdu.GetString( Key_ICMU_CalledNum, &strCalledNum );
	pdu.GetString( Key_ICMU_Context, &strContext );

	// 如果是内部分机，直接转自定义拨号方案，可以提高转接成功率: from-custom-dial
	if( strContext == "" )
	{
		strContext = AMI_CONTEXT_INTERNAL;
		
		TAssignedDevice* pDevice = NULL;
		if( m_DeviceMgr.Lookup( strCalledNum, pDevice ) == Smt_Success )
		{
			if( pDevice->m_DeviceType == DTYPE_EXTENSION )
			{
				strContext = AMI_CONTEXT_CUSTOM_DIAL;
			}
		}
	}		

	if( m_ConnectionMgr.Lookup(strOtherConnectionID, pOtherConnection) == Smt_Success )
	{
		strOtherChannelID = pOtherConnection->m_Channel;
	}

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		strChannelID = pConnection->m_Channel;
		pConnection->m_LastCommand = pdu;
		strActID = GenActionID( pdu );

		g_pAstConnector->Ami_Redirect( m_AmiHandle, strActID, strChannelID, strOtherChannelID, strCalledNum, strContext, AMI_PRIORITY ); // 
	}

	PrintLog(5,"[TConnectionState::OnCmdSingleStepTransfer] DeviceID<%s>, ActionID<%s>, ConnectionID<%s>, OtherChannelID<%s>, CalledNum<%s>, Context<%s>.",
			 strDeviceID.c_str(), strActID.c_str(), strConnectionID.c_str(), strOtherConnectionID.c_str(), strCalledNum.c_str(), strContext.c_str() );
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdSingleStepConference
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdSingleStepConference(Smt_Pdu &pdu)
{
	Smt_String strActID;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	Smt_Uint   nJoinMode;
	Smt_Uint   nTimeout;
	TConnection* pConnection = NULL;
	
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_JoinMode, &nJoinMode );
	pdu.GetUint  ( Key_ICMU_Timeout, &nTimeout );
	
	if( nTimeout == 0 )
	{
		nTimeout = DEFAULT_MAKECALLTIMEOUT;		
	}

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		pConnection->m_LastCommand = pdu;
		strActID = GenActionID( pdu );
				
		Smt_String strChannel = ParseCalledNum( strDeviceID );
		Smt_String strApplication = AMI_APP_CHANSPY;
		Smt_String strData;
		Smt_String strCallerID;

		if( nJoinMode == JOIN_ACTIVE )
		{
			strData = HLFormatStr("%s|qw", pConnection->m_Channel.c_str() );
		}
		else // JOIN_SILENT
		{
			strData = HLFormatStr("%s|q", pConnection->m_Channel.c_str() );
		}

		strCallerID = pConnection->m_DeviceID;

		g_pAstConnector->Ami_Originate( 
			m_AmiHandle, strActID, strChannel, 
			strApplication, strData, strCallerID, "", 
			HLFormatStr("%d", nTimeout ) );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdSingleStepConference] DeviceID<%s>, ActionID<%s>, ConnectionID<%s>, JoinMode<%d>, TimeOut<%d>.",
		strDeviceID.c_str(), strActID.c_str(), strConnectionID.c_str(), nJoinMode, nTimeout );
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdSendDTMF
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdSendDTMF(Smt_Pdu &pdu)
{
	Smt_String strActID;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	Smt_String strDigits;
	TConnection* pConnection = NULL;
	
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_DTMFDigits, &strDigits );
	
	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		strActID = GenActionID( pdu );
			
		Smt_String cDigit;
		Smt_Uint nCount = strDigits.length();
		for(Smt_Uint i=0; i<nCount; i++ )
		{
			cDigit = strDigits.substr( i, 1 );
			g_pAstConnector->Ami_PlayDTMF( m_AmiHandle, strActID, pConnection->m_Channel, cDigit );
		}		
	}
	
	PrintLog(5,"[TConnectionState::OnCmdSendDTMF] DeviceID<%s>, ActionID<%s>, ConnectionID<%s>, Digits<%s>.",
		strDeviceID.c_str(), strActID.c_str(), strConnectionID.c_str(), strDigits.c_str() );
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdStartRecord
参    数:
返回数值: 
功能描述: 使用 tmpfs 录音方式，即文件录到"/dev/shm/*.*";
          录音完成后，由录音服务器把文件转移到真实路径上;
		  录音服务器实现两个功能：in-out文件混音；wav-->mp3转换；
*****************************************************************************/
#define RECORD_MEMORY_PATH       "/dev/shm/"
#define RECORD_EXTEN_FILENAME    ".mp3"
Smt_Uint TConnectionState::OnCmdStartRecord(Smt_Pdu &pdu)
{
	Smt_String strActID;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	Smt_String strFileName;
	Smt_String strTmpFileName;
	TConnection* pConnection = NULL;
	
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_FileName, &strFileName );
	
	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		if( pConnection->m_Recording == Smt_BoolTRUE )
		{
			PrintLog(5,"[TConnectionState::OnCmdStartRecord] Device was Recording, DeviceID<%s>, ConnectionID<%s>.",
				strDeviceID.c_str(), strConnectionID.c_str() );
			return Smt_Fail;
		}
	
		char driver[256],dir[256],filename[256],ext[256];
		::HLSplitPath((char*)strFileName.c_str(),driver, dir, filename, ext); 

		strTmpFileName = HLFormatStr("%s%s", RECORD_MEMORY_PATH, filename );

		strFileName = HLFormatStr("%s%s%s%s",driver, dir, filename, RECORD_EXTEN_FILENAME );
		pdu.PutString( Key_ICMU_FileName, strFileName );

		strActID = GenActionID( pdu );

		g_pAstConnector->Ami_Monitor( m_AmiHandle, strActID, pConnection->m_Channel, strTmpFileName );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdStartRecord] DeviceID<%s>, ActionID<%s>, ConnectionID<%s>, FileName<%s>, TempFileName<%s>.",
		strDeviceID.c_str(), strActID.c_str(), strConnectionID.c_str(), strFileName.c_str(), strTmpFileName.c_str() );
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: RespStartRecord
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::RespStartRecord(Smt_Pdu &newpdu, Smt_Pdu &oripdu)
{
	Smt_Uint   nStatus;
	Smt_String strResult;
	newpdu.GetString( Key_ICMU_ResponseResult, &strResult );
	
	if( strResult == RESPONSE_EVENT_SUCCESS )
	{
		nStatus = Smt_Success;
	}
	else
		nStatus = Smt_Fail;

	Smt_String strDeviceID;
	Smt_String strConnectionID;
	Smt_String strFileName;
	oripdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	oripdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	oripdu.GetString( Key_ICMU_FileName, &strFileName );

	// resp action
	Smt_Pdu sendpdu;
	sendpdu = oripdu;
	
	sendpdu.m_Sender = oripdu.m_Receiver;
	sendpdu.m_Receiver = oripdu.m_Sender;
	sendpdu.m_MessageID = Resp_ICMU_StartRecord;
	sendpdu.m_Status = nStatus;
		
	if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );
	
	// event recording
	if( nStatus == Smt_Success )
	{
		TConnection* pConnection = NULL;
		if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
		{
			pConnection->m_Recording = Smt_BoolTRUE;
			pConnection->m_RecordFileName = strFileName;
			pConnection->EvtRecord( Evt_ICMU_DeviceRecording );
		}
	}

	PrintLog(5,"[TConnectionState::RespStartRecord] DeviceID<%s>, Status<%d>.",
		strDeviceID.c_str(), nStatus );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdStopRecord
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdStopRecord(Smt_Pdu &pdu)
{
	Smt_String strActID;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	TConnection* pConnection = NULL;
	
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	
	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		strActID = GenActionID( pdu );
	
		g_pAstConnector->Ami_StopMonitor( m_AmiHandle, strActID, pConnection->m_Channel );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdStopRecord] DeviceID<%s>, ActionID<%s>, ConnectionID<%s>.",
		strDeviceID.c_str(), strActID.c_str(), strConnectionID.c_str() );
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: RespStopRecord
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::RespStopRecord(Smt_Pdu &newpdu, Smt_Pdu &oripdu)
{
	Smt_Uint   nStatus;
	Smt_String strResult;
	newpdu.GetString( Key_ICMU_ResponseResult, &strResult );
	
	if( strResult == RESPONSE_EVENT_SUCCESS )
	{
		nStatus = Smt_Success;
	}
	else
		nStatus = Smt_Fail;
	
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	oripdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	oripdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	
	// resp action
	Smt_Pdu sendpdu;
	sendpdu = oripdu;
	
	sendpdu.m_Sender = oripdu.m_Receiver;
	sendpdu.m_Receiver = oripdu.m_Sender;
	sendpdu.m_MessageID = Resp_ICMU_StopRecord;
	sendpdu.m_Status = nStatus;
	
	if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );
	
	// event recording
	if( nStatus == Smt_Success )
	{
		TConnection* pConnection = NULL;
		if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
		{
			pConnection->m_Recording = Smt_BoolFALSE;
			pConnection->EvtRecord( Evt_ICMU_DeviceRecordEnd );
		}
	}
	
	PrintLog(5,"[TConnectionState::RespStopRecord] DeviceID<%s>, Status<%d>.",
		strDeviceID.c_str(), nStatus );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdAnswerCallEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdAnswerCallEx(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	TConnection* pConnection = NULL;
	Smt_Uint uHandle=0;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		uHandle =  pConnection->m_AGIHandle;
		pConnection->m_LastCommand = pdu;
		//g_pAstConnector->Agi_Answer( pConnection->m_AGIHandle );
		//Resp_AgiCmd( Smt_Success, pdu );
		Smt_Uint uRetu = g_pAstConnector->Agi_Answer( pConnection->m_AGIHandle );
		Resp_AgiCmd( uRetu, pdu );
	}

	PrintLog(5,"[TConnectionState::OnCmdAnswerCallEx] Handle<0x%x>, ConnectionID<%s>.",
		uHandle,strConnectionID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdHangupCallEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdHangupCallEx(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	TConnection* pConnection = NULL;
	Smt_Uint uHandle=0;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	
	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		uHandle =  pConnection->m_AGIHandle;
		pConnection->m_LastCommand = pdu;
		//g_pAstConnector->Agi_Hangup( pConnection->m_AGIHandle );
		//Resp_AgiCmd( Smt_Success, pdu );
		Smt_Uint uRetu = g_pAstConnector->Agi_Hangup( pConnection->m_AGIHandle );
		Resp_AgiCmd( uRetu, pdu );
	}

	PrintLog(5,"[TConnectionState::OnCmdHangupCallEx] Handle<0x%x>, ConnectionID<%s>.",
		uHandle,strConnectionID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdPlayFile
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdPlayFile(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	Smt_String strFileName;
	Smt_String strEscapeDigit;
	TConnection* pConnection = NULL;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_FileName, &strFileName );
	pdu.GetString( Key_ICMU_EscapeDigit, &strEscapeDigit );
	
	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		pConnection->m_LastCommand = pdu;
		pConnection->m_PlayFileName = strFileName;
/*		g_pAstConnector->Agi_StreamFile( pConnection->m_AGIHandle, strFileName, strEscapeDigit );

		Resp_AgiCmd( Smt_Success, pdu );

		pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
*/
		Smt_Uint uRetu = g_pAstConnector->Agi_StreamFile( pConnection->m_AGIHandle, strFileName, strEscapeDigit );
		Resp_AgiCmd( uRetu, pdu );
		if(uRetu ==  Smt_Success)
			pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdPlayFile] ConnectionID<%s>, FileName<%s>, EscapeDigit<%s>.",
		strConnectionID.c_str(), strFileName.c_str(), strEscapeDigit.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdPlayFileList
参    数:
返回数值: 
功能描述: 播放文件列表
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdPlayFileList(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	Smt_String strEscapeDigit;
	Smt_String strFileName;
	TConnection* pConnection = NULL;
	Smt_Kvset* kvFileNames = new Smt_Kvset[MAX_FILENAME_ARRAY];
	Smt_Uint   nFileCount;
	Smt_Uint   uAGIHandle=0;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_EscapeDigit, &strEscapeDigit );
	pdu.GetKvsetArray( Key_ICMU_FileNameArray, &kvFileNames, &nFileCount );

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		uAGIHandle = pConnection->m_AGIHandle;
		Smt_Uint i;

		pConnection->m_LastCommand = pdu;
		pConnection->m_PlayListEscapeDigit = strEscapeDigit;
		pConnection->m_PlayListIndex = 0;
		pConnection->m_PlayListTimeLen = 0;
	
		for(i=0; i<MAX_FILENAME_ARRAY; i++)
		{
			pConnection->m_PlayList[i] = "";
		}
		
		for (i=0; i<nFileCount; i++) 
		{	
			kvFileNames[i].GetString(Key_ICMU_FileName, &strFileName );

			pConnection->m_PlayListIndex++;
			pConnection->m_PlayList[pConnection->m_PlayListIndex] = strFileName;			

			PrintLog(5,"[TConnectionState::OnCmdPlayFileList] ConnectionID<%s>,Handle<0x%x>,FileName<%d><%s>.",
				strConnectionID.c_str(), uAGIHandle, pConnection->m_PlayListIndex, strFileName.c_str() );
		}
		
		pConnection->m_PlayListCurrIndex = 1;
/*		g_pAstConnector->Agi_StreamFile( pConnection->m_AGIHandle, pConnection->m_PlayList[1], strEscapeDigit );
		
		Resp_AgiCmd( Smt_Success, pdu );
		
		pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
*/
		Smt_Uint uRetu = g_pAstConnector->Agi_StreamFile( pConnection->m_AGIHandle, pConnection->m_PlayList[1], strEscapeDigit );
		Resp_AgiCmd( uRetu, pdu );
		if(uRetu ==  Smt_Success)
			pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdPlayFileList] ConnectionID<%s>,Handle<0x%x>,EscapeDigit<%s>.",
		strConnectionID.c_str(),uAGIHandle,strEscapeDigit.c_str() );

	if(kvFileNames != NULL)
	{
		delete [] kvFileNames;
		kvFileNames = NULL;
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdGetDigits
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdGetDigits(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	Smt_Uint   nTimeout;
	Smt_Uint   nMaxDigitsCount;
	TConnection* pConnection = NULL;

	Smt_String strFileName = DEFAULT_GETDIGITS_FILENAME;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetUint  ( Key_ICMU_Timeout, &nTimeout );
	pdu.GetUint  ( Key_ICMU_MaxDigitsCount, &nMaxDigitsCount );
	
	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		Resp_AgiCmd( Smt_Success, pdu );

		// 处理 playfile 的遗留数据
		Smt_Uint nCount = nMaxDigitsCount - pConnection->m_AGIData.length();

		if( nCount>0 )
		{
			pConnection->m_LastCommand = pdu;
			pConnection->m_PlayFileName = strFileName;
/*			g_pAstConnector->Agi_GetData( pConnection->m_AGIHandle, strFileName, HLFormatStr("%d",nTimeout), HLFormatStr("%d",nCount) );

			pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
*/
			Smt_Uint uRetu = g_pAstConnector->Agi_GetData( pConnection->m_AGIHandle, strFileName, HLFormatStr("%d",nTimeout), HLFormatStr("%d",nCount) );
			if(uRetu ==  Smt_Success)
				pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
		}
		else // DTMF 已足够
		{
			pConnection->EvtMedia( Evt_ICMU_Geting );
			pConnection->EvtMedia( Evt_ICMU_GetEnd );
		}		
	}
	
	PrintLog(5,"[TConnectionState::OnCmdGetDigits] ConnectionID<%s>, FileName<%s>, TimeOut<%d>, MaxDigitsCount<%d>.",
		strConnectionID.c_str(), strFileName.c_str(), nTimeout, nMaxDigitsCount );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdRecordEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdRecordEx(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	Smt_String strFileName;
	Smt_String strEscapeDigit;
	Smt_Uint   nTimeout;
	Smt_Uint   nSilence;
	TConnection* pConnection = NULL;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_FileName, &strFileName );
	pdu.GetString( Key_ICMU_EscapeDigit, &strEscapeDigit );
	pdu.GetUint  ( Key_ICMU_Timeout, &nTimeout );
	pdu.GetUint  ( Key_ICMU_Silence, &nSilence );
	
	if( nTimeout<MAX_AGIEVTTIMERLEN_RECORD)
	{
		nTimeout = 2*MAX_AGIEVTTIMERLEN_RECORD;
	}

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		pConnection->m_LastCommand = pdu;
		pConnection->m_PlayFileName = strFileName;
/*		g_pAstConnector->Agi_Record( pConnection->m_AGIHandle, strFileName, strEscapeDigit, HLFormatStr("%d",nTimeout), HLFormatStr("%d",nSilence) );
		
		Resp_AgiCmd( Smt_Success, pdu );
	
		pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN_RECORD, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
*/
		Smt_Uint uRetu = g_pAstConnector->Agi_Record( pConnection->m_AGIHandle, strFileName, strEscapeDigit, HLFormatStr("%d",nTimeout), HLFormatStr("%d",nSilence) );
		Resp_AgiCmd( uRetu, pdu );
		if(uRetu ==  Smt_Success)
			pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN_RECORD, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdRecordEx] ConnectionID<%s>, FileName<%s>.",
		strConnectionID.c_str(), strFileName.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdSendDTMFEx
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdSendDTMFEx(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	Smt_String strDTMFDigits;
	TConnection* pConnection = NULL;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_DTMFDigits, &strDTMFDigits );
	
	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		pConnection->m_LastCommand = pdu;
		pConnection->m_AGIData = strDTMFDigits;
/*		g_pAstConnector->Agi_Exec_SendDTMF( pConnection->m_AGIHandle, strDTMFDigits );
		
		Resp_AgiCmd( Smt_Success, pdu );
		
		pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
*/
		Smt_Uint uRetu = g_pAstConnector->Agi_Exec_SendDTMF( pConnection->m_AGIHandle, strDTMFDigits );
		Resp_AgiCmd( uRetu, pdu );
		if(uRetu ==  Smt_Success)
			pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdSendDTMFEx] ConnectionID<%s>, DTMFDigits<%s>.",
		strConnectionID.c_str(), strDTMFDigits.c_str() );

	return Smt_Success;
}

// 
// Smt_Uint TConnectionState::OnCmdPlayText(Smt_Pdu &pdu)
// {
// 	Smt_String strConnectionID;
// 	Smt_String strappOptions;
// 	TConnection* pConnection = NULL;
// 	
// 	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
// 	pdu.GetString( Key_ICMU_ApplicationOptions, &strappOptions );
// 	
// 	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
// 	{
// 		pConnection->m_LastCommand = pdu;
// 		pConnection->m_AGIData = strappOptions;
// 		g_pAstConnector->Agi_Exec_PlayText( pConnection->m_AGIHandle, strappOptions );
// 		Resp_AgiCmd( Smt_Success, pdu );
// 		
// 		pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
// 	}
// 	
// 	PrintLog(5,"[TConnectionState::OnCmdPlayText] ConnectionID<%s>, strappOptions<%s>.",
// 		strConnectionID.c_str(), strappOptions.c_str() );
// 
// 	return Smt_Success;
// }

/****************************************************************************
函 数 名: OnCmdExecAMI
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdExecAMI(Smt_Pdu &pdu)
{
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdExecAGI
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdExecAGI(Smt_Pdu &pdu)
{
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtActionTimer
参    数:
返回数值: 
功能描述: 超时删除 Cmd 存储的 Action Pdu
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtActionTimer(Smt_Uint senderobj)
{
	Smt_String strActID = "";
	TActionPdu* pAct = NULL;

	TActionMap::ITERATOR iter0( m_ActMgr );
	for (TActionMap::ENTRY *entry0 = 0;
		iter0.next (entry0) != 0;
		iter0.advance ())
	{
		pAct = entry0->int_id_;
		
		if((Smt_Uint)pAct == senderobj )
		{	
			strActID = pAct->m_ActionID;

			m_ActMgr.Remove( strActID );
			
			delete pAct;
			pAct = NULL;
			break;
		}
	}	

	PrintLog(5,"[TConnectionState::OnEvtActionTimer] ActionID<%s>.", strActID.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: Resp_AmiCmd
参    数:
返回数值: 
功能描述: 通用的 Resp 返回消息
*****************************************************************************/
Smt_Uint TConnectionState::Resp_AmiCmd(Smt_Pdu &newpdu, Smt_Pdu &oripdu)
{
	Smt_Uint   nStatus;
	Smt_String strResult;
	newpdu.GetString( Key_ICMU_ResponseResult, &strResult );

	if( strResult == RESPONSE_EVENT_SUCCESS )
		nStatus = Smt_Success;
	else
		nStatus = Smt_Fail;

	Smt_String strDeviceID;
	Smt_String strConnectionID;
	oripdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	oripdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );

	// resp action
	Smt_Pdu sendpdu;
	sendpdu = oripdu;
	
	sendpdu.m_Sender = oripdu.m_Receiver;
	sendpdu.m_Receiver = oripdu.m_Sender;
	sendpdu.m_Status = nStatus;

	switch (oripdu.m_MessageID)
	{
	case Cmd_ICMU_HangupCall:
		sendpdu.m_MessageID = Resp_ICMU_HangupCall;
		break;
	case Cmd_ICMU_SingleStepTransfer:
		sendpdu.m_MessageID = Resp_ICMU_SingleStepTransfer;
		break;
	case Cmd_ICMU_SingleStepConference:
		sendpdu.m_MessageID = Resp_ICMU_SingleStepConference;
		break;
	case Cmd_ICMU_SendDTMF:
		sendpdu.m_MessageID = Resp_ICMU_SendDTMF;
		break;
	case Cmd_ICMU_RouteSelected:
		sendpdu.m_MessageID = Resp_ICMU_RouteSelected;
		break;
	case Cmd_ICMU_SendMessage:
		sendpdu.m_MessageID = Resp_ICMU_SendMessage;
		break;
	default:
		break;				
	}
		
	if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );
	
	PrintLog(5,"[TConnectionState::Resp_AmiCmd] DeviceID<%s>, ConnectionID<%s>, Result<%s>.",
		strDeviceID.c_str(), strConnectionID.c_str(), strResult.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtAGIRequest
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
//Smt_Uint TConnectionState::OnEvtAGIRequest(Smt_Pdu &pdu)
TConnection* TConnectionState::OnEvtAGIRequest(Smt_Pdu &pdu)
{
	Smt_Uint   nAGIHandle;
	Smt_String strChannel;
	Smt_String strUniqueid;
	Smt_String strCallerID;
	Smt_String strExtension;
	Smt_String strAGIRequest;
	Smt_String strAGINetWorkScript;
	TAGIRequest* pAGIRequest = NULL;
	Smt_String strConnectionID;

	pdu.GetUint  ( Key_ICMU_Handle, &nAGIHandle );
	pdu.GetString( Key_ICMU_agi_channel, &strChannel );
	pdu.GetString( Key_ICMU_agi_uniqueid, &strUniqueid );
	pdu.GetString( Key_ICMU_agi_callerid, &strCallerID );
	pdu.GetString( Key_ICMU_agi_extension, &strExtension );
	pdu.GetString( Key_ICMU_agi_request, &strAGIRequest );
	pdu.GetString( Key_ICMU_agi_network_script, &strAGINetWorkScript );
/*
TConnection* pConnection = NULL;

  pConnection = LookupByChannel( strChannel );
  
	if( pConnection == NULL )
	{
	pConnection = LookupByDeadChannel( strChannel ); // lookup connection by dead channel again
	}
	
	  if( pConnection != NULL )
	  {
	  strConnectionID = pConnection->GetID();
	  
		pConnection->m_AGIHandle = nAGIHandle;
		
		  
			//add by caoyj 20120106 增加处理 //add by caoyj 20120221修改
			pConnection->m_Reason = CauseIVRRequest;
			pConnection->m_Source = pConnection->m_DeviceID;
			pConnection->m_Destination = strExtension;	
			if(pConnection->m_CallerID == "")
			{
			pConnection->m_CallerID = ChangeCallerIDByTAC(strCallerID, pConnection);		
			}
			////////////////////////////////////////////////////////////////////////
			
	}
*/
//add by caoyj 20120315
	if( m_AGIRequestMgr.Lookup(nAGIHandle, pAGIRequest) != Smt_Success )
	{
		pAGIRequest = new TAGIRequest();
		pAGIRequest->m_AGIHandle = nAGIHandle;
		pAGIRequest->m_Channel = strChannel;
		pAGIRequest->m_Uniqueid = strUniqueid;
		pAGIRequest->m_CallerID = strCallerID;
		pAGIRequest->m_Extension = strExtension;
		pAGIRequest->m_AGIRequest = strAGIRequest;
		pAGIRequest->m_AGINetWorkScript = strAGINetWorkScript;
		
		m_AGIRequestMgr.SetAt(nAGIHandle, pAGIRequest );
	}	
	
	TConnection* pConnection = NULL;
	do 
	{
		
		pConnection = LookupByChannel( strChannel );
		
		if( pConnection == NULL )
		{
			pConnection = LookupByDeadChannel( strChannel ); 
		}
		
		if( pConnection != NULL )
		{
			strConnectionID = pConnection->GetID();

			if( strAGINetWorkScript == AGI_SCRIPT_IVR_OUTBOUND)
			{// IVR Outbound Device 特殊处理，AGI有可能比AMI事件快需要关联？其他情况下这里不关联，由OnEvtUserEventTimeExpired来关联

				pConnection->m_AGIHandle = nAGIHandle;
			}
		}
	} while (0);


	PrintLog(5,"[TConnectionState::OnEvtAGIRequest] Handle<0x%x>, Channel<%s>, ConnectionID<%s>, AGIRequest<%s>, CallerID<%s>, Extension<%s>.",
		nAGIHandle,strChannel.c_str(), strConnectionID.c_str(), strAGIRequest.c_str(), strCallerID.c_str(), strExtension.c_str() );

	return pConnection;
}

/****************************************************************************
函 数 名: OnEvtAGIHandleOffline
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtAGIHandleOffline(Smt_Pdu &pdu)
{//add by caoyj 20120315
	TAGIRequest* pAGIRequest = NULL;
	Smt_Uint nAGIHandle;
	TConnection* pConnection = NULL;
	Smt_String strConnectionID="";
	
	pdu.GetUint(Key_ICMU_Handle, &nAGIHandle );
	
	if( m_AGIRequestMgr.Lookup(nAGIHandle, pAGIRequest) == Smt_Success )
	{
		pConnection = LookupByHandle(nAGIHandle);
		if(pConnection != NULL)
		{
			strConnectionID = pConnection->GetID();
			pConnection->m_AGIHandle = 0;
		}

		m_AGIRequestMgr.Remove( nAGIHandle );
		if( pAGIRequest != NULL )
		{
			delete pAGIRequest;
			pAGIRequest = NULL;
		}
	}
	
	PrintLog(5,"[TConnectionState::OnEvtAGIHandleOffline] AGIHandle<0x%x>,ConnectionID<%s>.", nAGIHandle,strConnectionID.c_str() );
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtAGIResponse
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TConnection* TConnectionState::OnEvtAGIResponse(Smt_Pdu &pdu)
{
	Smt_Uint   nAGIHandle;
	Smt_String strConnectionID;
	Smt_String strResult;
	Smt_String strEndPos;
	Smt_String strReason;
	TConnection* pConnection = NULL;
	TConnection* pRetConnection = NULL;
	Smt_Uint   nStatus ;
	Smt_Uint   nReason ;
	Smt_Uint   nTimeLen;
	Smt_Uint   uLastCommand=0;

	pdu.GetUint  ( Key_ICMU_Handle, &nAGIHandle );
	pdu.GetString( Key_ICMU_agi_result, &strResult );
	pdu.GetString( Key_ICMU_agi_endpos, &strEndPos );
	pdu.GetString( Key_ICMU_agi_reason, &strReason );

// 	PrintLog(5,"[TConnectionState::OnEvtAGIResponse] Begin Handle<0x%x>, Result<%s>, EndPos<%s>, strReason<%s>.",
// 		nAGIHandle, strResult.c_str(), strEndPos.c_str(), strReason.c_str() );

	////////////////////////////////////////////////
	/*if( ACE_OS::atoi(strResult.c_str()) >= 0 )
	{
		nStatus = Smt_Success;
		nReason = CauseAGISuccess;
	}
	else
	{
		nStatus = Smt_Fail;
		nReason = CauseAGIFail;
	}*/
	//add by caoyj 20110905
	//ACE_OS::atoi 最大只能<=2147483647的数，收号时这里会溢出
	strResult = HLTrimSpace(strResult);
	if(strResult.length() == 0 || strResult.substr(0,1) == "-")
	{
		nStatus = Smt_Fail;
		nReason = CauseAGIFail;
	}
	else
	{
		nStatus = Smt_Success;
		nReason = CauseAGISuccess;
	}
	////////////////////////////////////////////////

	if( strReason == AGI_RESPONSE_REASON_TIMEOUT )
	{
		nReason = CauseTimeout;
	}
	else if( strReason == AGI_RESPONSE_REASON_HANGUP )
	{
		nReason = CauseReleased;
	}
	else if( strReason == AGI_RESPONSE_REASON_WRITEFILE )
	{
		nReason = CauseWriteFileFail;
	}
	else if( strReason == AGI_RESPONSE_REASON_DTMF )
	{
		nReason = CauseReceiveDTMF;
	}

	if( (strEndPos == "0") &&
		(strResult == "0") )
	{
		nReason = CauseReadFileFail;
	}

	nTimeLen = ACE_OS::atoi(strEndPos.c_str())/8;     // AGI 的时间是按 8*X 计算的

	pConnection = LookupByHandle( nAGIHandle );
	if( pConnection != NULL )
	{
		uLastCommand = pConnection->m_LastCommand.m_MessageID;
		strConnectionID = pConnection->GetID();

		if( pConnection->m_AGIEvtTimerID != 0 )
		{
			ClearTimer( pConnection->m_AGIEvtTimerID );
			pConnection->m_AGIEvtTimerID = 0;
		}

		if( (pConnection->m_LastCommand.m_MessageID == Cmd_ICMU_AnswerCallEx)
			&& (nStatus == Smt_Success) )
		{
			pdu.m_Status = CauseConnected;
			pRetConnection = pConnection; 
			pConnection->m_Reason = nReason;
		}

		pConnection->m_AGITimeLen = nTimeLen;
		pConnection->m_AGIReason = nReason;

		if( pConnection->m_LastCommand.m_MessageID == Cmd_ICMU_PlayFile ) 
		{
			pConnection->m_AGIData = ASCIICodeToDTMF( strResult );
			pConnection->EvtMedia( Evt_ICMU_PlayEnd );
		}
		else if( pConnection->m_LastCommand.m_MessageID == Cmd_ICMU_GetDigits )
		{
			////////////////////////////////////////////////
			//if( ACE_OS::atoi(strResult.c_str()) >= 0 ) 
			//	pConnection->m_AGIData = pConnection->m_AGIData + strResult;
			//add by caoyj 20110905
			//ACE_OS::atoi 最大只能<=2147483647的数，收号时这里会溢出
			if(strResult.length()>0 && strResult.substr(0,1) != "-")
			   pConnection->m_AGIData += strResult;
			////////////////////////////////////////////////
			pConnection->EvtMedia( Evt_ICMU_GetEnd );
		}
		else if( pConnection->m_LastCommand.m_MessageID == Cmd_ICMU_RecordEx )
		{
			pConnection->m_AGIData = ASCIICodeToDTMF( strResult );
			pConnection->EvtMedia( Evt_ICMU_RecordEnd );
		}
		else if( pConnection->m_LastCommand.m_MessageID == Cmd_ICMU_SendDTMFEx )
		{
			////////////////////////////////////////////////
			//if( ACE_OS::atoi(strResult.c_str()) > 0 )
			//add by caoyj 20110905
			//ACE_OS::atoi 最大只能<=2147483647的数
			if(strResult != "0" && strResult != ""  )
			////////////////////////////////////////////////
			{
				pConnection->m_AGIReason = CauseAGIFail;
			}
			pConnection->EvtMedia( Evt_ICMU_SendEnd );
		}
		else if( pConnection->m_LastCommand.m_MessageID == Cmd_ICMU_PlayFileList )
		{
			// 播放正常并无 DTMF 输入
			////////////////////////////////////////////////
			//if( (ACE_OS::atoi(strResult.c_str()) == 0) &&
			//	(nReason == CauseAGISuccess) && 
			//	(pConnection->m_PlayListCurrIndex < pConnection->m_PlayListIndex ) ) 
			//add by caoyj 20110905
			//ACE_OS::atoi 最大只能<=2147483647的数
			if( (strResult == "0") &&
				(nReason == CauseAGISuccess) && 
				(pConnection->m_PlayListCurrIndex < pConnection->m_PlayListIndex ) ) 
			////////////////////////////////////////////////
			{
				pConnection->m_PlayListTimeLen = pConnection->m_PlayListTimeLen + nTimeLen;
				pConnection->m_PlayListCurrIndex++;
				g_pAstConnector->Agi_StreamFile( pConnection->m_AGIHandle, pConnection->m_PlayList[pConnection->m_PlayListCurrIndex], pConnection->m_PlayListEscapeDigit );
			}
			else
			{				
				pConnection->m_AGITimeLen = pConnection->m_PlayListTimeLen + nTimeLen;
				pConnection->m_AGIData = ASCIICodeToDTMF( strResult );
				pConnection->EvtMedia( Evt_ICMU_PlayEnd );
			}	
		}
		else if( pConnection->m_LastCommand.m_MessageID == Cmd_ICMU_Dial )
		{
			pConnection->EvtDialResponse();
		}
		else if( pConnection->m_LastCommand.m_MessageID == Cmd_ICMU_SendMessageEx )
		{
			pConnection->EvtMedia( Evt_ICMU_MessageSendEnd );
		}
		else if( pConnection->m_LastCommand.m_MessageID == Cmd_ICMU_SetSIPHeader )
		{
			pConnection->RespSetSIPHeader( nReason );
		}
		else if( pConnection->m_LastCommand.m_MessageID == Cmd_ICMU_GetSIPHeader )
		{
			pConnection->RespGetSIPHeader( nReason, strReason );
		}
		else
		{
			// do nothing
		}
	}
	
	PrintLog(5,"[TConnectionState::OnEvtAGIResponse] Handle<0x%x>,ConnectionID<%s>,Result<%s>,EndPos<%s>,strReason<%s>,LastCommand<%s>.",
		nAGIHandle,strConnectionID.c_str(), strResult.c_str(), strEndPos.c_str(), strReason.c_str(),GetIDName(uLastCommand).c_str() );

	return pRetConnection;
}

/****************************************************************************
函 数 名: Resp_AgiCmd
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::Resp_AgiCmd(Smt_Uint nresult, Smt_Pdu &oripdu)
{
	Smt_String strConnectionID;
	oripdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );

	Smt_Pdu sendpdu;
	sendpdu = oripdu;
	
	sendpdu.m_Sender = oripdu.m_Receiver;
	sendpdu.m_Receiver = oripdu.m_Sender;
	sendpdu.m_Status = nresult;
	
	switch (oripdu.m_MessageID)
	{
	case Cmd_ICMU_MakeCallEx:
		sendpdu.m_MessageID = Resp_ICMU_MakeCallEx;
		break;
	case Cmd_ICMU_AnswerCallEx:
		sendpdu.m_MessageID = Resp_ICMU_AnswerCallEx;
		break;
	case Cmd_ICMU_HangupCallEx:
		sendpdu.m_MessageID = Resp_ICMU_HangupCallEx;
		break;
	case Cmd_ICMU_PlayFile:
		sendpdu.m_MessageID = Resp_ICMU_PlayFile;
		break;
	case Cmd_ICMU_PlayFileList:
		sendpdu.m_MessageID = Resp_ICMU_PlayFileList;
		break;
	case Cmd_ICMU_GetDigits:
		sendpdu.m_MessageID = Resp_ICMU_GetDigits;
		break;
	case Cmd_ICMU_RecordEx:
		sendpdu.m_MessageID = Resp_ICMU_RecordEx;
		break;
	case Cmd_ICMU_SendDTMFEx:
		sendpdu.m_MessageID = Resp_ICMU_SendDTMFEx;
		break;
	case Cmd_ICMU_TTSPlay:
		sendpdu.m_MessageID = Resp_ICMU_TTSPlay;
		break;
	case Cmd_ICMU_Dial:
		sendpdu.m_MessageID = Resp_ICMU_Dial;
		break;
	case Cmd_ICMU_SendMessageEx:
		sendpdu.m_MessageID = Resp_ICMU_SendMessageEx;
		break;
	default:
		break;	
	}
	
	sendpdu.PutUint( Key_ICMU_Reason, nresult);

	if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );
	
	PrintLog(5,"[TConnectionState::Resp_AgiCmd] ConnectionID<%s>, sendMessageID<%s>,Result<%d>.",
		strConnectionID.c_str(), GetIDName(sendpdu.m_MessageID).c_str(),nresult );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtAGIEvtTimer
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtAGIEvtTimer(Smt_Uint senderobj)
{
	Smt_String strConnectionID;
	TConnection* pTmpConnection = NULL;
	TConnection* pConnection = NULL;
	Smt_Uint uLastCommand = 0;
	
	TConnectionMap::ITERATOR iter0( m_ConnectionMgr );
	for (TConnectionMap::ENTRY *entry0 = 0;
		iter0.next (entry0) != 0;
		iter0.advance ())
	{
		pTmpConnection = entry0->int_id_;
		
		if((Smt_Uint)pTmpConnection == senderobj )
		{	
			pConnection = pTmpConnection;
			break;
		}
	}	

	if( pConnection!= NULL )
	{
		uLastCommand = pConnection->m_LastCommand.m_MessageID;
		strConnectionID = pConnection->GetID();

		pConnection->m_AGIEvtTimerID = 0;

		switch (pConnection->m_LastCommand.m_MessageID)
		{
		case Cmd_ICMU_PlayFile:
		case Cmd_ICMU_PlayFileList:
			pConnection->EvtMedia( Evt_ICMU_Playing );
			break;
		case Cmd_ICMU_GetDigits:
			pConnection->EvtMedia( Evt_ICMU_Geting );
			break;
		case Cmd_ICMU_RecordEx:
			pConnection->EvtMedia( Evt_ICMU_Recording );
			break;
		case Cmd_ICMU_SendDTMFEx:
			pConnection->EvtMedia( Evt_ICMU_Sending );
			break;
		case Cmd_ICMU_SendMessageEx:
			pConnection->EvtMedia( Evt_ICMU_MessageSending );
			break;
		}
	}
	
	PrintLog(5,"[TConnectionState::OnEvtAGIEvtTimer] ConnectionID<%s>,LastCommand<%s>.", strConnectionID.c_str(),GetIDName(uLastCommand).c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtOriginateResponse
参    数:
返回数值: 
功能描述: 外呼时异步返回接口
*****************************************************************************/
TConnection* TConnectionState::OnEvtOriginateResponse(Smt_Pdu &pdu)
{
	Smt_String strActID;
	Smt_String strResult;
	Smt_Uint   nStatus;
	Smt_String strChannel;
	TConnection* pConnection = NULL;
	Smt_String strConnectionID;
	Smt_String strReason;
	Smt_Uint   nReason;

	pdu.GetString( Key_ICMU_ActionID, &strActID );
	pdu.GetString( Key_ICMU_Response, &strResult );
	pdu.GetString( Key_ICMU_Channel, &strChannel );
	pdu.GetString( Key_ICMU_Reason, &strReason );

	// set status
	if( strResult == RESPONSE_EVENT_SUCCESS )
	{
		nStatus = CauseSuccess;
	}
	else
	{
		nStatus = CauseFail;
	}

	// set Asterisk Reason code
	nReason = ACE_OS::atoi(strReason.c_str());
	
	// set connectinid
	pConnection = LookupByChannel( strChannel );
	if( pConnection != NULL )
	{
		strConnectionID = pConnection->GetID();
	}

	Smt_Pdu sendpdu;
	TActionPdu* pAct = NULL;
	if(m_ActMgr.Lookup( strActID, pAct) == Smt_Success )
	{	
		// resp action
		sendpdu = pAct->m_Pdu;
		sendpdu.m_Sender = pAct->m_Pdu.m_Receiver;
		sendpdu.m_Receiver = pAct->m_Pdu.m_Sender;
		
		switch (pAct->m_Pdu.m_MessageID)
		{
		case Cmd_ICMU_MakeCall: 
			sendpdu.m_MessageID = Resp_ICMU_MakeCall;
			pdu.m_Status = CauseSuccess;  
			break;
		case Cmd_ICMU_SingleStepConference:
			sendpdu.m_MessageID = Resp_ICMU_SingleStepConference;
			pdu.m_Status = CauseListening;
			if( pConnection != NULL )
			{
				pConnection->m_Reason = CauseListening;
			}
			break;
		default:break;
		}

		sendpdu.PutString( Key_ICMU_ConnectionID, strConnectionID );
		if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );	

		// get originate controler connectionid, fire fail event
		if( nReason == AST_CAUSE_NOTDEFINED )
		{
			Smt_String strControlerConnectionID;
			pAct->m_Pdu.GetString( Key_ICMU_ConnectionID, &strControlerConnectionID );
			if( pConnection == NULL &&  strControlerConnectionID != "")
			{
				m_ConnectionMgr.Lookup( strControlerConnectionID, pConnection);
				strConnectionID = strControlerConnectionID;
				pdu.m_Status = CauseFail;
			}
		}
		//add by caoyj 20120315
		// 单步转移(M) 场景，如果是 meetme 方式下，单步转移发起呼叫失败，挂断所有话路（被转移方话路）
		if( nStatus == CauseFail )
		{
			Smt_String strCurrActionID;
			Smt_String strVar;
			Smt_String strDeviceID;
			Smt_String strTmpConnectionID;
			TConnection* pTmpConnection = NULL;
			Smt_Int nPos1;
			
			pAct->m_Pdu.GetString( Key_ICMU_Variable, &strVar );
			pAct->m_Pdu.GetString( Key_ICMU_ConnectionID, &strTmpConnectionID );
			
			nPos1 = strVar.find( AMI_SINGLESTEPTRANSFER );
			
			if( nPos1>0 && m_ConnectionMgr.Lookup(strTmpConnectionID, pTmpConnection) == Smt_Success )
			{
				strDeviceID = pTmpConnection->m_DeviceID;
				pTmpConnection->m_LastCommand.Clear();
				
				strCurrActionID = GenActionID( pTmpConnection->m_LastCommand );	
				
				g_pAstConnector->Ami_Hangup( m_AmiHandle, strCurrActionID, pTmpConnection->m_Channel );
				
				PrintLog(3,"[TConnectionState::OnEvtOriginateResponse] Send HangupCall, DeviceID<%s>, CurrActionID<%s>, ConnectionID<%s>.", 
					strDeviceID.c_str(), strCurrActionID.c_str(), strTmpConnectionID.c_str() );
			}
		}
		// release data
		ClearTimer( pAct->m_TimerID );
		
		m_ActMgr.Remove( pAct->m_ActionID );
		
		if( pAct != NULL )
		{
			delete pAct;
			pAct = NULL;
		}
	}

	PrintLog(5,"[TConnectionState::OnEvtOriginateResponse] Response OK, <%s>, ActionID<%s>, ConnectionID<%s>, Result<%s>, Reason<%s>.",
		GetIDName(sendpdu.m_MessageID).c_str(), strActID.c_str(), strConnectionID.c_str(), strResult.c_str(), strReason.c_str() );

	return pConnection;
}

/****************************************************************************
函 数 名: ASCIICodeToDTMF
参    数:
返回数值: 
功能描述: ASCII 代码转换成字符
*****************************************************************************/
Smt_String TConnectionState::ASCIICodeToDTMF(Smt_String code)
{	
	Smt_String strRet = "";
	char cTemp ; 

	Smt_Int nTemp = ACE_OS::atoi(code.c_str());
	if( nTemp > 0 )
	{
		cTemp = nTemp;
		strRet = HLFormatStr("%c", cTemp );
	}

	return strRet;
}

/****************************************************************************
函 数 名: EvtLinkDown
参    数:
返回数值: 
功能描述: 发送LinkDown事件
*****************************************************************************/
Smt_Uint TConnectionState::EvtLinkDown(Smt_Uint reason)
{
	Smt_DateTime dtNow;

	Smt_Pdu send;
	send.m_Sender = GetGOR();
	send.m_Receiver = m_CallStateGOR;
	send.m_Status = Smt_Success;
	send.m_MessageID = Evt_ICMU_LinkDown;

	send.PutUint  (Key_ICMU_Reason, reason);
	send.PutString(Key_ICMU_TimeStamp, dtNow.FormatString());

	if( send.m_Receiver > 0 ) PostMessage( send );

	PrintLog(5,"[TConnectionState::EvtLinkDown] Reason<%d>.", reason );

	return Smt_Success;
}

/****************************************************************************
函 数 名: EvtLinkUp
参    数:
返回数值: 
功能描述: 发送 EvtLinkUp 事件
*****************************************************************************/
Smt_Uint TConnectionState::EvtLinkUp(Smt_Uint reason)
{
	Smt_DateTime dtNow;

	Smt_Pdu send;
	send.m_Sender = GetGOR();
	send.m_Receiver = m_CallStateGOR;
	send.m_Status = Smt_Success;
	send.m_MessageID = Evt_ICMU_LinkUp;

	send.PutUint(Key_ICMU_Reason, CauseSuccess );
	send.PutString(Key_ICMU_TimeStamp, dtNow.FormatString());

	if( send.m_Receiver > 0 ) PostMessage( send );

	PrintLog(5,"[TConnectionState::EvtLinkUp] Reason<%d>.", CauseSuccess );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtLinkDown
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtLinkDown(Smt_Pdu &pdu)
{
	Smt_Uint nReason;
	pdu.GetUint( Key_ICMU_Reason, &nReason);
	EvtLinkDown( nReason );

	// 清除 Connection/Action 信息
	ClearMapInfo();

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdDial
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdDial(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	Smt_String strCalledNum;
	Smt_Uint   nTimeout;
	TConnection* pConnection = NULL;
	Smt_String strTempCalledNum;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_CalledNum, &strCalledNum );
	pdu.GetUint  ( Key_ICMU_Timeout, &nTimeout );
	
	if( nTimeout <= 0 )
	{
		nTimeout = DEFAULT_MAKECALLTIMEOUT;
	}

	nTimeout = nTimeout/1000.00;

	strTempCalledNum = ParseCalledNumEx( strCalledNum );

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		pConnection->m_LastCommand = pdu;
		//g_pAstConnector->Agi_Exec_Dial( pConnection->m_AGIHandle, strTempCalledNum, HLFormatStr("%d", nTimeout ) );
		//Resp_AgiCmd( CauseSuccess, pdu );
		Smt_Uint uRetu = g_pAstConnector->Agi_Exec_Dial( pConnection->m_AGIHandle, strTempCalledNum, HLFormatStr("%d", nTimeout ) );
		Resp_AgiCmd( uRetu, pdu );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdDial] ConnectionID<%s>, CalledNum<%s>, Real-CalledNum<%s>, Timeout<%d>.",
		strConnectionID.c_str(), strCalledNum.c_str(), strTempCalledNum.c_str(), nTimeout );

	return Smt_Success;
}

/****************************************************************************
函 数 名: Dialplan_AddRouteSelected
参    数:
返回数值: 
功能描述: 
		初始化 from-custom-routeselected 拨号方案
		[from-custom-routeselected]
		exten => _XXXX,1,Dial(SIP/${EXTEN},20,tTr)
		exten => _XXXX,2,Playback(vm-nobodyavail)
		exten => _XXXX,3,Hangup()
		exten => _XXXX,102,Playback(busy-pls-hold)
		exten => _XXXX,103,Hangup()
		exten => i,1,Playback(pbx-invalid)
		exten => i,2,Hangup()
		exten => t,1,Playback(vm-nobodyavail)
		exten => t,2,Hangup()
*****************************************************************************/
Smt_Uint TConnectionState::Dialplan_AddRouteSelected()
{
	return 0;

	Smt_Uint nActID = 0;

	g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
		"_XXXX","1","Dial","SIP/${EXTEN}|20|tTr","from-custom-routeselected");
	g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
		"_XXXX","2","Playback","vm-nobodyavail","from-custom-routeselected");
	g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
		"_XXXX","3","Hangup","","from-custom-routeselected");
	g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
		"_XXXX","102","Playback","busy-pls-hold","from-custom-routeselected");
	g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
		"_XXXX","103","Hangup","","from-custom-routeselected");
	g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
		"i","1","Playback","pbx-invalid","from-custom-routeselected");
	g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
		"i","2","Hangup","","from-custom-routeselected");
	g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
		"t","1","Playback","vm-nobodyavailr","from-custom-routeselected");
	g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
		"t","2","Hangup","","from-custom-routeselected");

	return Smt_Success;
}

/****************************************************************************
函 数 名: Dialplan_AddDevice
参    数:
返回数值: 
功能描述: 增加设备：RouteDN、IVR-Inbound、IVR-Outbound

  #Route Device
  exten => 6005,1,Answer
  exten => 6005,2,UserEvent(RouteRequest|Channel: ${CHANNEL}|Context: ${CONTEXT}|Extension: ${EXTEN}|CallerID: ${CALLERID(num)}|Uniqueid: ${UNIQUEID} )
  exten => 6005,3,MusicOnHold()

  #IVR Inbound Device			
  exten => 6000,1,UserEvent(IVRRequest|Channel: ${CHANNEL}|Context: ${CONTEXT}|Extension: ${EXTEN}|CallerID: ${CALLERID(num)}|Uniqueid: ${UNIQUEID} ) 
  exten => 6000,2,AGI(agi://10.8.103.219:5040/TestFastAGI)

  #IVR Outbound Device	
  exten => 6003,1,Answer
  exten => 6003,2,AGI(agi://10.8.103.219:5040/TestFastAGI) 

*****************************************************************************/
Smt_Uint TConnectionState::Dialplan_AddDevice(TAssignedDevice* pdevice)
{
	return 0;

	if( pdevice == NULL ) return Smt_Fail;

	Smt_Uint nActID = 0;
	Smt_Int nPosAt;
	Smt_String strTempDn;
	Smt_String strTemp;
	Smt_String strDeviceID = pdevice->m_DeviceID;
	Smt_Uint   nDeviceType = pdevice->m_DeviceType;

	// get agi ip:prot info, ex:6001@10.8.103.219:5040
	nPosAt = strDeviceID.find("@");	
	if( nPosAt>0 )
	{
		strTempDn = strDeviceID.substr(0, nPosAt);	
		strTemp = strDeviceID.substr(nPosAt+1, strDeviceID.length()-nPosAt-1);	
		strTemp = HLFormatStr("AGI://%s//FastAGI", strTemp.c_str() );
	}
	else
	{
		strTempDn = strDeviceID;
	}

	if( nDeviceType == DTYPE_ROUTE )
	{
		g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
			strTempDn,"1","Answer","","from-internal-custom");
		g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
			strTempDn,"2","UserEvent","RouteRequest|Channel:${CHANNEL}|Context:${CONTEXT}|Extension:${EXTEN}|CallerID:${CALLERID(num}|Uniqueid:${UNIQUEID}","from-internal-custom");
		g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
			strTempDn,"3","MusicOnHold","","from-internal-custom");
	}
	else if( nDeviceType == DTYPE_IVR_INBOUND) 
	{
		g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
			strTempDn,"1","UserEvent","IVRRequest|Channel:${CHANNEL}|Context:${CONTEXT}|Extension:${EXTEN}|CallerID:${CALLERID(num}|Uniqueid:${UNIQUEID}","from-internal-custom");
		g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
			strTempDn,"2","AGI",strTemp,"from-internal-custom");
	}
	else if( nDeviceType == DTYPE_IVR_OUTBOUND)
	{
		g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
			strTempDn,"1","Answer","","from-internal-custom");
		g_pAstConnector->Ami_Command_AddExtension(m_AmiHandle, HLFormatStr("%d",nActID++),
			strTempDn,"2","AGI",strTemp,"from-internal-custom");
	}
	else
	{
		// do nothing
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: ReadXMLConfig
参    数:
返回数值: 
功能描述: 读取配置信息
*****************************************************************************/
Smt_Uint TConnectionState::ReadXMLConfig()
{	
	Smt_String strCurrPath = HLGetModuleFilePath();
	Smt_String m_CfgFileName = strCurrPath + "../conf/cti.xml";
	Smt_XMLParser xmlParser( m_CfgFileName );

	//
	// Read TRUNK Table
	//
	Smt_String strTrunkID;
	TTrunkGroup* pTrunkGroup;
	if( xmlParser.OpenTable("CC_CORE_TRUNK") == Smt_Success )
	{
		xmlParser.MoveFirst();
		
		while(xmlParser.IsEmpty() != Smt_BoolTRUE ) 
		{
			if( xmlParser.GetPropString("TRUNK_ID") != "" ) // if-1
			{
				strTrunkID = xmlParser.GetPropString("TRUNK_ID");
				if( m_TrunkGroupMgr.Lookup(strTrunkID, pTrunkGroup) == Smt_Success )
				{
					continue;
				}

				pTrunkGroup = new TTrunkGroup();
			
				pTrunkGroup->m_TrunkID =  strTrunkID;
				pTrunkGroup->m_TrunkName = xmlParser.GetPropString("TRUNK_NAME");
				pTrunkGroup->m_TrunkType = xmlParser.GetPropUint("TRUNK_TYPE");
				pTrunkGroup->m_TACCode = xmlParser.GetPropString("TAC_CODE");
				pTrunkGroup->m_CircuitCode = xmlParser.GetPropString("CIRCUIT_CODE");
				pTrunkGroup->m_ExtnDN = xmlParser.GetPropString("EXTN_DN");
		
				m_TrunkGroupMgr.SetAt( strTrunkID, pTrunkGroup );
				
				PrintLog(5,"[ReadTrunkTable] TRUNK_ID<%s>, TRUNK_NAME<%s>, TRUNK_TYPE<%d>, TAC_CODE<%s>, CIRCUIT_CODE<%s>, EXTN_DN<%s>.", 
					pTrunkGroup->m_TrunkID.c_str(), pTrunkGroup->m_TrunkName.c_str(), pTrunkGroup->m_TrunkType, pTrunkGroup->m_TACCode.c_str(), pTrunkGroup->m_CircuitCode.c_str(), pTrunkGroup->m_ExtnDN.c_str() );

				// Init trunkDN 
				Smt_String strTempExtnDN = pTrunkGroup->m_ExtnDN;
				Smt_Int nPos = strTempExtnDN.find("-");
				if( nPos < 0 )
				{
					TTrunkDN* pTrunkDN = new TTrunkDN();					
					pTrunkDN->m_ExtenDN = strTempExtnDN;
					pTrunkDN->m_CircuitCode = ACE_OS::atoi( pTrunkGroup->m_CircuitCode.c_str() );					
					pTrunkDN->m_pTrunkGroup = pTrunkGroup;
					pTrunkGroup->m_TrunkDNMgr.AddTail( pTrunkDN );
					
					PrintLog(5,"[ReadTrunkTable] EXTN_DN<%s>, CIRCUIT_CODE<%d>.", 
						strTempExtnDN.c_str(), pTrunkDN->m_CircuitCode );
				}
				else // ex: 5001-5030
				{	
					Smt_String strCircuitCode,strCircuitCodeFrom;
					Smt_Int nPos2, nCircuitCodeFrom;

					strCircuitCode = pTrunkGroup->m_CircuitCode;
					nPos2 = strCircuitCode.find("-");
					if( nPos2>0 )
					{
						strCircuitCodeFrom = strCircuitCode.substr( 0, nPos2 );
						nCircuitCodeFrom = ACE_OS::atoi( strCircuitCodeFrom.c_str() );
					}
					else
					{
						nCircuitCodeFrom = 0;
					}
				
					Smt_String strExtnFrom, strExtnTo, strTemp;	
					strExtnFrom = strTempExtnDN.substr( 0, nPos );
					strExtnTo = strTempExtnDN.substr( nPos+1, strTempExtnDN.length() );					

					for(Smt_Uint i = ACE_OS::atoi(strExtnFrom.c_str()); 
						i <= ACE_OS::atoi(strExtnTo.c_str()); i++ )
					{
						strTemp = HLFormatStr("%d", i);
						
						TTrunkDN* pTrunkDN = new TTrunkDN();
						
						pTrunkDN->m_ExtenDN = strTemp;
						pTrunkDN->m_CircuitCode = nCircuitCodeFrom;
						pTrunkDN->m_pTrunkGroup = pTrunkGroup;
						pTrunkGroup->m_TrunkDNMgr.AddTail( pTrunkDN );
						
						PrintLog(5,"[ReadTrunkTable] EXTN_DN<%s>, CIRCUIT_CODE<%d>.", 
							strTemp.c_str(), nCircuitCodeFrom );

						if( nCircuitCodeFrom!=0 )
						{
							nCircuitCodeFrom++;
						}
					} // end-for
				}
			} // end if-1
			xmlParser.MoveNext();

		} // end-while
		
		xmlParser.CloseTable();
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: DelPrefix
参    数:
返回数值: 
功能描述: 号码变换："|" 删除前缀号码  "." 号码不变换 
          前缀： 9|.&86.
		  95001 ==> 5001
		  865001 ==> 865001
*****************************************************************************/
Smt_Bool TConnectionState::DelPrefix(Smt_String tac, Smt_String prefix, Smt_String callednum, Smt_String& realcallednum)
{
	Smt_Bool   bRet = Smt_BoolFALSE;
	Smt_String strTAC = tac;
	Smt_String strPrefix = prefix;
	Smt_String strCalledNum = callednum;
	Smt_Int    nTACPos = strTAC.find( strPrefix.c_str() );
	Smt_String strDelFlag = "";
	
	if(nTACPos<0)
	{
		realcallednum = strCalledNum;
		return bRet;
	}

	strDelFlag = strTAC.substr( nTACPos+strPrefix.length(), 1);
	if( strDelFlag == "|" )
	{
		realcallednum = strCalledNum.substr(strPrefix.length(), strCalledNum.length() );
		bRet = Smt_BoolTRUE;
	}
	else if( strDelFlag == "." )
	{
		realcallednum = strCalledNum;
		bRet = Smt_BoolTRUE;
	}
	else // 不符合变换规则
	{
		realcallednum = strCalledNum;
		bRet = Smt_BoolFALSE;
	}

	PrintLog(5,"[TConnectionState::DelPrefix] TAC<%s>,OriCalledNum<%s>,CalledNum<%s>,DelFlag<%s>,Prefix<%s>.", 
		strTAC.c_str(), strCalledNum.c_str(), realcallednum.c_str(), strDelFlag.c_str(), strPrefix.c_str() );

	return bRet;
}

/****************************************************************************
函 数 名: ParseCalledNum
参    数:
返回数值: 
功能描述: 被叫号码分析，根据中继访问码，路由中继的通道 
          3001  ==> SIP/3001
          95001 ==> DAHDI/1/5001
		  85002 ==> SIP/ToHost208/5002
*****************************************************************************/
Smt_String TConnectionState::ParseCalledNum(Smt_String callednum)
{
	Smt_String strRetCalledNum = "";
	Smt_String strCalledNum = callednum;
	
	// 查找中继，根据最长字冠匹配
	TTrunkGroup* pTrunkGroup = NULL;
	Smt_Bool   bOk = Smt_BoolFALSE;
	Smt_String strTemp = strCalledNum;
	Smt_String strRealCalledNum = "";
	Smt_String strCircuitCode;
	Smt_String strTrunkName;
	Smt_Int    nPos = 0;
	
	do 
	{
		TTrunkGroupMap::ITERATOR iter(m_TrunkGroupMgr);
		for( iter = m_TrunkGroupMgr.begin();
			iter != m_TrunkGroupMgr.end();
			++iter )
		{
			pTrunkGroup = (*iter).int_id_;

			nPos = pTrunkGroup->m_TACCode.find( strTemp.c_str() );

			if( nPos >= 0 )
			{
				if( DelPrefix(pTrunkGroup->m_TACCode, strTemp, strCalledNum, strRealCalledNum ) == Smt_BoolFALSE )
				{
					break;
				}

				if( pTrunkGroup->m_TrunkType == DTYPE_TRUNK_SIP) 
				{
					strTrunkName = pTrunkGroup->m_TrunkName;
					strRetCalledNum = HLFormatStr("%s/%s/%s", CHANNELTYPE_SIP, strTrunkName.c_str(), strRealCalledNum.c_str() );
					bOk = Smt_BoolTRUE;
					break;
				}
				else // DTYPE_TRUNK_DAHDI 
				{
					strCircuitCode = pTrunkGroup->AllocateDahdiTrunk();					
					if( strCircuitCode!= "")
					{
						strRetCalledNum = HLFormatStr("%s/%s/%s", CHANNELTYPE_DAHDI, strCircuitCode.c_str(), strRealCalledNum.c_str() );
						bOk = Smt_BoolTRUE;
						break;
					}
				}
			}
		} // end-for

		if( bOk == Smt_BoolTRUE )
		{
			break;
		}

		strTemp = strTemp.substr(0, strTemp.length()-1 );

	} while ( strTemp.length()>0 );

	if( bOk == Smt_BoolFALSE ) // default value is "SIP"
	{
		strRetCalledNum = HLFormatStr("%s/%s", CHANNELTYPE_SIP, strCalledNum.c_str() );
	}

	PrintLog(5,"[TConnectionState::ParseCalledNum] strCalledNum<%s>, strRetCalledNum<%s>.", 
		strCalledNum.c_str(), strRetCalledNum.c_str() );

	return strRetCalledNum;
}

/****************************************************************************
函 数 名: ParseCalledNumEx
参    数:
返回数值: 
功能描述: 复合的被叫号码分析，根据 & 取各个段的值 
          3005&95001&85002 ==> SIP/3005&DAHDI/1/5001&SIP/ToHost208/5002
*****************************************************************************/
Smt_String TConnectionState::ParseCalledNumEx(Smt_String callednum)
{
	Smt_String strRetCalledNum = "";
	Smt_Int    nPosRef;
	Smt_String strTemp, strTempSrc, strTempDest;

	strTempSrc = callednum;
	nPosRef = strTempSrc.find("&");
	if( nPosRef>0 )
	{
		do 
		{
			strTemp = strTempSrc.substr(0, nPosRef);

			strTempDest = strTempDest + ParseCalledNum( strTemp ) + "&";
			
			strTempSrc = strTempSrc.substr(nPosRef+1, strTempSrc.length());

			nPosRef = strTempSrc.find("&");
		} while ( nPosRef>0 );

		strRetCalledNum = strTempDest + ParseCalledNum( strTempSrc ) ;
	}
	else
	{
		strRetCalledNum = ParseCalledNum( strTempSrc );
	}

	PrintLog(5,"[TConnectionState::ParseCalledNumEx] strCalledNum<%s>, strRetCalledNum<%s>.", 
		callednum.c_str(), strRetCalledNum.c_str() );

	return strRetCalledNum;
}

/****************************************************************************
函 数 名: OnEvtTrunkDnLockedTimer
参    数:
返回数值: 
功能描述: 释放锁定的未置忙的设备
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtTrunkDnLockedTimer(Smt_Uint senderobj)
{
	Smt_String strDeviceID = HLFormatStr("%d", senderobj);

	TTrunkGroup* pTrunkGroup = NULL;
	TTrunkGroupMap::ITERATOR iter(m_TrunkGroupMgr);
	for( iter = m_TrunkGroupMgr.begin();
		iter != m_TrunkGroupMgr.end();
		++iter )
	{
		pTrunkGroup = (*iter).int_id_;
		if ( pTrunkGroup->GetDeviceState( strDeviceID ) == ST_TRUNKDN_LOCKED )
		{
			pTrunkGroup->FreeDevice( strDeviceID );
			PrintLog(3,"[TConnectionState::OnEvtTrunkDnLockedTimer] strDeviceID<%s>.", 
				strDeviceID.c_str() );
			break;
		}
	}
	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtDTMFReceived
参    数:
返回数值: 
功能描述: 收到 SIP INFO 类型的 DTMF
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtDTMFReceived(Smt_Pdu &pdu)
{
	TConnection * pConnection = NULL;
	Smt_String strChannel;
	Smt_String strConnectionID="";
	Smt_String strDigits;
	
	pdu.GetString( Key_ICMU_Channel, &strChannel );
	pdu.GetString( Key_ICMU_Digits, &strDigits );
	
	pConnection = LookupByChannel( strChannel );

	if(pConnection != NULL)
	{
		strConnectionID = pConnection->GetID();	

		// DTMF Received 事件不需要处理
		//Smt_Uint nMaxDigitsCount;
		//pConnection->m_LastCommand.GetUint( Key_ICMU_MaxDigitsCount, &nMaxDigitsCount );
		//pConnection->m_AGIData = pConnection->m_AGIData + strDigits;
		//if( pConnection->m_AGIData.length() >= nMaxDigitsCount )
		//{
		//	pConnection->EvtMedia( Evt_ICMU_GetEnd );
		//	pConnection->m_LastCommand.Clear();
		//}	
	}
	
	PrintLog(5,"[TConnectionState::OnEvtDTMFReceived] ConnectionID<%s>, Channel<%s>, Digits<%s>.",
		strConnectionID.c_str(), strChannel.c_str(), strDigits.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtMessageReceived
参    数:
返回数值: 
功能描述: 收到 SIP MESSAGE 
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtMessageReceived(Smt_Pdu &pdu)
{
	TConnection * pConnection = NULL;
	Smt_String strChannel;
	Smt_String strConnectionID="";
	Smt_String strMessage;
	Smt_String strTmpMessage="";
	
	pdu.GetString( Key_ICMU_Channel, &strChannel );
	pdu.GetString( Key_ICMU_Message, &strMessage );
	
	// utf-8 ==> unicode
	Smt_XMLParser xmlParser( "" );
	char* szOut = xmlParser.U2G( (char*)strMessage.c_str() );
	if( szOut != NULL )
	{
		strTmpMessage = szOut;
		pdu.PutString( Key_ICMU_Message, strTmpMessage );
		
		ACE_OS::free(szOut);
		szOut = NULL;
	}
	
	pConnection = LookupByChannel( strChannel );
	if( pConnection != NULL )
	{
		strConnectionID = pConnection->GetID();
		pConnection->SendMessageEvent( pdu );	
	}
	
	PrintLog(5,"[TConnectionState::OnEvtMessageReceived] ConnectionID<%s>, Channel<%s>, Message<%s>.",
		strConnectionID.c_str(), strChannel.c_str(), strTmpMessage.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdSendMessage
参    数:
返回数值: 
功能描述: 坐席发送 SIP MESSAGE 消息
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdSendMessage(Smt_Pdu &pdu)
{
	Smt_String strActID;
	Smt_String strDeviceID;
	Smt_String strConnectionID;
	Smt_String strMessage;
	Smt_String strTmp="";
	TConnection* pConnection = NULL;
	
	pdu.GetString( Key_ICMU_DeviceID, &strDeviceID );
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_Message, &strMessage );
	
	// 限制长度 2011-05-09
	Smt_Uint nMessageLen = strMessage.length();
	if( nMessageLen> SIP_MESSAGE_BUFF_LEN)
	{
		strMessage = strMessage.substr(0, SIP_MESSAGE_BUFF_LEN);
		
		PrintLog(3,"[TConnectionState::OnCmdSendMessage] Message's Length > 200 Byte!");
	}

	// unicode ==> utf-8
	Smt_XMLParser xmlParser( "" );
	char* szOut = xmlParser.G2U( (char*)strMessage.c_str() );

	if( szOut != NULL)
	{
		strTmp = szOut;
	
		ACE_OS::free(szOut);
		szOut = NULL;
	}	

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		strActID = GenActionID( pdu );
		g_pAstConnector->Ami_SendText( m_AmiHandle, strActID, pConnection->m_Channel, strTmp );	
	}
	
	PrintLog(5,"[TConnectionState::OnCmdSendMessage] DeviceID<%s>, ActionID<%s>, ConnectionID<%s>, Message<%s>.",
		strDeviceID.c_str(), strActID.c_str(), strConnectionID.c_str(), strMessage.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnCmdSendMessageEx
参    数:
返回数值: 
功能描述: IVR 发送SIP MESSAGE 消息
*****************************************************************************/
Smt_Uint TConnectionState::OnCmdSendMessageEx(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	Smt_String strMessage;
	Smt_String strTmp="";
	TConnection* pConnection = NULL;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_Message, &strMessage );
	
	// 限制长度 2011-05-09
	Smt_Uint nMessageLen = strMessage.length();
	if( nMessageLen> SIP_MESSAGE_BUFF_LEN)
	{
		strMessage = strMessage.substr(0, SIP_MESSAGE_BUFF_LEN);
		
		PrintLog(3,"[TConnectionState::OnCmdSendMessageEx] Message's Length > 200 Byte!");
	}

	// unicode ==> utf-8
	Smt_XMLParser xmlParser( "" );
	char* szOut = xmlParser.G2U( (char*)strMessage.c_str() );
	if(szOut != NULL)
	{
		strTmp = szOut;
		
		ACE_OS::free(szOut);
		szOut = NULL;
	}

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		pConnection->m_LastCommand = pdu;
		pConnection->m_Message = strMessage;

/*		g_pAstConnector->Agi_SendText( pConnection->m_AGIHandle, strTmp );
		
		Resp_AgiCmd( Smt_Success, pdu );
		
		pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
*/
		Smt_Uint uRetu = g_pAstConnector->Agi_SendText( pConnection->m_AGIHandle, strTmp );
		Resp_AgiCmd( uRetu, pdu );
		if(uRetu ==  Smt_Success)
			pConnection->m_AGIEvtTimerID = SetSingleTimer(MAX_AGIEVTTIMERLEN, Evt_ICMU_AGIEvtTimer, (Smt_Uint)pConnection );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdSendMessageEx] ConnectionID<%s>, Message<%s>.",
		strConnectionID.c_str(), strMessage.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtMeetmeJoin
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TConnection* TConnectionState::OnEvtMeetmeJoin(Smt_Pdu &pdu)
{
	Smt_String strChannel;
	Smt_String strMeetmeNum;
	Smt_String strUserNum;
	Smt_String strConnectionID;
	TConnection* pConnection = NULL;

	pdu.GetString( Key_ICMU_Channel, &strChannel );
	pdu.GetString( Key_ICMU_MeetmeNum, &strMeetmeNum );
	pdu.GetString( Key_ICMU_Usernum, &strUserNum );
	
	pConnection = LookupByChannel( strChannel );
	if( pConnection != NULL )
	{
		pConnection->m_Reason = CauseMeetme;
		pConnection->m_MeetmeNum = strMeetmeNum;
		pConnection->m_UserNum = ACE_OS::atoi(strUserNum.c_str());
		strConnectionID = pConnection->GetID();
	}

	PrintLog(5,"[TConnectionState::OnEvtMeetmeJoin] ConnectionID<%s>, MeetmeNum<%s>, UserNum<%s>.",
		strConnectionID.c_str(), strMeetmeNum.c_str(), strUserNum.c_str() );
	
	return pConnection;
}

/****************************************************************************
函 数 名: OnEvtMeetmeLeave
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
TConnection* TConnectionState::OnEvtMeetmeLeave(Smt_Pdu &pdu)
{
	Smt_String strChannel;
	Smt_String strMeetmeNum;
	Smt_String strUserNum;
	Smt_String strConnectionID;
	TConnection* pConnection = NULL;
	
	pdu.GetString( Key_ICMU_Channel, &strChannel );
	pdu.GetString( Key_ICMU_MeetmeNum, &strMeetmeNum );
	pdu.GetString( Key_ICMU_Usernum, &strUserNum );
	
	pConnection = LookupByChannel( strChannel );
	if( pConnection != NULL )
	{
		pConnection->m_Reason = CauseMeetLeave;
		pConnection->m_MeetmeNum = strMeetmeNum;
		pConnection->m_UserNum = ACE_OS::atoi(strUserNum.c_str());
		strConnectionID = pConnection->GetID();
	}

	PrintLog(5,"[TConnectionState::OnEvtMeetmeLeave] ConnectionID<%s>, MeetmeNum<%s>, UserNum<%s>.",
		strConnectionID.c_str(), strMeetmeNum.c_str(), strUserNum.c_str() );
	
	return pConnection;
}

// TConnection* TConnectionState::OnEvtPlayTextEnd(Smt_Pdu &pdu)
// {
// 	Smt_String strChannel;
// 	Smt_String strMeetmeNum;
// 	Smt_String strStatus;
// 	Smt_String strConnectionID;
// 	TConnection* pConnection = NULL;
// 	
// 	pdu.GetString( Key_ICMU_Channel, &strChannel );
// 	pdu.GetString( Key_ICMU_Status, &strStatus );
// 	
// 	pConnection = LookupByChannel( strChannel );
// 	if( pConnection != NULL )
// 	{
// 		pConnection->m_Reason = CausePlayText;
// 		pConnection->m_AGIReason = atoi(strStatus.c_str());
// 		strConnectionID = pConnection->GetID();
// 		pConnection->EvtMedia( Evt_ICMU_TTSPlayEnd );
// 	}
// 	PrintLog(5,"[TConnectionState::OnEvtMeetmeLeave]pConnection<0x%x> ConnectionID<%s>,  PlayText Status<%s>.",
// 		pConnection,strConnectionID.c_str(), strStatus.c_str() );
// 	
// 	return pConnection;	
// }

/****************************************************************************
函 数 名: ChangeCallerIDByTAC
参    数:
返回数值: 
功能描述: 去掉中继访问码
*****************************************************************************/
Smt_String TConnectionState::ChangeCallerIDByTAC(Smt_String callerid, TConnection* pconnection)
{
	TConnection* pConnection = pconnection;
	Smt_String strCallerID = callerid;
	if( pConnection == NULL )
	{
		return "";
	}
	if( strCallerID == "" )
	{
		return "";
	}

	Smt_String strRet = strCallerID;
	Smt_String strTemp = strCallerID;
	Smt_String strRealCallerNum = "";
	Smt_Int    nPos = 0;
	
	do 
	{	
		nPos = pConnection->m_TACCode.find( strTemp.c_str() );
		
		if( nPos >= 0 )
		{
			DelPrefix(pConnection->m_TACCode, strTemp, strCallerID, strRealCallerNum );
			strRet = strRealCallerNum;
			break;
		}
		
		strTemp = strTemp.substr(0, strTemp.length()-1 );
		
	} while ( strTemp.length()>0 );
	
	PrintLog(5,"[TConnectionState::ChangeCallerIDByTAC] ConnectID<%s>,OriCallerID<%s>,CallerID<%s>.", 
		pConnection->GetID().c_str(), strCallerID.c_str(), strRet.c_str() );

	return strRet;
}

/****************************************************************************
函 数 名: OnRespGetVariable
参    数:
返回数值: 
功能描述: 获取从 PBX-Channel 得到的通道变量
*****************************************************************************/
Smt_Uint TConnectionState::OnRespGetVariable(Smt_Pdu &newpdu, Smt_Pdu &oripdu)
{
	Smt_String strConnectionID;
	Smt_String strVariable;
	Smt_String strValue;
	TConnection* pConnection = NULL;
	
	oripdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	newpdu.GetString( Key_ICMU_Variable, &strVariable );
	newpdu.GetString( Key_ICMU_Value, &strValue );

	if( strValue == AMI_CHANNEL_VARIABLE_NULL )
	{
		strValue = "";
	}

	// 特定参数分析 ConsultingParty
	// "ConsultationCall,ConsultingParty<xxxx>,ConsultedParty<xxxx>"
	Smt_Int nPos1,nPos2;
	Smt_String strTemp;
	Smt_String strConsultingParty="";
	Smt_String strConsultedParty="";
	nPos1 = strValue.find( AMI_CONSULTINGPARTY );
	if( nPos1>0 )
	{
		strTemp = strValue.substr(nPos1, strValue.length()-nPos1 );
		nPos1 = strTemp.find("<");
		nPos2 = strTemp.find(">");
		strConsultingParty = strTemp.substr(nPos1+1, nPos2-nPos1-1 );
	}

	nPos1 = strValue.find( AMI_CONSULTEDPARTY );
	if( nPos1>0 )
	{
		strTemp = strValue.substr(nPos1, strValue.length()-nPos1 );
		nPos1 = strTemp.find("<");
		nPos2 = strTemp.find(">");
		strConsultedParty = strTemp.substr(nPos1+1, nPos2-nPos1-1 );
	}

	// "SingleStepTransfer,TransferringParty<3004>"
	Smt_String strTransferredParty="";
	nPos1 = strValue.find( AMI_TRANSFERREDPARTY );
	if( nPos1>0 )
	{
		nPos1 = strValue.find("<");
		nPos2 = strValue.find(">");
		strTransferredParty = strValue.substr(nPos1+1, nPos2-nPos1-1 );
	}

	//add by caoyj 20120315
	// "MeetmeNum<xxxx>"
	Smt_String strMeetmeNum = "";
	nPos1 = strValue.find( AMI_MEETME_NUM );
	if( nPos1>0 )
	{
		nPos1 = strValue.find("<");
		nPos2 = strValue.find(">");
		strMeetmeNum = strValue.substr(nPos1+1, nPos2-nPos1-1 );
	}

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection ) == Smt_Success )
	{
		pConnection->m_Variable = strValue;
		strConsultedParty = ChangeCallerIDByTAC(strConsultedParty, pConnection);

		Smt_Pdu sendpdu;		
		sendpdu.m_Sender = GetGOR();
		sendpdu.m_Receiver = m_CallStateGOR;	
		sendpdu.m_MessageID = Evt_ICMU_ChannelDataReached;
		sendpdu.m_Status = Smt_Success;
		
		sendpdu.PutString( Key_ICMU_Channel, pConnection->m_Channel );
		sendpdu.PutString( Key_ICMU_ConnectionID, pConnection->GetID() );
		sendpdu.PutString( Key_ICMU_DeviceID, pConnection->m_DeviceID );
		sendpdu.PutString( Key_ICMU_Variable, strValue );
		sendpdu.PutString( Key_ICMU_ConsultingParty, strConsultingParty );
		sendpdu.PutString( Key_ICMU_ConsultedParty, strConsultedParty );
		sendpdu.PutString( Key_ICMU_TransferredParty, strTransferredParty );
		sendpdu.PutString( Key_ICMU_MeetmeNum, strMeetmeNum );

		if( sendpdu.m_Receiver > 0 ) PostMessage( sendpdu );
	}

	TConnection* pOtherConnection = NULL;
	if( strConsultingParty != "")
	{
		pOtherConnection = LookupByDeviceID( strConsultingParty );
		if( pOtherConnection != NULL )
		{
			pConnection->m_Source = strConsultingParty;
			pConnection->m_Destination = pConnection->m_DeviceID;
			pConnection->m_OtherConnectionID = pOtherConnection->GetID();
		}
	}

	if( strTransferredParty != "")
	{
		pOtherConnection = LookupByDeviceID( strTransferredParty );
		if( pOtherConnection != NULL )
		{
			pConnection->m_Source = strTransferredParty;
			pConnection->m_Destination = pConnection->m_DeviceID;
			pConnection->m_OtherConnectionID = pOtherConnection->GetID();
		}
	}

	PrintLog(5,"[TConnectionState::OnRespGetVariable] ConnectionID<%s>, ConsultingParty<%s>, ConsultedParty<%s>, TransferredParty<%s>, MeetmeNum<%s>, Variable<%s>, Value<%s>.",
		strConnectionID.c_str(), strConsultingParty.c_str(), strConsultedParty.c_str(), strTransferredParty.c_str(), strMeetmeNum.c_str(), strVariable.c_str(), strValue.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtGetVarTimer
参    数:
返回数值: 
功能描述: 从 pbx-channel 获取 var1 变量
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtGetVarTimer(Smt_Uint senderobj)
{
	Smt_String strConnnectionID;
	Smt_String strChannelID;
	Smt_String strDeviceID;
	TConnection* pConnection = NULL;
	Smt_String strActID;
	
	Smt_Pdu* pTemp = (Smt_Pdu*)senderobj;
	
	pTemp->GetString( Key_ICMU_ConnectionID, &strConnnectionID );
	pTemp->GetString( Key_ICMU_Channel, &strChannelID );
	pTemp->GetString( Key_ICMU_DeviceID, &strDeviceID );
	
	if( m_ConnectionMgr.Lookup(strConnnectionID, pConnection) == Smt_Success )
	{
		Smt_Pdu sendpdu;
		sendpdu.m_MessageID = Cmd_ICMU_GetVariable;
		sendpdu.PutString( Key_ICMU_ConnectionID, strConnnectionID );
		sendpdu.PutString( Key_ICMU_DeviceID, strDeviceID );
		
		strActID = g_pConnState->GenActionID( sendpdu );
		
		g_pAstConnector->Ami_GetVar( pConnection->m_AMIHandle, strActID, pConnection->m_Channel, AMI_CHANNEL_VARIABLE_VAR1 );
		
	}
	
	if(pTemp != NULL)
	{
		delete pTemp;
		pTemp = NULL;
	}
	
	PrintLog(5,"[TConnectionState::OnEvtGetVarTimer] ConnectionID<%s>,ChannelID<%s>,ActID<%s>.",
		strConnnectionID.c_str(), strChannelID.c_str(), strActID.c_str() );
	
	return Smt_Success;
}

Smt_Uint TConnectionState::OnCmdSetSIPHeader(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	TConnection* pConnection = NULL;
	Smt_String strHeaderKey;
	Smt_String strHeaderValue;
	Smt_Uint uHandle=0;
	
	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_DataKey, &strHeaderKey );
	pdu.GetString( Key_ICMU_DataValue, &strHeaderValue );
	
	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		uHandle =  pConnection->m_AGIHandle;
		pConnection->m_LastCommand = pdu;
		g_pAstConnector->Agi_SIPAddHeader( pConnection->m_AGIHandle,strHeaderKey, strHeaderValue );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdSetSIPHeader] Handle<0x%x>, ConnectionID<%s>, HeaderKey<%s>, HeaderValue<%s>.",
		uHandle,strConnectionID.c_str(), strHeaderKey.c_str(), strHeaderValue.c_str() );

	return Smt_Success;
}

Smt_Uint TConnectionState::OnCmdGetSIPHeader(Smt_Pdu &pdu)
{
	Smt_String strConnectionID;
	TConnection* pConnection = NULL;
	Smt_String strHeaderKey;
	Smt_Uint uHandle=0;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );
	pdu.GetString( Key_ICMU_DataKey, &strHeaderKey );

	if( m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success )
	{
		uHandle =  pConnection->m_AGIHandle;
		pConnection->m_LastCommand = pdu;
		g_pAstConnector->Agi_SIPGetHeader( pConnection->m_AGIHandle, strHeaderKey );
	}
	
	PrintLog(5,"[TConnectionState::OnCmdGetSIPHeader] Handle<0x%x>, ConnectionID<%s>, HeaderKey<%s>.",
		uHandle,strConnectionID.c_str(), strHeaderKey.c_str() );

	return Smt_Success;
}
/****************************************************************************
函 数 名: OnEvtNewexten
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtNewexten(Smt_Pdu &pdu)
{
	TConnection * pConnection = NULL;
	Smt_String strChannel;
	Smt_String strConnectionID="";
	Smt_String strContext;
	
	pdu.GetString( Key_ICMU_Channel, &strChannel );
	pdu.GetString( Key_ICMU_Context, &strContext );
	
	pConnection = LookupByChannel( strChannel );
	if( pConnection != NULL )
	{
		strConnectionID = pConnection->GetID();

		if( strContext == AMI_CONTEXT_INTERNAL_XFER )
		{
			pConnection->m_CurrContext = AMI_CONTEXT_INTERNAL_XFER;
		}
	}

	PrintLog(5,"[TConnectionState::OnEvtNewexten] ConnectionID<%s>, Channel<%s>, Context<%s>.",
		strConnectionID.c_str(), strChannel.c_str(), strContext.c_str() );

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtUserEventTimer
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint TConnectionState::OnEvtUserEventTimer(Smt_Uint timerid,Smt_Uint senderobj)
{
	Smt_String strConnectionID;
	TConnection* pConnection = NULL;
	TConnection* pTmpConnection = NULL;
	
	for( TConnectionMap::ITERATOR 
		iter = m_ConnectionMgr.begin();
		iter != m_ConnectionMgr.end(); 
		iter++ )
	{
		pTmpConnection = (*iter).int_id_;
		if( Smt_Uint(pTmpConnection) == senderobj )
		{
			pConnection = pTmpConnection;
			break;
		}
	}
	
	if( pConnection != NULL )
	{
		strConnectionID = pConnection->GetID();
		
		Smt_Pdu sendpdu;
		sendpdu.m_Sender = GetGOR();
		sendpdu.m_Receiver = GetGOR();
		sendpdu.m_Status = Smt_Success;
		sendpdu.m_MessageID = Evt_ICMU_UserEventTimerExpired;
		sendpdu.PutString( Key_ICMU_ConnectionID, strConnectionID );
		PostMessage( sendpdu );
	}
	else
	{
		PrintLog(5,"[TConnectionState::OnEvtUserEventTimer] senderobj<%d>,TimerID<%d>.",
			 senderobj, timerid );
	}

	return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtUserEventTimerExpired
参    数:
返回数值: 
功能描述: 关联 AGI Socket和 Connection 句柄
*****************************************************************************/
TConnection* TConnectionState::OnEvtUserEventTimerExpired(Smt_Pdu &pdu)
{
	Smt_String  strConnectionID;
	TConnection* pConnection = NULL;
	Smt_Bool    bFind = Smt_BoolFALSE;

	pdu.GetString( Key_ICMU_ConnectionID, &strConnectionID );

	if(m_ConnectionMgr.Lookup(strConnectionID, pConnection) == Smt_Success)
	{
		TAGIRequest* pTmpAGIRequest = NULL;
		TAGIRequestMap::ITERATOR iter(m_AGIRequestMgr);
		for( iter = m_AGIRequestMgr.begin();
			 iter != m_AGIRequestMgr.end();
			 ++iter )
		{
			pTmpAGIRequest = (*iter).int_id_;
			if ( pTmpAGIRequest->m_Uniqueid == pConnection->m_Uniqueid )
			{
				pConnection->m_AGIHandle = pTmpAGIRequest->m_AGIHandle;

				bFind = Smt_BoolTRUE;

				PrintLog(5, "[TConnectionState::OnEvtUserEventTimerExpired] ConnectionID<%s>, AGIHandle<0x%x>.",
					pConnection->GetID().c_str(), pConnection->m_AGIHandle );
				break;
			}
		}
	}

	if( bFind == Smt_BoolTRUE )
	{
		pdu.m_Status = CauseIVRRequest;
	}
	else // Smt_BoolFALSE
	{
		pdu.m_Status = CauseAGIInvalid;
	}

	return pConnection;
}
