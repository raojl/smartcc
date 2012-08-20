//***************************************************************************
//      Copyright (C) 2008 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : SmtContact5.0
//      Create Date : 2008.05.15												
//      Author      : Shenzj								
//      Discription : ≈‰÷√Œƒº˛¿‡
//      Modify By   :											
//***************************************************************************

#ifndef Smt_TCONFIG_HH
#define Smt_TCONFIG_HH


/////////////////////////////////////////////////////////////////////////////
// TConfigFile code
class TCMUDLLConfigFile
{
public:
	TCMUDLLConfigFile(Smt_String filename);

public:
	//
	// MCUDLL config
	//
	Smt_String m_CMUDLLIP;
	Smt_Uint   m_CMUDLLPort;
	Smt_String m_ServerName;
	Smt_String m_ServerLogName;
	Smt_Uint   m_ServerLogLevel;
	Smt_String m_UserName;
	Smt_String m_UserLogName;
	Smt_Uint   m_UserLogLevel;
	Smt_String m_LSIP;
	Smt_Uint   m_LSPort;

	Smt_String m_CMUIP;
	Smt_Uint   m_CMUPort;

    Smt_Uint   ReadConfigFile();
	Smt_String m_FileName;
};

extern  TCMUDLLConfigFile* g_pCMUDLLCfg;
/************************************************************************/

#endif
