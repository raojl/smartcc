//***************************************************************************
//      Copyright (C) 2009 SMART Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CMU1.0
//      Create Date : 2009.11.26												
//      Author      : shenzj
//      Discription : CMUDLL Macro and Event Struct Defines
//      Modify By   :
//***************************************************************************

#ifndef H_SMART_CMUAPIDEFS_H
#define H_SMART_CMUAPIDEFS_H

/************************************************************************/
/* macro type define													                                */
/************************************************************************/
#define CMU_STRING_LENGTH               32
#define CMU_LONG_STRING_LENGTH          256

typedef unsigned long CMU_Uint;
typedef char          CMU_String[CMU_STRING_LENGTH];
typedef char          CMU_LString[CMU_LONG_STRING_LENGTH];

/************************************************************************/
/* enum define    				                                        */
/************************************************************************/ 
enum Csta_Event_Class
{
	// Csta Event Class
	CSTA_CONFEVENT                  = 1,
	CSTA_CALLEVENT                  = 2,
	CSTA_DEVICEEVENT                = 3,
	CSTA_ROUTEEVENT                 = 4,
	CSTA_MEDIAEVENT                 = 5,
	CSTA_DEVICERECORDEVENT          = 6,
	CSTA_DEVICEMESSAGEEVENT         = 7,
	CSTA_SYSTEMEVENT                = 8,
};

enum Csta_Event_Type
{                       
	// Csta_ConfEvent For CTI               
	CONF_CTCASSIGN                  = 1 ,
	CONF_CTCDEASSIGN                = 2 ,
	CONF_CTCMAKECALL                = 3 ,
	CONF_CTCHANGUPCALL              = 4 ,
	CONF_CTCHOLDCALL                = 5 ,
	CONF_CTCRETRIEVECALL            = 6 ,
	CONF_CTCCONSULTATIONCALL        = 7 ,
	CONF_CTCCONFERENCECALL          = 8 ,
	CONF_CTCTRANSFERCALL            = 9 ,
	CONF_CTCRECONNECTCALL           = 10,
	CONF_CTCSINGLESTEPTRANSFER      = 11,
	CONF_CTCSINGLESTEPCONFERENCE    = 12,
	CONF_CTCROUTESELECTED           = 13,
	CONF_CTCSENDDTMF                = 14,
	CONF_CTCSTARTRECORD             = 15,
	CONF_CTCSTOPRECORD              = 16,
	CONF_CTCSETDATAVALUE            = 17,
	CONF_CTCGETDATAVALUE            = 18,
	CONF_CTCSNAPSHOT                = 19,
	CONF_CTCSENDMESSAGE             = 20,
	
	// Csta_ConfEvent For IVR     
	CONF_GC_OPEN                    = 31,
	CONF_GC_CLOSE                   = 32,
	CONF_GC_MAKECALL                = 33,
	CONF_GC_ANSWERCALL              = 34,
	CONF_GC_HANGUPCALL              = 35,
	CONF_GC_BLINDTRANSFER           = 36,
	CONF_GC_DIAL                    = 37,
	CONF_DX_PLAY                    = 38,
	CONF_DX_ADDIOTTDATA             = 39,
	CONF_DX_PLAYIOTTDATA            = 40,
	CONF_DX_GETDIG                  = 41,
	CONF_DX_DIAL                    = 42,
	CONF_DX_REC                     = 43,
	CONF_DX_SENDMESSAGE             = 44,
	CONF_FX_SENDFAX                 = 45,
	CONF_FX_RCVFAX                  = 46,
	CONF_SUBSCRIBECALLEVENT         = 47,
	CONF_UNSUBSCRIBECALLEVENT       = 48,
	CONF_DX_SETSIPHEADER            = 49,
	CONF_DX_GETSIPHEADER            = 50,

