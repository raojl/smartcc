//***************************************************************************
// cmuapi.cpp : implementation file
//***************************************************************************
#include "include/cmuapi.h"
#include "TUser.h"
#include "TConfig.h"

//
// User Init Type
//
enum UserInitTypes 
{
	INITTYPE_UNKNOWN       = 0,
	INITTYPE_INNER         = 1,        
	INITTYPE_EXTERN        = 2,              
};

Smt_Server*  g_pCMUDLLServerHandle = NULL;
Smt_Uint     g_UserInitType = INITTYPE_INNER;

Smt_Uint InitUser()
{
	if( g_pCMUDLLUser == NULL )
	{
		// get the execute file path
		Smt_String strCurrPath = HLGetModuleFilePath();
		Smt_String strCfgFileName = strCurrPath + DEFAULT_CONFIGFILENAME;
		g_pCMUDLLCfg = new TCMUDLLConfigFile( strCfgFileName );

		if( g_pCMUDLLCfg->ReadConfigFile() != Smt_Success)
		{
			delete g_pCMUDLLCfg;
			g_pCMUDLLCfg = NULL;

			ACE_DEBUG ((LM_DEBUG, "[CMUAPI::InitUser] Read Config File Fail, <%s>.\n", 
				strCfgFileName.c_str() ));

			return Err_ICMU_NoConfigFile;
		}

		// check dir, if dir is not existed, then create it.
		Smt_String strTemp1, strTemp2;
		strTemp2 = strCurrPath + g_pCMUDLLCfg->m_ServerLogName;
		HLCreateDirRecursion( strTemp1, strTemp2 );
		
		// Create server handle
		if( g_UserInitType == INITTYPE_INNER )
		{
			g_pCMUDLLCfg->m_ServerLogName = strCurrPath + g_pCMUDLLCfg->m_ServerLogName;
			g_pCMUDLLServerHandle = new Smt_Server(
				g_pCMUDLLCfg->m_ServerName,
				g_pCMUDLLCfg->m_ServerLogLevel, g_pCMUDLLCfg->m_ServerLogName,
				g_pCMUDLLCfg->m_CMUDLLIP , g_pCMUDLLCfg->m_CMUDLLPort, Smt_BoolFALSE, Smt_BoolTRUE );
			
			g_pCMUDLLServerHandle->m_LSIP = g_pCMUDLLCfg->m_LSIP;
			g_pCMUDLLServerHandle->m_LSPort = g_pCMUDLLCfg->m_LSPort;
		}

		// Create User
		g_pCMUDLLCfg->m_UserLogName = strCurrPath + g_pCMUDLLCfg->m_UserLogName;
		g_pCMUDLLUser = new TUser(
			g_pCMUDLLCfg->m_UserName, 
			g_pCMUDLLCfg->m_UserLogLevel, g_pCMUDLLCfg->m_UserLogName, 
			g_pCMUDLLServerHandle );

		ACE_DEBUG ((LM_DEBUG, "[CMUAPI::InitUser] CfgFileName<%s>, UserLogName<%s>, Level<%d>.\n", 
			strCfgFileName.c_str(), g_pCMUDLLCfg->m_UserLogName.c_str(), g_pCMUDLLCfg->m_UserLogLevel ));

		g_pCMUDLLServerHandle->AttachUser( g_pCMUDLLUser );
		
		g_pCMUDLLServerHandle->AddRemoteServer( g_pCMUDLLCfg->m_CMUIP, g_pCMUDLLCfg->m_CMUPort );

		if( g_UserInitType == INITTYPE_INNER )
			g_pCMUDLLServerHandle->Run();

		Smt_Uint nCount = 0;
		while(g_pCMUDLLUser->m_CallStateGOR == 0 )
		{
			HLSleep( DEFAULT_CMUDLLINITTIME );
			if( (nCount++) >= 3 ) break;
		}
	}

	return Smt_Success;
}

