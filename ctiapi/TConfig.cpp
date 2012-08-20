//***************************************************************************
// TConfig.cpp : implementation file
//***************************************************************************
#include "CMUInterface.h"
#include "TConfig.h"

TCMUDLLConfigFile* g_pCMUDLLCfg = NULL;

/////////////////////////////////////////////////////////////////////////////
// TCMUDLLConfigFile code
TCMUDLLConfigFile::TCMUDLLConfigFile(Smt_String filename)
{
	m_FileName = filename;
}

Smt_Uint TCMUDLLConfigFile::ReadConfigFile()
{
	Smt_String strVal;
	Smt_PrivateProfile pfFile( m_FileName );

	//
	// MCUDLL config
	//
	pfFile.GetPrivateProfileString("CMUDLLConfiger", "CMUDLLIP", "", strVal );
	m_CMUDLLIP = strVal;
	
	m_CMUDLLPort = pfFile.GetPrivateProfileInt("CMUDLLConfiger", "CMUDLLPort", 0 );
	
	pfFile.GetPrivateProfileString("CMUDLLConfiger", "ServerName", "", strVal );
	m_ServerName = strVal;
	
	pfFile.GetPrivateProfileString("CMUDLLConfiger", "ServerLog", "", strVal );
	m_ServerLogName = strVal;
	
	m_ServerLogLevel = pfFile.GetPrivateProfileInt("CMUDLLConfiger", "ServerLogLevel", 0 );
	
	pfFile.GetPrivateProfileString("CMUDLLConfiger", "UserName", "", strVal );
	m_UserName = strVal;
	
	pfFile.GetPrivateProfileString("CMUDLLConfiger", "UserLog", "", strVal );
	m_UserLogName = strVal;
	
	m_UserLogLevel = pfFile.GetPrivateProfileInt("CMUDLLConfiger", "UserLogLevel", 0 );

	pfFile.GetPrivateProfileString("LSConfiger", "LSIP", "", strVal );
	m_LSIP = strVal;
	
	m_LSPort = pfFile.GetPrivateProfileInt("LSConfiger", "LSPort", 0 );

	pfFile.GetPrivateProfileString("CMUConfiger", "CMUIP", "", strVal );
	m_CMUIP = strVal;
	
	m_CMUPort = pfFile.GetPrivateProfileInt("CMUConfiger", "CMUPort", 0 );

	ACE_DEBUG ((LM_DEBUG, "[CMUAPI::ReadConfigFile] m_UserLogName<%s>, m_LSIP<%s>, m_CMUIP<%s>.\n", 
		m_UserLogName.c_str(), m_LSIP.c_str(), m_CMUIP.c_str() ));

	if(m_CMUIP=="" || m_CMUPort==0  )
	{
		return Smt_Fail;
	}

	return Smt_Success;
}

//#end file
