//***************************************************************************
// CMUService_linux.cpp : implementation file
//***************************************************************************

#include "TASTConnector.h"
#include "TCallState.h"
#include "TDeviceState.h"
#include "TConnectionState.h"
#include "TConfig.h"

Smt_Server* g_pServerHandle = NULL;
	
int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	// get the execute file path
	Smt_String strCurrPath = HLGetModuleFilePath();
	
	// read ini file
	g_pCfg = new TConfigFile( strCurrPath + "../conf/base.conf" );
	
	if( g_pCfg->ReadConfigFile() != Smt_Success)
	{
		ACE_OS::printf("[ReadConfigFile] Fail, IP Or Port Is NULL." );
		delete g_pCfg;
		g_pCfg = NULL;
	
		return 0;
	}
	
	// check dir, if dir is not existed, then create it.
	Smt_String strTemp1, strTemp2;
	strTemp2 = strCurrPath + g_pCfg->m_CMUServerLogName;
	HLCreateDirRecursion( strTemp1, strTemp2 );
	
	// Create server handle
	g_pCfg->m_CMUServerLogName = strCurrPath + g_pCfg->m_CMUServerLogName;
	g_pServerHandle = new Smt_Server(
		g_pCfg->m_CMUServerName,
		g_pCfg->m_CMUServerLogLevel, g_pCfg->m_CMUServerLogName,
		g_pCfg->m_CMUIP , g_pCfg->m_CMUPort, Smt_BoolTRUE, Smt_BoolTRUE );

	// Create connection-state
	g_pCfg->m_CMUUserLogName = strCurrPath + g_pCfg->m_CMUUserLogName;
	g_pConnState = new TConnectionState(
		CMU_CONNECTIONSTATE, 
		g_pCfg->m_CMUUserLogLevel, g_pCfg->m_CMUUserLogName, 
		g_pServerHandle );
	g_pServerHandle->AttachUser( g_pConnState );

	// Create call-state
	Smt_String strCallStateLogName = strCurrPath + g_pCfg->m_CMUCallStateLogName;
	g_pCallState = new TCallState( 
		CMU_CALLSTATENAME, 
		g_pCfg->m_CMUCallStateLogLevel, strCallStateLogName, 
		g_pServerHandle );
	g_pServerHandle->AttachUser( g_pCallState );

	// Create device-state
	Smt_String strDeviceStateLogName = strCurrPath + g_pCfg->m_CMUDeviceStateLogName;
	g_pDeviceState = new TDeviceState( 
		CMU_DEVICESTATENAME, 
		g_pCfg->m_CMUDeviceStateLogLevel, strDeviceStateLogName, 
		g_pServerHandle );
	g_pServerHandle->AttachUser( g_pDeviceState );
	
	g_pServerHandle->Run();
		
	g_pConnState->PrintLog( 5, "[TCMUService::Runing......]" );
	
	while(1) HLSleep(1000) ;
	
	/*
	Smt_Byte buff[128];		
	while (scanf("%s",(char*)buff) >0)
	{
		if( (Smt_String((char*)buff) == "exit") || 
			  (Smt_String((char*)buff) == "e"))
		{
			break;
		}
	}
	*/
 
	g_pConnState->PrintLog( 5, "[TCMUService::OnStop]" );

		if(g_pServerHandle != NULL)
	{
		g_pServerHandle->DetachUser( g_pConnState );
		g_pServerHandle->DetachUser( g_pCallState );
		g_pServerHandle->DetachUser( g_pDeviceState);
	}

	if( g_pConnState != NULL )
	{
		delete g_pConnState;
		g_pConnState = NULL;
	}

	if( g_pCallState != NULL )
	{
		delete g_pCallState;
		g_pCallState = NULL;
	}

	if( g_pDeviceState != NULL )
	{
		delete g_pDeviceState;
		g_pDeviceState = NULL;
	}

	// Destroy server handle
	if( g_pServerHandle != NULL )
	{
		delete g_pServerHandle;
		g_pServerHandle = NULL;
	}
	
	if(g_pCfg != NULL)
	{
		delete g_pCfg;
		g_pCfg = NULL;
	}
	
	return 0;
}


