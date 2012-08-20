//***************************************************************************
// TLSUser.cpp : implementation file
//***************************************************************************
#include "CMUInterface.h"
#include "TConfig.h"


TConfigFile* g_pCfg = NULL;

/////////////////////////////////////////////////////////////////////////////
// TConfigFile code
TConfigFile::TConfigFile(Smt_String filename)
{
	m_FileName = filename;
}

Smt_Uint TConfigFile::ReadConfigFile()
{
	Smt_String strVal;
	Smt_PrivateProfile pfFile( m_FileName );

	//
	// CMU config
	//
	pfFile.GetPrivateProfileString("CMUConfiger", "CMUIP", "", strVal );
	m_CMUIP = strVal;
	
	m_CMUPort = pfFile.GetPrivateProfileInt("CMUConfiger", "CMUPort", 0 );
	
	pfFile.GetPrivateProfileString("CMUConfiger", "ServerName", "", strVal );
	m_CMUServerName = strVal;
	
	pfFile.GetPrivateProfileString("CMUConfiger", "ServerLog", "", strVal );
	m_CMUServerLogName = strVal;
	
	m_CMUServerLogLevel = pfFile.GetPrivateProfileInt("CMUConfiger", "ServerLogLevel", 0 );
	
	pfFile.GetPrivateProfileString("CMUConfiger", "UserName", "", strVal );
	m_CMUUserName = strVal;
	
	pfFile.GetPrivateProfileString("CMUConfiger", "UserLog", "", strVal );
	m_CMUUserLogName = strVal;
	
	m_CMUUserLogLevel = pfFile.GetPrivateProfileInt("CMUConfiger", "UserLogLevel", 0 );

	pfFile.GetPrivateProfileString("CMUConfiger", "CallStateLog", "", strVal );
	m_CMUCallStateLogName = strVal;

	m_CMUCallStateLogLevel = pfFile.GetPrivateProfileInt("CMUConfiger", "CallStateLogLevel", 0 );

	pfFile.GetPrivateProfileString("CMUConfiger", "DeviceStateLog", "", strVal );
	m_CMUDeviceStateLogName = strVal;
	
	m_CMUDeviceStateLogLevel = pfFile.GetPrivateProfileInt("CMUConfiger", "DeviceStateLogLevel", 0 );

	//
	// Asterisk info config
	//
	pfFile.GetPrivateProfileString("CMUConfiger", "ASTIP", "", strVal );
	m_ASTIP = strVal;

	m_AMIPort = pfFile.GetPrivateProfileInt("CMUConfiger", "AMIPort", 0 );

	pfFile.GetPrivateProfileString("CMUConfiger", "AMIUser", "", strVal );
	m_AMIUser = strVal;

	pfFile.GetPrivateProfileString("CMUConfiger", "AMIPassword", "", strVal );
	m_AMIPassword = strVal;

	pfFile.GetPrivateProfileString("CMUConfiger", "ASTConnLog", "", strVal );
	m_ASTConnLog = strVal;
	
	m_ASTConnLogLevel = pfFile.GetPrivateProfileInt("CMUConfiger", "ASTConnLogLevel", 0 );
	
	pfFile.GetPrivateProfileString("CMUConfiger", "AGIIP", "", strVal );
	m_AGIIP = strVal;
	
	m_AGIPort = pfFile.GetPrivateProfileInt("CMUConfiger", "AGIPort", 0 );

	m_nSSTransTimerLen      = pfFile.GetPrivateProfileInt("CMUConfiger", "SSTransTimerLen", 0 );
	m_nPreStateTimerLen     = pfFile.GetPrivateProfileInt("CMUConfiger", "PreStateTimerLen", 0 );

	return Smt_Success;
}

//#end file
