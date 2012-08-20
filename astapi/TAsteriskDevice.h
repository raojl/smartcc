//**************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : IVR5.0   	
//      Create Date : 2009.10.10      	
//      Author      : Linyl    
//      Discription : Diaglic HMP Device Class
//***************************************************************************

#ifndef Smt_ASTERISKDEVICE_HH
#define Smt_ASTERISKDEVICE_HH

#include "TBaseDevice.h"
#include "Smt_XML.h"

#include "cmuapidefs.h"
#include "cmuapi.h"
#ifdef _VC
#pragma comment(lib, "CMUAPI.lib")
#else

#endif

#define     ChannelUnknown  0
#define     ChannelIdle     1
#define     ChannelBusy     2

#define     MakeCallSuccess 1
#define     MakeCallConnect 2

#define     GCDailNULL      0
#define     GCDailInvoke    1
#define     GCDailConnect   2

#define     MAXBUFF         1024

//BOOLEAN
#define BOOLEAN_TRUE           "boolean_true"
#define BOOLEAN_FALSE          "boolean_false"

//数字
#define DIGIT_0           "digit_0"
#define DIGIT_1           "digit_1"
#define DIGIT_2           "digit_2"
#define DIGIT_3           "digit_3"
#define DIGIT_4           "digit_4"
#define DIGIT_5           "digit_5"
#define DIGIT_6           "digit_6"
#define DIGIT_7           "digit_7"
#define DIGIT_8           "digit_8"
#define DIGIT_9           "digit_9"

//单位
#define UNIT_TEN                   "unit_ten"
#define UNIT_HUNDRED               "unit_hundred"
#define UNIT_THOUSAND              "unit_thousand"
#define UNIT_TEN_THOUSAND          "unit_ten_thousand"
#define UNIT_MEGA                  "unit_mega"

//金额
#define MONEY_BUCK                  "money_buck"
#define MONEY_DIME                  "money_dime"
#define MONEY_CENT                  "money_cent"

//符号*,#,-,.
#define SYMBOL_STAR                  "symbol_star"
#define SYMBOL_WELL                  "symbol_well"
#define SYMBOL_MINUS                 "symbol_minus"
#define SYMBOL_DOT                   "symbol_dot"

//日期
#define DATE_YEAR                    "date_year"
#define DATE_MONTH                   "date_month"
#define DATE_DAY                     "date_day"
#define DATE_HOUR                    "date_hour"
#define DATE_MINUTE                  "date_minute"
#define DATE_SECOND                  "date_second"
#define DATE_WEEK                    "date_week"

//PHONE*
#define PHONE_STAR                  "phone_star"


enum COMBOTYPE
{
	    COMBO_UNKNOW = 0,
		COMBO_BOOLEAN ,
		COMBO_DATE ,     
		COMBO_DIGITS,    
		COMBO_CURRENCY,  
		COMBO_NUMBER,    
		COMBO_PHONE ,    
		COMBO_TIME ,     
		COMBO_AUDIOFILE ,
};

enum ResourceType
{
	RES_NULL = 0,
		RES_DXPLAY,
		RES_DXRECDOUBLE,
		RES_DXRECORD,
		RES_DXSENDDTMF,
		RES_DXRECVDTMF,
		RES_MMPLAYAUDIO,
		RES_MMRECORDAUDIO,
		RES_MMPLAYVIDEO,
		RES_MMRECORDVIDEO,
		RES_FXSENDFAX,
		RES_FXRECVFAX,
		RES_DXSENDMSG,
		RES_FXPLAYTTS
};

enum TimerType
{
	TimerType_Repeating      =    1,
	TimerType_Single         =    2
};

enum MatchType
{
   Match_NULL=0,
   Match_Head,
   Match_Tail
};
//每个中继中添加的匹配规则
class TMatchData
{
public:
	Smt_String m_strDeviceID; //被叫号码
	Smt_Uint   m_nInvokeID;//调用句柄
	Smt_Uint   m_nChanID;//返回的句柄
	