	// CSTA_CALLEVENT                
	CSTA_CALLINITIATED              = 61,
	CSTA_CALLDELIVERED              = 62,
	CSTA_CALLCONNECTED              = 63,
	CSTA_CALLHELD                   = 64,
	CSTA_CALLRETRIEVED              = 65,
	CSTA_CALLCONFERENCED            = 66,
	CSTA_CALLQUEUED                 = 67,
	CSTA_CALLTRANSFERRED            = 68,
	CSTA_CALLCLEARED                = 69,
	CSTA_CALLFAILED                 = 70,

	// CSTA_DEVICEEVENT              
	CSTA_DEVICE_BACKINSERVICE       = 81 ,
	CSTA_DEVICE_OUTOFSERVICE        = 82 ,
	CSTA_DEVICE_OFFHOOK             = 83 ,
	CSTA_DEVICE_INBOUNDCALL         = 84 ,
	CSTA_DEVICE_DESTSEIZED          = 85 ,
	CSTA_DEVICE_TPANSWERED          = 86 ,
	CSTA_DEVICE_OPANSWERED          = 87 ,
	CSTA_DEVICE_TPSUSPENDED         = 88 ,
	CSTA_DEVICE_OPHELD              = 89 ,
	CSTA_DEVICE_TPRETRIEVED         = 90 ,
	CSTA_DEVICE_OPRETRIEVED         = 91 ,
	CSTA_DEVICE_TPDISCONNECTED      = 92 ,
	CSTA_DEVICE_OPDISCONNECTED      = 93 ,
	CSTA_DEVICE_TPTRANSFERRED       = 94 ,
	CSTA_DEVICE_OPTRANSFERRED       = 95 ,
	CSTA_DEVICE_TPCONFERENCED       = 96 ,
	CSTA_DEVICE_OPCONFERENCED       = 97 ,
	CSTA_DEVICE_DESTBUSY            = 98 ,
	CSTA_DEVICE_DESTINVALID         = 99 ,
	CSTA_DEVICE_ROUTEREQUEST        = 100,
	CSTA_DEVICE_ROUTEEND            = 101,
	CSTA_DEVICE_QUEUED              = 102,

	// CSTA_MEDIAEVENT              
	CSTA_MEDIA_PLAYING              = 111,
	CSTA_MEDIA_PLAYEND              = 112,
	CSTA_MEDIA_SENDING              = 113,
	CSTA_MEDIA_SENDEND              = 114,
	CSTA_MEDIA_GETING               = 115,
	CSTA_MEDIA_GETEND               = 116,
	CSTA_MEDIA_RECORDING            = 117,
	CSTA_MEDIA_RECORDEND            = 118,
	CSTA_MEDIA_MESSAGESENDING       = 119,
	CSTA_MEDIA_MESSAGESENDEND       = 120,

	// CSTA_DEVICERECORDEVENT              
	CSTA_DEVICE_RECORDING           = 131,
	CSTA_DEVICE_RECORDEND           = 132,
	CSTA_DEVICE_MESSAGE             = 133,

	// CSTA_SYSTEMEVENT              
	CSTA_LINKDOWN                   = 141,
	CSTA_LINKUP                     = 142, 
	
	//CSTA_TTEVENT
	CSTA_TTSPLAYING					= 143,
	CSTA_TTSPLAYEND					= 144
};

enum CMUDLL_CallState
{
	cmuK_CALL_UNKNOWNSTATE          = 0,
	cmuK_CALL_NULL                  = 1,
	cmuK_CALL_INITIATED             = 2,
	cmuK_CALL_DELIVERED             = 3,
	cmuK_CALL_QUEUED                = 4,
	cmuK_CALL_CONNECTED             = 5,
	cmuK_CALL_HELD                  = 6,
	cmuK_CALL_CONFERENCED           = 7,
	cmuK_CALL_CONFERENCED_HELD      = 8,
	cmuK_CALL_FAILED                = 9,
	cmuK_CALL_SSTRANSFERRING        = 10,
	cmuK_CALL_PRETRANSFER           = 11,
	cmuK_CALL_PREDELIVERED          = 12,
};

