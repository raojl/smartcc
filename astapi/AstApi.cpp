// IVRAsterisk.cpp : Defines the entry point for the DLL application.
//

#include "AstApi.h"
#include "TAsteriskDevice.h"

// BOOL APIENTRY DllMain( HANDLE hModule, 
//                        DWORD  ul_reason_for_call, 
//                        LPVOID lpReserved
// 					 )
// {
//     switch (ul_reason_for_call)
// 	{
// 		case DLL_PROCESS_ATTACH:
// 		case DLL_THREAD_ATTACH:
// 		case DLL_THREAD_DETACH:
// 		case DLL_PROCESS_DETACH:
// 			break;
//     }
//     return TRUE;
// }


TAsteriskDevice *g_pDevice = NULL;

Smt_Uint StartDevice(Smt_User *pUser, Smt_Server *pServer, Smt_Pdu pdu)
{
	if(g_pDevice == NULL)
	{
		g_pDevice = new TAsteriskDevice;
		if(g_pDevice != NULL)
		{//放到消息队列中异步执行	
			return g_pDevice->StartUp(pUser, pServer, pdu);		
		}
	}
	return Smt_Fail;
}

Smt_Uint ProcessCmd(Smt_Pdu pdu)
{
	if(g_pDevice != NULL)
	{
		return g_pDevice->PutMessageEx(pdu);
	}
	return Smt_Fail;
}

Smt_Uint StopDevice()
{
	if(g_pDevice != NULL)
	{//同步执行	
		g_pDevice->Stop();
		delete g_pDevice;
		g_pDevice = NULL;
	}
	
	return Smt_Success;
}
