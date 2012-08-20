//***************************************************************************
//      Copyright (C) 2009 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CMU
//      Create Date : 2010.9.3												
//      Author      : Shenzj								
//      Discription : Device ¿‡
//      Modify By   :											
//***************************************************************************

#ifndef Smt_TDEVICE_HH
#define Smt_TDEVICE_HH

#include "CMUInterface.h"
#include "TConnID.h"

class TDeviceState;

/////////////////////////////////////////////////////////////////////////////
// TRouteID code
class TRouteID
{
public:
	TRouteID()
	{
		m_RouteID = 0;
		m_CallID = 0;
		m_OldCallID = 0;
		m_RouteDN = "";
		m_OtherParty = "";
		m_OtherConnID = "";
		m_CallingParty = "";
		m_CalledParty = "";
	}
	~TRouteID(){;}
public:
	Smt_Uint   m_RouteID;
	Smt_Uint   m_CallID;
	Smt_Uint   m_OldCallID;
	Smt_String m_RouteDN;
	Smt_String m_OtherParty;
	Smt_String m_OtherConnID;
	Smt_String m_CallingParty;
	Smt_String m_CalledParty;
};
typedef Smt_Map<Smt_Uint, TRouteID*> TRouteIDMap;

/////////////////////////////////////////////////////////////////////////////
// TDevice code
class TDevice: public TConnID, public Smt_StateObject
{
public:
	TDevice(Smt_String objid, Smt_Uint type, Smt_Uint initstate, TDeviceState* powner );
	~TDevice();

public: 
	// virtual method
	Smt_Uint OnUnexpectedEvent(Smt_Pdu &pdu);
	Smt_Uint OnStateExit(Smt_Pdu &pdu);
	Smt_Uint OnStateEnter(Smt_Pdu &pdu);

	// action method
	Smt_Uint EvtOffHook(Smt_Pdu &pdu);
	Smt_Uint EvtInboundCall(Smt_Pdu &pdu);
	Smt_Uint EvtDestSeized(Smt_Pdu &pdu);
	Smt_Uint EvtTpAnswered(Smt_Pdu &pdu);
	Smt_Uint EvtOpAnswered(Smt_Pdu &pdu);
	Smt_Uint EvtTpDisconnected_RetrieveCall(Smt_Pdu &pdu);
	Smt_Uint EvtTpDisconnected(Smt_Pdu &pdu);
	Smt_Uint EvtOpDisconnected(Smt_Pdu &pdu);
	Smt_Uint EvtHeld(Smt_Pdu &pdu);
	Smt_Uint EvtTpRetrieved(Smt_Pdu &pdu);
	Smt_Uint EvtOpRetrieved(Smt_Pdu &pdu);
	Smt_Uint ActHoldOtherParty(Smt_Pdu &pdu);
	Smt_Uint SetTimer(Smt_Pdu &pdu);
	Smt_Uint EvtConsultDestSeized(Smt_Pdu &pdu);
	Smt_Uint EvtTpTransferred(Smt_Pdu &pdu);
	Smt_Uint EvtOpTransferred(Smt_Pdu &pdu);
	Smt_Uint EvtTpConferenced(Smt_Pdu &pdu);
	Smt_Uint EvtOpConferenced(Smt_Pdu &pdu);
	Smt_Uint EvtConfTpDisconnected(Smt_Pdu &pdu);
	Smt_Uint EvtTp_OpDisconnected(Smt_Pdu &pdu);
	Smt_Uint EvtDestBusy_RetrieveCall(Smt_Pdu &pdu);
	Smt_Uint EvtDestInvalid_RetrieveCall(Smt_Pdu &pdu);
	Smt_Uint EvtDestFail_RetrieveCall(Smt_Pdu &pdu);
	Smt_Uint ActRetrieveCall(TDevice* pdevice);
	Smt_Uint EvtTp_OpTransferred(Smt_Pdu &pdu);
	Smt_Uint EvtDestBusy(Smt_Pdu &pdu);
	Smt_Uint EvtDestBusy_TpDisconnected(Smt_Pdu &pdu);
	Smt_Uint EvtTp_OpConferenced(Smt_Pdu &pdu);
	Smt_Uint EvtRouteRequest(Smt_Pdu &pdu);	
	Smt_Uint EvtRouteEnd(Smt_Pdu &pdu);	
	Smt_Uint EvtReRoute(TRouteID* prouteid, Smt_Uint reason);	
	Smt_Uint EvtQueued_RouteRequest(Smt_Pdu &pdu);
	Smt_Uint EvtDestSeized_RouteEnd(Smt_Pdu &pdu);
	Smt_Uint EvtTpDisconnected_RouteEnd(Smt_Pdu &pdu);
	Smt_Uint EvtTransferDestSeized(Smt_Pdu &pdu);
	Smt_Uint ActSingleStepTransfer();

	void PrintLog(Smt_Uint loglevel, const char* fmt, ... );
	Smt_Uint SendDeviceEvent(Smt_Uint evtid, Smt_Uint reason);
	Smt_Uint SendOtherEvent(Smt_Pdu& pdu);
	Smt_Uint SetCallData(Smt_Pdu pdu);
	Smt_Uint InitData();
	Smt_Uint ClearTimer();
	Smt_Uint ConvState(Smt_Uint state);
	Smt_String ConvOtherParty(Smt_String otherparty);

public:
	Smt_Uint   m_CallID;         // current call-id
	Smt_Uint   m_OldCallID;      // previous call-id
	Smt_Uint   m_SecOldCallID;   // consultation call-id
	Smt_String m_OtherParty;
	Smt_String m_ThirdParty;
	Smt_String m_CallingParty;
	Smt_String m_CalledParty;
	Smt_Uint   m_Reason;
	Smt_Pdu    m_LastCommand;
	Smt_String m_Variable;
	Smt_Uint   m_TimerID;
	Smt_Uint   m_TimerReason;

public: 
	/////////////////////////////////////////////////////////////////////////////
	// TDeviceMonitor code
	class TDeviceMonitor
	{
	public:
		Smt_Uint m_MonitorGOR;
		Smt_Uint m_DeviceRefID;
	};
	typedef Smt_Map<Smt_Uint, TDeviceMonitor*> TDeviceMonitorMap;

	TDeviceMonitorMap m_MonitorMgr;
	Smt_Uint AddMonitor(Smt_Uint monitorgor);
	Smt_Uint RemoveMonitor(Smt_Uint devicerefid);
	Smt_Uint RemoveMonitorByGOR(Smt_Uint monitorgor);
	Smt_Bool IsMonitored(Smt_Uint devicerefid);
};

typedef Smt_Map<Smt_String,TDevice*> TDeviceMap;
/************************************************************************/

#endif // Smt_TDEVICE_HH