Smt_Uint UninitUser()
{
	try
	{		
		if(g_pCMUDLLServerHandle != NULL)
			g_pCMUDLLServerHandle->DetachUser( g_pCMUDLLUser );

		if( g_pCMUDLLUser != NULL )
		{
			delete g_pCMUDLLUser;
			g_pCMUDLLUser = NULL;
		}
		
		if( (g_UserInitType == INITTYPE_INNER) &&
			(g_pCMUDLLServerHandle != NULL) )
		{
			delete g_pCMUDLLServerHandle;
			g_pCMUDLLServerHandle = NULL;
		}
		
		if(g_pCMUDLLCfg != NULL)
		{
			delete g_pCMUDLLCfg;
			g_pCMUDLLCfg = NULL;
		}
	}
	catch (...)
	{
	}
	
	return Smt_Success;
}

/****************************************************************************
函 数 名: cmInitUser
参    数:
返回数值: 
功能描述: 调用 cmInitUser 接口方法初始化通讯连接时，CMUDLLIP、CMUDLLPort、ServerName、ServerLog、ServerLogLevel 配置无效
*****************************************************************************/

unsigned long CMUAPI cmInitUser(unsigned long handle)
{
	// init Server Handle
	g_pCMUDLLServerHandle = (Smt_Server*)handle;
	g_UserInitType = INITTYPE_EXTERN;
	g_pServerBase = g_pCMUDLLServerHandle->m_pServerBase;

	InitUser();

	return Smt_Success;
}

unsigned long CMUAPI cmUninitUser()
{
	UninitUser();
	return Smt_Success;
}

unsigned long CMUAPI ctcAssign( 
							unsigned long invokeid, 
							char* deviceid, 
							unsigned long devicetype)
{
	CMU_Uint nRet = 0;
	
	InitUser();

	nRet = g_pCMUDLLUser->CmdAssign( invokeid, deviceid, devicetype );

	if( nRet != Smt_Success ) nRet = Smt_Fail;

	return nRet;
}
							
unsigned long CMUAPI ctcDeassign( 
							unsigned long channelid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdDeassign( channelid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}
							
unsigned long CMUAPI ctcMakeCall( 
							unsigned long channelid, 
							char* callednum, 
							char* callernum )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdMakeCall( channelid, callednum, callernum );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}
							
