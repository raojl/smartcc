#include "TAsteriskDevice.h"

TMatchData::TMatchData()
{

}
TMatchData::~TMatchData()
{

}
Smt_Uint TMatchData::MatchResult(Smt_String strDNIS)
{
	Smt_Uint nRet = Smt_Fail;

	if (m_IsMatch == 0)//不包含. 即不包含匹配规则
	{
		if(strDNIS == m_strDeviceID) //只判断被叫号码与IVR配置中的号码是否相同
		{
			nRet = Smt_Success;
		}
	}
	else//包含.
	{
		Smt_String strTmp;
		if(strDNIS.length() >= m_MatchStrLen)
		{
			if(m_MatchType == Match_Head)//头部匹配
			{			
				strTmp = strDNIS.substr(0, m_MatchStrLen);

				if (strTmp == m_MatchStr)
				{
					nRet = Smt_Success;
				}
			}
			else if (m_MatchType == Match_Tail)//尾部匹配
			{
				strTmp = strDNIS.substr(strDNIS.length() - m_MatchStrLen, m_MatchStrLen);
				if (strTmp == m_MatchStr)
				{
					nRet = Smt_Success;
				}
			}
			else
			{
				
			}
		}		
	}
	return nRet;
}
TChannelData::TChannelData()
{
	
	m_ChNo = 0;
	m_ChStat = 0;
	m_ANI = "";
	m_DNIS = "";
	m_pLog = NULL;     
}

TChannelData::~TChannelData()
{
	m_ChNo = 0;
	m_ChStat = 0;
	m_ANI = "";
	m_DNIS = "";
	m_pLog = NULL; 
}
void TChannelData::PrintLog(Smt_Uint loglevel, const char* fmt, ... )
{
	if(m_pLog == NULL) return;
	
	char buf[Smt_LOGBUFF_LEN];
	va_list list;
	va_start(list,fmt);
	ACE_OS::vsnprintf(buf, Smt_LOGBUFF_LEN, fmt, list);
	va_end(list);
	m_pLog->Trace(loglevel, buf);
}
void DtmfCache::Reset()
{
    m_flag = 0;
    m_maxlen = 0;
	m_starttime = 0;
}
DtmfCache::DtmfCache()
{
    Reset();
}
DtmfCache::~DtmfCache()
{
    Reset();
}
TTrunkData::TTrunkData()
{
	m_InvokeID = 0;;
	m_DeviceID = "";
	m_DeviceType = 0;
	m_ChanID = 0;
	m_TrunkID = 0;
	m_TrunkDirect = 0;
	m_TrunkType = "";
    m_CurrentCh = 0;
	m_MatchCount = 0;
}
TTrunkData::~TTrunkData()
{
	m_InvokeID = 0;;
	m_DeviceID = "";
	m_DeviceType = 0;
	m_ChanID = 0;
	m_TrunkID = 0;
	m_TrunkDirect = 0;
	m_TrunkType = "";
    m_CurrentCh = 0;
	m_MatchCount = 0;
}
TCallData::TCallData()
{
	m_TpDisconnect = 0;
	m_CRN = 0;
	m_ChNo = 0;
	m_TrunkDeviceID = "";
	m_ResState = 0;
	m_pChInfo = NULL;
	m_MakeCall = 0;
	m_IsGcDail = GCDailNULL;
	m_DtmfCache.Reset();
	m_SipHeaderKey = "";
	m_SipHeaderValue = "";
}
TCallData::~TCallData()
{
	m_CRN = 0;
	m_ChNo = 0;
	m_TrunkDeviceID = "";
	m_ResState = 0;
	m_pChInfo = NULL;
	m_DtmfCache.Reset();
}
TAsteriskDevice::TAsteriskDevice()
{
	m_LinkState = Smt_Fail;
	m_EventState = Smt_Fail;
	m_Init = 0;
	m_LogLv = 5;          
	m_strCurrPath = "";   
	m_DefaultWavPath = ""; 
	m_WavLibPath = "";     
	m_SendFaxPath = "";    
	m_RecvFaxPath = "";    
	m_RecordPath = "";   
	m_nDotLen = 2;
	m_gSipHeaderKey = "";
}
TAsteriskDevice::~TAsteriskDevice()
{
	m_LinkState = Smt_Fail;
	m_EventState = Smt_Fail;
	m_Init = 0;
	m_LogLv = 5;          
	m_strCurrPath = "";   
	m_DefaultWavPath = ""; 
	m_WavLibPath = "";     
	m_SendFaxPath = "";    
	m_RecvFaxPath = "";    
	m_RecordPath = "";    
	DeleteAllData();
}
Smt_Uint TAsteriskDevice::DeleteAllData()
{
	//删除callmap
	TCallData *pCall = NULL;
	for (CallMap::ITERATOR iter= m_CallMap.begin(); iter != m_CallMap.end(); iter++)
	{
		pCall = (*iter).int_id_;
		if(pCall != NULL)
		{
			delete pCall;
			pCall = NULL;
		}
	}
	m_CallMap.RemoveAll();
	
	//删除chmap 与 trunkmap
	TTrunkData *pTrunk = NULL;
	for (TrunkMap::ITERATOR iter1= m_TrunkMap.begin(); iter1 != m_TrunkMap.end(); iter1++)
	{
		pTrunk = (*iter1).int_id_;
		TChannelData *pChData = NULL;
		for (ChannelMap::ITERATOR iter2= pTrunk->m_SubChInfo.begin(); iter2 != pTrunk->m_SubChInfo.end(); iter2++)
		{
			pChData = (*iter2).int_id_;
			if(pChData != NULL)
			{
				delete pChData;
				pChData = NULL;
			}
		}
		pTrunk->m_SubChInfo.RemoveAll();
		
		if(pTrunk != NULL)
		{
			delete pTrunk;
			pTrunk = NULL;
		}
	}
	m_TrunkMap.RemoveAll();

	//删除TimerMap
	TimerItem *pTmerItem = NULL;
	for (TimerItemMap::ITERATOR iter3= m_TimerMap.begin(); iter3 != m_TimerMap.end(); iter3++)
	{
		pTmerItem = (*iter3).int_id_;
		if(pTmerItem != NULL)
		{
			delete pTmerItem;
			pTmerItem = NULL;
		}
	}
	m_TimerMap.RemoveAll();

	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ReadCfgFile()
{
	Smt_String strCfgFile;
	m_strCurrPath = HLGetModuleFilePath();
	strCfgFile = HLFormatStr("%sMCSCFG.xml", m_strCurrPath.c_str());

	Smt_String strTemp1, strTemp2;	
#ifdef _WIN32
	strTemp2 = m_strCurrPath + Smt_String("../logs\\ChannelLogs\\");
#else
	strTemp2 = m_strCurrPath + Smt_String("../logs/ChannelLogs/");
#endif
	HLCreateDirRecursion( strTemp1, strTemp2 );

	return Smt_Success;
}
Smt_Uint TAsteriskDevice::FindCallByCRN(Smt_Uint nCRN, TCallData *&pCallData)
{
	Smt_Uint nRet = Smt_Fail;
	if(m_CallMap.Lookup(nCRN, pCallData) == Smt_Success)
	{
		nRet = Smt_Success;
	}
	return nRet;
}
Smt_Uint TAsteriskDevice::FindCallByChannel(Smt_Uint channel, TCallData *&pCallData)
{
	Smt_Uint nRet = Smt_Fail;
	TCallData *pCall = NULL;
	for (CallMap::ITERATOR iter= m_CallMap.begin(); iter != m_CallMap.end(); iter++)
	{
		pCall = (*iter).int_id_;
		if(pCall->m_ChNo == channel)
		{
			pCallData = pCall;
			nRet = Smt_Success;
			break;
		}
	}
	return nRet;
}

Smt_Uint TAsteriskDevice::AkPlayFile(Smt_Uint channel, Smt_String termkey, Smt_String content)
{
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		if(pCall->m_ResState == RES_NULL || pCall->m_ResState == RES_DXPLAY )
		{
			Smt_String strBuff = content;
			bool bFlag = false;
			do 
			{
				int nPosB = strBuff.find("|");
				int nPosE = strBuff.find(",");
				if(nPosB <0 || nPosE < 0)break;
				else
				{
					Smt_String strContent = strBuff.substr(0, nPosB);
					Smt_String strType = strBuff.substr(nPosB, 4);
					strBuff = strBuff.substr(nPosE+1, strBuff.length() - nPosE);
					int nFileType = GetFilePlayType(strType);
					bFlag = FillPlayStringIott(channel, nFileType, strContent);				
				}
			} while (bFlag);
			
			Smt_String strTermkey;
			if(termkey == "@")
			{
				strTermkey = "0123456789*#";
			}
			else if (termkey == "")
			{
				strTermkey = "A";
			}
			else
			{
				strTermkey = termkey;
			}
			if(bFlag)
			{
				pCall->m_ResState = RES_DXPLAY;
				dx_playiottdata(pCall->m_CRN, (char*)(strTermkey.c_str()));
				pCall->m_pChInfo->PrintLog(5, "[TAsteriskDevice::AkPlayFile] AkPlayFile CRN<%d> Content<%s> Termkey<%s>.", pCall->m_CRN, content.c_str(), termkey.c_str());
			}
		}
		else
		{
			RespPlay(pCall->m_ChNo, Smt_Fail); 
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_Play, CauseFuctionFail, "Resource Busy");
			pCall->m_pChInfo->PrintLog(3, "[TAsteriskDevice::AkPlayFile] Resource Busy. CRN<%d> Content<%s> Termkey<%s>.", pCall->m_CRN, content.c_str(), termkey.c_str());
		}		
	}
    return Smt_Success;
}
Smt_Uint TAsteriskDevice::AkPlayDTMF(Smt_Uint channel, Smt_String content)
{
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		if(pCall->m_ResState == RES_NULL)
		{
			pCall->m_ResState = RES_DXSENDDTMF;
			dx_dial(pCall->m_CRN, (char*)(content.c_str()));
			pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::AkPlayDTMF] PlayDTMF CRN<%d> Content<%s>", pCall->m_CRN, content.c_str());
		}
		else
		{
			RespPlay(pCall->m_ChNo, Smt_Fail); 
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_Play, CauseFuctionFail, "Resource Busy");
			pCall->m_pChInfo->PrintLog(3,"[TAsteriskDevice::AkPlayDTMF] Resource Busy. CRN<%d> Content<%s>", pCall->m_CRN, content.c_str());
		}
	}
    return Smt_Success;
}

Smt_Uint TAsteriskDevice::AkPlayTTS(Smt_Uint channel, Smt_String termkey, Smt_String content)
{
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		if(pCall->m_ResState == RES_NULL)
		{
			pCall->m_ResState = RES_FXPLAYTTS;
			Smt_String tmpCon = content;
			Smt_String strTermkey;
			if(termkey == "@")
			{
				strTermkey = "0123456789*#";
			}
			else if (termkey == "")
			{
				strTermkey = "A";
			}
			else
			{
				strTermkey = termkey;
			}
			if (strTermkey != "")
			{
				tmpCon = tmpCon + "|" + strTermkey;
			}
			//dx_playtts(pCall->m_CRN,(char*)tmpCon.c_str());
			//	dx_sendmessage(pCall->m_CRN, (char*)(content.c_str()));
			pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::AkPlayTTS] CRN<%d> termkey<%s>  Content<%s>", pCall->m_CRN, termkey.c_str(), content.c_str());
		}
		else
		{
			RespPlay(pCall->m_ChNo, Smt_Fail); 
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_Play, CauseFuctionFail, "Resource Busy");
			pCall->m_pChInfo->PrintLog(3,"[TAsteriskDevice::AkPlayTTS] Resource Busy. CRN<%d> termkey<%s>  Content<%s>", pCall->m_CRN, termkey.c_str(), content.c_str());
		}
	}
    return Smt_Success;
}

