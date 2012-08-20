//***************************************************************************
// TASTConnector.cpp : implementation file
//***************************************************************************

#include "TASTConnector.h"
#include "TConfig.h"
#include "TConnectionState.h"

TASTConnector* g_pAstConnector = NULL;

/////////////////////////////////////////////////////////////////////////////
// Public Function code
#define MAX_TRIMSTRINGBUFF       128
Smt_String TrimSpace(Smt_String src)
{	
	if( src.length()>=MAX_TRIMSTRINGBUFF ) return src;

	char cDest[MAX_TRIMSTRINGBUFF];
	char cSrc[MAX_TRIMSTRINGBUFF];

	ACE_OS::strcpy( cSrc, src.c_str());

	int i=0;	

	while(cSrc[i++] == ' ') ;	

	ACE_OS::strcpy( cDest, cSrc+i-1 );
	
	int len = ACE_OS::strlen(cDest);
	
	if(len == 0) return "";
	
	while(cDest[--len] == ' ') ;
	
	cDest[len+1] = 0;

	return Smt_String(cDest);
}

/////////////////////////////////////////////////////////////////////////////
// TASTConnector code

TASTConnector::TASTConnector( Smt_Uint loglevel, Smt_String logpath,
						Smt_String localip, Smt_Uint listenport, 
						Smt_Uint listenoption )
: Smt_SimpleServer(loglevel,logpath,localip,listenport,listenoption)
{
	CRLF = "\r\n";
	SPACE = " ";
	//m_EvtFiltered = Smt_BoolTRUE;
}

TASTConnector::~TASTConnector()
{
}

Smt_Uint TASTConnector::HandleMessage(Smt_Pdu& pdu)
{
	Smt_Uint nHandle;		
	pdu.GetUint( Key_IBaseLib_ChannelHandle, &nHandle );

	TASTBuffer* pHandle = NULL;
	if( m_HandleMgr.Lookup(nHandle, pHandle) == Smt_Success )
	{
		if( pHandle->m_HandleType == HANDLE_TYPE_AMI )
		{
			ParseAMIEvent( pHandle, pdu );
		}
		else // m_HandleType == HANDLE_TYPE_AGI
		{
			ParseAGIEvent( pHandle, pdu );
		}
	}
	else
	{
		PrintLog(3, "[TASTConnector::HandleMessage] Can not find handle<0x%x>.",nHandle);	
	}

	return Smt_Success;
}

Smt_Uint TASTConnector::OnTcpOnline(Smt_String remoteip,Smt_Uint remoteport, Smt_Uint handle, Smt_Uint chtype)
{
	PrintLog(3,"[TASTConnector::OnTcpOnline] RemoteIP<%s>, RemotePort<%d>, Handle<0x%x>, ChannelType<%d>.",
		remoteip.c_str(), remoteport, handle, chtype );

	Smt_Uint nHandleType;

	if( chtype == Smt_Tcp_ClientChannel )
	{
		nHandleType = HANDLE_TYPE_AMI;

		// AMI Online Message
		Ami_Login( handle, ACTION_LINKCONNECTED, g_pCfg->m_AMIUser, g_pCfg->m_AMIPassword );
		g_pConnState->m_AmiHandle = handle;
	}
	else // Smt_Tcp_ServerChannel
	{
		nHandleType = HANDLE_TYPE_AGI;
		// AGI Online
	}

	if( m_HandleMgr.FindByKey( handle ) != Smt_Success )
	{
		TASTBuffer* pHandle = new TASTBuffer();
		pHandle->Create( Smt_DATA_SIZE * MAX_RINGBUFFERSIZE );
		pHandle->m_Handle = handle;
		pHandle->m_HandleType = nHandleType;
		m_HandleMgr.SetAt( handle, pHandle );
	}

	return Smt_Success;
}

