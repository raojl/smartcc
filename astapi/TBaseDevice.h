//**************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : IVR5.0   	
//      Create Date : 2009.10.10      	
//      Author      : Linyl    
//      Discription : BaseDevice Class
//***************************************************************************


#ifndef Smt_BASEDEVICE_HH
#define Smt_BASEDEVICE_HH
#include "IVRInterface.h"


#define MAXCHNAME               64
#define MAXCHANNEL              1024

class TBaseDevice: public Smt_Thread_Ex
{
public:

	TBaseDevice();
	~TBaseDevice();
	Smt_Uint StartUp(Smt_User *pUser, Smt_Server *pServer, Smt_Pdu pdu);
	Smt_Uint Run();
	Smt_Uint PutMessageEx(Smt_Pdu pdu);
	int svc(void);
	Smt_Uint HandleMessage(Smt_Pdu &pdu);
	
	virtual void HandleTimerItem(Smt_Pdu &pdu){}
	virtual Smt_Uint GetDeviceEvent() = 0;
	virtual Smt_Uint InitDevice(Smt_Pdu pdu) = 0;
    virtual Smt_Uint Stop() = 0;

	//
	//CallControl
	//
	virtual Smt_Uint AnswerCall(Smt_Uint channel, Smt_Uint rings);
	virtual Smt_Uint MakeCall(Smt_Uint channel, Smt_String ani, Smt_String dnis);
	virtual Smt_Uint HangupCall(Smt_Uint channel);
	virtual Smt_Uint TransferCall(Smt_Uint channel, Smt_Uint transfertype, Smt_String ani, Smt_String dnis, Smt_Uint nTransTimeOut);
	virtual Smt_Uint ModifyCall(Smt_Uint channel, Smt_String audioaddr, Smt_Uint audioport, Smt_Uint audiotype, Smt_String videoaddr, Smt_Uint videoport, Smt_Uint videotype);
	virtual Smt_Uint HoldCall(Smt_Uint channel);
	virtual Smt_Uint RetrieveCall(Smt_Uint channel);
	virtual Smt_Uint ConChannel(Smt_Uint firstch, Smt_Uint secondch, Smt_Uint mode);
	virtual Smt_Uint DisconChannel(Smt_Uint firstch, Smt_Uint secondch);

	//
	//MediaControl
	//
	virtual Smt_Uint PlayMedia(Smt_Uint channel, Smt_Uint type, Smt_String termkey, Smt_String content, Smt_Kvset* pMemArray, Smt_Uint nMemCount, Smt_Pdu pdu);
	virtual Smt_Uint RecordMedia(Smt_Uint channel, Smt_Uint type, Smt_String termkey,Smt_String content, Smt_Uint rectime, Smt_Uint silenttime, Smt_Uint recdouble);
	virtual Smt_Uint GetDTMF(Smt_Uint channel, Smt_Uint maxlen, Smt_Uint starttime, Smt_Uint innertime, Smt_String termkey);
	virtual Smt_Uint RecvFax(Smt_Uint channel, Smt_String filename);
	virtual Smt_Uint SendFax(Smt_Uint channel, Smt_String filename, Smt_Uint startpage, Smt_Uint pagenum);
	virtual Smt_Uint StopMedia(Smt_Uint channel);
	virtual Smt_Uint RtpInit(Smt_Uint channel, Smt_String audioaddr, Smt_Uint audioport, Smt_Uint audiotype, Smt_String videoaddr, Smt_Uint videoport, Smt_Uint videotype);
	virtual Smt_Uint RtpClose(Smt_Uint channel);
    virtual Smt_Uint SetSipHead(Smt_Uint channel, Smt_String sipheadkey, Smt_String sipheadvalue);
	virtual Smt_Uint GetSipHead(Smt_Uint channel, Smt_String sipheadkey, Smt_String sipheadvalue);
	//
	//Conference
	//
	virtual Smt_Uint ConfCreate();
	virtual Smt_Uint ConfAddParty();
	virtual Smt_Uint ConfRemParty();
	virtual Smt_Uint ConfDelete();
	virtual Smt_Uint ConfSetAttr();
	virtual Smt_Uint ConfGetAttr();
	virtual Smt_Uint ConfSetPartyAttr();
	virtual Smt_Uint ConfGetPartyAttr();

