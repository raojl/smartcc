
// The following ifdef block is the standard way of creating macros which make exporting 
// from a DLL simpler. All files within this DLL are compiled with the IVRASTERISK_EXPORTS
// symbol defined on the command line. this symbol should not be defined on any project
// that uses this DLL. This way any other project whose source files include this file see 
// IVRASTERISK_API functions as being imported from a DLL, wheras this DLL sees symbols
// defined with this macro as being exported.

#include "IVRInterface.h"

#ifdef IVRASTERISK_EXPORTS
#define DllExport_API extern "C" __declspec(dllexport)
#else
#define DllExport_API extern "C"
#endif


DllExport_API Smt_Uint StartDevice(Smt_User *pUser, Smt_Server *pServer, Smt_Pdu pdu);
DllExport_API Smt_Uint ProcessCmd(Smt_Pdu pdu);
DllExport_API Smt_Uint StopDevice();