enum CMUDLL_DeviceStatus
{
	cmuK_UnknownState               = 0,
	cmuK_ActiveState                = 2,
	cmuK_SilentState                = 3,
	cmuK_DeliverState               = 4,
	cmuK_FailState                  = 8,
	cmuK_HoldState                  = 16,
	cmuK_InitiateState              = 32,
	cmuK_QueueState                 = 64,
	cmuK_NullState                  = 128,
	cmuK_ReceiveState               = 256,
	cmuK_UnavailableState           = 512,
};                         

enum CMUDLL_DeviceType
{
	cmuK_DTYPE_EXTENSION            = 1,
	cmuK_DTYPE_ROUTE                = 2,
	cmuK_DTYPE_IVR_INBOUND          = 3,
	cmuK_DTYPE_IVR_OUTBOUND         = 4,	  
	cmuK_DTYPE_TRUNK_SIP            = 5,  
	cmuK_DTYPE_TRUNK_DAHDI          = 6,   
};

enum CMUDLL_JoinMode
{
	cmuK_JOINMODE_ACTIVE            = 1,
	cmuK_JOINMODE_SILENT            = 2,
};                              

enum CMUDLL_ReasonTypes
{
	cmuK_NotKnown                       = 0 ,
	cmuK_Success                        = 1 ,
	cmuK_Fail                           = 2 ,
	cmuK_InvChannelID                   = 3 ,
	cmuK_InvCallID                      = 4 ,
	cmuK_InvDeviceID                    = 5 ,
	cmuK_InvRouteID                     = 6 ,
	cmuK_Timeout                        = 7 ,
	cmuK_RouteRequest                   = 8 ,
	cmuK_RouteEnd                       = 9 ,
	cmuK_DestBusy                       = 10,
	cmuK_DestInvalid                    = 11,
	cmuK_NoActiveCall                   = 12,
	cmuK_Initiated                      = 13,
	cmuK_NormalReleased                 = 14,
	cmuK_SingleStepTransferred          = 15,
	cmuK_SingleStepConferenced          = 16,
	cmuK_Retrieved                      = 17,
	cmuK_Released                       = 18,
	cmuK_Registered                     = 19,
	cmuK_Unregistered                   = 20,
	cmuK_WriteFileFail                  = 21,
	cmuK_ReadFileFail                   = 22,
	cmuK_ReceiveDTMF                    = 23,
	cmuK_Listening                      = 24,
	cmuK_NoAnswer                       = 25,
	cmuK_Recording                      = 26,
	cmuK_RecordEnd                      = 27,
	cmuK_Originated                     = 28,
	cmuK_Alerting                       = 29,
	cmuK_Connected                      = 30,
	cmuK_CallRejected                   = 31,
	cmuK_PBXNetWorkError                = 32,
	cmuK_DLLNetWorkError                = 33,
	cmuK_HoldRequest                    = 34,
	cmuK_Meetme                         = 35,
	cmuK_AtMeetme                       = 36,
	cmuK_NoActiveConnID                 = 37,
	cmuK_DestSeized                     = 38,
	cmuK_InboundCall                    = 39,
	cmuK_OpAnswered                     = 40,
	cmuK_TpAnswered                     = 41,
	cmuK_OpDisconnected                 = 42,
	cmuK_TpDisconnected                 = 43,
	cmuK_TpHeld                         = 44,
	cmuK_OpHeld                         = 45,
	cmuK_TpConsulted                    = 46,
	cmuK_OpConsulted                    = 47,
	cmuK_TpConferenced                  = 48,
	cmuK_OpConferenced                  = 49,
	cmuK_TpRetrieved                    = 50,
	cmuK_OpRetrieved                    = 51,
	cmuK_TpReConnected                  = 52,
	cmuK_OpReConnected                  = 53,
	cmuK_TpTransferred                  = 54,
	cmuK_OpTransferred                  = 55,
	cmuK_InvDeviceState                 = 56,
	cmuK_MeetLeave                      = 57,
	cmuK_AtMeetLeave                    = 58,
	cmuK_OneActiveConnID                = 59,
	cmuK_IVRRequest                     = 60,
	cmuK_SingleStepTransferredEx        = 61,   
	cmuK_AGISuccess                     = 62,
	cmuK_AGIFail                        = 63, 
	cmuK_Dial                           = 64,
	cmuK_ConsultOpAnswered              = 65,
	cmuK_ConsultTpAnswered              = 66,
};                             
                               