Smt_Uint TAsteriskDevice::AkRecFile(Smt_Uint channel, Smt_String termkey, Smt_String content, Smt_Uint rectime, Smt_Uint slienttime)
{
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		if(pCall->m_ResState == RES_NULL)
		{
			pCall->m_ResState = RES_DXRECORD;
			Smt_String recFile;
			Smt_String strTermkey;
			if(termkey == "@")
			{
				strTermkey = "0123456789*#";
			}
			else if (termkey == "")
			{
				strTermkey = "A";
			}
			else
			{
				strTermkey = termkey;
			}
			GetFileName(recFile, content, m_RecordPath);
			dx_rec(pCall->m_CRN, (char*)(recFile.c_str()), (char*)(strTermkey.c_str()), rectime, slienttime);
			pCall->m_pChInfo->PrintLog(5, "[TAsteriskDevice::AkRecFile] RecFile CRN<%d> Content<%s> Termkey<%s> rectime<%d> slientTime<%d>",
			pCall->m_CRN, content.c_str(), termkey.c_str(), rectime, slienttime);
		}
		else
		{
			RespRecord(pCall->m_ChNo, Smt_Fail);
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_Record, CauseFuctionFail, "Resource Busy");
			pCall->m_pChInfo->PrintLog(3, "[TAsteriskDevice::AkRecFile] Resource Busy. CRN<%d> Content<%s> Termkey<%s> rectime<%d> slientTime<%d>",
				pCall->m_CRN, content.c_str(), termkey.c_str(), rectime, slienttime);
		}		
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::AkGetDTMF(Smt_Uint channel, Smt_Uint maxlen, Smt_Uint starttime)
{
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		if(pCall->m_ResState == RES_NULL)
		{
			pCall->m_ResState = RES_DXRECVDTMF;
			dx_getdig(pCall->m_CRN, starttime, maxlen);
		    pCall->m_pChInfo->PrintLog(5, "[TAsteriskDevice::AkGetDTMF] GetDTMF CRN<%d> Maxlen<%d> StartTime<%d>", pCall->m_CRN, maxlen, starttime);
		}
		else if(pCall->m_ResState == RES_DXPLAY)
		{//缓冲数据
			pCall->m_DtmfCache.m_flag = 1;
			pCall->m_DtmfCache.m_maxlen = maxlen;
			pCall->m_DtmfCache.m_starttime = starttime;
		}
		else
		{
            RespGetDTMF(pCall->m_ChNo, Smt_Fail);
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_GetDTMF, CauseFuctionFail, "Resource Busy");
			pCall->m_pChInfo->PrintLog(3, "[TAsteriskDevice::AkGetDTMF] Resource Busy. CRN<%d> Maxlen<%d> StartTime<%d>", pCall->m_CRN, maxlen, starttime);
		}
		
	}
	return Smt_Success;
}

Smt_Uint TAsteriskDevice::InitDevice(Smt_Pdu pdu)
{//这里只是传送配置数据

	ReadCfgFile();

	Smt_Uint i, j, k, ChCount = 0;
    Smt_Uint tmpUser = 0;
	Smt_Uint tmpServer = 0;
	Smt_String CfgFile;
	pdu.GetUint(Key_IVR_MCSUser, &tmpUser);
	pdu.GetUint(Key_IVR_MCSServer, &tmpServer);
	m_pUser = (Smt_User*)tmpUser;
	m_pServer = (Smt_Server*)tmpServer;
	pdu.GetString(Key_IVR_MSConfig, &CfgFile);

	//board info
	Smt_Uint nSysParamArray = 0;
	Smt_Kvset *pSysParamArray = new Smt_Kvset[MAX_KVSETARRAYITEM];
    pdu.GetKvsetArray( Key_IVR_SysParamArray, &pSysParamArray, &nSysParamArray );
	for(i = 0; i<nSysParamArray; i++)
	{
		Smt_String ParamName;
		Smt_String ParamValue;
		pSysParamArray[i].GetString(Key_IVR_ParamName, &ParamName);
		pSysParamArray[i].GetString(Key_IVR_ParamValue, &ParamValue);
		
		if(ParamName == Smt_String("CallTimeOut"))
		{
			//m_CallOutTime = ACE_OS::atoi(ParamValue.c_str());
		}
		else if (ParamName == Smt_String("MaxOnlineTime"))
		{
            //MaxTimeOut = ACE_OS::atoi(ParamValue.c_str());
		}
		else if (ParamName == Smt_String("LogLevel"))
		{
            m_LogLv = ACE_OS::atoi(ParamValue.c_str());
		}
		else if (ParamName == Smt_String("B2BUA"))
		{
			//m_B2BUA = ACE_OS::atoi(ParamValue.c_str());
		}
		else if (ParamName == Smt_String("SystemError"))
		{
			//m_SysErrVoice = ParamValue;
		}
		else if (ParamName == Smt_String("DependCall"))
		{
			//DependCall = ParamValue;
		}
		else if (ParamName == Smt_String("DefaultWavPath"))
		{
			m_DefaultWavPath = ParamValue;
		}
		else if (ParamName == Smt_String("WavLibPath"))
		{
			m_WavLibPath = ParamValue;
		}
		else if (ParamName == Smt_String("SendFaxPath"))
		{
			m_SendFaxPath = ParamValue;
		}
		else if (ParamName == Smt_String("RecvFaxPath"))
		{
			m_RecvFaxPath = ParamValue;
		}
		else if (ParamName == Smt_String("RecordPath"))
		{
			m_RecordPath = ParamValue;
		}
		else if (ParamName == Smt_String("DotLen"))
		{
			m_nDotLen = ACE_OS::atoi(ParamValue.c_str());
		}
		else if (ParamName == Smt_String("SipHeaderKey"))
		{
			m_gSipHeaderKey = ParamValue;
		}
	}
	if(pSysParamArray != NULL)
	{
		delete [] pSysParamArray;
		pSysParamArray = NULL;
	}

	//trunk info
	Smt_Uint nTrunkArray = 0;
	Smt_Kvset *pTrunkArray = new Smt_Kvset[MAX_KVSETARRAYITEM];
    pdu.GetKvsetArray( Key_IVR_TrunkArray, &pTrunkArray, &nTrunkArray );
	for(i = 0, k = 0; i < nTrunkArray; i++)
	{
		Smt_Uint TrunkID;
		Smt_String TrunkType;
		Smt_Uint TrunkDirection;
		
		pTrunkArray[i].GetUint(Key_IVR_TrunkID, &TrunkID);
		pTrunkArray[i].GetString(Key_IVR_TrunkType, &TrunkType);
		pTrunkArray[i].GetUint(Key_IVR_CallDirection, &TrunkDirection);	
		
		TTrunkData *pData = new TTrunkData;
		pData->m_TrunkID = TrunkID;
		pData->m_TrunkDirect = TrunkDirection;
		pData->m_TrunkType = TrunkType;
		pData->m_DeviceID = TrunkType.substr(8,TrunkType.length()-8);//截取掉asterisk
		pData->m_InvokeID = 1000+i;//invokeid从1000开始
		if(TrunkDirection == 1)
		{//呼入
			pData->m_DeviceType = 3;
		}
		else if(TrunkDirection == 2)
		{//呼出
			pData->m_DeviceType = 4;
		}
		else
		{//默认呼入 //或返回错误
            pData->m_DeviceType = 3;
		}

			Smt_String strTMP = pData->m_DeviceID;
			Smt_String strMatch;
			int strCount = 0;
			Smt_Int nPos = 0;
			do 
			{

// 				FILE *p = NULL;
// 				p = fopen("./123.txt","ab");
// 				if(p != NULL)
// 				{
// 					fwrite(strTMP.c_str(),1,strTMP.length(),p);
// 					fclose(p);
// 				}

				if (strCount >= 127)
				{
					break;
				}



				nPos = strTMP.find(",");
				if (nPos < 0)
				{
					strMatch = strTMP;
					strTMP = "";
				}
				else
				{
					strMatch = strTMP.substr(0, nPos);
					strTMP = strTMP.substr(nPos+1, strTMP.length() - nPos - 1);
				}



				nPos = strMatch.find(".");
				if(nPos < 0)//不包含匹配规则
				{
						pData->m_MatchData[strCount].m_strDeviceID = strMatch;
						pData->m_MatchData[strCount].m_nInvokeID = 1000+strCount;
						pData->m_MatchData[strCount].m_IsMatch = 0;
						pData->m_MatchCount = strCount+1;
				}
				else//包含匹配规则
				{
					if (nPos == 0)
					{
						pData->m_MatchData[strCount].m_strDeviceID = strMatch;
						pData->m_MatchData[strCount].m_nInvokeID = 1000+strCount;
						pData->m_MatchData[strCount].m_IsMatch = 1;
						pData->m_MatchCount = strCount+1;
						
						pData->m_MatchData[strCount].m_MatchType = Match_Tail;
						pData->m_MatchData[strCount].m_MatchStr = strMatch.substr(1, strMatch.length()-1);
						pData->m_MatchData[strCount].m_MatchStrLen = pData->m_MatchData[0].m_MatchStr.length();			
					}
					else if(nPos == (strMatch.length() -1))
					{
						pData->m_MatchData[strCount].m_strDeviceID = strMatch;
						pData->m_MatchData[strCount].m_nInvokeID = 1000+strCount;
						pData->m_MatchData[strCount].m_IsMatch = 1;
						pData->m_MatchCount = strCount+1;
						
						pData->m_MatchData[strCount].m_MatchType = Match_Head;
						pData->m_MatchData[strCount].m_MatchStr = strMatch.substr(0, strMatch.length()-1);
						pData->m_MatchData[strCount].m_MatchStrLen = pData->m_MatchData[0].m_MatchStr.length();
						
					}
					else
					{
						//不支持的匹配规则
						
					}
					
				}

				strCount++;

				nPos = strTMP.length();

			} while (nPos > 0);


	    //////////////////////////////////////////////////////////////////////////
		
		m_TrunkMap.SetAt(pData->m_DeviceID, pData);
	}
	if(pTrunkArray != NULL)
	{
		delete [] pTrunkArray;
		pTrunkArray = NULL;
	}

    //channel info
	Smt_Uint nChannelArray = 0;
	Smt_Kvset *pChannelArray = new Smt_Kvset[MAX_KVSETARRAYITEM];
    pdu.GetKvsetArray( Key_IVR_ChannelArray, &pChannelArray, &nChannelArray );
	for(i = 0; i<nChannelArray; i++)
//	for(i = nChannelArray -1; i >= 0; i--)
	{
		Smt_String ChannelRange;
		Smt_Uint TrunkId;
		
		pChannelArray[i].GetString(Key_IVR_ChannelRange, &ChannelRange);
		pChannelArray[i].GetUint(Key_IVR_TrunkID, &TrunkId);

		Smt_Int nPos = ChannelRange.find("-");
		if(nPos < 0)
		{//eg.1
			j = ACE_OS::atoi(ChannelRange.c_str());
			if(j <= MAXCHANNEL && j > 0)
			{
				TTrunkData *pTrunk = NULL;
				for (TrunkMap::ITERATOR iter= m_TrunkMap.begin(); iter != m_TrunkMap.end(); iter++)
				{
					pTrunk = (*iter).int_id_;
					if(pTrunk->m_TrunkID == TrunkId)
					{
						TChannelData *pChData = new TChannelData;
						pChData->m_ChNo = j;
						pChData->m_ChStat = ChannelUnknown;
						char cLog[MAXBUFF] = "";
				        ACE_OS::sprintf(cLog, "%s../logs/ChannelLogs/CH%03d.log", m_strCurrPath.c_str(), j);
						pChData->m_pLog = new Smt_TraceLog();
						pChData->m_pLog->OpenLog(cLog, m_LogLv, 50*1024*1024);
				        pTrunk->m_SubChInfo.SetAt(j, pChData);
						pChData->PrintLog(4,"[TAsteriskDevice::InitDevice] <%s> Channel Init....", pTrunk->m_DeviceID.c_str());
					}
				}
			}		
		}
		else
		{//eg.1-30
			Smt_String strFrom, strTo, strTemp;
			strFrom = ChannelRange.substr( 0, nPos );
			strTo = ChannelRange.substr( nPos+1, ChannelRange.length());
			Smt_Int nFrom = ACE_OS::atoi(strFrom.c_str());
			Smt_Int nTo = ACE_OS::atoi(strTo.c_str());

			if(nFrom <= MAXCHANNEL && nFrom > 0 && nTo <= MAXCHANNEL && nTo > 0 && nTo > nFrom)
			{
				//for(j =nFrom; j <= nTo; j++ )
                for(j = nTo; j >= nFrom; j-- )
				{
					TTrunkData *pTrunk = NULL;
					for (TrunkMap::ITERATOR iter= m_TrunkMap.begin(); iter != m_TrunkMap.end(); iter++)
					{
						pTrunk = (*iter).int_id_;
						if(pTrunk->m_TrunkID == TrunkId)
						{
							TChannelData *pChData = new TChannelData;
							pChData->m_ChNo = j;
							pChData->m_ChStat = ChannelUnknown;
							char cLog[MAXBUFF] = "";
							ACE_OS::sprintf(cLog, "%s../logs/ChannelLogs/CH%03d.log", m_strCurrPath.c_str(), j);
							pChData->m_pLog = new Smt_TraceLog();
						    pChData->m_pLog->OpenLog(cLog, m_LogLv, 50*1024*1024);
							pTrunk->m_SubChInfo.SetAt(j, pChData);
							pChData->PrintLog(4,"[TAsteriskDevice::InitDevice] <%s> Channel Init....", pTrunk->m_DeviceID.c_str());
						}
					}			
				}
			}			
		}		
	}
	if(pChannelArray != NULL)
	{
		delete [] pChannelArray;
		pChannelArray = NULL;
	}

	char strLog[MAXBUFF] = "";
	ACE_OS::sprintf(strLog, "%s../logs/ChannelLogs/GlobalLog.log", m_strCurrPath.c_str());
	m_Ch0.m_pLog = new Smt_TraceLog();
	m_Ch0.m_pLog->OpenLog(strLog, m_LogLv, 50*1024*1024);
	m_Ch0.PrintLog(4,"[TAsteriskDevice::InitDevice] Read Config File Ok.");

	cmInitUser(tmpServer);
	m_Ch0.PrintLog(4,"[TAsteriskDevice::InitDevice] pServer<%d>.", tmpServer);
	m_Init = 1;

	m_TimeAdapter.activate();
    
	return Smt_Success;
}