	Smt_Uint   m_IsMatch;//是否包含.
	Smt_Uint   m_MatchType;//匹配规则 前部，后部
	Smt_String m_MatchStr;//匹配字符
	Smt_Uint   m_MatchStrLen;//匹配字符长度
public:
	TMatchData();
	~TMatchData();
	Smt_Uint MatchResult(Smt_String strDNIS);
};

//通道数据
class TChannelData
{
public:
	
	Smt_Uint m_ChNo;
	Smt_Uint m_ChStat;
	Smt_String m_ANI;
	Smt_String m_DNIS;
	Smt_TraceLog*	m_pLog;         //日志指针
public:
	TChannelData();
	~TChannelData();
	void PrintLog(Smt_Uint loglevel, const char* fmt, ... );
};

typedef Smt_Map<Smt_Uint, TChannelData*>  ChannelMap;
//中继数据 每一个asterisk设备看做一个中继
class TTrunkData
{
public:
	Smt_Uint m_InvokeID;
	Smt_String m_DeviceID;
	Smt_Uint m_DeviceType;
	Smt_Uint m_ChanID;

	Smt_Uint m_TrunkID;
	Smt_Uint m_TrunkDirect;
	Smt_String m_TrunkType;
	ChannelMap m_SubChInfo;

    Smt_Uint m_CurrentCh;
	TMatchData m_MatchData[128];
	Smt_Uint m_MatchCount;
public:
	TTrunkData();
	~TTrunkData();
};



class DtmfCache
{
public:
	int m_flag;               //是否生效标志
	int m_maxlen;             //最大dtmf长度
	int m_starttime;          //最大时长
public:
	void Reset();
	DtmfCache();
	~DtmfCache();	
};
//呼叫数据 
class TCallData
{
public:
	Smt_Uint m_CRN;
	Smt_Uint m_ChNo;
	Smt_String m_TrunkDeviceID;
	Smt_Uint m_ResState;
	TChannelData *m_pChInfo;
	DtmfCache m_DtmfCache;
	Smt_Uint m_MakeCall;
	Smt_Uint m_IsGcDail;
	Smt_Uint m_TpDisconnect;
	Smt_Uint m_TimerID;

	Smt_String m_SipHeaderKey;
	Smt_String m_SipHeaderValue;

	TCallData();
	~TCallData();
};



typedef Smt_Map<Smt_String, TTrunkData*>  TrunkMap;
typedef Smt_Map<Smt_Uint, TCallData*>  CallMap;

class TAsteriskDevice: public TBaseDevice
{
public:
    TAsteriskDevice();
	~TAsteriskDevice();
	
	virtual Smt_Uint InitDevice(Smt_Pdu pdu);
	virtual Smt_Uint Stop();
	virtual Smt_Uint GetDeviceEvent();
	
	
	
	class TimerItem : public ACE_Event_Handler
	{
	public:
		int m_TimerID;
		int m_TimerType;
		int m_MessageID;
		void *m_pData;
		TAsteriskDevice *pDevice;
		TimerItem()
		{
			m_TimerID = 0;
			m_TimerType = 0;
			m_MessageID = 0;
			m_pData = NULL;
			pDevice = NULL;
		}
		virtual ~TimerItem()
		{
			m_TimerID = 0;
			m_TimerType = 0;
			m_MessageID = 0;
			m_pData = NULL;
			pDevice = NULL;
		}		
		
		int handle_timeout(const ACE_Time_Value &current_time, const void *act = 0 )
		{		
			if(act != NULL)
			{
				TimerItem *pTask = (TimerItem *)act;
				Smt_Pdu pdu;
				pdu.m_MessageID = Evt_IBaseLib_HandleTimerItem;
				pdu.PutUint(Key_IBaseLib_TimerItemHandle, pTask->m_TimerID);
				pTask->pDevice->PutMessageEx(pdu);
			}
			return Smt_Success;
		}
	};

