//***************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CMU
//      Create Date : 2009.10.14											
//      Author      : Shenzj								
//      Discription : Asterisk 客户端，含AMI、AGI接口的连接、解析等
//      Modify By   :											
//***************************************************************************
#ifndef Smt_TASTConnector_HH
#define Smt_TASTConnector_HH

#include "CMUInterface.h"
#include "RingBuffer.h"

/////////////////////////////////////////////////////////////////////////////
// TASTBuffer code
#define HANDLE_TYPE_AMI                         1    
#define HANDLE_TYPE_AGI                         2                                     
class TASTBuffer: public CRingBuffer
{
public:
	TASTBuffer()
	{ 
		m_Handle = NULL;
	}
	~TASTBuffer(){;}
public:
	Smt_Uint m_Handle;
	Smt_Uint m_HandleType;
	Smt_Pdu  m_AgiEvt;  //add by caoyj 20120106,放在AGI句柄上，防止待发事件被覆盖问题
};

/////////////////////////////////////////////////////////////////////////////
// TASTConnector code

class TASTConnector: public Smt_SimpleServer
{
public: 
	typedef Smt_Map<Smt_Uint, TASTBuffer*> THandleBufferMap;
public:
	TASTConnector(Smt_Uint loglevel, Smt_String logpath,
		Smt_String localip, Smt_Uint listenport, 
		Smt_Uint listenoption = Smt_BoolFALSE );
	~TASTConnector();

	Smt_Uint HandleMessage(Smt_Pdu& pdu);
	Smt_Uint OnTcpOnline(Smt_String remoteip,Smt_Uint remoteport, Smt_Uint handle, Smt_Uint chtype);
	Smt_Uint OnTcpOffline(Smt_String remoteip,Smt_Uint remoteport, Smt_Uint handle, Smt_Uint chtype);
	Smt_Uint OnTimer(Smt_Uint& timerid, Smt_Uint& messageid, Smt_Uint& senderobj);

public:
	Smt_Uint Ami_ExecAMI(Smt_Uint handle, Smt_String ami );
	Smt_Uint Ami_Login(Smt_Uint handle, Smt_String actid, Smt_String username, Smt_String password);
	Smt_Uint Ami_Logoff(Smt_Uint handle, Smt_String actid);
	Smt_Uint Ami_SIPShowPeer(Smt_Uint handle, Smt_String actid, Smt_String peer );
	Smt_Uint Ami_Command_ShowQueue(Smt_Uint handle, Smt_String actid, Smt_String queue );
	Smt_Uint Ami_Originate(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String exten, Smt_String callerid, Smt_String variable, Smt_String timeout, Smt_String context, Smt_String priority );   // 分机外呼 
	Smt_Uint Ami_Originate(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String application, Smt_String data, Smt_String callerid, Smt_String variable, Smt_String timeout );                       // 分机监听 
	Smt_Uint Ami_Hangup(Smt_Uint handle, Smt_String actid, Smt_String channel );
	Smt_Uint Ami_Redirect(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String extrachannel, Smt_String exten, Smt_String context, Smt_String priority );
	Smt_Uint Ami_PlayDTMF(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String digit );    // 一次只能播放一个 DTMF            
	Smt_Uint Ami_Monitor(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String filename );  // 仅支持 wav 文件格式的 MIX 录音      
	Smt_Uint Ami_StopMonitor(Smt_Uint handle, Smt_String actid, Smt_String channel );
	Smt_Uint Ami_Command_AddExtension(Smt_Uint handle, Smt_String actid, Smt_String extn, Smt_String priority, Smt_String application, Smt_String data, Smt_String context );
	Smt_Uint Ami_SendText(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String message );    
	Smt_Uint Ami_GetVar(Smt_Uint handle, Smt_String actid, Smt_String channel, Smt_String variable );    

	Smt_Uint Agi_ExecAGI(Smt_Uint handle, Smt_String agi );
	Smt_Uint Agi_Answer(Smt_Uint handle);
	Smt_Uint Agi_Hangup(Smt_Uint handle);
	Smt_Uint Agi_StreamFile(Smt_Uint handle, Smt_String filename, Smt_String digit);
	Smt_Uint Agi_GetData(Smt_Uint handle, Smt_String filename, Smt_String timeout, Smt_String maxdigits );
	Smt_Uint Agi_Record(Smt_Uint handle, Smt_String filename, Smt_String escapedigit, Smt_String timeout, Smt_String silence);  // 仅支持 wav 格式，每次录音都会发出 BEEP 音
	Smt_Uint Agi_Exec_SendDTMF(Smt_Uint handle, Smt_String digits);
	Smt_Uint Agi_Exec_Dial(Smt_Uint handle, Smt_String destnations, Smt_String timeout);
	Smt_Uint Agi_Exec_RetryDial(Smt_Uint handle, Smt_String announcefile, Smt_String sleeptime, Smt_String loopcount, Smt_String destnations, Smt_String timeout); //遇忙自动回呼
	Smt_Uint Agi_SendText(Smt_Uint handle, Smt_String message );    
	Smt_Uint Agi_SIPAddHeader(Smt_Uint handle, Smt_String headkey, Smt_String headvalue);
	Smt_Uint Agi_SIPGetHeader(Smt_Uint handle, Smt_String headkey);
	Smt_Uint Agi_Exec_PlayText(Smt_Uint handle, Smt_String options );

private:
	Smt_String  CRLF;
	Smt_String  SPACE;
	THandleBufferMap m_HandleMgr;
	Smt_Pdu     m_AmiEvt;
//	Smt_Pdu     m_AgiEvt;  //add by caoyj 20120106,放在AGI句柄上，防止待发事件被覆盖问题

	Smt_Uint SendCmd(Smt_Uint handle, Smt_String command );
	Smt_Uint ParseLine(Smt_String src, Smt_String &key , Smt_String &value );
	Smt_Uint ParseAMIEvent(TASTBuffer* phandle, Smt_Pdu& pdu);
	Smt_Uint ParseAGIEvent(TASTBuffer* phandle, Smt_Pdu& pdu);
};

extern TASTConnector* g_pAstConnector;
extern Smt_String TrimSpace(Smt_String src);
/************************************************************************/

#endif //
