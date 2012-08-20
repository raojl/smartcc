//***************************************************************************
//      Copyright (C) 2009 Holly Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : CMU1.0
//      Create Date : 2009.11.26												
//      Author      : shenzj@hollycrm.com
//      Discription : CMUDLL API Defines, Support C/C++ 
//      Modify By   :
//***************************************************************************

#ifndef H_HOLLY_CMUAPI_H
#define H_HOLLY_CMUAPI_H

#include "cmuapidefs.h"

#ifndef CMUAPI
#ifdef _WIN32
#define CMUAPI	__cdecl
#else
#define CMUAPI
#endif // _WIN32
#endif // CMUAPI

#if defined (__cplusplus)
extern "C" {
#endif  /* __cplusplus */

unsigned long CMUAPI ctcAssign( 
							unsigned long invokeid, 
							char* deviceid, 
							unsigned long devicetype );
							
unsigned long CMUAPI ctcDeassign( 
							unsigned long channelid );
							
unsigned long CMUAPI ctcMakeCall( 
							unsigned long channelid, 
							char* callednum, 
							char* callernum );
							
unsigned long CMUAPI ctcHangupCall( 
							unsigned long channelid, 
							unsigned long callid );

unsigned long CMUAPI ctcClearCall( 
							unsigned long channelid, 
							unsigned long callid );

unsigned long CMUAPI ctcHoldCall( 
						    unsigned long channelid, 
						    unsigned long callid );

unsigned long CMUAPI ctcRetrieveCall( 
						    unsigned long channelid, 
						    unsigned long callid );

unsigned long CMUAPI ctcConsultationCall( 
							unsigned long channelid, 							
							unsigned long callid,
							char* callednum,
							char* callernum,
							unsigned long timeout );

unsigned long CMUAPI ctcConferenceCall( 
							unsigned long channelid, 
							unsigned long holdcallid,
							unsigned long activecallid );

unsigned long CMUAPI ctcTransferCall( 
							unsigned long channelid, 
							unsigned long holdcallid,
							unsigned long activecallid );

unsigned long CMUAPI ctcReconnectCall( 
							unsigned long channelid, 
							unsigned long holdcallid,
							unsigned long activecallid );
							
unsigned long CMUAPI ctcSingleStepTransfer( 
							unsigned long channelid, 
							char* callednum,
							unsigned long callid );

unsigned long CMUAPI ctcSingleStepConference( 
							unsigned long channelid,
							char* callednum, 
							unsigned long callid, 
							unsigned long mode );

unsigned long CMUAPI ctcGetEvent( 
							Csta_Event* event, 
							unsigned long timeout );

unsigned long CMUAPI ctcRouteSelected ( 
							unsigned long channelid,
							unsigned long routeid, 
							char* callednum,
							char* agentid );

unsigned long CMUAPI ctcSendDTMF( 
							unsigned long channelid,
							unsigned long callid, 
							char* digits );

unsigned long CMUAPI ctcStartRecord( 
							unsigned long channelid, 
							unsigned long callid, 
							char* filename );

unsigned long CMUAPI ctcStopRecord(
							unsigned long channelid, 
							unsigned long callid );

unsigned long CMUAPI ctcSetDataValue( 
							unsigned long channelid, 
							unsigned long callid,
							char* key,
							char* value );

unsigned long CMUAPI ctcGetDataValue( 
							unsigned long channelid, 
							unsigned long callid,
							char* key );
							
unsigned long CMUAPI ctcSnapshot( 
							unsigned long channelid );

unsigned long CMUAPI ctcSendMessage( 
							unsigned long channelid,
							unsigned long callid, 
							char* message );
							
unsigned long CMUAPI gc_open( 
							unsigned long invokeid, 
							char* deviceid, 
							unsigned long devicetype );

unsigned long CMUAPI gc_close( 
							unsigned long channelid );

unsigned long CMUAPI gc_makecall( 
							unsigned long invokeid,
							char* deviceid,
							char* callednum,
							char* callernum,
							unsigned long timeout );

unsigned long CMUAPI gc_answercall(
							unsigned long callid );
							
unsigned long CMUAPI gc_hangupcall(
							unsigned long callid );

unsigned long CMUAPI gc_blindtransfer( 
							unsigned long callid, 
							char* callednum );

unsigned long CMUAPI gc_dial( 
							unsigned long callid, 
							char* callednum,
							unsigned long timeout );

unsigned long CMUAPI dx_play( 
							unsigned long callid, 
							char* filename,
							char* escapedigits );

unsigned long CMUAPI dx_addiottdata( 
							unsigned long callid, 
							char* filename );

unsigned long CMUAPI dx_playiottdata( 
							unsigned long callid, 
							char* escapedigits );

unsigned long CMUAPI dx_getdig( 
							unsigned long callid, 
							unsigned long timeout,
							unsigned long maxcount );

unsigned long CMUAPI dx_dial( 
							unsigned long callid, 
							char* digits );
							
unsigned long CMUAPI dx_rec( 
							unsigned long callid,
							char* filename, 
							char* escapedigits, 
							unsigned long timeout, 
							unsigned long silence );

unsigned long CMUAPI dx_sendmessage( 
							unsigned long callid, 
							char* message );

unsigned long CMUAPI dx_setsipheader( 
							unsigned long callid,
							char* headerkey,
							char* headervalue );

unsigned long CMUAPI dx_getsipheader( 
							unsigned long callid,
							char* headerkey);

unsigned long CMUAPI fx_sendfax( 
							unsigned long channelid,unsigned long callid,char* options  );
							
unsigned long CMUAPI fx_rcvfax(
							unsigned long callid );

unsigned long CMUAPI fx_playtext(unsigned long callid,char* options  );
							
unsigned long CMUAPI cmSubscribeCallEvent();

unsigned long CMUAPI cmUnsubscribeCallEvent();

unsigned long CMUAPI cmInitUser(unsigned long handle);

unsigned long CMUAPI cmUninitUser();

#if defined (__cplusplus)
}
#endif  /* __cplusplus */


#endif // H_HOLLY_CMUAPI_H
