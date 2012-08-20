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
class TConfigFile
{
public:
	TConfigFile(Smt_String filename);

public:
	//
	// CMU config
	//
	Smt_String m_CMUIP;
	Smt_Uint   m_CMUPort;
	Smt_String m_CMUServerName;
	Smt_String m_CMUServerLogName;
	Smt_Uint   m_CMUServerLogLevel;
	Smt_String m_CMUUserName;
	Smt_String m_CMUUserLogName;
	Smt_Uint   m_CMUUserLogLevel;
	Smt_String m_CMUCallStateLogName;
	Smt_Uint   m_CMUCallStateLogLevel;
	Smt_String m_CMUDeviceStateLogName;
	Smt_Uint   m_CMUDeviceStateLogLevel;

	Smt_String m_ASTIP;                 
	Smt_Uint   m_AMIPort;  
	Smt_String m_AMIUser;      
	Smt_String m_AMIPassword;      
	Smt_String m_AGIIP;              
	Smt_Uint   m_AGIPort;            
	Smt_String m_ASTConnLog;            
	Smt_Uint   m_ASTConnLogLevel;

	Smt_Uint   m_nSSTransTimerLen;
	Smt_Uint   m_nPreStateTimerLen;

    Smt_Uint   ReadConfigFile();
	Smt_String m_FileName;
};

extern  TConfigFile* g_pCfg;

#endif