Smt_Uint TAsteriskDevice::Stop()
{
	m_IsThreadRun = Smt_BoolFALSE;
    CloseConfigedDevices();
	cmUninitUser();
	DeleteAllData();
	m_TimeAdapter.deactivate();
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::GetDeviceEvent()
{
	if(m_Init == 1)
	{
		if(m_EventState == Smt_Fail)
		{
// 			if(cmSubscribeCallEvent() != Smt_Success)
// 			{
// 				
// 			}
			HLSleep(20);
//			m_Init = 2;
		}
		Csta_Event event;
		long tmVal = 0;//是否可以即时返回
		if(ctcGetEvent(&event, tmVal) == Smt_Success)
		{
			ProcDeviceEvent(event);
			return 1;
		}
	}	
	return 0;
}

Smt_Uint TAsteriskDevice::ProcDeviceEvent(Csta_Event &event)
{

//	m_Ch0.PrintLog(4,"[TAsteriskDevice::OpenConfigedDevices] 3 <%d>",event.evtclass);
	switch(event.evtclass)
	{
	case CSTA_CONFEVENT:
		OnCstaConfEvent(event);
		break;
	case CSTA_CALLEVENT:
		OnCstaCallEvent(event);
		break;
	case CSTA_DEVICEEVENT:
		OnCstaDeviceEvent(event);
		break;
	case CSTA_ROUTEEVENT:
		OnCstaRouteEvent(event);
		break;
	case CSTA_MEDIAEVENT:
		OnCstaMediaEvent(event);
		break;
	case CSTA_DEVICERECORDEVENT:
		OnCstaDeviceRecordEvent(event);
		break;
	case CSTA_SYSTEMEVENT:
		OnCstaSystemEvent(event);
		break;
	default:
		break;
	}
	return Smt_Success;
}

Smt_Uint TAsteriskDevice::OnCstaConfEvent(Csta_Event &event)
{
//	m_Ch0.PrintLog(4,"[TAsteriskDevice::OpenConfigedDevices] 3 <%d>",event.evttype);
	switch (event.evttype)
	{
	case CONF_CTCASSIGN                  :
		break; 
	case CONF_CTCDEASSIGN                :
		break; 
	case CONF_CTCMAKECALL                :
		break; 
	case CONF_CTCHANGUPCALL              :
		break; 
	case CONF_CTCSINGLESTEPTRANSFER      :
		break; 
	case CONF_CTCSINGLESTEPCONFERENCE    :
		break; 
	case CONF_CTCROUTESELECTED           :
		break; 
	case CONF_CTCSENDDTMF                :
		break; 
	case CONF_CTCSTARTRECORD             :
		break; 
	case CONF_CTCSTOPRECORD              :
		break; 
	case CONF_CTCSETDATAVALUE            :
		break; 
	case CONF_CTCGETDATAVALUE            :
		break; 
	case CONF_GC_OPEN                    :
		OnEvtConfGcOpen(event);
		break; 
	case CONF_GC_CLOSE                   :
		break; 
	case CONF_GC_MAKECALL                :
		OnEvtConfGcMakeCall(event);
		break; 
	case CONF_GC_ANSWERCALL              :
		OnEvtConfGcAnswerCall(event);
		break; 
	case CONF_GC_HANGUPCALL              :
		OnEvtConfGcHangupCall(event);
		break; 
	case CONF_GC_BLINDTRANSFER           :
		OnEvtConfBlindTransfer(event);
		break; 
	case CONF_GC_DIAL:
		OnEvtConfGcDial(event);
		break;
	case CONF_DX_PLAY                    :
		break; 
	case CONF_DX_ADDIOTTDATA             :
		break; 
	case CONF_DX_PLAYIOTTDATA            :
		OnEvtConfPlayIottData(event);
		break; 
	case CONF_DX_GETDIG                  :
		OnEvtConfGetDig(event);
		break; 
	case CONF_DX_DIAL                    :
		OnEvtConfDial(event);
		break; 
	case CONF_DX_REC                     :
		OnEvtConfRec(event);
		break; 
	case CONF_FX_SENDFAX                 :
		break; 
	case CONF_FX_RCVFAX                  :
		break; 
	case CONF_SUBSCRIBECALLEVENT         :
		OnEvtConfSubscribe(event);
		break; 
	case CONF_UNSUBSCRIBECALLEVENT       :
		break; 
	case CONF_DX_SENDMESSAGE             ://5.0.0.1
		OnEvtConfSendMessage(event);
		break;
	case CONF_DX_SETSIPHEADER            :
		OnEvtConfSetSipHeader(event);
		break;
	case CONF_DX_GETSIPHEADER            :
		OnEvtConfGetSipHeader(event);
		break;
    default:
		break;
	}

	return Smt_Success;
}	

Smt_Uint TAsteriskDevice::OnCstaCallEvent(Csta_Event &event)
{
	switch (event.evttype)
	{
	case CSTA_CALLINITIATED   :
		OnEvtCallInitiated(event);
		break; 
	case CSTA_CALLDELIVERED   :
		OnEvtCallDelivered(event);
		break; 
	case CSTA_CALLCONNECTED   :
		OnEvtCallConnected(event);
		break; 
	case CSTA_CALLHELD        :
		OnEvtCallHeld(event);
		break; 
	case CSTA_CALLRETRIEVED   :
		OnEvtCallRetrieved(event);
		break; 
	case CSTA_CALLCONFERENCED :
		OnEvtCallConfrenced(event);
		break; 
	case CSTA_CALLQUEUED      :
		OnEvtCallQueued(event);
		break; 
	case CSTA_CALLTRANSFERRED :
		OnEvtCallTransfered(event);
		break; 
	case CSTA_CALLCLEARED     :
		OnEvtCallCleared(event);
		break; 
	case CSTA_CALLFAILED      :
		OnEvtCallFailed(event);
		break; 
	default:
		break;
	}
	return Smt_Success;
}		
Smt_Uint TAsteriskDevice::OnCstaDeviceEvent(Csta_Event &event)
{//设备事件 忽略
	
	return Smt_Success;
}		
Smt_Uint TAsteriskDevice::OnCstaRouteEvent(Csta_Event &event)
{//路由事件 忽略
	
	return Smt_Success;
}	
Smt_Uint TAsteriskDevice::OnCstaMediaEvent(Csta_Event &event)
{
	switch (event.evttype)
	{
	case CSTA_MEDIA_PLAYING   :
	//case CSTA_MEDIA_TTSPLAYING:
	//	OnEvtMediaPlaying(event);
		break;                   
	case CSTA_MEDIA_PLAYEND   :
	//case  CSTA_MEDIA_TTSPLAYEND:
	//	OnEvtMediaPlayEnd(event);
		break;                   
	case CSTA_MEDIA_SENDING   :
		OnEvtMediaSending(event);
		break;                   
	case CSTA_MEDIA_SENDEND   :
		OnEvtMediaSendEnd(event);
		break;                   
	case CSTA_MEDIA_GETING    :
		OnEvtMediaGeting(event);
		break;                   
	case CSTA_MEDIA_GETEND    :
		OnEvtMediaGetEnd(event);
		break;                   
	case CSTA_MEDIA_RECORDING :
		OnEvtMediaRecording(event);
		break;                   
	case CSTA_MEDIA_RECORDEND :
		OnEvtMediaRecordEnd(event);
		break; 
	case CSTA_MEDIA_MESSAGESENDING:
		break;
	case CSTA_MEDIA_MESSAGESENDEND://5.0.0.1
		OnEvtMediaMessageSendEnd(event);
		break;
	default:
		break;
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnCstaDeviceRecordEvent(Csta_Event &event)
{//座席事件 忽略
	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnCstaSystemEvent(Csta_Event &event)
{//当获取到linkup事件时订阅call事件
 //当获取到linkdown事件时 停止所有操作
	switch (event.evttype)
	{
	case CSTA_LINKDOWN:
		OnEvtLinkDown(event);
		break;
	case CSTA_LINKUP:
		OnEvtLinkUp(event);
		break;
	default:
		break;
	}	
	return Smt_Success;
}

Smt_Uint TAsteriskDevice::OnEvtLinkUp(Csta_Event &event)
{
//	m_LinkState = Smt_Success;
	if(cmSubscribeCallEvent() != Smt_Success)
	{
	}
	m_Ch0.PrintLog(3,"[TAsteriskDevice::OnEvtLinkUp] LinkUp.");
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtLinkDown(Csta_Event &event)
{
//	m_LinkState = Smt_Fail;
	m_EventState = Smt_Fail;
	//发出挂机消息 结束流程
	TCallData *pCall = NULL;
	for (CallMap::ITERATOR iter= m_CallMap.begin(); iter != m_CallMap.end(); iter++)
	{
		pCall = (*iter).int_id_;
		Smt_String strCallRefID = HLFormatStr("%d", pCall->m_CRN);
		EvtDisconnected(pCall->m_ChNo, strCallRefID, ActTpDisconnected, "ActTpDisconnected");
		pCall->m_pChInfo->m_ChStat = ChannelUnknown;//ChannelIdle;//zhangcc
		if(pCall != NULL)
		{//删除所有呼叫数据
			delete pCall;
			pCall = NULL;
		}
	}
	m_CallMap.RemoveAll();
	m_Ch0.PrintLog(3, "[TAsteriskDevice::OnEvtLinkDown] LinkDown Clear Data.");
	return Smt_Success;
}

Smt_Uint TAsteriskDevice::OnEvtConfSubscribe(Csta_Event &event)
{
	if(event.u.evtconf.u.confgcResponse.Reason == cmuK_Success)
	{
		if(m_EventState != Smt_Success)
		{
			m_EventState = Smt_Success;  
		    OpenConfigedDevices();
//			m_Ch0.PrintLog(4,"[TAsteriskDevice::OpenConfigedDevices] 1 ");
			ReportDeviceState(SERVICESTATE_NORMAL, 0, 0, "Subscribe Success");
//			m_Ch0.PrintLog(4,"[TAsteriskDevice::OpenConfigedDevices] 2 ");
		}	
	}
	else
	{
        m_EventState = Smt_Fail;
	}

	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfGcOpen(Csta_Event &event)
{//返回系统事件
	Smt_Uint nInvokeID = event.u.evtconf.u.confgc_open.InvokeID;
	Smt_Uint nReason = event.u.evtconf.u.confgc_open.Reason;
	TTrunkData *pTrunk = NULL;
	for (TrunkMap::ITERATOR iter= m_TrunkMap.begin(); iter != m_TrunkMap.end(); iter++)
	{
		pTrunk = (*iter).int_id_; 

		int i;
		for(i = 0; i < pTrunk->m_MatchCount; i++)
		{
			if(pTrunk->m_MatchData[i].m_nInvokeID == nInvokeID)
			{
				pTrunk->m_ChanID = event.u.evtconf.u.confgc_open.ChannelID;
				pTrunk->m_MatchData[i].m_nChanID = event.u.evtconf.u.confgc_open.ChannelID;
				//置所有子通道标识为可用
				TChannelData *pChData = NULL;
				ChannelMap::ITERATOR iter1(pTrunk->m_SubChInfo);
				for (iter1 = pTrunk->m_SubChInfo.begin(); iter1 != pTrunk->m_SubChInfo.end(); iter1++)
				{
					pChData = (*iter1).int_id_;
					if(nReason == cmuK_Success)
					{
						
						if(pChData->m_ChStat != ChannelIdle)
							EvtIdle(pChData->m_ChNo, "", ActNULL, "");
						pChData->m_ChStat = ChannelIdle;
						pChData->PrintLog(4, "[TAsteriskDevice::OnEvtConfGcOpen] Channel state is <IDLE>");
						
					}
					else
					{
						pChData->PrintLog(4, "[TAsteriskDevice::OnEvtConfGcOpen] Channel Open Fail.Reason<%d>", nReason);
					}				
				}
				if(pTrunk->m_SubChInfo.GetCount() != 0)
				{
					iter1 = pTrunk->m_SubChInfo.begin();
					pChData = (*iter1).int_id_;
					pTrunk->m_CurrentCh = pChData->m_ChNo;
				}
				break;
			}
		}
	}

	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfGcMakeCall(Csta_Event &event)
{
	Smt_Uint nInvokeID = event.u.evtconf.u.confgc_makecall.InvokeID;
	Smt_Uint nReason = event.u.evtconf.u.confgc_makecall.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgc_makecall.CallID;
	Smt_String strCallRefID = HLFormatStr("%d", nCRN);
	
	TTrunkData *pTrunk = NULL;	
	Smt_Uint nFind = Smt_Fail;
	for (TrunkMap::ITERATOR iter= m_TrunkMap.begin(); iter != m_TrunkMap.end(); iter++)
	{
		pTrunk = (*iter).int_id_;
		TChannelData *pChData = NULL;
		for (ChannelMap::ITERATOR iter1= pTrunk->m_SubChInfo.begin(); iter1 != pTrunk->m_SubChInfo.end(); iter1++)
		{
			pChData = (*iter1).int_id_;
			if(pChData->m_ChNo == nInvokeID)
			{
				if(nReason == cmuK_Success)
				{
					TCallData *pCall = NULL;
					pCall = new TCallData;
					pCall->m_ChNo = pChData->m_ChNo;
					pCall->m_CRN = nCRN;
					pCall->m_ResState = RES_NULL;
					pCall->m_TrunkDeviceID = pTrunk->m_DeviceID;
					pCall->m_pChInfo = pChData;
					pCall->m_MakeCall = MakeCallSuccess;
					m_CallMap.SetAt(nCRN, pCall);
					RespMakeCall(nInvokeID, Smt_Success);
					EvtProceeding(pCall->m_ChNo, strCallRefID, pChData->m_ANI, pChData->m_DNIS, CauseProceeding, "Proceeding");
				}
				else
				{
					pChData->m_ChStat = ChannelIdle;	
					RespMakeCall(nInvokeID, Smt_Fail);
				}

				pChData->PrintLog(4,"[TAsteriskDevice::OnEvtConfGcMakeCall] Crn<%d> Reason<%d>", nCRN, nReason);
				nFind = Smt_Success;
				break;
			}
		}
		if(nFind == Smt_Success)break;
	}

	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfGcAnswerCall(Csta_Event &event)
{
	Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success)
		{
			RespAnswerCall(pCall->m_ChNo, Smt_Success);
		}
		else
		{
            RespAnswerCall(pCall->m_ChNo, Smt_Fail);
		}
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtConfGcAnswerCall] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}

Smt_Uint TAsteriskDevice::OnEvtConfGcHangupCall(Csta_Event &event)
{
	Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success)
		{
			RespHangupCall(pCall->m_ChNo, Smt_Success);
		}
		else
		{
            RespHangupCall(pCall->m_ChNo, Smt_Fail);
		}
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtConfGcHangupCall] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfBlindTransfer (Csta_Event &event)
{
	Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success)
		{
			RespTransferCall(pCall->m_ChNo, Smt_Success);
		}
		else
		{
			RespTransferCall(pCall->m_ChNo, Smt_Fail);
		}
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtConfBlindTransfer] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfGcDial(Csta_Event &event)
{
    Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success)
		{
			RespTransferCall(pCall->m_ChNo, Smt_Success);
		}
		else
		{
			RespTransferCall(pCall->m_ChNo, Smt_Fail);
		}
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtConfGcDial] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfPlayIottData(Csta_Event &event)
{
	Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success)
		{
			RespPlay(pCall->m_ChNo, Smt_Success);
			EvtPlayBegin(pCall->m_ChNo);
		}
		else
		{
            RespPlay(pCall->m_ChNo, Smt_Fail); 
		}
        pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtConfPlayIottData] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfGetDig(Csta_Event &event)
{
	Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success)
		{
			RespGetDTMF(pCall->m_ChNo, Smt_Success);
			EvtGetDTMFBegin(pCall->m_ChNo);
		}
		else
		{
            RespGetDTMF(pCall->m_ChNo, Smt_Fail); 
		}
        pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtConfGetDig] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfDial(Csta_Event &event)
{
	Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success)
		{
			RespPlay(pCall->m_ChNo, Smt_Success);
			EvtPlayBegin(pCall->m_ChNo);
		}
		else
		{
            RespPlay(pCall->m_ChNo, Smt_Fail); 
		}
        pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtConfDial] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfRec(Csta_Event &event)
{
	Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success)
		{
			RespRecord(pCall->m_ChNo, Smt_Success);
			EvtRecordBegin(pCall->m_ChNo);
		}
		else
		{
            RespRecord(pCall->m_ChNo, Smt_Fail); 
		}
        pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtConfRec] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	
	return Smt_Success;
}