unsigned long CMUAPI ctcHangupCall( 
							unsigned long channelid, 
							unsigned long callid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdHangupCall( channelid, callid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcClearCall( 
							 unsigned long channelid, 
							 unsigned long callid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdClearCall( channelid, callid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcHoldCall( 
							unsigned long channelid, 
							unsigned long callid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdHoldCall( channelid, callid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcRetrieveCall( 
							unsigned long channelid, 
							unsigned long callid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdRetrieveCall( channelid, callid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcConsultationCall( 
							unsigned long channelid, 							
							unsigned long callid,
							char* callednum,
							char* callernum,
							unsigned long timeout )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdConsultationCall( channelid, callid, callednum, callernum, timeout );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcConferenceCall( 
							unsigned long channelid, 
							unsigned long holdcallid,
							unsigned long activecallid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdConferenceCall( channelid, holdcallid, activecallid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcTransferCall( 
							unsigned long channelid, 
							unsigned long holdcallid,
							unsigned long activecallid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdTransferCall( channelid, holdcallid, activecallid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcReconnectCall( 
							unsigned long channelid, 
							unsigned long holdcallid,
							unsigned long activecallid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdReconnectCall( channelid, holdcallid, activecallid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}
							
unsigned long CMUAPI ctcSingleStepTransfer( 
							unsigned long channelid, 
							char* callednum,
							unsigned long callid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdSingleStepTransfer( channelid, callednum, callid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcSingleStepConference( 
							unsigned long channelid,
							char* callednum, 
							unsigned long callid, 
							unsigned long mode )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdSingleStepConference( channelid, callednum, callid, mode );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcGetEvent( 
							Csta_Event* event, 
							unsigned long timeout )
{
	CMU_Uint nRet = 0;
	InitUser();
	nRet = g_pCMUDLLUser->GetEventQueue(event, timeout );
	if( (nRet == Err_ICMU_Timeout) ||
		(nRet == Err_ICMU_MemoryException) )
	{
		nRet = Smt_Fail;
	}
	else
	{
		nRet = Smt_Success;
	}

	return nRet;
}

unsigned long CMUAPI ctcRouteSelected ( 
							unsigned long channelid,
							unsigned long routeid, 
							char* callednum,
							char* agentid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdRouteSelected( channelid, routeid, callednum, agentid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcSendDTMF( 
							unsigned long channelid,
							unsigned long callid, 
							char* digits )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdSendDTMF( channelid, callid, digits );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcStartRecord( 
							unsigned long channelid, 
							unsigned long callid, 
							char* filename )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdStartRecord( channelid, callid, filename );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcStopRecord(
							unsigned long channelid, 
							unsigned long callid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdStopRecord( channelid, callid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcSetDataValue( 
							unsigned long channelid, 
							unsigned long callid,
							char* key,
							char* value )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdSetData( channelid, callid, key, value );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcGetDataValue( 
							unsigned long channelid, 
							unsigned long callid,
							char* key )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdGetData( channelid, callid, key );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}
						
unsigned long CMUAPI ctcSnapshot( 
							unsigned long channelid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdDeviceSnapshot( channelid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI ctcSendMessage( 
							unsigned long channelid,
							unsigned long callid, 
							char* message )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdSendMessage( channelid, callid, message  );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI gc_open( 
							unsigned long invokeid, 
							char* deviceid, 
							unsigned long devicetype )
{
	CMU_Uint nRet = 0;
	InitUser();
	nRet = g_pCMUDLLUser->CmdAssignEx( invokeid, deviceid, devicetype );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI gc_close( 
							unsigned long channelid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdDeassignEx( channelid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI gc_makecall( 
							unsigned long invokeid,
							char* deviceid,
							char* callednum,
							char* callernum,
							unsigned long timeout )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdMakeCallEx( invokeid, deviceid, callednum, callernum, timeout );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI gc_answercall(
							unsigned long callid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdAnswerCallEx( callid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}
							
unsigned long CMUAPI gc_hangupcall(
							unsigned long callid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdHangupCallEx( callid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI gc_blindtransfer( 
							unsigned long callid, 
							char* callednum )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdSingleStepTransferEx( callid, callednum );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI gc_dial( 
							unsigned long callid, 
							char* callednum,
							unsigned long timeout )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdDial( callid, callednum, timeout);
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI dx_play( 
							unsigned long callid, 
							char* filename,
							char* escapedigits )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdPlayFile( callid, filename, escapedigits );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI dx_addiottdata( 
							unsigned long callid, 
							char* filename )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdAddFileList( callid, filename );
	return nRet;
}

unsigned long CMUAPI dx_playiottdata( 
							unsigned long callid, 
							char* escapedigits )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdPlayFileList( callid, escapedigits );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI dx_getdig( 
							unsigned long callid, 
							unsigned long timeout,
							unsigned long maxcount )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdGetDigits( callid, timeout, maxcount );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI dx_dial( 
							unsigned long callid, 
							char* digits )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdSendDTMFEx( callid, digits );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}
							
unsigned long CMUAPI dx_rec( 
							unsigned long callid,
							char* filename, 
							char* escapedigits, 
							unsigned long timeout, 
							unsigned long silence )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdRecordEx( callid, filename, escapedigits, timeout, silence );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI dx_sendmessage( 
							unsigned long callid, 
							char* message )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdSendMessageEx( callid, message );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI dx_setsipheader( 
							unsigned long callid,
							char* headerkey,
							char* headervalue )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdSetSIPHeader( callid, headerkey, headervalue );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI dx_getsipheader( 
							unsigned long callid,
							char* headerkey)
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdGetSIPHeader( callid, headerkey );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI fx_sendfax(unsigned long channelid,unsigned long callid,char* options  )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdSendFax(channelid, callid ,options );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}
							
unsigned long CMUAPI fx_rcvfax(unsigned long callid )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdReceiveFax( callid );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI fx_playtext(unsigned long callid,char* options  )
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdTTSPlay( callid ,options );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}
							
unsigned long CMUAPI cmSubscribeCallEvent()
{
	CMU_Uint nRet = 0;
	InitUser();
	nRet = g_pCMUDLLUser->CmdSubscribeCallEvent( );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

unsigned long CMUAPI cmUnsubscribeCallEvent()
{
	CMU_Uint nRet = 0;
	nRet = g_pCMUDLLUser->CmdUnsubscribeCallEvent( );
	if( nRet != Smt_Success ) nRet = Smt_Fail;
	return nRet;
}