	typedef Smt_Map<Smt_Uint, TimerItem*>  TimerItemMap;

	Smt_Uint SetTimer(Smt_Uint delay, Smt_Uint MessageID, void *pData = NULL)
	{
		TimerItem *pTmerItem = new TimerItem;
		pTmerItem->pDevice = this;
		pTmerItem->m_MessageID = MessageID;
		pTmerItem->m_TimerType = TimerType_Repeating;
		if(pData != NULL)
		  pTmerItem->m_pData = pData;

		Smt_TimeValue tmAbsoluteTime;
		Smt_TimeValue tmDelay;
		
		tmDelay.msec((Smt_Int)delay);
		tmAbsoluteTime = ACE_OS::gettimeofday() + tmDelay;
		pTmerItem->m_TimerID = m_TimeAdapter.schedule (pTmerItem, pTmerItem, tmAbsoluteTime, tmDelay);
		
		m_TimerMap.SetAt(pTmerItem->m_TimerID, pTmerItem);
		
		return pTmerItem->m_TimerID;
	}
	Smt_Uint SetSingleTimer(Smt_Uint delay, Smt_Uint MessageID, void *pData = NULL)
	{
		TimerItem *pTmerItem = new TimerItem;
		pTmerItem->pDevice = this;
		pTmerItem->m_MessageID = MessageID;
		pTmerItem->m_TimerType = TimerType_Single;
		if(pData != NULL)
		  pTmerItem->m_pData = pData;

		Smt_TimeValue tmAbsoluteTime;
		Smt_TimeValue tmDelay;
		
		tmDelay.msec((Smt_Int)delay);
	    tmAbsoluteTime = ACE_OS::gettimeofday() + tmDelay;
		pTmerItem->m_TimerID = m_TimeAdapter.schedule (pTmerItem, pTmerItem, tmAbsoluteTime);

		m_TimerMap.SetAt(pTmerItem->m_TimerID, pTmerItem);



		return pTmerItem->m_TimerID;
	}
	Smt_Uint ClearTimer(Smt_Uint TimerID)
	{
		TimerItem *pTmerItem = NULL;
		if(m_TimerMap.Lookup(TimerID, pTmerItem) == Smt_Success)
		{
			m_TimerMap.Remove(TimerID);
			if(pTmerItem != NULL)
			{
				delete pTmerItem;
				pTmerItem = NULL;
			}
		}
		m_TimeAdapter.cancel(TimerID);

		return Smt_Success;
	}
	void HandleTimerItem(Smt_Pdu &pdu)
	{	
		Smt_Uint TimerID;
		TimerItem *pTimerItem = NULL;
		pdu.GetUint(Key_IBaseLib_TimerItemHandle, &TimerID);
		if(m_TimerMap.Lookup(TimerID, pTimerItem) == Smt_Success)
		{
			//do something
			OnTimer(pTimerItem->m_TimerID, pTimerItem->m_MessageID, pTimerItem->m_pData);
// 			TCallData *pCallData = NULL;
// 			pCallData = (TCallData *) pTmerItem->m_pData;
// 			if (pCallData != NULL)
// 			{
// 				pCallData->m_pChInfo->PrintLog(4, "[OnTimer]");
// 			}
			pTimerItem->pDevice->m_Ch0.PrintLog(5,"[OnTimer] %d", TimerID);
			if(pTimerItem->m_TimerType == TimerType_Single)
			{
				m_TimerMap.Remove(TimerID);
				if(pTimerItem != NULL)
				{
					delete pTimerItem;
					pTimerItem = NULL;
				}
			}
		}
	}