Smt_Uint TAsteriskDevice::OnEvtCallInitiated(Csta_Event &event)
{
	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtCallDelivered(Csta_Event &event)
{//区分呼入呼出
	//匹配呼入时的被叫号码
	Smt_String strANI  = Smt_String(event.u.evtcall.CallingParty);
	Smt_String strDNIS = Smt_String(event.u.evtcall.CalledParty);
	Smt_String strAlert = Smt_String(event.u.evtcall.AlertingParty);
	Smt_Uint   nCRN    = event.u.evtcall.CallID;
	Smt_String strCallRefID = HLFormatStr("%d", nCRN);
    TCallData *pCall = NULL;

	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		//已存在的呼叫 触发条件包括 makecall blindtransfercall
		//当makecall时 振铃方为被叫号码
		if(pCall->m_pChInfo->m_DNIS == strAlert && pCall->m_IsGcDail == GCDailNULL)
		{
			EvtAlerting(pCall->m_ChNo, strCallRefID, CauseAlerting, "Alerting");            
		}
		pCall->m_pChInfo->PrintLog(4, "[TAsteriskDevice::OnEvtCallDelivered] CRN<%d>", nCRN);
	}
	else
	{
		Smt_Uint nFind = Smt_Fail;
		TTrunkData *pTrunk = NULL;
		for (TrunkMap::ITERATOR iter= m_TrunkMap.begin(); iter != m_TrunkMap.end(); iter++)
		{
			pTrunk = (*iter).int_id_;

			if (strDNIS == "")
			{
				break;
			}

			int i;
			for(i = 0; i < pTrunk->m_MatchCount; i++)
			{
				if(pTrunk->m_MatchData[i].MatchResult(strDNIS) == Smt_Success)
				{
					nFind = Smt_Success;
					break;
				}
			}

			if(nFind == Smt_Success)
				break;

		//	if(strDNIS == pTrunk->m_DeviceID)
		//	{
		//		nFind = Smt_Success;
		//		break;
		//	}
		}
		if(nFind == Smt_Success)
		{//确认为呼入

			TChannelData *pChData = NULL;
			ChannelMap::ITERATOR iter1(pTrunk->m_SubChInfo);
			nFind = Smt_Fail;
			for (iter1 = pTrunk->m_SubChInfo.begin(); iter1 != pTrunk->m_SubChInfo.end(); iter1++)
			{
				//只找到当前通道的迭代器
				pChData = (*iter1).int_id_;
				if(pChData->m_ChNo == pTrunk->m_CurrentCh)
				{
					nFind = Smt_Success;
					break;
				}
			}

			if(nFind == Smt_Success)
			{
				nFind = Smt_Fail;
				ChannelMap::ITERATOR iter2(pTrunk->m_SubChInfo);
				for (iter2 = iter1;iter2 != pTrunk->m_SubChInfo.end(); iter2++)
				{
					pChData = (*iter2).int_id_;
					if(pChData->m_ChStat == ChannelIdle)
					{
						nFind = Smt_Success;
                        break;
					}
				}
				if(nFind != Smt_Success)
				{
					for (iter2 = pTrunk->m_SubChInfo.begin();iter2 != iter1; iter2++)
					{
						pChData = (*iter2).int_id_;
						if(pChData->m_ChStat == ChannelIdle)
						{
							nFind = Smt_Success;
							break;
						}
					}
				}
				if(nFind == Smt_Success)
				{

					//GetSipHeader 这里先取sipheader 防止answer的消息会覆盖getsipheader消息
					//if(m_gSipHeaderKey != "")
					//{
					//	dx_getsipheader(nCRN, (char*)(m_gSipHeaderKey.c_str()));
					//}

					pChData->m_ChStat = ChannelBusy;
					pChData->m_ANI = strANI;
					pChData->m_DNIS = strDNIS;
					TCallData *pCallData = NULL;
					pCallData = new TCallData;
					pCallData->m_ChNo = pChData->m_ChNo;
					pCallData->m_CRN = nCRN;
					pCallData->m_ResState = RES_NULL;
					pCallData->m_TrunkDeviceID = pTrunk->m_DeviceID;
					pCallData->m_pChInfo = pChData;
					m_CallMap.SetAt(nCRN, pCallData);
					
					EvtOffering(pCallData->m_ChNo, strCallRefID, strANI, strDNIS, CHANNELTYPE_SIP, "");
					pCallData->m_pChInfo->PrintLog(4, "[TAsteriskDevice::OnEvtCallDelivered] Call Offered. ANI<%s> DNIS<%s> CRN<%d>",
						strANI.c_str(), strDNIS.c_str(), nCRN);
				
					iter2++;
					if(iter2 == pTrunk->m_SubChInfo.end())
						iter2 = pTrunk->m_SubChInfo.begin();
					pChData = (*iter2).int_id_;
					pTrunk->m_CurrentCh = pChData->m_ChNo;

//					pCallData->m_TimerID = SetTimer(1000, nCRN);
				}
				else
				{
					gc_hangupcall(nCRN);//add by linyl at 2012-01-04
	            	m_Ch0.PrintLog(3,"[TAsteriskDevice::OnEvtCallDelivered] Max channel reached.Hangup this call.");
				}
			}
		}
		else
		{//寻找呼出信息
			
		}
	}
	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtCallConnected(Csta_Event &event)
{
	Smt_String strANI  = Smt_String(event.u.evtcall.CallingParty);
	Smt_String strDNIS = Smt_String(event.u.evtcall.CalledParty);
	Smt_String strAnswer = Smt_String(event.u.evtcall.AnsweringParty);
	TCallData *pCall = NULL;
	Smt_Uint nCRN = event.u.evtcall.CallID;
	Smt_String strCallRefID = HLFormatStr("%d", nCRN);
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(pCall->m_IsGcDail == GCDailInvoke)
		{//同振 共振对方应答 这里发送disconnect事件
			pCall->m_IsGcDail = GCDailConnect;
			pCall->m_pChInfo->m_ChStat = ChannelIdle;//zhangcc
			EvtTransfered(pCall->m_ChNo, strCallRefID, CauseConnected, "Call connected"); //linyl add at 2012-01-09             
			pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtCallConnected] Nomarl transfer success.CRN<%d>", nCRN);

			//延时1.5S发送disconnected事件
			pCall->m_TimerID = SetSingleTimer(1500, nCRN);

			return Smt_Success;
		}
		//
		Smt_Uint nCode;
		Smt_String strDesc;
		if(pCall->m_MakeCall == MakeCallSuccess)
		    pCall->m_MakeCall = MakeCallConnect;
		if(pCall->m_TrunkDeviceID == strAnswer)
		{
			nCode = ActTpAnswered;
			strDesc = "ActTpAnswered";
		}
		else
		{
			nCode = ActOpAnswered;
			strDesc = "ActOpAnswered";
		}
		EvtConnected(pCall->m_ChNo, strCallRefID, nCode, strDesc);
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtCallConnected] Call Connected.CRN<%d> %s", nCRN, strDesc.c_str());
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtCallHeld(Csta_Event &event)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtCallRetrieved(Csta_Event &event)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtCallConfrenced(Csta_Event &event)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtCallQueued(Csta_Event &event)
{
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtCallTransfered(Csta_Event &event)
{
	TCallData *pCall = NULL;
	Smt_Uint nCRN = event.u.evtcall.CallID;
	Smt_String strCallRefID = HLFormatStr("%d", nCRN);
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		EvtDisconnected(pCall->m_ChNo, strCallRefID, ActTpTransfered, "ActTpTransfered");
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtCallTransfered] Call Transfered.Channel IDLE.CRN<%d>", nCRN);
		pCall->m_pChInfo->m_ChStat = ChannelIdle;//zhangcc
		m_CallMap.Remove(nCRN);
		if(pCall != NULL)
		{
			delete pCall;
			pCall = NULL;
		}
	}
	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtCallCleared(Csta_Event &event)
{
	TCallData *pCall = NULL;
	Smt_Uint nCRN = event.u.evtcall.CallID;
	Smt_Uint nReason = event.u.evtcall.Reason;
	Smt_String strCallRefID = HLFormatStr("%d", nCRN);

	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		//同振 共振结束
		//if(pCall->m_IsGcDail == 2) return Smt_Success;

		Smt_Uint nTmp;
		Smt_String strBuff;
		pCall->m_pChInfo->m_ChStat = ChannelIdle;
			
		if(pCall->m_MakeCall == MakeCallSuccess)
		{
			nTmp = CauseMakeCallFail;
			strBuff = "MakeCallFail";
		}
		else if(pCall->m_TpDisconnect == 1)
		{
			nTmp = ActTpDisconnected;
	    	strBuff = "ActTpDisconnected";
		}
		else
		{
			nTmp = ActOpDisconnected;
	    	strBuff = "ActOpDisconnected";
		}

		EvtDisconnected(pCall->m_ChNo, strCallRefID, nTmp, strBuff);
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtCallCleared] Call Released.Channel state is <IDLE>.CRN<%d> Reason<%d>",
			                          pCall->m_CRN, nReason);
		m_CallMap.Remove(nCRN);
		if(pCall != NULL)
		{
			delete pCall;
			pCall = NULL;
		}
	}

