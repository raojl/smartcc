#include "TBaseDevice.h"

TBaseDevice::TBaseDevice()
{
	m_ChNum = 0;
	m_pUser = NULL;
}

TBaseDevice::~TBaseDevice()
{
	m_ChNum = 0;
	m_pUser = NULL;
}

Smt_Uint TBaseDevice::CheckUserAndChannel(Smt_Uint channel)
{
	if(m_pUser == NULL || channel >= MAXCHANNEL || channel < 1)
	{
		return 0;
	}
	return 1;
}

Smt_Uint TBaseDevice::PutMessageEx(Smt_Pdu pdu)
{
	Smt_TimeValue tmVal;
	tmVal.msec( Smt_QUEUE_WAIT_TIMES );	
	tmVal += ACE_OS::gettimeofday();
	Smt_Pdu* pPdu = new Smt_Pdu( pdu );
	if(putq( pPdu, &tmVal ) == -1)
	{
		if(pPdu != NULL)
		{
			delete pPdu;
			pPdu = NULL;
		}
	}
	return Smt_Success;
}

Smt_Uint TBaseDevice::StartUp(Smt_User *pUser, Smt_Server *pServer, Smt_Pdu pdu)
{
	ACE::init();
	//启动线程
	Run();
	//初始化消息放入消息队列
	pdu.m_MessageID = Evt_IVR_InitDevice;
	pdu.PutUint(Key_IVR_MCSUser,(Smt_Uint)pUser);
	pdu.PutUint(Key_IVR_MCSServer, (Smt_Uint)pServer);
	PutMessageEx(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::Run()
{
	m_IsThreadRun = Smt_BoolTRUE;
	water_marks( 1, 16 * 1024 );	//SET_LWM
	water_marks( 3, 1024 * 1024 );	//SET_HWM
	
	// start thread pool
	return activate(THR_NEW_LWP|THR_JOINABLE, 1, 0, ACE_DEFAULT_THREAD_PRIORITY, -1, this, 0, 0, 0, m_ThreadNames);
}

int TBaseDevice::svc(void)
{
	Smt_Pdu* pPdu;
	int nEventNum = 0;
	while(m_IsThreadRun == Smt_BoolTRUE )	
	{	
		Smt_Uint flag = 0;
		//这里设置的时间会使getq立即返回
		Smt_TimeValue tmVal = ACE_OS::gettimeofday();

 		if (GetDeviceEvent() != 0 )//获取底层设备事件
 			flag++;

		if( getq( pPdu,&tmVal) != -1 )
		{
			try
			{
				if( HandleMessage( *pPdu ) != Smt_Success )
				{
				}
			}
			catch (...)
			{
			}
			
			if(pPdu != NULL)
			{
				delete pPdu;
				pPdu = NULL;
			}

            flag++;
		}
		if(flag == 0)
		{
			HLSleep(10);
		}

		//每处理100个网络/板卡事件 Sleep
		nEventNum++;
		if(nEventNum > 99)
		{
            nEventNum = 0;
			HLSleep(10);
		}
	}	
	return Smt_Success;
}

Smt_Uint TBaseDevice::HandleMessage(Smt_Pdu &pdu)
{
	switch(pdu.m_MessageID)
	{
	//callcontrol interface
	case Cmd_IVR_AnswerCall:
		OnCmdAnswerCall(pdu);
		break;
	case Cmd_IVR_MakeCall:
		OnCmdMakeCall(pdu);
		break;
	case Cmd_IVR_HangupCall:
		OnCmdHangupCall(pdu);
		break;
	case Cmd_IVR_TransferCall:
		OnCmdTransferCall(pdu);
		break;
	case Cmd_IVR_ModifyCall:
		OnCmdModifyCall(pdu);
		break;
	case Cmd_IVR_HoldCall:
		OnCmdHoldCall(pdu);
		break;
	case Cmd_IVR_RetrieveCall:
		OnCmdRetrieveCall(pdu);
		break;

    //mediacontrol interface
	case Cmd_IVR_Play:
		OnCmdPlay(pdu);
		break;
	case Cmd_IVR_Record:
		OnCmdRecord(pdu);
		break;
	case Cmd_IVR_GetDTMF:
		OnCmdGetDTMF(pdu);
		break;
	case Cmd_IVR_RecvFax:
		OnCmdRecvFax(pdu);
		break;
    case Cmd_IVR_SendFax:
		OnCmdSendFax(pdu);
		break;
	case Cmd_IVR_StopMedia:
		OnCmdStopMedia(pdu);
		break;
	case Cmd_IVR_ConChannel:
		OnCmdConChannel(pdu);
		break;
	case Cmd_IVR_DisconChannel:
		OnCmdDisconChannel(pdu);
		break;
	case Cmd_IVR_RtpInit:
		OnCmdRtpInit(pdu);
		break;
	case Cmd_IVR_RtpClose:
		OnCmdRtpClose(pdu);
		break;
	case Cmd_IVR_SetSipHead:
		OnCmdSetSipHead(pdu);
		break;
	case Cmd_IVR_GetSipHead:
		OnCmdGetSipHead(pdu);
		break;
	case Evt_IVR_InitDevice:
		InitDevice(pdu);
		break;

	case Evt_IBaseLib_HandleTimerItem:
		HandleTimerItem(pdu);
		break;

	default:
		break;
	}

	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdAnswerCall(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	Smt_Uint rings;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetUint(Key_IVR_RingTimes, &rings);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	AnswerCall(channel, rings);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdMakeCall(Smt_Pdu &pdu)
{
	Smt_Uint channel;
    Smt_String ani;
	Smt_String dnis;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetString(Key_IVR_ANI, &ani);
	pdu.GetString(Key_IVR_DNIS, &dnis);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	MakeCall(channel, ani, dnis);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdHangupCall(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	HangupCall(channel);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdTransferCall(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	Smt_Uint transfertype;
	Smt_String ani;
	Smt_String dnis;
	Smt_Uint nTransTimeOut=0;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetUint(Key_IVR_TransType, &transfertype);
	pdu.GetString(Key_IVR_TransANI, &ani);
	pdu.GetString(Key_IVR_TransDNIS, &dnis);
	pdu.GetUint(Key_IVR_TransTimeOut, &nTransTimeOut);
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	if (nTransTimeOut == 0)
	{
		nTransTimeOut = 30*1000;
	}
	TransferCall(channel, transfertype, ani, dnis, nTransTimeOut);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdModifyCall(Smt_Pdu &pdu)
{
    Smt_Uint channel;
	Smt_String audioaddr;
	Smt_Uint audioport;
	Smt_Uint audiotype;
	Smt_String videoaddr;
	Smt_Uint videoport;
	Smt_Uint videotype;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetString(Key_IVR_AudioAddress, &audioaddr);
	pdu.GetUint(Key_IVR_AudioPort, &audioport);
	pdu.GetUint(Key_IVR_AudioType, &audiotype);
	pdu.GetString(Key_IVR_VideoAddress, &videoaddr);
	pdu.GetUint(Key_IVR_VideoPort, &videoport);
	pdu.GetUint(Key_IVR_VideoType, &videotype);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	ModifyCall(channel, audioaddr, audioport, audiotype, videoaddr, videoport, videotype);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdHoldCall(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	HoldCall(channel);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdRetrieveCall(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	RetrieveCall(channel);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdPlay(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	Smt_Uint type;
	Smt_String termkey;
	Smt_String content;
	Smt_Kvset* kvTTS = new Smt_Kvset[MAX_TTSMEMARRAYNUM];
	Smt_Uint nTTSCount = 0;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetUint(Key_IVR_ContentType, &type);
	pdu.GetString(Key_IVR_TermKey, &termkey);
	pdu.GetString(Key_IVR_PlayContent, &content);	
	pdu.GetKvsetArray( Key_IVR_TTSMemArray, &kvTTS, &nTTSCount );	
	
	if(!CheckUserAndChannel(channel))
	{
		if(kvTTS != NULL)
		{
			delete [] kvTTS;
			kvTTS = NULL;
		}
		return Smt_Fail;
	}
	
	PlayMedia(channel, type, termkey, content, kvTTS, nTTSCount, pdu);
	
	if(kvTTS !=NULL )
	{
		delete [] kvTTS;
		kvTTS = NULL;
	}
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdRecord(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	Smt_Uint type;
	Smt_String termkey;
	Smt_String content;
	Smt_Uint rectime;
	Smt_Uint silenttime;
	Smt_Uint isdoublerec;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetUint(Key_IVR_RecType, &type);
	pdu.GetString(Key_IVR_TermKey, &termkey);
	pdu.GetString(Key_IVR_RecFile, &content);
	pdu.GetUint(Key_IVR_RecTime, &rectime);
	pdu.GetUint(Key_IVR_SilentTime, &silenttime);
	pdu.GetUint(Key_IVR_RecordDouble, &isdoublerec);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	RecordMedia(channel, type, termkey, content, rectime, silenttime, isdoublerec);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdGetDTMF(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	Smt_Uint maxlen;
	Smt_Uint starttime;
	Smt_Uint innertime;
	Smt_String termkey;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetUint(Key_IVR_MaxLen, &maxlen);
	pdu.GetUint(Key_IVR_StartTime, &starttime);
	pdu.GetUint(Key_IVR_InnerTime, &innertime);
	pdu.GetString(Key_IVR_TermKey, &termkey);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	GetDTMF(channel, maxlen, starttime, innertime, termkey);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdRecvFax(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	Smt_String faxfile;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetString(Key_IVR_FaxName, &faxfile);
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	RecvFax(channel, faxfile);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdSendFax(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	Smt_String faxfile;
	Smt_Uint startpage;
	Smt_Uint pagenum;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetString(Key_IVR_FaxName, &faxfile);
	pdu.GetUint(Key_IVR_FaxStartPage, &startpage);
	pdu.GetUint(Key_IVR_FaxPageNum, &pagenum);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	SendFax(channel, faxfile, startpage, pagenum);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdStopMedia(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	StopMedia(channel);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdConChannel(Smt_Pdu &pdu)
{
	Smt_Uint firstch;
	Smt_Uint secondch;
	Smt_Uint mode;
	pdu.GetUint(Key_IVR_ChannelID, &firstch);
	pdu.GetUint(Key_IVR_ConChannelID, &secondch);
	pdu.GetUint(Key_IVR_ConMode, &mode);

	ConChannel(firstch, secondch, mode);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdDisconChannel(Smt_Pdu &pdu)
{
	Smt_Uint firstch;
	Smt_Uint secondch;
	pdu.GetUint(Key_IVR_ChannelID, &firstch);
	pdu.GetUint(Key_IVR_ConChannelID, &secondch);

	DisconChannel(firstch, secondch);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdRtpInit(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	Smt_String audioaddr;
	Smt_Uint audioport;
	Smt_Uint audiotype;
	Smt_String videoaddr;
	Smt_Uint videoport;
	Smt_Uint videotype;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetString(Key_IVR_AudioAddress, &audioaddr);
	pdu.GetUint(Key_IVR_AudioPort, &audioport);
	pdu.GetUint(Key_IVR_AudioType, &audiotype);
	pdu.GetString(Key_IVR_VideoAddress, &videoaddr);
	pdu.GetUint(Key_IVR_VideoPort, &videoport);
	pdu.GetUint(Key_IVR_VideoType, &videotype);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	RtpInit(channel, audioaddr, audioport, audiotype, videoaddr, videoport, videotype);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdRtpClose(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	RtpClose(channel);
	return Smt_Success;
}

Smt_Uint TBaseDevice::OnCmdSetSipHead(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	Smt_String strKey;
	Smt_String strValue;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetString(Key_IVR_SipHeadKey, &strKey);
	pdu.GetString(Key_IVR_SipHeadValue, &strValue);
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	SetSipHead(channel, strKey, strValue);
	return Smt_Success;
}
Smt_Uint TBaseDevice::OnCmdGetSipHead(Smt_Pdu &pdu)
{
	Smt_Uint channel;
	Smt_String strKey;
	Smt_String strValue;
	pdu.GetUint(Key_IVR_ChannelID, &channel);
	pdu.GetString(Key_IVR_SipHeadKey, &strKey);
	pdu.GetString(Key_IVR_SipHeadValue, &strValue);

	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	GetSipHead(channel, strKey, strValue);
	return Smt_Success;
}
//
Smt_Uint TBaseDevice::RespAnswerCall(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_AnswerCall;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();

	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespMakeCall(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_MakeCall;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);
	return Smt_Success;
}

Smt_Uint TBaseDevice::RespHangupCall(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_HangupCall;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);
	
	return Smt_Success;
}

Smt_Uint TBaseDevice::RespTransferCall(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_TransferCall;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespModifyCall(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_ModifyCall;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespHoldCall(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_HoldCall;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespRetrieveCall(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_RetrieveCall;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespPlay(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_Play;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);


	return Smt_Success;
}

Smt_Uint TBaseDevice::RespRecord(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_Record;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespGetDTMF(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_GetDTMF;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespRecvFax(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_RecvFax;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespSendFax(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_SendFax;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespStopMedia(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_StopMedia;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespConChannel(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_ConChannel;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespDisconChannel(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_DisconChannel;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespRtpInit(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_RtpInit;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespRtpClose(Smt_Uint channel, Smt_Uint state)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_RtpClose;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::RespSetSipHead(Smt_Uint channel, Smt_Uint state, Smt_String sipheadkey, Smt_String sipheadvalue)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_SetSipHead;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_SipHeadKey, sipheadkey);
	pdu.PutString(Key_IVR_SipHeadValue, sipheadvalue);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}
Smt_Uint TBaseDevice::RespGetSipHead(Smt_Uint channel, Smt_Uint state, Smt_String sipheadkey, Smt_String sipheadvalue)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Resp_IVR_GetSipHead;
	pdu.m_Status = state;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_SipHeadKey, sipheadkey);
	pdu.PutString(Key_IVR_SipHeadValue, sipheadvalue);
	m_pUser->PostMessage(pdu);
	return Smt_Success;
}

Smt_Uint TBaseDevice::DeviceTaskFail(Smt_Uint channel, Smt_Uint cmd, Smt_Uint errcode, Smt_String errdesc)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Evt_IVR_TaskFail;
	pdu.m_Status = Smt_Success;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();

	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutUint(Key_IVR_CmdID, cmd);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	m_pUser->PostMessage(pdu);
	
	return Smt_Success;
}

Smt_Uint TBaseDevice::ReportDeviceState(Smt_Uint sysState, Smt_Uint maxchannel, Smt_Uint errcode, Smt_String errdesc)
{
	Smt_Pdu pdu;
	pdu.m_MessageID = Evt_IVR_ServiceReport;
	pdu.m_Status = Smt_Success;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	
	pdu.PutUint(Key_IVR_ServiceState, sysState);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

//
//CallControl
//
Smt_Uint TBaseDevice::AnswerCall(Smt_Uint channel, Smt_Uint rings){return Smt_Fail;}
Smt_Uint TBaseDevice::MakeCall(Smt_Uint channel, Smt_String ani, Smt_String dnis){return Smt_Fail;}
Smt_Uint TBaseDevice::HangupCall(Smt_Uint channel){return Smt_Fail;}
Smt_Uint TBaseDevice::TransferCall(Smt_Uint channel, Smt_Uint transfertype, Smt_String ani, Smt_String dnis, Smt_Uint nTransTimeOut){return Smt_Fail;}
Smt_Uint TBaseDevice::ModifyCall(Smt_Uint channel, Smt_String audioaddr, Smt_Uint audioport, Smt_Uint audiotype, Smt_String videoaddr, Smt_Uint videoport, Smt_Uint videotype){return Smt_Fail;}
Smt_Uint TBaseDevice::HoldCall(Smt_Uint channel){return Smt_Fail;}
Smt_Uint TBaseDevice::RetrieveCall(Smt_Uint channel){return Smt_Fail;}

//  
//MeTdiaControl
//  
Smt_Uint TBaseDevice::PlayMedia(Smt_Uint channel, Smt_Uint type, Smt_String termkey, Smt_String content, Smt_Kvset* pMemArray, Smt_Uint nMemCount, Smt_Pdu pdu){return Smt_Fail;}
Smt_Uint TBaseDevice::RecordMedia(Smt_Uint channel, Smt_Uint type, Smt_String termkey, Smt_String content, Smt_Uint rectime, Smt_Uint slienttime, Smt_Uint recdouble){return Smt_Fail;}
Smt_Uint TBaseDevice::GetDTMF(Smt_Uint channel, Smt_Uint maxlen, Smt_Uint starttime, Smt_Uint innertime, Smt_String termkey){return Smt_Fail;}
Smt_Uint TBaseDevice::RecvFax(Smt_Uint channel, Smt_String filename){return Smt_Fail;}
Smt_Uint TBaseDevice::SendFax(Smt_Uint channel, Smt_String filename, Smt_Uint startpage, Smt_Uint pagenum){return Smt_Fail;}
Smt_Uint TBaseDevice::StopMedia(Smt_Uint channel){return Smt_Fail;}
Smt_Uint TBaseDevice::ConChannel(Smt_Uint firstch, Smt_Uint secondch, Smt_Uint mode){return Smt_Fail;}
Smt_Uint TBaseDevice::DisconChannel(Smt_Uint firstch, Smt_Uint secondch){return Smt_Fail;}
Smt_Uint TBaseDevice::RtpInit(Smt_Uint channel, Smt_String audioaddr, Smt_Uint audioport, Smt_Uint audiotype, Smt_String videoaddr, Smt_Uint videoport, Smt_Uint videotype){return Smt_Fail;}
Smt_Uint TBaseDevice::RtpClose(Smt_Uint channel){return Smt_Fail;}
Smt_Uint TBaseDevice::SetSipHead(Smt_Uint channel, Smt_String sipheadkey, Smt_String sipheadvalue){return Smt_Fail;}
Smt_Uint TBaseDevice::GetSipHead(Smt_Uint channel, Smt_String sipheadkey, Smt_String sipheadvalue){return Smt_Fail;}

//  
//Conference
//  
Smt_Uint TBaseDevice::ConfCreate(){return Smt_Fail;}
Smt_Uint TBaseDevice::ConfAddParty(){return Smt_Fail;}
Smt_Uint TBaseDevice::ConfRemParty(){return Smt_Fail;}
Smt_Uint TBaseDevice::ConfDelete(){return Smt_Fail;}
Smt_Uint TBaseDevice::ConfSetAttr(){return Smt_Fail;}
Smt_Uint TBaseDevice::ConfGetAttr(){return Smt_Fail;}
Smt_Uint TBaseDevice::ConfSetPartyAttr(){return Smt_Fail;}
Smt_Uint TBaseDevice::ConfGetPartyAttr(){return Smt_Fail;}

//  
//CallEvent
//  
Smt_Uint TBaseDevice::EvtUnavailable(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_Unavailable;

	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_CallRefID, callrefid);

	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtIdle(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;

	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_Idle;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_CallRefID, callrefid);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtOffering(Smt_Uint channel, Smt_String callrefid, Smt_String ani, Smt_String dnis, Smt_Uint chtype, Smt_String sessionid)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Evt_IVR_Offering;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutString(Key_IVR_ANI, Smt_String(ani));
	pdu.PutString(Key_IVR_DNIS, Smt_String(dnis));
	pdu.PutUint(Key_IVR_ChannelType, chtype);
	pdu.PutString(Key_IVR_SessionID, Smt_String(sessionid));
	pdu.PutString(Key_IVR_CallRefID, callrefid);

	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtProceeding(Smt_Uint channel, Smt_String callrefid, Smt_String ani, Smt_String dnis, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_Proceeding;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_ANI, Smt_String(ani));
	pdu.PutString(Key_IVR_DNIS, Smt_String(dnis));
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_CallRefID, callrefid);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtAlerting(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_Alerting;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_CallRefID, callrefid);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtConnected(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_Connected;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_CallRefID, callrefid);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtTransfered(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_Transfered;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_CallRefID, callrefid);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtDisconnected(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_Disconnected;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_CallRefID, callrefid);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtRetrieved(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_Retrieved;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_CallRefID, callrefid);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtHeld(Smt_Uint channel, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_Held;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_CallRefID, callrefid);
	pdu.PutString(Key_IVR_CallRefID, callrefid);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtConChannel(Smt_Uint firstch, Smt_Uint secondch, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc)
{
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_ConChannel;

	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, firstch);
	pdu.PutUint(Key_IVR_ConChannelID, secondch);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtDisconChannel(Smt_Uint firstch, Smt_Uint secondch, Smt_String callrefid, Smt_Uint errcode, Smt_String errdesc)
{
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_DisconChannel;

	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, firstch);
	pdu.PutUint(Key_IVR_ConChannelID, secondch);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_CallRefID, callrefid);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}
//  
//MediaEvent
//  
Smt_Uint TBaseDevice::EvtPlayBegin(Smt_Uint channel)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Evt_IVR_PlayBegin;

	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);

	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtPlayEnd(Smt_Uint channel, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_PlayEnd;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtRecordBegin(Smt_Uint channel)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Evt_IVR_RecordBegin;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtRecordEnd(Smt_Uint channel, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_RecordEnd;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtGetDTMFBegin(Smt_Uint channel)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Evt_IVR_GetDTMFBegin;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtGetDTMFEnd(Smt_Uint channel, Smt_String dtmpbuf, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_GetDTMFEnd;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutString(Key_IVR_GotDTMF, Smt_String(dtmpbuf));
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtRecvFaxBegin(Smt_Uint channel)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Evt_IVR_RecvFaxBegin;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtRecvFaxEnd(Smt_Uint channel, Smt_Uint faxpage, Smt_Uint errcode, Smt_String errdesc, Smt_String faxfile)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_RecvFaxEnd;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutUint(Key_IVR_RecvFaxPage, faxpage);
	pdu.PutString(Key_IVR_FaxName, Smt_String(faxfile));
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtSendFaxBegin(Smt_Uint channel)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = Smt_Success;
	pdu.m_MessageID = Evt_IVR_SendFaxBegin;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

Smt_Uint TBaseDevice::EvtSendFaxEnd(Smt_Uint channel, Smt_Uint faxpage, Smt_Uint errcode, Smt_String errdesc)
{
	if(!CheckUserAndChannel(channel))return Smt_Fail;
	
	Smt_Pdu pdu;
	pdu.m_Sender = m_pUser->GetGOR();
	pdu.m_Receiver = m_pUser->GetGOR();
	pdu.m_Status = errcode;
	pdu.m_MessageID = Evt_IVR_SendFaxEnd;
	
	Smt_DateTime dateNow;
	Smt_String strNow = dateNow.FormatString();
	pdu.PutUint(Key_IVR_ChannelID, channel);
	pdu.PutString(Key_IVR_EvtTime, strNow);
	pdu.PutUint(Key_IVR_CauseID, errcode);
	pdu.PutString(Key_IVR_CauseDesc, errdesc);
	pdu.PutUint(Key_IVR_SentFaxPage, faxpage);
	
	m_pUser->PostMessage(pdu);

	return Smt_Success;
}

//  
//ConfEvent
//  
Smt_Uint TBaseDevice::EvtConfCreate()
{return Smt_Success;}
Smt_Uint TBaseDevice::EvtConfAddParty()
{return Smt_Success;}
Smt_Uint TBaseDevice::EvtConfRemParty()
{return Smt_Success;}
Smt_Uint TBaseDevice::EvtConfDelete()
{return Smt_Success;}
Smt_Uint TBaseDevice::EvtConfAttr()
{return Smt_Success;}
Smt_Uint TBaseDevice::EvtConfPartyAttr()
{return Smt_Success;}