	void OnTimer(Smt_Uint TimerID, Smt_Uint MessageID, void *Data)
	{

		//这里的MessageID传入的CRN,如想传入其他ID,需大于CMU所能产生的CallID。
		TCallData *pCallData = NULL;
		if(FindCallByCRN(MessageID, pCallData) == Smt_Success)
		{
			if(pCallData->m_TimerID == TimerID)
			{
				Smt_String strCallRefID = HLFormatStr("%d", MessageID);
				EvtDisconnected(pCallData->m_ChNo, strCallRefID, CauseTransfered, "Call released.");
				pCallData->m_pChInfo->PrintLog(5, "[TAsteriskDevice::OnTimer] Nomarl tranfer end, Call Released.CRN<%d>.",MessageID);
				
				m_CallMap.Remove(MessageID);
				if(pCallData != NULL)
				{
					delete pCallData;
					pCallData = NULL;
				}
			}
		}
	}

	

public:
	//functions
	Smt_Uint DeleteAllData();
	Smt_Uint ReadCfgFile();
	Smt_Uint FindCallByCRN(Smt_Uint nCRN, TCallData *&pCallData);
	Smt_Uint FindCallByChannel(Smt_Uint channel, TCallData *&pCallData);

	Smt_Uint OpenConfigedDevices();
	Smt_Uint CloseConfigedDevices();
	Smt_Uint BlindTransfer(Smt_Uint channel, Smt_String ANI, Smt_String DNIS);
	Smt_Uint NormalTransfer(Smt_Uint channel, Smt_String ANI, Smt_String DNIS, Smt_Uint nTransTimeOut);
	Smt_Uint AkPlayFile(Smt_Uint channel, Smt_String termkey, Smt_String content);
	Smt_Uint AkPlayTTS(Smt_Uint channel, Smt_String termkey, Smt_String content);
	Smt_Uint AkPlayDTMF(Smt_Uint channel, Smt_String content);
	Smt_Uint AkRecFile(Smt_Uint channel, Smt_String termkey, Smt_String content, Smt_Uint rectime, Smt_Uint slienttime);
	Smt_Uint AkGetDTMF(Smt_Uint channel, Smt_Uint maxlen, Smt_Uint starttime);
	Smt_Uint AkPlayMessage(Smt_Uint channel, Smt_String content);

	Smt_Uint GetFilePlayType(Smt_String strType);
	Smt_String ConvertPosToUnitName(Smt_Uint pos);
	Smt_String ConvertDigitToFileName(Smt_Uint i);
	bool FillDateIott(Smt_Uint channel, Smt_String datestr);
	bool FillMoneyIott(Smt_Uint channel, Smt_String strMoney);
	bool FillNumberIott(Smt_Uint channel, Smt_String strNumber);
	bool FillIntegerIott(Smt_Uint channel, Smt_String strInteger, bool bZero = false);
	bool FillIntegerIott_Ex(Smt_Uint channel, Smt_String strInteger);
	bool FillFileIott(Smt_Uint channel, Smt_String strFileName, Smt_Uint nFileLibType);
	bool FillDigitIott(Smt_Uint channel, Smt_String strDigit);
	bool FillPlayStringIott(Smt_Uint channel, Smt_Uint FileType, Smt_String FileName);
    bool FillPhoneIott(Smt_Uint channel, Smt_String strNumber);
	bool FillTimeIott(Smt_Uint channel, Smt_String strTime);
	bool FillBoolIott(Smt_Uint channel, Smt_String strBool);
	Smt_Uint GetFileName(Smt_String &strfile, Smt_String strcontent, Smt_String strpath);
	//event process
	Smt_Uint ProcDeviceEvent        (Csta_Event &event);
	Smt_Uint OnCstaConfEvent        (Csta_Event &event);	
	Smt_Uint OnCstaCallEvent        (Csta_Event &event);		
    Smt_Uint OnCstaDeviceEvent      (Csta_Event &event);		
	Smt_Uint OnCstaRouteEvent       (Csta_Event &event);	
	Smt_Uint OnCstaMediaEvent       (Csta_Event &event);
	Smt_Uint OnCstaDeviceRecordEvent(Csta_Event &event);
	Smt_Uint OnCstaSystemEvent      (Csta_Event &event);