// 	CallMap::ITERATOR iter(m_CallMap);
// 	for (CallMap::ENTRY *entry = 0;iter.next (entry) != 0; )
// 	{		
// 		pCall = entry->int_id_;
// 		iter.advance();
// 		if(pCall->m_CRN == nCRN)
// 		{
// 			pCall->m_pChInfo->m_ChStat = ChannelIdle;
// 			EvtDisconnected(pCall->m_ChNo, ActOpDisconnected, "OpDisconnected");
// 			pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtCallCleared] Call Released.CRN<%d>", pCall->m_CRN);
// 			m_CallMap.Remove(nCRN);
// 
// 			if(pCall != NULL)
// 			{
// 				delete pCall;
// 				pCall = NULL;
// 			}
// 			break;
// 		}
// 	}

	m_Ch0.PrintLog(5,"[TAsteriskDevice::OnEvtCallCleared] Current Call Number<%d>", m_CallMap.GetCount());
	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtCallFailed(Csta_Event &event)
{
	TCallData *pCall = NULL;
	Smt_Uint nCRN = event.u.evtcall.CallID;
	Smt_Uint nReason = event.u.evtcall.Reason;
	Smt_String strCallRefID = HLFormatStr("%d", nCRN);
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(pCall->m_IsGcDail == GCDailInvoke)
		{//外呼的电话不能做共振
			Smt_Uint nDail;
			Smt_String strDailDesc;
			pCall->m_IsGcDail = GCDailNULL;
			if(nReason == cmuK_DestBusy)
			{
				nDail = CauseMakeCallDestBusy;
                strDailDesc = "DestBusy";
			}
			else if(nReason == cmuK_DestInvalid)
			{
				nDail = CauseMakeCallDestInvalid;
                strDailDesc = "DestInvalid";
			}
			else if(nReason == cmuK_Timeout)
			{
				nDail = CauseMakeCallTimeOut;
                strDailDesc = "Timeout";
			}
			else
			{
				nDail = CauseMakeCallFail;
                strDailDesc = "Unkonwn";
			}
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_TransferCall, nDail, strDailDesc);
			//EvtConnected(pCall->m_ChNo, strCallRefID, nDail, strDailDesc);
			pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtCallFailed] Gc_Dail Fail. CRN<%d> Reason<%s>", nCRN, strDailDesc.c_str()); 
			return Smt_Success;
		}
		//
		if(pCall->m_MakeCall == MakeCallSuccess)
		{   
			Smt_Uint nFail;
			Smt_String strFail;
			switch (nReason)
			{
			case cmuK_DestBusy:
				nFail = CauseMakeCallDestBusy;
				strFail = "MakeCallDestBusy";
				break;
			case cmuK_CallRejected:
				nFail = CauseMakeCallRejected;
				strFail = "MakeCallRejected";
				break;
			default:
				nFail = CauseMakeCallFail;
				strFail = "MakeCallFail";
				break;
			}
            EvtDisconnected(pCall->m_ChNo, strCallRefID, nFail, strFail);
			pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtCallFailed] MakeCall Fail %s.Channel IDLE.CRN<%d> Reason<%d>", strFail.c_str(), nCRN, nReason);
		}
		else
		{
			EvtDisconnected(pCall->m_ChNo, strCallRefID, ActTpTransfered, "ActTpTransfered");
		    pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::OnEvtCallFailed] Call Transfered Fail.Channel IDLE.CRN<%d> Reason<%d>", nCRN, nReason);
		}		

		pCall->m_pChInfo->m_ChStat = ChannelIdle;//zhangcc
		m_CallMap.Remove(nCRN);
		if(pCall != NULL)
		{
			delete pCall;
			pCall = NULL;
		}
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtMediaPlaying(Csta_Event &event)
{	
//     Smt_Uint nCRN = event.u.evtmedia.CallRefID;
// 	Smt_Uint nReason = event.u.evtmedia.Reason;
// 	TCallData *pCall = NULL;
// 	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
// 	{
// 		EvtPlayBegin(pCall->m_ChNo);
// 	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtMediaPlayEnd(Csta_Event &event)
{	
	Smt_Uint nCRN = event.u.evtmedia.CallID;
	Smt_Uint nReason = event.u.evtmedia.Reason;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		pCall->m_ResState = RES_NULL;
		if(nReason == cmuK_AGISuccess || nReason == cmuK_AGIFail)
		{
            EvtPlayEnd(pCall->m_ChNo, CauseMediaEndNomal, "CauseMediaEndNomal");
		}
		else
		{
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_Play, CauseFuctionFail, "CauseFuctionFail");
			if(nReason == cmuK_WriteFileFail || nReason == cmuK_ReadFileFail)
				pCall->m_pChInfo->PrintLog(3,"[TAsteriskDevice::OnEvtMediaPlayEnd] File Read/Write Fail.");
			//EvtPlayEnd(pCall->m_ChNo, nReason, "");
		}
		pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtMediaPlayEnd] Crn<%d> Reason<%d>", nCRN, nReason);

		if(pCall->m_DtmfCache.m_flag == 1)
		{
			AkGetDTMF(pCall->m_ChNo, pCall->m_DtmfCache.m_maxlen, pCall->m_DtmfCache.m_starttime);
			pCall->m_DtmfCache.Reset();
		}
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtMediaSending(Csta_Event &event)
{	
// 	Smt_Uint nCRN = event.u.evtmedia.CallRefID;
// 	Smt_Uint nReason = event.u.evtmedia.Reason;
// 	TCallData *pCall = NULL;
// 	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
// 	{
// 		EvtPlayBegin(pCall->m_ChNo);
// 	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtMediaSendEnd(Csta_Event &event)
{	
	Smt_Uint nCRN = event.u.evtmedia.CallID;
	Smt_Uint nReason = event.u.evtmedia.Reason;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		pCall->m_ResState = RES_NULL;
		if(nReason == cmuK_AGISuccess || nReason == cmuK_AGIFail)
		{
            EvtPlayEnd(pCall->m_ChNo, CauseMediaEndNomal, "CauseMediaEndNomal");
		}
		else
		{
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_Play, CauseFuctionFail, "CauseFuctionFail");
		}
		pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtMediaSendEnd] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtMediaGeting(Csta_Event &event)
{	
// 	Smt_Uint nCRN = event.u.evtmedia.CallRefID;
// 	Smt_Uint nReason = event.u.evtmedia.Reason;
// 	TCallData *pCall = NULL;
// 	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
// 	{
// 		EvtGetDTMFBegin(pCall->m_ChNo);
// 	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtMediaGetEnd(Csta_Event &event)
{	
	Smt_Uint nCRN = event.u.evtmedia.CallID;
	Smt_Uint nReason = event.u.evtmedia.Reason;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		pCall->m_ResState = RES_NULL;
		if(nReason == cmuK_AGISuccess || nReason == cmuK_AGIFail || nReason == cmuK_Timeout)
		{
            EvtGetDTMFEnd(pCall->m_ChNo, event.u.evtmedia.DTMFDigits,CauseMediaEndDigit, "CauseMediaEndDigit");
		}
		else
		{
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_GetDTMF, CauseFuctionFail, "CauseFuctionFail");
		}
		pCall->m_DtmfCache.Reset();
		pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtMediaGetEnd] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtMediaRecording(Csta_Event &event)
{	
// 	Smt_Uint nCRN = event.u.evtmedia.CallRefID;
// 	Smt_Uint nReason = event.u.evtmedia.Reason;
// 	TCallData *pCall = NULL;
// 	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
// 	{
// 		EvtRecordBegin(pCall->m_ChNo);
// 	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtMediaRecordEnd(Csta_Event &event)
{	
	Smt_Uint nCRN = event.u.evtmedia.CallID;
	Smt_Uint nReason = event.u.evtmedia.Reason;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		pCall->m_ResState = RES_NULL;
		if(nReason == cmuK_AGISuccess || nReason == cmuK_AGIFail || nReason == cmuK_Timeout || nReason == cmuK_Released || nReason == cmuK_ReceiveDTMF)
		{
            EvtRecordEnd(pCall->m_ChNo, CauseMediaEndNomal, "CauseMediaEndNomal");
		}
		else
		{
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_Record, CauseFuctionFail, "CauseFuctionFail");
			if(nReason == cmuK_WriteFileFail || nReason == cmuK_ReadFileFail)
				pCall->m_pChInfo->PrintLog(3,"[TAsteriskDevice::OnEvtMediaRecordEnd] File Read/Write Fail.");
		}
		pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtMediaRecordEnd] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}

