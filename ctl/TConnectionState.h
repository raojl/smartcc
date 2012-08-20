//***************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CMU
//      Create Date : 2009.10.21												
//      Author      : Shenzj								
//      Discription : Connection ×´Ì¬»úÀà
//      Modify By   :											
//***************************************************************************
#ifndef Smt_TCONNECTIONSTATE_HH
#define Smt_TCONNECTIONSTATE_HH

#include "CMUInterface.h"
#include "TConnection.h"
#include "Smt_XML.h"

/////////////////////////////////////////////////////////////////////////////
// TConnectionState code
class TConnectionState: public Smt_StateService
{
public:
	class TAssignedDevice;

public:
	TConnectionState( Smt_String name,
				 Smt_Uint loglevel, Smt_String logpath,
				 Smt_Server* pserver);

	~TConnectionState();

public: // virtual method
	Smt_Uint OnUserOnline(Smt_Uint sender, Smt_String sendername);
	Smt_Uint OnUserOffline(Smt_Uint sender, Smt_String sendername);
	Smt_Uint OnTimer(Smt_Uint& timerid, Smt_Uint& messageid, Smt_Uint& senderobj);
	Smt_Uint InitStates();
	Smt_StateObject * PrehandleMessage(Smt_Pdu &pdu);
	
public: 
	// Event handle method
	TConnection* OnEvtNewchannel(Smt_Pdu &pdu);
	TConnection* OnEvtNewcallerid(Smt_Pdu &pdu);        
	TConnection* OnEvtDial(Smt_Pdu &pdu);               
	TConnection* OnEvtLink(Smt_Pdu &pdu);               
	TConnection* OnEvtUnlink(Smt_Pdu &pdu);             
	TConnection* OnEvtHangup(Smt_Pdu &pdu);             
	TConnection* OnEvtHold(Smt_Pdu &pdu);             
	TConnection* OnEvtUnhold(Smt_Pdu &pdu);      
	TConnection* OnEvtUserEvent(Smt_Pdu &pdu);       
	Smt_Uint OnEvtAMIResponse(Smt_Pdu &pdu);
	Smt_Uint OnEvtPeerStatus(Smt_Pdu &pdu);
	Smt_Uint OnEvtActionTimer(Smt_Uint senderobj);
	//Smt_Uint OnEvtAGIRequest(Smt_Pdu &pdu);
	TConnection* OnEvtAGIRequest(Smt_Pdu &pdu);
	TConnection* OnEvtAGIResponse(Smt_Pdu &pdu);
	Smt_Uint OnEvtAGIEvtTimer(Smt_Uint senderobj);
	TConnection* OnEvtOriginateResponse(Smt_Pdu &pdu);
	Smt_Uint OnEvtLinkDown(Smt_Pdu &pdu);
	Smt_Uint EvtLinkDown(Smt_Uint reason);
	Smt_Uint EvtLinkUp(Smt_Uint reason);
	Smt_Uint OnEvtDTMFReceived(Smt_Pdu &pdu);
	Smt_Uint OnEvtMessageReceived(Smt_Pdu &pdu);
	TConnection* OnEvtMeetmeJoin(Smt_Pdu &pdu);
	TConnection* OnEvtMeetmeLeave(Smt_Pdu &pdu);
	Smt_Uint OnEvtAGIHandleOffline(Smt_Pdu &pdu);//add by caoyj 20120315
	Smt_Uint OnEvtNewexten(Smt_Pdu &pdu);//add by caoyj 20120315
	Smt_Uint OnEvtUserEventTimer(Smt_Uint timerid,Smt_Uint senderobj);//add by caoyj 20120315
	TConnection* OnEvtUserEventTimerExpired(Smt_Pdu &pdu);//add by caoyj 20120315