/************************************************************************/
/* event struct define					 								                        */
/************************************************************************/ 
                               
// Csta_ConfEvent              
typedef struct Conf_ctcAssign  
{                              
	CMU_Uint    InvokeID;        
	CMU_Uint    ChannelID;       
	CMU_String  DeviceID;        
	CMU_Uint    DeviceStatus;    
	CMU_String  DeviceIP;        
	CMU_Uint    DevicePort;      
	CMU_Uint    Reason;          
} Conf_ctcAssign;              
                               
typedef struct Conf_ctcResponse
{                              
	CMU_Uint    ChannelID;       
	CMU_Uint    Reason;          
	CMU_Uint    CallID;          
	CMU_Uint    RouteID;         
} Conf_ctcResponse;            
                        
typedef struct Conf_ctcGetDataValue
{                              
	CMU_Uint    ChannelID;       
	CMU_Uint    Reason;          
	CMU_LString DataValue;       
} Conf_ctcGetDataValue;        
                               
typedef struct Conf_SnapshotInfo {
	CMU_Uint    CallID;          
	CMU_Uint    CallState;       
} Conf_SnapshotInfo;           
                               
#define CMU_MAXCALL_ITEMS               16
typedef struct Conf_SnapshotData {
	CMU_Uint    Count;           
	Conf_SnapshotInfo Info[CMU_MAXCALL_ITEMS];
} Conf_SnapshotData;           
                               
typedef struct Conf_ctcSnapshot
{
	CMU_Uint    ChannelID;
	CMU_Uint    Reason;
	Conf_SnapshotData SnapshotData;
} Conf_ctcSnapshot;

typedef struct Conf_gc_open
{
	CMU_Uint    InvokeID;
	CMU_Uint    ChannelID;
	CMU_Uint    Reason;
} Conf_gc_open;

typedef struct Conf_gc_Response
{
	CMU_Uint    CallID;
	CMU_Uint    Reason;
} Conf_gc_Response;

typedef struct Conf_gc_makecall
{
	CMU_Uint    InvokeID;                /* 对应流程对象ID */
	CMU_Uint    CallID;
	CMU_Uint    Reason;
} Conf_gc_makecall;

typedef struct Conf_dx_getsipheader
{
	CMU_Uint    CallID;
	CMU_Uint    Reason;
	CMU_LString  HeaderKey;
	CMU_LString  HeaderValue;
} Conf_dx_getsipheader;

typedef struct Csta_ConfEvent
{
	union 
	{
		Conf_ctcAssign               confAssign;
		Conf_ctcGetDataValue         confGetDataValue;
		Conf_ctcSnapshot             confSnapshot;
		Conf_ctcResponse             confctcResponse;
		Conf_gc_open                 confgc_open;
		Conf_gc_makecall             confgc_makecall;		
		Conf_dx_getsipheader         confdx_getsipheader;
		Conf_gc_Response             confgcResponse;
	} u;
} Csta_ConfEvent;

// Csta_CallEvent
typedef struct Csta_CallEvent
{
	CMU_Uint    CallID;
	CMU_Uint    CallState;
	CMU_String  CallingParty;
	CMU_String  CalledParty;
	CMU_String  InitiatedParty;
	CMU_String  AnsweringParty;
	CMU_String  AlertingParty;
	CMU_String  HoldingParty;
	CMU_String  RetrievingParty;
	CMU_String  ConsultingParty;
	CMU_String  TransferringParty;
	CMU_String  ConferencingParty;
	CMU_Uint    Reason;
	CMU_String  TimeStamp;
} Csta_CallEvent;

