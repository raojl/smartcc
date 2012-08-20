// CMU.cpp : 定义控制台应用程序的入口点。
//

#include "TASTConnector.h"
#include "TCallState.h"
#include "TDeviceState.h"
#include "TConnectionState.h"
#include "TConfig.h"

Smt_Server* g_pServerHandle = NULL;

class TCMUService: public Smt_WinService
{
public:
	TCMUService(const char* szServiceName,const char* szServiceDisName)
		:Smt_WinService(szServiceName, szServiceDisName)
	{;}
	~TCMUService(void){;}
	void OnRun();
	void OnStop();	
};

void TCMUService::OnRun()
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
		
		LogAppEvent(EVENTLOG_ERROR_TYPE, EVMSG_STARTED, m_szServiceName, "Read Config File <base.conf> Fail");
		return;
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
}

void TCMUService::OnStop()
{
	// Destroy connection-state
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
}

///////////////////////////////////////////////////////////////////////
// main function
int ACE_TMAIN(int argc, ACE_TCHAR* argv[])
{
	TCMUService sService("CMUService","Smt CMUService");
	if( sService.ParseCommand( argc, argv ) == FALSE )
	{
		sService.StartService();
	}
	
	return 0;
}