	// Command handle method
	Smt_Uint OnCmdAssign(Smt_Pdu &pdu);
	Smt_Uint OnCmdMakeCall(Smt_Pdu &pdu);           
	Smt_Uint OnCmdHangupCall(Smt_Pdu &pdu);
	Smt_Uint OnCmdSingleStepTransfer(Smt_Pdu &pdu); 
	Smt_Uint OnCmdSingleStepConference(Smt_Pdu &pdu);
	Smt_Uint OnCmdRouteSelected(Smt_Pdu &pdu); 
	Smt_Uint OnCmdSendDTMF(Smt_Pdu &pdu);
	Smt_Uint OnCmdStartRecord(Smt_Pdu &pdu);
	Smt_Uint OnCmdStopRecord(Smt_Pdu &pdu);
	Smt_Uint OnCmdAnswerCallEx(Smt_Pdu &pdu);
	Smt_Uint OnCmdHangupCallEx(Smt_Pdu &pdu);
	Smt_Uint OnCmdPlayFile(Smt_Pdu &pdu);
	Smt_Uint OnCmdPlayFileList(Smt_Pdu &pdu);
	Smt_Uint OnCmdGetDigits(Smt_Pdu &pdu);
	Smt_Uint OnCmdRecordEx(Smt_Pdu &pdu);
	Smt_Uint OnCmdSendDTMFEx(Smt_Pdu &pdu);
	Smt_Uint OnCmdExecAMI(Smt_Pdu &pdu);
	Smt_Uint OnCmdExecAGI(Smt_Pdu &pdu);
	Smt_Uint OnCmdSendMessage(Smt_Pdu &pdu);
	Smt_Uint OnCmdSendMessageEx(Smt_Pdu &pdu);
	Smt_Uint OnCmdSetSIPHeader(Smt_Pdu &pdu);
	Smt_Uint OnCmdGetSIPHeader(Smt_Pdu &pdu);

	Smt_Uint RespAssign(Smt_Pdu &newpdu, Smt_Pdu &oripdu);
	Smt_Uint Resp_AmiCmd(Smt_Pdu &newpdu, Smt_Pdu &oripdu);
	Smt_Uint RespStartRecord(Smt_Pdu &newpdu, Smt_Pdu &oripdu);
	Smt_Uint RespStopRecord(Smt_Pdu &newpdu, Smt_Pdu &oripdu);
	Smt_Uint Resp_AgiCmd(Smt_Uint nresult, Smt_Pdu &oripdu);
	Smt_String GetIDName(Smt_Uint nid);
	Smt_Uint ReleaseConnection( TConnection* pconn );
	Smt_Uint OnCmdDial(Smt_Pdu &pdu);
	Smt_Uint OnEvtTrunkDnLockedTimer(Smt_Uint senderobj);
	Smt_Uint OnRespGetVariable(Smt_Pdu &newpdu, Smt_Pdu &oripdu);
	Smt_Uint OnEvtGetVarTimer(Smt_Uint senderobj);

	// add dialplan
	Smt_Uint Dialplan_AddRouteSelected();
	Smt_Uint Dialplan_AddDevice(TAssignedDevice* pdevice);
	Smt_Uint ReadXMLConfig();

public:
	TConnectionMap m_ConnectionMgr;
	Smt_Uint   m_AmiHandle;
	Smt_Uint   m_CallStateGOR;
	Smt_Uint   m_DeviceStateGOR;

public:
	Smt_Lock   m_Lock;
	Smt_Uint   m_UniqueIndex;	
	Smt_Uint   m_ActionIndex;	

	Smt_String GenConnectionID();
	Smt_String GenActionID(Smt_Pdu &pdu);
	TConnection* LookupByChannel(Smt_String channel);
	TConnection* LookupByDeadChannel(Smt_String channel);
	TConnection* LookupByHandle(Smt_Uint handle);
	TConnection* LookupByDeviceID(Smt_String deviceid);
	Smt_Uint   GetDeviceID( Smt_String channel, Smt_String& deviceid, Smt_Uint& devicetype, Smt_String& taccode);
	Smt_String ASCIICodeToDTMF(Smt_String code);
	Smt_Uint   ClearMapInfo();
	Smt_String ParseCalledNum(Smt_String callednum);
	Smt_String ParseCalledNumEx(Smt_String callednum);
	Smt_String ChangeCallerIDByTAC(Smt_String callerid, TConnection* pconnection);
	Smt_Bool   DelPrefix(Smt_String tac, Smt_String prefix, Smt_String callednum, Smt_String& realcallednum);

public:
	/////////////////////////////////////////
	// TActionPdu
	class TActionPdu
	{
	public:
		TActionPdu()
		{
			m_ActionID = "";
			m_TimerID = 0;
		}
		~TActionPdu(){;}
	public:
		Smt_Pdu    m_Pdu;
		Smt_String m_ActionID;
		Smt_Uint   m_TimerID;
	};
	typedef Smt_Map<Smt_String, TActionPdu*> TActionMap;
	TActionMap m_ActMgr;