// Csta_DeviceEvent
typedef struct Csta_DeviceEvent
{
	CMU_Uint    ChannelID;
	CMU_String  MonitorParty;	
	CMU_Uint    DeviceType;
	CMU_Uint    DeviceState;
	CMU_Uint    CallID;
	CMU_Uint    OldCallID;
	CMU_Uint    SecOldCallID;
	CMU_String  OtherParty;
	CMU_String  CallingParty;
	CMU_String  CalledParty;
	CMU_String  ThirdParty;
	CMU_Uint    Reason;
	CMU_String  TimeStamp;
} Csta_DeviceEvent;

// Csta_RouteEvent
typedef struct Csta_RouteEvent
{
	CMU_Uint    ChannelID;
	CMU_String  DeviceID;
	CMU_Uint    RouteID;
	CMU_Uint    CallID;
	CMU_Uint    OldCallID;
	CMU_String  OtherParty;
	CMU_String  CallingParty;
	CMU_String  CalledParty;
	CMU_Uint    Reason;
	CMU_String  TimeStamp;
} Csta_RouteEvent;

// Csta_DeviceRecordEvent
typedef struct Csta_DeviceRecordEvent
{
	CMU_Uint    ChannelID;
	CMU_Uint    CallID;
	CMU_String  OtherParty;
	CMU_String  CallingParty;
	CMU_String  CalledParty;
	CMU_LString FileName;
	CMU_Uint    TimeLen;
	CMU_Uint    Reason;
	CMU_String  TimeStamp;
} Csta_DeviceRecordEvent;

// Csta_DeviceMessageEvent
typedef struct Csta_DeviceMessageEvent
{
	CMU_Uint    ChannelID;
	CMU_String  MonitorParty;
	CMU_Uint    CallID;
	CMU_LString Message;
	CMU_Uint    Reason;
	CMU_String  TimeStamp;
} Csta_DeviceMessageEvent;

// Csta_MediaEvent
typedef struct Csta_MediaEvent
{
	CMU_Uint    CallID;
	CMU_String  CallingParty;      
	CMU_String  CalledParty;    
	CMU_LString FileName;
	CMU_LString DTMFDigits;
	CMU_LString Message;           /* 仅输出消息的前 256 个字节 */
	CMU_Uint    TimeLen;
	CMU_Uint    Reason;
	CMU_String  TimeStamp;
} Csta_MediaEvent;

// Csta_SystemEvent
typedef struct Csta_LinkDown
{
	CMU_Uint    Reason;
	CMU_String  TimeStamp;
} Csta_LinkDown;

typedef struct Csta_LinkUp 
{
	CMU_Uint    Reason;
	CMU_String  TimeStamp;
} Csta_LinkUp;

typedef struct Csta_SystemEvent
{
	union 
	{
		Csta_LinkDown           evtLinkDown;
		Csta_LinkUp             evtLinkUp;
	} u;
} Csta_SystemEvent;


// Csta_Event
#define CSTA_MAX_HEAP	1024
typedef struct Csta_Event
{
	CMU_Uint  evtclass;
	CMU_Uint  evttype;
	union 
	{
		Csta_ConfEvent               evtconf;
		Csta_DeviceEvent             evtdevice;
		Csta_RouteEvent	             evtroute;
		Csta_CallEvent               evtcall;
		Csta_MediaEvent              evtmedia;
		Csta_DeviceRecordEvent       evtdevicerecord;
		Csta_DeviceMessageEvent      evtdevicemessage;
		Csta_SystemEvent             evtsystem;
	} u;
	char	heap[CSTA_MAX_HEAP];
} Csta_Event;

#endif // H_HOLLY_CMUAPIDEFS_H