Smt_Uint TASTConnector::OnTcpOffline(Smt_String remoteip,Smt_Uint remoteport, Smt_Uint handle, Smt_Uint chtype)
{
	PrintLog(3,"[TASTConnector::OnTcpOffline] RemoteIP<%s>, RemotePort<%d>, Handle<0x%x>, ChannelType<%d>.",
		remoteip.c_str(), remoteport, handle, chtype );

	if( chtype == Smt_Tcp_ClientChannel )
	{
		// AMI Offline
		g_pConnState->m_AmiHandle = 0;

		Smt_Pdu pdu;
		pdu.m_Receiver = g_pConnState->GetGOR();
		pdu.m_MessageID = Evt_ICMU_LinkDown;
		pdu.PutUint( Key_ICMU_Reason, CausePBXNetWorkError );
		if( pdu.m_Receiver > 0 ) g_pConnState->PostMessage( pdu );
	}
	else // Smt_Tcp_ServerChannel
	{
		// AGI Offline	
		//add by caoyj 20120315
		Smt_Pdu pdu;
		pdu.m_Receiver = g_pConnState->GetGOR();
		pdu.m_MessageID = Evt_ICMU_AGIHandleOffline;
		pdu.PutUint( Key_ICMU_Handle, handle );
		g_pConnState->PostMessage( pdu );
		///////////////////////////////////////
	}

	TASTBuffer* pHandle = NULL;
	if( m_HandleMgr.Lookup(handle, pHandle) == Smt_Success )
	{
		m_HandleMgr.Remove( handle );
		if( pHandle != NULL )
		{
			delete pHandle;
			pHandle = NULL;
		}
	}

	return Smt_Success;
}

Smt_Uint TASTConnector::OnTimer(Smt_Uint& timerid, Smt_Uint& messageid, Smt_Uint& senderobj)
{
	return Smt_Success;
}

Smt_Uint TASTConnector::SendCmd(Smt_Uint handle, Smt_String command )
{
	Smt_Uint nRet = Smt_Fail;
	Smt_Pdu pdu;
	pdu.m_MessageID = Evt_IBaseLib_SendData;

	pdu.PutUint(Key_IBaseLib_ChannelHandle, handle );
	pdu.PutByteArray(Key_IBaseLib_DataBuffer, (Smt_ByteArray)command.c_str(), command.length() );
	
	if(handle>0)
		nRet = PostMessage( pdu );

	if( nRet != Smt_Success )
	{
		PrintLog(1, "[TASTConnector::SendCmd] Post Command Fail<0x%x>, Command<%s>,Handle<0x%x>.",
			nRet, command.c_str(), handle );
	}

	return nRet;
}