Smt_Uint TAsteriskDevice::OpenConfigedDevices()
{
	TTrunkData *pTrunk = NULL;	
	for (TrunkMap::ITERATOR iter= m_TrunkMap.begin(); iter != m_TrunkMap.end(); iter++)
	{
		pTrunk = (*iter).int_id_;

		int i;
		for(i = 0; i < pTrunk->m_MatchCount; i++)
		{
			Smt_Uint nInvokeID = pTrunk->m_MatchData[i].m_nInvokeID;
			Smt_String strDeviceID = pTrunk->m_MatchData[i].m_strDeviceID;
			Smt_Uint nDeviceType = pTrunk->m_DeviceType;
			gc_open(nInvokeID, (char *)(strDeviceID.c_str()), nDeviceType);
			m_Ch0.PrintLog(4,"[TAsteriskDevice::OpenConfigedDevices] Open Device<%s> InvokeID<%d> Type<%d>", 
			              strDeviceID.c_str(), nInvokeID, nDeviceType);
		}
		//gc_open(pTrunk->m_InvokeID, (char *)(pTrunk->m_DeviceID.c_str()), pTrunk->m_DeviceType);
		//m_Ch0.PrintLog(4,"[TAsteriskDevice::OpenConfigedDevices] Open Device<%s> InvokeID<%d> Type<%d>", 
		//	              pTrunk->m_DeviceID.c_str(), pTrunk->m_InvokeID, pTrunk->m_DeviceType);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::CloseConfigedDevices()
{
	TTrunkData *pTrunk = NULL;
	for (TrunkMap::ITERATOR iter= m_TrunkMap.begin(); iter != m_TrunkMap.end(); iter++)
	{
		pTrunk = (*iter).int_id_;

		int i;
		for (i = 0; i < pTrunk->m_MatchCount; i++)
		{
			gc_close(pTrunk->m_MatchData[i].m_nChanID);	
		}		
	}

    m_Ch0.PrintLog(4,"[TAsteriskDevice::CloseConfigedDevices] Close All Configed Devices.");
	return Smt_Success;
}

//////////////////////////////////////////////////////////////////////////
Smt_Uint TAsteriskDevice::AnswerCall(Smt_Uint channel, Smt_Uint rings)
{	
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		gc_answercall(pCall->m_CRN);
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::AnswerCall] Answer Call.CRN<%d>", pCall->m_CRN);
	}
	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::MakeCall(Smt_Uint channel, Smt_String ani, Smt_String dnis)
{	
	TTrunkData *pTrunk = NULL;
	for (TrunkMap::ITERATOR iter= m_TrunkMap.begin(); iter != m_TrunkMap.end(); iter++)
	{
		pTrunk = (*iter).int_id_;
		TChannelData *pChData = NULL;
		for (ChannelMap::ITERATOR iter1= pTrunk->m_SubChInfo.begin(); iter1 != pTrunk->m_SubChInfo.end(); iter1++)
		{
			pChData = (*iter1).int_id_;
			if(pChData->m_ChNo == channel)
			{
				if(pChData->m_ChStat == ChannelIdle)
				{
					//以通道号码作为InvokeID
					pChData->m_ChStat = ChannelBusy;
					pChData->m_ANI = ani;
					pChData->m_DNIS = dnis;
					gc_makecall(pChData->m_ChNo, (char*)(pTrunk->m_DeviceID.c_str()), (char*)(dnis.c_str()), (char*)(ani.c_str()), 60000);
				}
				pChData->PrintLog(5,"[TAsteriskDevice::MakeCall] InvokeID<%d> Device<%s> ANI<%s> DNIS<%s>", pChData->m_ChNo,pTrunk->m_DeviceID.c_str(),ani.c_str(),dnis.c_str());
                return Smt_Success;
			}
		}
	}

	return Smt_Success;
}
Smt_Uint TAsteriskDevice::HangupCall(Smt_Uint channel)
{	
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		pCall->m_TpDisconnect = 1;
		gc_hangupcall(pCall->m_CRN);
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::HangupCall] Hangup Call.CRN<%d>", pCall->m_CRN);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::TransferCall(Smt_Uint channel, Smt_Uint transfertype, Smt_String ani, Smt_String dnis, Smt_Uint nTransTimeOut)
{	
	switch (transfertype)
	{
	case TRANSFERTYPE_BLIND:
		BlindTransfer(channel, ani, dnis);
		break;
	case TRANSFERTYPE_SUPER:
		NormalTransfer(channel, ani, dnis, nTransTimeOut);
		break;
	default:
		break;
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::BlindTransfer(Smt_Uint channel, Smt_String ANI, Smt_String DNIS)
{
    TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		gc_blindtransfer(pCall->m_CRN, (char*)(DNIS.c_str()));
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::BlindTransfer] Call BlindTransfer.CRN<%d> DNIS<%s>", pCall->m_CRN, DNIS.c_str());
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::NormalTransfer(Smt_Uint channel, Smt_String ANI, Smt_String DNIS, Smt_Uint nTransTimeOut)
{
    TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		gc_dial(pCall->m_CRN, (char*)(DNIS.c_str()), nTransTimeOut);
		pCall->m_IsGcDail = GCDailInvoke;
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::NormalTransfer] Call NormalTransfer.CRN<%d> DNIS<%s> nTransTimeOut<%d>", pCall->m_CRN, DNIS.c_str(), nTransTimeOut);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ModifyCall(Smt_Uint channel, Smt_String audioaddr, Smt_Uint audioport, Smt_Uint audiotype, Smt_String videoaddr, Smt_Uint videoport, Smt_Uint videotype)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::HoldCall(Smt_Uint channel)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::RetrieveCall(Smt_Uint channel)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ConChannel(Smt_Uint firstch, Smt_Uint secondch, Smt_Uint mode)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::DisconChannel(Smt_Uint firstch, Smt_Uint secondch)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::PlayMedia(Smt_Uint channel, Smt_Uint type, Smt_String termkey, Smt_String content, Smt_Kvset* pMemArray, Smt_Uint nMemCount, Smt_Pdu pdu)
{	
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{		
		pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::PlayMedia] CRN<%d> channel<%d> type<%d> termkey<%s>  Content<%s>", pCall->m_CRN, channel, type, termkey.c_str(), content.c_str());
	}
	if(type == CONTENTTYPE_TTS)
	{//asterisk memory play support?
		
		Smt_String strContent = Smt_String(content);
		int nPos = strContent.find(COMBOTYPE_TTS);
		if (nPos > 0)
		{
			Smt_String	strBuff = strContent.substr(0, nPos);
			AkPlayTTS(channel,termkey,strBuff);
		}
	}
	else if(type == CONTENTTYPE_COMBO)
	{
		int nPos;
		Smt_String strContent = Smt_String(content);
		Smt_String strBuff;
		
		if((nPos = strContent.find(COMBOTYPE_VIDEOFILE)) > 0)
		{//asterisk video play support?
			
		}
		else if((nPos = strContent.find(COMBOTYPE_SENDDTMF)) > 0)
		{
			strBuff = strContent.substr(0, nPos);
			AkPlayDTMF(channel, strBuff);
		}
		else if((nPos = strContent.find(COMBOTYPE_SENDMSG)) > 0)//5.0.0.1
		{
			strBuff = strContent.substr(0, nPos);
			AkPlayMessage(channel, strBuff);
		}
		else
		{
			AkPlayFile(channel, termkey, content);
		}
	}
	else
	{
		AkPlayFile(channel, termkey, content);
	}

	return Smt_Success;
}
Smt_Uint TAsteriskDevice::RecordMedia(Smt_Uint channel, Smt_Uint type, Smt_String termkey,Smt_String content, Smt_Uint rectime, Smt_Uint silenttime, Smt_Uint recdouble)
{	
	switch(type)
	{
	case RECTYPE_AUDIOFILE:
		{
			if(!recdouble)
				AkRecFile(channel, termkey, content, rectime, silenttime);
			else
			{
				//DxRecDouble(channel, termkey, content, rectime, slienttime);
			}
		}	
		break;
	case RECTYPE_VIDEOFILE:
		//MMRecVideo(channel, termkey, content);
		break;
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::GetDTMF(Smt_Uint channel, Smt_Uint maxlen, Smt_Uint starttime, Smt_Uint innertime, Smt_String termkey)
{	
	AkGetDTMF(channel, maxlen, innertime);
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::RecvFax(Smt_Uint channel, Smt_String filename)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::SendFax(Smt_Uint channel, Smt_String filename, Smt_Uint startpage, Smt_Uint pagenum)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::StopMedia(Smt_Uint channel)
{	
	EvtPlayEnd(channel, CauseMediaEndNomal, "CauseMediaEndNomal");
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::RtpInit(Smt_Uint channel, Smt_String audioaddr, Smt_Uint audioport, Smt_Uint audiotype, Smt_String videoaddr, Smt_Uint videoport, Smt_Uint videotype)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::RtpClose(Smt_Uint channel)
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::SetSipHead(Smt_Uint channel, Smt_String sipheadkey, Smt_String sipheadvalue)
{
	int nRet = Smt_Fail;
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		if(m_gSipHeaderKey != "")
		{
			nRet = Smt_Success;
			pCall->m_SipHeaderKey = m_gSipHeaderKey;
			dx_setsipheader(pCall->m_CRN, (char*)(m_gSipHeaderKey.c_str()), (char*)(sipheadvalue.c_str()));
		}
		else
		{
			RespSetSipHead(pCall->m_ChNo, Smt_Fail, "", "");
		}
		pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::SetSipHead] CRN<%d>,nRet<%d> key<%s>, value<%s>.", pCall->m_CRN, nRet, m_gSipHeaderKey.c_str(), sipheadvalue.c_str());

	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::GetSipHead(Smt_Uint channel, Smt_String sipheadkey, Smt_String sipheadvalue)
{
//GetSipHeader 需要在answer之前调用，否则会覆盖getsipheader消息而取不到

	int nRet = Smt_Fail;
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		if(m_gSipHeaderKey != "")
		{
			//若此呼叫上已取过SipHead,则直接返回成功.
			if(pCall->m_SipHeaderValue != "")
			{
				RespGetSipHead(pCall->m_ChNo, Smt_Success, m_gSipHeaderKey, pCall->m_SipHeaderValue);
				pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::GetSipHead] Success get cache value.CRN<%d>,nRet<%d> key<%s> value<%s>.", pCall->m_CRN , nRet, m_gSipHeaderKey.c_str(), pCall->m_SipHeaderValue.c_str());
			}
			else
			{
				dx_getsipheader(pCall->m_CRN, (char*)(m_gSipHeaderKey.c_str()));
				pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::GetSipHead] CRN<%d>,nRet<%d> key<%s> value<%s>.", pCall->m_CRN , nRet, m_gSipHeaderKey.c_str(), pCall->m_SipHeaderValue.c_str());		
			}
			
			nRet = Smt_Success;
		}
		else
		{
			RespGetSipHead(pCall->m_ChNo, Smt_Fail, m_gSipHeaderKey, pCall->m_SipHeaderValue);
			pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::GetSipHead] Fail.CRN<%d>,nRet<%d> key<%s> value<%s>.", pCall->m_CRN , nRet, m_gSipHeaderKey.c_str(), pCall->m_SipHeaderValue.c_str());		
		}

		//pCall->m_pChInfo->PrintLog(4,"[TAsteriskDevice::GetSipHead] CRN<%d>,nRet<%d> key<%s> value<%s>.", pCall->m_CRN , nRet, m_gSipHeaderKey.c_str(), pCall->m_SipHeaderValue.c_str());		
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ConfCreate()
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ConfAddParty()
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ConfRemParty()
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ConfDelete()
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ConfSetAttr()
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ConfGetAttr()
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ConfSetPartyAttr()
{	
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::ConfGetPartyAttr()
{	
	return Smt_Success;
}

//////////////////////////////////////////////////////////////////////////
/****************************************************************************
函 数 名: FillDateIott
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillDateIott(Smt_Uint channel, Smt_String datestr)
{
	Smt_String strBuff = datestr;
	bool bFillFileFlag = false;
	if(strBuff.length() < 8 ) return bFillFileFlag;

	Smt_String strYear = strBuff.substr(0, 4);
	Smt_String strMon  = strBuff.substr(4, 2);
	Smt_String strDay  = strBuff.substr(6, 2);

	bFillFileFlag = FillDigitIott(channel, strYear);
	bFillFileFlag = FillFileIott(channel, Smt_String(DATE_YEAR), 0);

	bFillFileFlag = FillIntegerIott(channel, strMon);
	bFillFileFlag = FillFileIott(channel, Smt_String(DATE_MONTH), 0);

	bFillFileFlag = FillIntegerIott(channel, strDay);
	bFillFileFlag = FillFileIott(channel, Smt_String(DATE_DAY), 0);

	int nPos = strBuff.find("/");
	if(nPos > 0)
	{
		Smt_String strWeek = strBuff.substr(nPos+1, strBuff.length() - nPos -1);

		bFillFileFlag = FillFileIott(channel, Smt_String(DATE_WEEK), 0);
		bFillFileFlag = FillDigitIott(channel, strWeek);
	}

	return bFillFileFlag;
}

/****************************************************************************
函 数 名: FillDateIott
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillMoneyIott(Smt_Uint channel, Smt_String strMoney)
{
	Smt_String strBuff = strMoney;
	bool bFillFileFlag = false;
	int nRadixPoint;
	Smt_String strInteger, strDecimal;
	Smt_String strFileName;

	if((strBuff.length() < 1) || (strBuff == Smt_String("0")))
	{
		return FillFileIott(channel, Smt_String(DIGIT_0), 0);
	}
	Smt_String strTmp = strBuff.substr(0, 1);
	if(strTmp == Smt_String("-") && strBuff.length() >= 2)
	{
		strBuff = strBuff.substr(1, strBuff.length() -1);
		bFillFileFlag=FillFileIott(channel,Smt_String(SYMBOL_MINUS),0);
	}

	nRadixPoint = strBuff.find(".");	
	if(nRadixPoint == -1)
	{
		strInteger = strBuff;
	}	
	else
	{
		strInteger = strBuff.substr(0, nRadixPoint);
		strDecimal = strBuff.substr(nRadixPoint+1, strBuff.length() - nRadixPoint - 1);
	}

	int nDecimalLen=strDecimal.length();
	if(nDecimalLen>0)
	{
		if(nDecimalLen > 2)
		{
			strDecimal = strDecimal.substr(0, 2);
			nDecimalLen = 2;
		}
	}


	if(ACE_OS::atoi(strInteger.c_str()) != 0)
	{	
		bFillFileFlag=FillIntegerIott_Ex(channel,strInteger);
		bFillFileFlag=FillFileIott(channel,Smt_String(MONEY_BUCK),0);
	}
	else if((nDecimalLen<=0)||(ACE_OS::atoi(strDecimal.c_str())==0))
	{
		bFillFileFlag=FillFileIott(channel,Smt_String(DIGIT_0),0);
		bFillFileFlag=FillFileIott(channel,Smt_String(MONEY_BUCK),0);
		return bFillFileFlag;
	}	

	Smt_String strTmp1 = strDecimal.substr(0,1);
	Smt_String strTmp2;
	if(nDecimalLen == 2)
	    strTmp2 = strDecimal.substr(1,1);	

	if(ACE_OS::atoi(strTmp1.c_str())>0)
	{
		strFileName=ConvertDigitToFileName(ACE_OS::atoi(strTmp1.c_str()));
		if(strFileName.length() > 2)
		{
			bFillFileFlag=FillFileIott(channel,strFileName,0);
			bFillFileFlag=FillFileIott(channel,Smt_String(MONEY_DIME),0);
		}
	}
	
	if((nDecimalLen > 1) && ACE_OS::atoi(strTmp2.c_str())>0)
	{	
		strFileName=ConvertDigitToFileName(ACE_OS::atoi(strTmp2.c_str()));
		if(strFileName.length() > 2)
		{
			bFillFileFlag=FillFileIott(channel,strFileName,0);
			bFillFileFlag=FillFileIott(channel,Smt_String(MONEY_CENT),0);
		}
	}

	return bFillFileFlag;
}

/****************************************************************************
函 数 名: FillDateIott
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillNumberIott(Smt_Uint channel, Smt_String strNumber)
{
	Smt_String strBuff = strNumber;
	bool bFillFileFlag = false;
	int nRadixPoint;
	Smt_String strInteger, strDecimal;
	Smt_String strFileName;

	if((strBuff.length() < 1) || (strBuff == Smt_String("0")))
	{
		return FillFileIott(channel, Smt_String(DIGIT_0), 0);
	}
	Smt_String strTmp = strBuff.substr(0, 1);
	if(strTmp == Smt_String("-") && strBuff.length() >= 2)
	{
		strBuff = strBuff.substr(1, strBuff.length() -1);
		bFillFileFlag=FillFileIott(channel,Smt_String(SYMBOL_MINUS),0);
	}

	nRadixPoint = strBuff.find(".");
	if(nRadixPoint == -1)
	{
		strInteger = strBuff;
	}	
	else
	{
		strInteger = strBuff.substr(0, nRadixPoint);
		strDecimal = strBuff.substr(nRadixPoint+1, strBuff.length() - nRadixPoint - 1);
	}

	if(strInteger.length() <= 0)
	{
		FillFileIott(channel,Smt_String(DIGIT_0),0);
		if(ACE_OS::atoi(strDecimal.c_str()) <= 0)
		{//如果小数为0，直接返回
			return true;
		}
	}
	else
	{
		bFillFileFlag = FillIntegerIott_Ex(channel,strInteger);
		if(!bFillFileFlag)
		{
			return false;
		}
		
	}

	/*
	int nDecimalLen=strDecimal.length();
	if(nDecimalLen>0)
	{
		if(nDecimalLen > 2)
		{
			strDecimal = strDecimal.substr(0, 2);
			nDecimalLen = 2;
		}
		
		if(ACE_OS::atoi(strDecimal.c_str()) > 0)
		{
			Smt_String strTmp1 = strDecimal.substr(0, 1);
			Smt_String strTmp2;
			if(nDecimalLen == 2)
				strTmp2 = strDecimal.substr(1, 1);
			bFillFileFlag = FillFileIott(channel,Smt_String(SYMBOL_DOT),0);
			strFileName=ConvertDigitToFileName(ACE_OS::atoi(strTmp1.c_str()));
			if(strFileName.length() > 2)
			{
				bFillFileFlag=FillFileIott(channel,strFileName,0);
			}
			
			if(nDecimalLen == 2 && ACE_OS::atoi(strTmp2.c_str())>0)
			{
				strFileName=ConvertDigitToFileName(ACE_OS::atoi(strTmp2.c_str()));
				if(strFileName.length() > 2)
				{
					bFillFileFlag=FillFileIott(channel,strFileName,0);
				}
			}
		}
	}
	*/
	//////////////////////////////////////////////////////////////////////////
	int nDecimalLen=strDecimal.length();
	if(nDecimalLen>0)
	{
		if(nDecimalLen > m_nDotLen)
		{
			strDecimal = strDecimal.substr(0, m_nDotLen);
			nDecimalLen = m_nDotLen;
		}
		if(ACE_OS::atoi(strDecimal.c_str()) > 0)
		{
			bFillFileFlag = FillFileIott(channel,Smt_String(SYMBOL_DOT),0);
			int i;
			for(i = 0; i < nDecimalLen; i++)
			{
				Smt_String strTmp1 = strDecimal.substr(i, 1);
				Smt_String strTmp2 = strDecimal.substr(i, nDecimalLen-i);
				//防止后面的数字都为0
				if(ACE_OS::atoi(strTmp2.c_str()) >0)
				{
					strFileName=ConvertDigitToFileName(ACE_OS::atoi(strTmp1.c_str()));
					if(strFileName.length() > 2)
					{
						bFillFileFlag=FillFileIott(channel,strFileName,0);
					}
				}								
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////

	return bFillFileFlag;
}

/****************************************************************************
函 数 名: FillDateIott
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillIntegerIott(Smt_Uint channel, Smt_String strInteger, bool bZero)
{
	Smt_String strBuff = strInteger;
	int nIntegerLen = strBuff.length();
    bool	 bFillFileFlag = false;
	Smt_String strLeftNumber;
	Smt_String strSingleDigit;
	Smt_String strDigitFileName;
	Smt_String strPreDigit = Smt_String("N");

	if(bZero)
	{
		strPreDigit = Smt_String("Y");
	}

	int i;
	for(i = 0; i < nIntegerLen; i++)
	{
		strLeftNumber = strBuff.substr(i, nIntegerLen - i);

		if(ACE_OS::atoi(strLeftNumber.c_str()) == 0)
		{
			break;
		}

		strSingleDigit = strBuff.substr(i, 1);
		if(strSingleDigit != Smt_String("0"))
		{
			if((strSingleDigit != Smt_String("1")) || (nIntegerLen != 2)||(i != 0))
			{
				strDigitFileName=ConvertDigitToFileName(ACE_OS::atoi(strSingleDigit.c_str()));
				if(strDigitFileName.length() > 2)
				{
					bFillFileFlag = FillFileIott(channel, strDigitFileName, 0);
				}
			}
			
			if(nIntegerLen-i != 1)
			{
				strDigitFileName=ConvertPosToUnitName(nIntegerLen-i);
				bFillFileFlag = FillFileIott(channel, strDigitFileName, 0);
			}
			strPreDigit=strSingleDigit;
		}
		//ingnore left 0
		else 
		{
			if((strPreDigit != Smt_String("0")) && (strPreDigit != Smt_String("N")))
			{
				bFillFileFlag = FillFileIott(channel,Smt_String(DIGIT_0),0);
			}
			
			strPreDigit=strSingleDigit;
		}
	}
	return bFillFileFlag;
}

/****************************************************************************
函 数 名: FillDateIott
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillIntegerIott_Ex(Smt_Uint channel, Smt_String strInteger)
{
    Smt_String strBuff = Smt_String(strInteger);
	Smt_String strDigitFileName;
	bool	 bFillFileFlag=false;
	
	int nCount = strBuff.length();
	int nLeft = nCount%4;
	int nIndex = nCount/4;
	
	if(nLeft == 0)
	{
		nLeft = 4;
		nIndex --;
	}
	Smt_String strTmp;
	int i=0;
	while(nIndex >= 0)
	{
		strTmp = strBuff.substr(i, nLeft);
		i += nLeft;
		nLeft = 4;
		
		if(ACE_OS::atoi(strTmp.c_str()) == 0)
		{
			nIndex --;
			continue;
		}
		
		if(nIndex == 2)
		{
			bFillFileFlag = FillIntegerIott(channel, strTmp);
		}
		else
		{
			bFillFileFlag = FillIntegerIott(channel, strTmp, true);
		}
		
		if(nIndex == 2)
		{
			strDigitFileName=ConvertPosToUnitName(9);
			bFillFileFlag = FillFileIott(channel,strDigitFileName,0);
		}
		else if(nIndex == 1)
		{
			strDigitFileName=ConvertPosToUnitName(5);
			bFillFileFlag = FillFileIott(channel,strDigitFileName,0);
		}	
		nIndex --;	
	}
	
	return bFillFileFlag;
}

/****************************************************************************
函 数 名: FillDateIott
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillFileIott(Smt_Uint channel, Smt_String strFileName, Smt_Uint nFileLibType)
{
	Smt_String filename;
	Smt_String strTmp;
	if(nFileLibType == 0)//库文件
	{
		GetFileName(filename, strFileName, m_WavLibPath);
	}
	else if(nFileLibType == 1)//用户语音文件
	{
        GetFileName(filename, strFileName, m_DefaultWavPath);
	}
	else
	{
	}

	//去掉文件后缀 .wav .vox .mp3
	if(filename.length() > 4)
	{
		strTmp = filename.substr(filename.length() - 4, 4);
		
		if((ACE_OS::strcasecmp(strTmp.c_str(), ".wav") == 0) || (ACE_OS::strcasecmp(strTmp.c_str(), ".mp3") == 0) ||
			(ACE_OS::strcasecmp(strTmp.c_str(), ".vox") == 0))
		{
         
			filename = filename.substr(0, filename.length() - 4);		
		}

	}
	

	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		dx_addiottdata(pCall->m_CRN, (char*)(filename.c_str()));
		pCall->m_pChInfo->PrintLog(5, "[TAsteriskDevice::FillFileIott] Add File<%s>", filename.c_str());
		//dx_addiottdata(pCall->m_CRN, (char*)(strFileName.c_str()));
		return true;
	}
	return false;
}

/****************************************************************************
函 数 名: FillDateIott
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillDigitIott(Smt_Uint channel, Smt_String strDigit)
{
	char strBuff[MAXBUFF] = "";
	ACE_OS::strncpy(strBuff, strDigit.c_str(), MAXBUFF);
	bool	 bFillFileFlag=false;
	char *p = strBuff;
	while(*p != '\0')
	{
		Smt_String strFile;
		switch(*p)
		{
		case '-':
			strFile = Smt_String(SYMBOL_MINUS);
			break;
		case '<':
			strFile = Smt_String("Small_Char");
			break;
		case '>':
			strFile = Smt_String("Big_Char");
			break;
		case '*':
			strFile = Smt_String(SYMBOL_STAR);
			break;
		case '#':
			strFile = Smt_String(SYMBOL_WELL);
			break;
		case '0':
			strFile = Smt_String(DIGIT_0);
			break;
		case '1':
			strFile = Smt_String(DIGIT_1);
			break;
		case '2':
			strFile = Smt_String(DIGIT_2);
			break;
		case '3':
			strFile = Smt_String(DIGIT_3);
			break;
		case '4':
			strFile = Smt_String(DIGIT_4);
			break;
		case '5':
			strFile = Smt_String(DIGIT_5);
			break;
		case '6':
			strFile = Smt_String(DIGIT_6);
			break;
		case '7':
			strFile = Smt_String(DIGIT_7);
			break;
		case '8':
			strFile = Smt_String(DIGIT_8);
			break;
		case '9':
			strFile = Smt_String(DIGIT_9);
			break;
		case '.':
			strFile = Smt_String(SYMBOL_DOT);
			break;
		default:
			if(((*p >= 'A') && (*p <= 'Z')) || ((*p >= 'a') && (*p <= 'z')))
			{
				strFile = HLFormatStr("char_%c", *p);
			}
			break;
			
		}

		if(strFile.length() > 1)
		{
			bFillFileFlag = FillFileIott(channel, strFile, 0);
		}
		p++;
	}
    

 	return bFillFileFlag;
}

/****************************************************************************
函 数 名: FillDateIott
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillPlayStringIott(Smt_Uint channel, Smt_Uint FileType, Smt_String FileName)
{
	bool bFillFlag = false;
	switch (FileType)
	{
	case COMBO_BOOLEAN:
		bFillFlag = FillBoolIott(channel, FileName);
		break;
	case COMBO_DATE:   
		bFillFlag = FillDateIott(channel, FileName);
		break;
	case COMBO_DIGITS:
		bFillFlag = FillDigitIott(channel, FileName);		
		break;
	case COMBO_CURRENCY: 
		bFillFlag = FillMoneyIott(channel, FileName);
		break;
	case COMBO_NUMBER:  
		bFillFlag = FillNumberIott(channel, FileName);
		break;
	case COMBO_PHONE: 
		bFillFlag = FillPhoneIott(channel, FileName);
		break;
	case COMBO_TIME:
		bFillFlag = FillTimeIott(channel, FileName);	
		break;
	case COMBO_AUDIOFILE:
        bFillFlag = FillFileIott(channel, FileName, 1);
		break;
	}
	return bFillFlag;
}

/****************************************************************************
函 数 名: GetFilePlayType
参    数:
返回数值: 
功能描述: 获取播放组合串类型
*****************************************************************************/
Smt_Uint TAsteriskDevice::GetFilePlayType(Smt_String strType)
{
	Smt_Uint nType = COMBO_UNKNOW;
	if(strType == Smt_String(COMBOTYPE_BOOLEAN))
		nType = COMBO_BOOLEAN;
	else if(strType == Smt_String(COMBOTYPE_DATE))
		nType = COMBO_DATE;
	else if(strType == Smt_String(COMBOTYPE_DIGITS))
		nType = COMBO_DIGITS;
	else if(strType == Smt_String(COMBOTYPE_CURRENCY))
		nType = COMBO_CURRENCY;
	else if(strType == Smt_String(COMBOTYPE_NUMBER))
		nType = COMBO_NUMBER;
	else if(strType == Smt_String(COMBOTYPE_PHONE))
		nType = COMBO_PHONE;
	else if(strType == Smt_String(COMBOTYPE_TIME))
		nType = COMBO_TIME;
	else if(strType == Smt_String(COMBOTYPE_AUDIOFILE))
		nType = COMBO_AUDIOFILE;
	else
		nType = COMBO_UNKNOW;

	return nType;
}

/****************************************************************************
函 数 名: ConvertDigitToFileName
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
Smt_String TAsteriskDevice::ConvertDigitToFileName(Smt_Uint digit)
{
	Smt_String digit_file_name;
	
	switch(digit)
	{
	case 0:
		digit_file_name=Smt_String(DIGIT_0);
		break;	
	case 1:	
		digit_file_name=Smt_String(DIGIT_1);
		break;	
	case 2:
		digit_file_name=Smt_String(DIGIT_2);
		break;	
	case 3:	
		digit_file_name=Smt_String(DIGIT_3);
		break;
	case 4:
		digit_file_name=Smt_String(DIGIT_4);
		break;
	case 5:
		digit_file_name=Smt_String(DIGIT_5);
		break;
	case 6:
		digit_file_name=Smt_String(DIGIT_6);
		break;
	case 7:
		digit_file_name=Smt_String(DIGIT_7);
		break;
	case 8:
		digit_file_name=Smt_String(DIGIT_8);
		break;
	case 9:
		digit_file_name=Smt_String(DIGIT_9);
		break;
	case '*':
		digit_file_name=Smt_String(SYMBOL_STAR);
		break;
	case '#':
		digit_file_name=Smt_String(SYMBOL_WELL);
		break;
	case '-':
		digit_file_name=Smt_String(SYMBOL_MINUS);
		break;
	case '.':
		digit_file_name=Smt_String(SYMBOL_DOT);
		break;
	}
	return digit_file_name;

}

/****************************************************************************
函 数 名: ConvertDigitToFileName
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
Smt_String TAsteriskDevice::ConvertPosToUnitName(Smt_Uint pos)
{
	Smt_String unit_file_name;
	switch(pos)
	{
	case 2://十ten
		unit_file_name=Smt_String(UNIT_TEN);
		break;
	case 3://百hundred
		unit_file_name=Smt_String(UNIT_HUNDRED);
		break;
	case 4://千thousand
		unit_file_name=Smt_String(UNIT_THOUSAND);
		break;
	case 5://万ten_thousand
		unit_file_name=Smt_String(UNIT_TEN_THOUSAND);
		break;
	case 6:
		unit_file_name=Smt_String(UNIT_TEN);
		break;
	case 7:
		unit_file_name=Smt_String(UNIT_HUNDRED);
		break;
	case 8:
		unit_file_name=Smt_String(UNIT_THOUSAND);
		break;
	case 9:
		unit_file_name=Smt_String(UNIT_MEGA);
		break;
	case 10:
		unit_file_name=Smt_String(UNIT_TEN);
		break;
	case 11:
		unit_file_name=Smt_String(UNIT_HUNDRED);
		break;
	case 12:
		unit_file_name=Smt_String(UNIT_THOUSAND);
		break;
	case 13://万ten_thousand
		unit_file_name=Smt_String(UNIT_TEN_THOUSAND);
		break;
	case 14:
		unit_file_name=Smt_String(UNIT_TEN);
		break;
	case 15:
		unit_file_name=Smt_String(UNIT_HUNDRED);
		break;
	case 16:
		unit_file_name=Smt_String(UNIT_THOUSAND);
		break;
	case 17:
		unit_file_name=Smt_String(UNIT_MEGA);
		break;
	case 18:
		unit_file_name=Smt_String(UNIT_TEN);
		break;
	case 19:
		unit_file_name=Smt_String(UNIT_HUNDRED);
		break;
	case 20:
		unit_file_name=Smt_String(UNIT_THOUSAND);
		break;
	case 21:
		unit_file_name=Smt_String(UNIT_TEN_THOUSAND);
		break;
	}
	return unit_file_name;

}

/****************************************************************************
函 数 名: ConvertDigitToFileName
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillPhoneIott(Smt_Uint channel, Smt_String strNumber)
{
	char strBuff[MAXBUFF] = "";
	ACE_OS::strncpy(strBuff, strNumber.c_str(), MAXBUFF);	
	bool	 bFillFileFlag=false;	
	char *p = strBuff;
	while(*p != '\0')
	{
		Smt_String strFile;
		switch(*p)
		{
		case '*':
			strFile = Smt_String(PHONE_STAR);
			break;
		case '0':
			strFile = Smt_String(DIGIT_0);
			break;
		case '1':
			strFile = Smt_String(DIGIT_1);
			break;
		case '2':
			strFile = Smt_String(DIGIT_2);
			break;
		case '3':
			strFile = Smt_String(DIGIT_3);
			break;
		case '4':
			strFile = Smt_String(DIGIT_4);
			break;
		case '5':
			strFile = Smt_String(DIGIT_5);
			break;
		case '6':
			strFile = Smt_String(DIGIT_6);
			break;
		case '7':
			strFile = Smt_String(DIGIT_7);
			break;
		case '8':
			strFile = Smt_String(DIGIT_8);
			break;
		case '9':
			strFile = Smt_String(DIGIT_9);
			break;
		case '.':
			strFile = Smt_String(SYMBOL_DOT);
			break;
		default:
			break;			
		}
		
		if(strFile.length() > 1)
		{
			bFillFileFlag = FillFileIott(channel, strFile, 0);
		}
		p++;
	}	
 	return bFillFileFlag;
}

/****************************************************************************
函 数 名: ConvertDigitToFileName
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillTimeIott(Smt_Uint channel, Smt_String strTime)
{
	Smt_String strBuff = Smt_String(strTime);	
	bool bFillFileFlag = false;	
	if(strBuff.length() < 6 ) return bFillFileFlag;
	
	Smt_String strHour = strBuff.substr(0, 2);
	Smt_String strMin  = strBuff.substr(2, 2);
	Smt_String strSec  = strBuff.substr(4, 2);
	
	bFillFileFlag = FillIntegerIott(channel, strHour);
	bFillFileFlag = FillFileIott(channel, Smt_String(DATE_HOUR), 0);
	
	bFillFileFlag = FillIntegerIott(channel, strMin);
	bFillFileFlag = FillFileIott(channel, Smt_String(DATE_MINUTE), 0);
	
	bFillFileFlag = FillIntegerIott(channel, strSec);
	bFillFileFlag = FillFileIott(channel, Smt_String(DATE_SECOND), 0);

	return bFillFileFlag;
}

/****************************************************************************
函 数 名: ConvertDigitToFileName
参    数:
返回数值: 
功能描述: 填充iott数据
*****************************************************************************/
bool TAsteriskDevice::FillBoolIott(Smt_Uint channel, Smt_String strBool)
{
	bool	 bFillFileFlag=false;
	Smt_String strBuff = Smt_String(strBool);
	if(strBuff == Smt_String("true"))
	{   
		bFillFileFlag = FillFileIott(channel, Smt_String(BOOLEAN_TRUE), 0);
	}
	else if(strBuff == Smt_String("false"))
	{
    	bFillFileFlag = FillFileIott(channel, Smt_String(BOOLEAN_FALSE), 0);
	}
	else
	{
	}

	return bFillFileFlag;
}
/****************************************************************************
函 数 名: GetFileName
参    数:
返回数值: 
功能描述: 获取文件名
*****************************************************************************/
Smt_Uint TAsteriskDevice::GetFileName(Smt_String &strfile, Smt_String strcontent, Smt_String strpath)
{
    if(strcontent.strstr("file://") != -1)
	{
		strfile = strcontent.substr(7);
	}
	else
	{
		if(strcontent.strstr("\\") != -1 || strcontent.strstr("/") != -1)
		{
			strfile = strcontent;
		}
		else
		{
			strfile = HLFormatStr("%s/%s", strpath.c_str(), strcontent.c_str());		
		}
	}
	return true;
}

/****************************************************************************
函 数 名: AkPlayMessage
参    数:
返回数值: 
功能描述: 播放Message
*****************************************************************************/
Smt_Uint TAsteriskDevice::AkPlayMessage(Smt_Uint channel, Smt_String content)
{
	TCallData *pCall = NULL;
	if(FindCallByChannel(channel, pCall) == Smt_Success)
	{
		if(pCall->m_ResState == RES_NULL)
		{
			pCall->m_ResState = RES_DXSENDMSG;
			dx_sendmessage(pCall->m_CRN, (char*)(content.c_str()));
			pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::AkPlayMessage] CRN<%d> Content<%s>", pCall->m_CRN, content.c_str());
		}
		else
		{
			RespPlay(pCall->m_ChNo, Smt_Fail); 
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_Play, CauseFuctionFail, "Resource Busy");
			pCall->m_pChInfo->PrintLog(3,"[TAsteriskDevice::AkPlayMessage] Resource Busy. CRN<%d> Content<%s>", pCall->m_CRN, content.c_str());
		}
	}
    return Smt_Success;
}

/****************************************************************************
函 数 名: OnEvtConfSendMessage
参    数:
返回数值: 
功能描述: 处理返回
*****************************************************************************/
Smt_Uint TAsteriskDevice::OnEvtConfSendMessage(Csta_Event &event)
{
	Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success || nReason == cmuK_AGISuccess )
		{
			RespPlay(pCall->m_ChNo, Smt_Success);
			EvtPlayBegin(pCall->m_ChNo);
		}
		else
		{
            RespPlay(pCall->m_ChNo, Smt_Fail); 
		}
        pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtConfSendMessage] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfSetSipHeader(Csta_Event &event)
{
	Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success || nReason == cmuK_AGISuccess )
		{
			RespSetSipHead(pCall->m_ChNo, Smt_Success, "", "");
		}
		else
		{
			RespSetSipHead(pCall->m_ChNo, Smt_Fail, "", "");    
		}
        pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtConfSetSipHeader] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
Smt_Uint TAsteriskDevice::OnEvtConfGetSipHeader(Csta_Event &event)
{
	Smt_Uint nReason = event.u.evtconf.u.confgcResponse.Reason;
	Smt_Uint nCRN    = event.u.evtconf.u.confgcResponse.CallID;
	Smt_String strSipHeadKey = event.u.evtconf.u.confdx_getsipheader.HeaderKey;
	Smt_String strSipHeadValue = event.u.evtconf.u.confdx_getsipheader.HeaderValue;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		if(nReason == cmuK_Success || nReason == cmuK_AGISuccess )
		{
			if(strSipHeadValue != "")
			{
				pCall->m_SipHeaderValue = strSipHeadValue;
			}
			RespGetSipHead(pCall->m_ChNo, Smt_Success, m_gSipHeaderKey, pCall->m_SipHeaderValue);			
		}	
		else
		{
			RespGetSipHead(pCall->m_ChNo, Smt_Fail, m_gSipHeaderKey, pCall->m_SipHeaderValue);
		}
        pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtConfGetSipHeader] Crn<%d> Reason<%d> Key<%s> Value<%s>.", nCRN, nReason, strSipHeadKey.c_str(), strSipHeadValue.c_str());
	}
	return Smt_Success;
}
/****************************************************************************
函 数 名: OnEvtMediaMessageSendEnd
参    数:
返回数值: 
功能描述: 处理返回
*****************************************************************************/
Smt_Uint TAsteriskDevice::OnEvtMediaMessageSendEnd(Csta_Event &event)
{	
	Smt_Uint nCRN = event.u.evtmedia.CallID;
	Smt_Uint nReason = event.u.evtmedia.Reason;
	TCallData *pCall = NULL;
	if(FindCallByCRN(nCRN, pCall) == Smt_Success)
	{
		pCall->m_ResState = RES_NULL;
		if(nReason == cmuK_AGISuccess || nReason == cmuK_AGIFail)
		{
            EvtPlayEnd(pCall->m_ChNo, CauseMediaEndNomal, "CauseMediaEndNomal");
		}
		else
		{
			DeviceTaskFail(pCall->m_ChNo, Cmd_IVR_Play, CauseFuctionFail, "CauseFuctionFail");
		}
		pCall->m_pChInfo->PrintLog(5,"[TAsteriskDevice::OnEvtMediaMessageSendEnd] Crn<%d> Reason<%d>", nCRN, nReason);
	}
	return Smt_Success;
}