	//
	//CallEvent
	//
	Smt_Uint EvtUnavailable(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtIdle(Smt_Uint channel,Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtOffering(Smt_Uint channel, Smt_String callrefid, Smt_String ani, Smt_String dnis, Smt_Uint chtype, Smt_String sessionid);
	Smt_Uint EvtProceeding(Smt_Uint channel, Smt_String callrefid, Smt_String ani, Smt_String dnis, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtAlerting(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtConnected(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtTransfered(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtDisconnected(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtRetrieved(Smt_Uint channel ,Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtHeld(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtConChannel(Smt_Uint firstch, Smt_Uint secondch, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtDisconChannel(Smt_Uint firstch, Smt_Uint secondch, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc);

	//
	//MediaEvent
	//
    Smt_Uint EvtPlayBegin(Smt_Uint channel);
	Smt_Uint EvtPlayEnd(Smt_Uint channel, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtRecordBegin(Smt_Uint channel);
	Smt_Uint EvtRecordEnd(Smt_Uint channel, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtGetDTMFBegin(Smt_Uint channel);
	Smt_Uint EvtGetDTMFEnd(Smt_Uint channel, Smt_String dtmpbuf, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint EvtRecvFaxBegin(Smt_Uint channel);
	Smt_Uint EvtRecvFaxEnd(Smt_Uint channel, Smt_Uint faxpage, Smt_Uint errcode, Smt_String errdesc, Smt_String faxfile);
	Smt_Uint EvtSendFaxBegin(Smt_Uint channel);
	Smt_Uint EvtSendFaxEnd(Smt_Uint channel, Smt_Uint faxpage, Smt_Uint errcode, Smt_String errdesc);

	//
	//ConfEvent
	//
	Smt_Uint EvtConfCreate();
	Smt_Uint EvtConfAddParty();
	Smt_Uint EvtConfRemParty();
	Smt_Uint EvtConfDelete();
	Smt_Uint EvtConfAttr();
	Smt_Uint EvtConfPartyAttr();

	//Commands
	Smt_Uint OnCmdAnswerCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdMakeCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdHangupCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdTransferCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdModifyCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdHoldCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdRetrieveCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdPlay(Smt_Pdu &pdu);
	Smt_Uint OnCmdRecord(Smt_Pdu &pdu);
	Smt_Uint OnCmdGetDTMF(Smt_Pdu &pdu);
	Smt_Uint OnCmdRecvFax(Smt_Pdu &pdu);
	Smt_Uint OnCmdSendFax(Smt_Pdu &pdu);
	Smt_Uint OnCmdStopMedia(Smt_Pdu &pdu);
	Smt_Uint OnCmdConChannel(Smt_Pdu &pdu);
	Smt_Uint OnCmdDisconChannel(Smt_Pdu &pdue);
	Smt_Uint OnCmdRtpInit(Smt_Pdu &pdu);
    Smt_Uint OnCmdRtpClose(Smt_Pdu &pdu);
	Smt_Uint OnCmdSetSipHead(Smt_Pdu &pdu);
	Smt_Uint OnCmdGetSipHead(Smt_Pdu &pdu);

	//Responds
	Smt_Uint RespAnswerCall(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespMakeCall(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespHangupCall(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespTransferCall(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespModifyCall(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespHoldCall(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespRetrieveCall(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespPlay(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespRecord(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespGetDTMF(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespRecvFax(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespSendFax(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespStopMedia(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespConChannel(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespDisconChannel(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespRtpInit(Smt_Uint channel, Smt_Uint state);
    Smt_Uint RespRtpClose(Smt_Uint channel, Smt_Uint state);
	Smt_Uint RespSetSipHead(Smt_Uint channel, Smt_Uint state, Smt_String sipheadkey, Smt_String sipheadvalue);
	Smt_Uint RespGetSipHead(Smt_Uint channel, Smt_Uint state, Smt_String sipheadkey, Smt_String sipheadvalue);

	//sys
	Smt_Uint DeviceTaskFail(Smt_Uint channel, Smt_Uint cmd, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint ReportDeviceState(Smt_Uint sysState, Smt_Uint maxchannel, Smt_Uint errcode, Smt_String errdesc);
	Smt_Uint CheckUserAndChannel(Smt_Uint channel);
public:
	Smt_Uint m_ChNum;
	Smt_User *m_pUser;
	Smt_Server *m_pServer;
	Smt_Uint m_IsThreadRun;
	Smt_Uint m_ThreadNames[5];
protected:
private:
		
};


#endif
