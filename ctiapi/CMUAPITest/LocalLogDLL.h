#ifndef __HH_LOGDLL_H_
#define __HH_LOGDLL_H_

extern "C"
{
	void WINAPI OpenLog(LPCTSTR FileName, long nLevel, unsigned long FileSize );
	void WINAPI CloseLog();
	void WINAPI PrintLog(long nLevel,LPCTSTR fmt,...);	
	bool WINAPI SetLogLevel(long nLevel);
}

#endif //__HH_LOGDLL_H_