    //sys event
	Smt_Uint OnEvtLinkUp            (Csta_Event &event);
	Smt_Uint OnEvtLinkDown          (Csta_Event &event);

	//conf event
	Smt_Uint OnEvtConfSubscribe     (Csta_Event &event);
	Smt_Uint OnEvtConfGcOpen        (Csta_Event &event);
	Smt_Uint OnEvtConfGcMakeCall    (Csta_Event &event);
	Smt_Uint OnEvtConfGcAnswerCall  (Csta_Event &event);
	Smt_Uint OnEvtConfGcHangupCall  (Csta_Event &event);
	Smt_Uint OnEvtConfPlayIottData  (Csta_Event &event);
	Smt_Uint OnEvtConfGetDig        (Csta_Event &event);
	Smt_Uint OnEvtConfDial          (Csta_Event &event);
    Smt_Uint OnEvtConfRec           (Csta_Event &event);
	Smt_Uint OnEvtConfBlindTransfer (Csta_Event &event);
	Smt_Uint OnEvtConfGcDial        (Csta_Event &event);
	Smt_Uint OnEvtConfSendMessage   (Csta_Event &event);
	Smt_Uint OnEvtConfSetSipHeader  (Csta_Event &event);
	Smt_Uint OnEvtConfGetSipHeader  (Csta_Event &event);


	//call event
	Smt_Uint OnEvtCallInitiated     (Csta_Event &event);
	Smt_Uint OnEvtCallDelivered     (Csta_Event &event);
	Smt_Uint OnEvtCallConnected     (Csta_Event &event);
	Smt_Uint OnEvtCallHeld          (Csta_Event &event);
	Smt_Uint OnEvtCallRetrieved     (Csta_Event &event);
	Smt_Uint OnEvtCallConfrenced    (Csta_Event &event);
	Smt_Uint OnEvtCallQueued        (Csta_Event &event);
	Smt_Uint OnEvtCallTransfered    (Csta_Event &event);
	Smt_Uint OnEvtCallCleared       (Csta_Event &event);
	Smt_Uint OnEvtCallFailed        (Csta_Event &event);
	
    //media event
	Smt_Uint OnEvtMediaPlaying      (Csta_Event &event);
	Smt_Uint OnEvtMediaPlayEnd      (Csta_Event &event);
	Smt_Uint OnEvtMediaSending      (Csta_Event &event);
	Smt_Uint OnEvtMediaSendEnd      (Csta_Event &event);
	Smt_Uint OnEvtMediaGeting       (Csta_Event &event);
	Smt_Uint OnEvtMediaGetEnd       (Csta_Event &event);
	Smt_Uint OnEvtMediaRecording    (Csta_Event &event);
	Smt_Uint OnEvtMediaRecordEnd    (Csta_Event &event);
	Smt_Uint OnEvtMediaMessageSendEnd   (Csta_Event &event);

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

public:
	Smt_Uint m_Init;             //是否获取配置
	Smt_Uint m_LinkState;        //link状态
	Smt_Uint m_EventState;       //订阅事件状态

	TrunkMap m_TrunkMap;           //中继容器
	CallMap m_CallMap;             //呼叫数据容器

	TChannelData m_Ch0;            //打印日志用
	Smt_Uint   m_LogLv;          //日志级别
	Smt_String m_strCurrPath;    //当前程序路径
	Smt_String m_DefaultWavPath; //默认语音文件位置
	Smt_String m_WavLibPath;     //语音库文件位置
	Smt_String m_SendFaxPath;    //发送传真默认位置
	Smt_String m_RecvFaxPath;    //接收传真默认位置
	Smt_String m_RecordPath;     //录音文件默认位置
	int m_nDotLen;                      //播放Number类型的组合语音时，播放的小数点后面N位。
	Smt_String m_gSipHeaderKey;  //MCS的配置文件中SipHeaderKey

	Smt_TimerAdapter m_TimeAdapter;
	TimerItemMap m_TimerMap;
};

#endif