Smt_Uint TASTConnector::ParseAMIEvent(TASTBuffer* phandle, Smt_Pdu& pdu)
{
	Smt_Uint nByteArrayLen;	
	Smt_ByteArray cByteArray;
	
	pdu.GetByteArray(Key_IBaseLib_DataBuffer, &cByteArray, &nByteArrayLen );

	phandle->WriteBinary( (char*)cByteArray, nByteArrayLen );

	Smt_String strResp, strSubStr;
	Smt_String strKey, strValue;
	Smt_Int    nTemp;
	Smt_Uint   i;
	Smt_Bool   bIsData = Smt_BoolFALSE;

	while( phandle->ReadTextLine(strResp) )
	{
		//strSubStr = strResp.substr( 0, strResp.length()-2 );  // 去掉 "/r/n" 字串
		bIsData = Smt_BoolFALSE;
		for(i=0; i<strResp.length(); i++)
		{
			nTemp = *(strResp.substr( strResp.length()-i, 1 ).c_str());
			if( nTemp == 0 /*'\0'*/ || nTemp == 10 /*'\n'*/  || nTemp == 13/*'\r'*/)
				continue;			
			else
			{
				bIsData = Smt_BoolTRUE;
				break;
			}
		}	
		
		if( bIsData == Smt_BoolTRUE )
		{
			strSubStr = strResp.substr( 0, strResp.length()-i+1 );  // 去掉 "\r\n" 字串
		}
		else
		{
			strSubStr = strResp.substr( 0, strResp.length()-2 ); 
		}

		PrintLog(4, "[AMI EVT] <--- %s", strSubStr.c_str() );	
		
		if( (strSubStr == "")
			&& (m_AmiEvt.m_MessageID != Smt_Invalid_Value) )
		{
			m_AmiEvt.m_Receiver = g_pConnState->GetGOR();
			m_AmiEvt.PutUint( Key_ICMU_Handle, phandle->m_Handle );
			if( m_AmiEvt.m_Receiver > 0 ) g_pConnState->PostMessage( m_AmiEvt );
			m_AmiEvt.Clear();
			continue;
		}

		ParseLine(strSubStr, strKey, strValue );

		if( (strKey == "Response") &&
			(m_AmiEvt.m_MessageID !=Evt_ICMU_OriginateResponse) )    // AMI Response 
		{
			//m_EvtFiltered = Smt_BoolFALSE;			
			m_AmiEvt.m_MessageID = Evt_ICMU_AMIResponse;
			m_AmiEvt.PutString( Key_ICMU_ResponseResult , strValue );
			continue;
		}

		if( ACE_OS::strcasecmp( strKey.c_str(), "Event" ) == 0 )
		{
			//m_EvtFiltered = Smt_BoolFALSE;

			if( ACE_OS::strcasecmp(strValue.c_str(), "Newchannel" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_Newchannel;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "Newcallerid" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_Newcallerid;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "Dial" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_Dial;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "Link" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_Link;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "Unlink" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_Unlink;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "Hangup" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_Hangup;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "Hold" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_Hold;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "Unhold" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_Unhold;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "Join" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_Join;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "Leave" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_Leave;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "ParkedCall" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_ParkedCall;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "UnParkedCall" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_UnParkedCall;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "ParkedCallGiveUp" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_ParkedCallGiveUp;					
			else if( ACE_OS::strcasecmp(strValue.c_str(), "UserEvent" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_UserEvent;					
			else if( ACE_OS::strcasecmp(strValue.c_str(), "PeerStatus" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_PeerStatus;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "OriginateResponse" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_OriginateResponse;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "MeetmeJoin" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_MeetmeJoin;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "MeetmeLeave" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_MeetmeLeave;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "Newexten" ) == 0 )//add by caoyj 20120315
				m_AmiEvt.m_MessageID = Evt_ICMU_Newexten;

			// asterisk 1.4.24.1 修改了 chan_sip.c 后收到的事件
			else if( ACE_OS::strcasecmp(strValue.c_str(), "DTMFReceived" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_DTMFReceived;
			else if( ACE_OS::strcasecmp(strValue.c_str(), "MessageReceived" ) == 0 )
				m_AmiEvt.m_MessageID = Evt_ICMU_MessageReceived;
// 			else if (ACE_OS::strcasecmp(strValue.c_str(), "SmtTTS_Event" ) == 0 )
// 				m_AmiEvt.m_MessageID = Evt_ICMU_TTSPlayEnd;

			else // filter the others event
			{		
				//m_EvtFiltered = Smt_BoolTRUE;
			}

			continue;
		}

		//if( m_EvtFiltered == Smt_BoolTRUE ) // filter Newstate/Newexten/ExtensionStatus 事件
		//{
		//	continue;
		//}
		
		if( ACE_OS::strcasecmp( strKey.c_str(), "Channel" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Channel , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "CallerID" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_CallerID , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "CallerIDNum" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_CallerIDNum , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "CallerIDName" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_CallerIDName , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Uniqueid" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Uniqueid , strValue );					
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Source" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Source , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Destination" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Destination , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "SrcUniqueID" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_SrcUniqueID , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "DestUniqueID" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_DestUniqueID , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Channel1" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Channel1 , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Channel2" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Channel2 , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Uniqueid1" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Uniqueid1 , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Uniqueid2" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Uniqueid2 , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "CallerID1" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_CallerID1 , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "CallerID2" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_CallerID2 , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "ActionID" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_ActionID , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Message" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Message , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Cause" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Cause , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Cause-txt" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Causetxt , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Extension" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Extension , strValue );			
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Exten" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Exten , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "From" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_From , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Timeout" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Timeout , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Queue" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Queue , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Position" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Position , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Count" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Count , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "OriginalPosition" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_OriginalPosition , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "HoldTime" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_HoldTime , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "UserEvent" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_UserEvent , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Context" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Context , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Status" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Status , strValue );				
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Peer" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Peer , strValue );				
		else if( ACE_OS::strcasecmp( strKey.c_str(), "PeerStatus" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_PeerStatus , strValue );	
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Address-IP" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_AddressIP , strValue );				
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Address-Port" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_AddressPort , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "ObjectName" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_ObjectName , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Response" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Response , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Reason" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Reason , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Content-Type" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_ContentType , strValue );				
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Digits" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Digits , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "To" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_To , strValue );	
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Meetme" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_MeetmeNum , strValue );
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Usernum" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Usernum , strValue );		
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Variable" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Variable , strValue );		
		else if( ACE_OS::strcasecmp( strKey.c_str(), "Value" ) == 0 )
			m_AmiEvt.PutString( Key_ICMU_Value , strValue );		

		else // filter the others fields					
		{
		}
	}// while

	return Smt_Success;
}

Smt_Uint TASTConnector::ParseAGIEvent(TASTBuffer* phandle, Smt_Pdu& pdu)
{
	Smt_Uint nByteArrayLen;	
	Smt_ByteArray cByteArray;
	
	pdu.GetByteArray(Key_IBaseLib_DataBuffer, &cByteArray, &nByteArrayLen );

	phandle->WriteBinary( (char*)cByteArray, nByteArrayLen );
	
	Smt_String strResp, strSubStr, strKey, strValue;
	Smt_Int npos = 0;

	while( phandle->ReadTextLine(strResp) )
	{
		strSubStr = strResp.substr( 0, strResp.length()-1 );
//		PrintLog(4, "[AGI EVT] Handle<0x%x><--- %s", phandle->m_Handle,strSubStr.c_str() );	
		
		npos = strSubStr.find( AGI_RESPONSE_200 );

		PrintLog(5, "[AGI EVT] Handle<0x%x> npos<%d>,MessageID<%d>,Data<%s>", phandle->m_Handle,npos,phandle->m_AgiEvt.m_MessageID,strSubStr.c_str() );	

		if( npos<0 )
		{
			if( (strSubStr == "") 
				&& (phandle->m_AgiEvt.m_MessageID != Smt_Invalid_Value) )
			{
				phandle->m_AgiEvt.m_Receiver = g_pConnState->GetGOR();
				phandle->m_AgiEvt.PutUint( Key_ICMU_Handle, phandle->m_Handle );
				if( phandle->m_AgiEvt.m_Receiver > 0 ) 
				{
/*					if(phandle->m_AgiEvt.m_MessageID == Evt_ICMU_AGIRequest)
					{//add by caoyj 20120106
//						PrintLog(5, "[AGI EVT] Handle<0x%x> SendTo <%d> Evt_ICMU_AGIRequest success", phandle->m_Handle,phandle->m_AgiEvt.m_Receiver );	
						//在一定压力下，AMi事件滞后于AGI事件，需要先补发Newchannel事件,
						Smt_String strChannel;
						Smt_String strCallerIDNum;
						Smt_String strCallerIDName;
						Smt_String strUniqueid;
						Smt_String strConnectionID;
						
	                    phandle->m_AgiEvt.GetString( Key_ICMU_agi_channel , &strChannel );
						phandle->m_AgiEvt.GetString( Key_ICMU_agi_callerid, &strCallerIDNum );
						phandle->m_AgiEvt.GetString( Key_ICMU_agi_calleridname, &strCallerIDName );
						phandle->m_AgiEvt.GetString( Key_ICMU_agi_uniqueid, &strUniqueid );

						Smt_Pdu pdu;
						pdu.m_MessageID=Evt_ICMU_Newchannel;
						pdu.m_Receiver = g_pConnState->GetGOR();
						pdu.PutUint( Key_ICMU_Handle,g_pConnState->m_AmiHandle);
						pdu.PutString( Key_ICMU_Channel, strChannel );
						pdu.PutString( Key_ICMU_CallerIDNum, strCallerIDNum );
						pdu.PutString( Key_ICMU_CallerIDName, strCallerIDName );
						pdu.PutString( Key_ICMU_Uniqueid, strUniqueid );
						g_pConnState->PostMessage( pdu );

					}
//					PrintLog(5, "[AGI EVT] Handle<0x%x> SendTo <%d> Msg<%d> success", phandle->m_Handle,phandle->m_AgiEvt.m_Receiver,phandle->m_AgiEvt.m_MessageID );	
*/
					g_pConnState->PostMessage( phandle->m_AgiEvt );
				}
				else
				{
					PrintLog(5, "[AGI EVT] Handle<0x%x> SendTo <%d> Msg<%d> fail", phandle->m_Handle,phandle->m_AgiEvt.m_Receiver,phandle->m_AgiEvt.m_MessageID );	
				}
				phandle->m_AgiEvt.Clear();
				continue;
			}
			
			ParseLine(strSubStr, strKey, strValue );
			
			if( ACE_OS::strcasecmp( strKey.c_str(), "agi_network_script" ) == 0 ) // AGI Request
			{
				phandle->m_AgiEvt.m_MessageID = Evt_ICMU_AGIRequest;
				phandle->m_AgiEvt.PutString( Key_ICMU_agi_network_script , strValue );
//				PrintLog(5, "[AGI EVT] Handle<0x%x> set Msg<%d>-Smt_Invalid_Value<%d> ok", phandle->m_Handle,phandle->m_AgiEvt.m_MessageID,Smt_Invalid_Value );	
				continue;
			}
			
			if( ACE_OS::strcasecmp( strKey.c_str(), "agi_request" ) == 0 )
				phandle->m_AgiEvt.PutString( Key_ICMU_agi_request , strValue );
			else if( ACE_OS::strcasecmp( strKey.c_str(), "agi_channel" ) == 0 )
				phandle->m_AgiEvt.PutString( Key_ICMU_agi_channel , strValue );
			else if( ACE_OS::strcasecmp( strKey.c_str(), "agi_uniqueid" ) == 0 )
				phandle->m_AgiEvt.PutString( Key_ICMU_agi_uniqueid , strValue );
			else if( ACE_OS::strcasecmp( strKey.c_str(), "agi_callerid" ) == 0 )
				phandle->m_AgiEvt.PutString( Key_ICMU_agi_callerid , strValue );
			else if( ACE_OS::strcasecmp( strKey.c_str(), "agi_calleridname" ) == 0 )
				phandle->m_AgiEvt.PutString( Key_ICMU_agi_calleridname , strValue );
			else if( ACE_OS::strcasecmp( strKey.c_str(), "agi_dnid" ) == 0 )
				phandle->m_AgiEvt.PutString( Key_ICMU_agi_dnid , strValue );
			else if( ACE_OS::strcasecmp( strKey.c_str(), "agi_rdnis" ) == 0 )
				phandle->m_AgiEvt.PutString( Key_ICMU_agi_rdnis , strValue );
			else if( ACE_OS::strcasecmp( strKey.c_str(), "agi_context" ) == 0 )
				phandle->m_AgiEvt.PutString( Key_ICMU_agi_context , strValue );
			else if( ACE_OS::strcasecmp( strKey.c_str(), "agi_extension" ) == 0 )
				phandle->m_AgiEvt.PutString( Key_ICMU_agi_extension , strValue );
			else // filter others fields
			{
			}
		}
		else // npos>=0  200 result=? (timeout) endpos=?
		{
			Smt_Int nResultPos;
			Smt_Int nReasonPos1,nReasonPos2;
			Smt_Int nEndPos;
			Smt_Int nSpacePos;
			Smt_Int nEqualPos;
			Smt_String strTemp;
			Smt_String strValue;

			nResultPos = strSubStr.find( AGI_RESPONSE_RESULT );
			strTemp = strSubStr.substr(nResultPos, strSubStr.length()-nResultPos );
			nEqualPos = strTemp.find("=");
			nSpacePos = strTemp.find(" ");
			strValue = strTemp.substr(nEqualPos+1, nSpacePos-nEqualPos-1 );
			phandle->m_AgiEvt.PutString( Key_ICMU_agi_result , strValue );

			nEndPos = strSubStr.find( AGI_RESPONSE_ENDPOS );
			strTemp = strSubStr.substr(nEndPos, strSubStr.length()-nEndPos );
			nEqualPos = strTemp.find("=");
			nSpacePos = strTemp.find(" ");
			strValue = strTemp.substr(nEqualPos+1, nSpacePos-nEqualPos-1 );
			phandle->m_AgiEvt.PutString( Key_ICMU_agi_endpos , strValue );

			nReasonPos1 = strSubStr.find( "(" );
			nReasonPos2 = strSubStr.find( ")" );
			if( nReasonPos1>0 )
			{
				strTemp = strSubStr.substr(nReasonPos1+1, nReasonPos2-nReasonPos1-1 );
 				phandle->m_AgiEvt.PutString( Key_ICMU_agi_reason, strTemp );
			}

			phandle->m_AgiEvt.m_Receiver = g_pConnState->GetGOR();
			phandle->m_AgiEvt.m_MessageID = Evt_ICMU_AGIResponse;
			phandle->m_AgiEvt.PutUint( Key_ICMU_Handle, phandle->m_Handle );
			if( phandle->m_AgiEvt.m_Receiver > 0 ) g_pConnState->PostMessage( phandle->m_AgiEvt );
			phandle->m_AgiEvt.Clear();
		}
		
	} // while

	return Smt_Success;
}

Smt_Uint TASTConnector::Ami_ExecAMI(Smt_Uint handle, Smt_String command )
{	
	PrintLog(4, "[TASTConnector::Ami_ExecAMI] %s", command.c_str() );	
	
	return SendCmd( handle, command );
}

Smt_Uint TASTConnector::Agi_ExecAGI(Smt_Uint handle, Smt_String agi )
{	
	PrintLog(4, "[TASTConnector::Agi_ExecAGI] %s", agi.c_str() );
	
	return SendCmd( handle, agi );
}

Smt_Uint TASTConnector::Ami_Login(Smt_Uint handle, Smt_String actid, Smt_String username, Smt_String password)
{
	Smt_String strAct;
	strAct+= CRLF;
	strAct+= "Action: Login" + CRLF; 
	strAct+= "Username: " + username + CRLF;
	strAct+= "Secret: " + password + CRLF;
	strAct+= "Events: on" + CRLF;
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;

	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );

	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_Logoff(Smt_Uint handle, Smt_String actid)
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: Logoff" + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;

	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );

	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_SIPShowPeer(Smt_Uint handle, Smt_String actid, Smt_String peer )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: SIPShowPeer" + CRLF; 
	strAct+= "Peer: " + peer + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;

	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_Command_ShowQueue(Smt_Uint handle, Smt_String actid, Smt_String queue )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: Command" + CRLF; 
	strAct+= "Command: Show Queue " + queue + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_Originate(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String exten, Smt_String callerid, Smt_String variable, Smt_String timeout, Smt_String context, Smt_String priority )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: Originate" + CRLF; 
	strAct+= "Channel: " + channel + CRLF; 
	strAct+= "Exten: " + exten + CRLF; 
	strAct+= "CallerID: " + callerid + CRLF; 
	strAct+= "Variable: " + variable + CRLF;
	strAct+= "Context: " + context + CRLF; 
	strAct+= "Priority: " + priority + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= "Async: true" + CRLF;
	strAct+= "Timeout: " + timeout + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_Originate(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String application, Smt_String data, Smt_String callerid, Smt_String variable, Smt_String timeout )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: Originate" + CRLF; 
	strAct+= "Channel: " + channel + CRLF; 
	strAct+= "Application: " + application + CRLF; 
	strAct+= "Data: " + data + CRLF; 
	strAct+= "CallerID: " + callerid + CRLF; 
	strAct+= "Variable: " + variable + CRLF;
	strAct+= "Async: true" + CRLF;
	strAct+= "Timeout: " + timeout + CRLF;
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_Hangup(Smt_Uint handle, Smt_String actid, Smt_String channel )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: Hangup" + CRLF; 
	strAct+= "Channel: " + channel + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_Redirect(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String extrachannel, Smt_String exten, Smt_String context, Smt_String priority )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: Redirect" + CRLF; 
	strAct+= "Channel: " + channel + CRLF; 

	if( extrachannel != "")
	{
		strAct+= "ExtraChannel: " + extrachannel + CRLF; 
	}

	strAct+= "Exten: " + exten + CRLF; 
	strAct+= "Context: " + context + CRLF; 
	strAct+= "Priority: " + priority + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_PlayDTMF(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String digit )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: PlayDTMF" + CRLF; 
	strAct+= "Channel: " + channel + CRLF; 
	strAct+= "Digit: " + digit + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_Monitor(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String filename )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: Monitor" + CRLF; 
	strAct+= "Channel: " + channel + CRLF; 
	strAct+= "File: " + filename + CRLF; 
	strAct+= "Format: wav" + CRLF; 
	strAct+= "Mix: 0" + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_StopMonitor(Smt_Uint handle, Smt_String actid, Smt_String channel )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: StopMonitor" + CRLF; 
	strAct+= "Channel: " + channel + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_SendText(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String message )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: SendText" + CRLF; 
	strAct+= "Channel: " + channel + CRLF; 
	strAct+= "Message: " + message + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Ami_GetVar(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String variable )
{
	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: GetVar" + CRLF; 
	strAct+= "Channel: " + channel + CRLF; 
	strAct+= "Variable: " + variable + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_Answer(Smt_Uint handle)
{
	Smt_String strAct;
	strAct = "Answer" + SPACE;

	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );

	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_Hangup(Smt_Uint handle)
{
	Smt_String strAct;
	strAct = "Hangup" + SPACE;

	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );

	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_StreamFile(Smt_Uint handle, Smt_String filename, Smt_String digit)
{
	Smt_String strAct;
	strAct = "Stream File" + SPACE;
	strAct+= filename + SPACE;
	strAct+= digit; 

	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );

	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_GetData(Smt_Uint handle, Smt_String filename, Smt_String timeout, Smt_String maxdigits )
{
	Smt_String strAct;
	strAct = "Get Data" + SPACE;
	strAct+= filename + SPACE;
	strAct+= timeout + SPACE;
	strAct+= maxdigits;
	
	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_Record(Smt_Uint handle, Smt_String filename, Smt_String escapedigit, Smt_String timeout, Smt_String silence)
{
	Smt_String strAct;
	strAct = "Record File" + SPACE;
	strAct+= filename + SPACE;
	strAct+= "wav" + SPACE;
	strAct+= escapedigit + SPACE;
	strAct+= timeout + SPACE;
	strAct+= "BEEP" + SPACE;
	strAct+= "s=" + silence;
	
	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_Exec_SendDTMF(Smt_Uint handle, Smt_String digits)
{
	Smt_String strAct;
	strAct = "Exec" + SPACE;
	strAct+= "SendDTMF" + SPACE;
	strAct+= digits;

	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_Exec_PlayText(Smt_Uint handle, Smt_String options )
{
	Smt_String strAct;
	strAct = "Exec" + SPACE;
	strAct+= "Smttts" + SPACE;
	strAct+= options;
	
	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::ParseLine(Smt_String src, Smt_String &key, Smt_String &value )
{   //012345
	//XX: XX
	Smt_Int nPos = src.find(":");
	if( nPos > 0 )
	{
		key = src.substr(0, nPos);
		value = src.substr( nPos + 1, src.length() - (nPos+1) );
		value = TrimSpace( value );
	}
	else
	{
		key = "";
		value = "";
	}
	return Smt_Success;
}

Smt_Uint TASTConnector::Ami_Command_AddExtension(Smt_Uint handle, Smt_String actid, Smt_String extn, Smt_String priority, Smt_String application, Smt_String data, Smt_String context )
{
	Smt_String strTmp;
	strTmp = HLFormatStr("%s,%s,%s,%s into %s replace",
		extn.c_str(), priority.c_str(), application.c_str(), data.c_str(), context.c_str() );

	Smt_String strAct = "";
	strAct+= CRLF;
	strAct+= "Action: Command" + CRLF; 
	strAct+= "Command: Dialplan Add Extension " + strTmp + CRLF; 
	strAct+= "ActionID: " + actid + CRLF;
	strAct+= CRLF;
	
	PrintLog(4, "[AMI CMD] %s", strAct.c_str() );	

	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_Exec_Dial(Smt_Uint handle, Smt_String destnations, Smt_String timeout)
{
	Smt_String strAct;
	strAct = "Exec" + SPACE;
	strAct+= "Dial" + SPACE;
	//strAct+= HLFormatStr("%s|%s|tTr", destnations.c_str(), timeout.c_str());
	//add by caoyj 20120221
	strAct+= HLFormatStr("%s|%s|tTm(default)", destnations.c_str(), timeout.c_str());
	
	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_Exec_RetryDial(Smt_Uint handle, Smt_String announcefile, Smt_String sleeptime, Smt_String loopcount, Smt_String destnations, Smt_String timeout)
{
	Smt_String strAct;
	strAct = "Exec" + SPACE;
	strAct+= "RetryDial" + SPACE;
	strAct+= HLFormatStr("%s|%s|%s|%s|%s|tTr", 
		announcefile.c_str(), sleeptime.c_str(), loopcount.c_str(), destnations.c_str(), timeout.c_str());

	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_SendText(Smt_Uint handle, Smt_String message )
{
	Smt_String strAct;
	strAct = "Send Text" + SPACE;
	strAct+= message;
	
	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_SIPAddHeader(Smt_Uint handle, Smt_String headkey, Smt_String headvalue)
{
	// EXEC SIPAddHeader X-BaseKey:1234

	Smt_String strAct;
	strAct = "EXEC" + SPACE;
	strAct+= "SIPAddHeader" + SPACE;
	strAct+= headkey + ":" + headvalue;
	
	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );
	
	return SendCmd( handle, strAct );
}

Smt_Uint TASTConnector::Agi_SIPGetHeader(Smt_Uint handle, Smt_String headkey)
{
	//GET VARIABLE SIP_HEADER(X-Best-Codec)

	Smt_String strAct;
	strAct = "GET VARIABLE" + SPACE;
	strAct+= "SIP_HEADER(" + headkey + ")";
	
	PrintLog(4, "[AGI CMD] Handle<0x%x>---> %s", handle, strAct.c_str() );
	
	return SendCmd( handle, strAct );
}