	/////////////////////////////////////////
	// TDevice	
	class TAssignedDevice
	{
	public:
		TAssignedDevice()
		{
			m_DeviceType = 0;
			m_DeviceID = "";
		}
		~TAssignedDevice(){;}
	public:
		Smt_Uint m_DeviceType;
		Smt_String m_DeviceID;
	};
	typedef Smt_Map<Smt_String, TAssignedDevice*> TDeviceMap;
	TDeviceMap m_DeviceMgr;
	
	/////////////////////////////////////////
	// TTrunkDN code
	enum TrunkDNStatus
	{
		ST_TRUNKDN_IDLE    = 0,	
		ST_TRUNKDN_BUSY    = 1,
		ST_TRUNKDN_LOCKED  = 2,
	};
	class TTrunkGroup;
	class TTrunkDN: public Smt_DLList_Node
	{
	public:
		TTrunkDN()
		{
			m_ExtenDN = "";
			m_CircuitCode = 0;
			m_State = ST_TRUNKDN_IDLE;
			m_pTrunkGroup = NULL;
			m_TimerID = 0;
		};
		~TTrunkDN(){;}
	public:
		Smt_String m_ExtenDN;
		Smt_Uint   m_CircuitCode;
		Smt_Uint   m_State;
		Smt_Uint   m_TimerID;
		TTrunkGroup* m_pTrunkGroup;
	};

	/////////////////////////////////////////
	// TTrunkGroup code
	class TTrunkGroup
	{
	public:
		TTrunkGroup();
		~TTrunkGroup();
	public:
		Smt_String m_TrunkID;
		Smt_String m_TrunkName;
		Smt_Uint   m_TrunkType;
		Smt_String m_TACCode;
		Smt_String m_CircuitCode;
		Smt_String m_ExtnDN;
		Smt_Uint   m_CurrID;

		typedef Smt_DLList<TTrunkDN> TTrunkDNList;
		TTrunkDNList m_TrunkDNMgr;

		Smt_String GetDahdiTrunkDn(Smt_Uint circuitcode); 
		Smt_String AllocateDahdiTrunk();                     
		Smt_String AllocateSipTrunk();
		Smt_Uint FreeDevice(Smt_String dn);
		Smt_Uint FindDevice(Smt_String dn);
		Smt_Uint GetDeviceState(Smt_String dn);
	};

	typedef Smt_Map<Smt_String, TTrunkGroup*> TTrunkGroupMap;
	TTrunkGroupMap m_TrunkGroupMgr;

	/////////////////////////////////////////
	// TAGIRequest  add by caoyj 20120315
	class TAGIRequest
	{
	public:
		TAGIRequest()
		{
			m_AGIHandle = 0;
			m_Channel = "";
			m_Uniqueid = "";
			m_CallerID = "";
			m_Extension = "";
			m_AGIRequest = "";
			m_AGINetWorkScript = "";
		}
		~TAGIRequest(){;}
	public:
		Smt_Uint   m_AGIHandle;
		Smt_String m_Channel;
		Smt_String m_Uniqueid;
		Smt_String m_CallerID;
		Smt_String m_Extension;
		Smt_String m_AGIRequest;
		Smt_String m_AGINetWorkScript;
	};
	typedef Smt_Map<Smt_Uint, TAGIRequest*> TAGIRequestMap;
	TAGIRequestMap m_AGIRequestMgr;
};

extern TConnectionState*  g_pConnState;
/************************************************************************/

#endif // Smt_TCONNECTIONSTATE_HH
