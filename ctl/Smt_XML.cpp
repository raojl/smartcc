//***************************************************************************
// Smt_XML.cpp : implementation file
//***************************************************************************

#include "Smt_XML.h"

/////////////////////////////////////////////////////////////////////////////
// Smt_XMLParser code

Smt_XMLParser::Smt_XMLParser( Smt_String filename )
{
	m_FileName = filename;

	m_XMLDoc = NULL;	                   
	m_CurrNode = NULL;		              
	m_XPathObj = NULL;             
	m_NewChildNode = NULL; 

	xmlKeepBlanksDefault( 1 );
}

Smt_XMLParser::~Smt_XMLParser()
{
	if( m_XMLDoc != NULL )
		CloseTable();
}

/****************************************************************************
函 数 名: OpenTable
参    数:
返回数值: 
功能描述: 打开一个XML的表节点
*****************************************************************************/
Smt_Uint Smt_XMLParser::OpenTable(Smt_String tlbname)
{
	Smt_Uint nRetVal = Smt_Success;

	try
	{
		m_XMLDoc = xmlReadFile(m_FileName.c_str(),"GB2312",XML_PARSE_RECOVER);   //解析文件 
		
		if (NULL == m_XMLDoc) return Smt_Fail;  
		
		xmlChar szXpath[256];
		::sprintf((char*)szXpath, "/Configer/Tables/Table[@TableName='%s']", tlbname.c_str());
		
		m_XPathObj = GetNodeSet(m_XMLDoc,szXpath);                                //查询并得到结果
		
		if ( m_XPathObj == NULL ) 
		{
			xmlFreeDoc(m_XMLDoc);
			m_XMLDoc = NULL;
			return Smt_Fail;
		}
		
		char *szValue = NULL;
		xmlNodeSetPtr nodeset = m_XPathObj->nodesetval;
		
		if ( nodeset->nodeNr != 1) 
		{
			xmlXPathFreeObject ( m_XPathObj );
			m_XPathObj = NULL;

			xmlFreeDoc(m_XMLDoc);
			m_XMLDoc = NULL;

			return Smt_Fail;
		}
		
		m_CurrNode = nodeset->nodeTab[0];
	}
	catch (...)
	{
		nRetVal = Smt_Fail;
	}
	
	return nRetVal;
}

/****************************************************************************
函 数 名: CloseTable
参    数:
返回数值: 
功能描述: 关闭表节点
*****************************************************************************/
Smt_Uint Smt_XMLParser::CloseTable()
{
	Smt_Uint nRetVal = Smt_Success;

	try
	{
		if( m_XPathObj != NULL)
			xmlXPathFreeObject ( m_XPathObj );
		
		if( m_XMLDoc != NULL )
			xmlFreeDoc(m_XMLDoc);
		
		m_XPathObj = NULL;
		m_XMLDoc = NULL;
	}
	catch (...)
	{
		nRetVal = Smt_Fail;		
	}

	return nRetVal;
}

/****************************************************************************
函 数 名: GetString
参    数:
返回数值: 
功能描述: 根据字段名称获取字段值（字符串型）
*****************************************************************************/
Smt_String Smt_XMLParser::GetPropString(Smt_String fieldname)
{	
	Smt_String strRet = "";
	
	try
	{
		char *szValue = NULL;
		
		if (xmlHasProp(m_CurrNode, BAD_CAST fieldname.c_str()) )
		{
			szValue = U2G((char*)xmlGetProp(m_CurrNode,BAD_CAST  fieldname.c_str() ));
			if (szValue != NULL) 
			{
				strRet = szValue;
				ACE_OS::free(szValue);
			}
		}	
	}
	catch (...)
	{
		strRet = "";
	}
	
	return strRet;
}

/****************************************************************************
函 数 名: GetUint
参    数:
返回数值: 
功能描述: 根据字段名称获取字段值（数值型）
*****************************************************************************/
Smt_Uint Smt_XMLParser::GetPropUint(Smt_String fieldname)
{	
	Smt_Uint nRetVal = 0;
	
	try
	{
		char *szValue = NULL;
		
		if (xmlHasProp(m_CurrNode, BAD_CAST fieldname.c_str()) )
		{
			szValue = U2G((char*)xmlGetProp(m_CurrNode,BAD_CAST fieldname.c_str() ));
			if (szValue != NULL) 
			{
				nRetVal = ACE_OS::atoi(szValue);
				ACE_OS::free(szValue);
			}
		}
	}
	catch (...)
	{
		nRetVal = 0;	
	}
	
	return nRetVal;
}

/****************************************************************************
函 数 名: AddNewRecord
参    数:
返回数值: 
功能描述: 新增一条记录
*****************************************************************************/
Smt_Uint Smt_XMLParser::AddNewRecord()
{
	Smt_Uint nRetVal = Smt_Success;

	try
	{
		m_NewChildNode = xmlNewNode(NULL, BAD_CAST "RecRow");
		
		xmlAddChild( m_CurrNode, m_NewChildNode );
	}
	catch (...)
	{
		nRetVal = Smt_Fail;
	}
	
	return nRetVal;
}

/****************************************************************************
函 数 名: PutPropString
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint Smt_XMLParser::PutPropString(Smt_String fieldname, Smt_String value )
{
	Smt_Uint nRetVal = Smt_Success;
	try
	{
		char* szOut = G2U( (char*)value.c_str() );
		
		xmlNewProp(m_NewChildNode, BAD_CAST fieldname.c_str(), BAD_CAST szOut);
		
		ACE_OS::free(szOut);
		szOut = NULL;
	}
	catch (...)
	{
		nRetVal = Smt_Fail;	
	}

	return nRetVal;
}

/****************************************************************************
函 数 名: PutPropUint
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint Smt_XMLParser::PutPropUint(Smt_String fieldname, Smt_Uint value )
{
	Smt_Uint nRetVal = Smt_Success;
	try
	{
		Smt_String strValue = HLFormatStr( "%d", value );

		char* szOut = G2U( (char*)strValue.c_str() );
		
		xmlNewProp(m_NewChildNode, BAD_CAST fieldname.c_str(), BAD_CAST szOut);
		
		ACE_OS::free(szOut);
		szOut = NULL;
	}
	catch (...)
	{
		nRetVal = Smt_Fail;	
	}
	
	return nRetVal;
}

/****************************************************************************
函 数 名: PutContentString
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint Smt_XMLParser::PutContentString( Smt_String value )
{
	Smt_Uint nRetVal = Smt_Success;
	try
	{
		char* szOut = G2U( (char*)value.c_str() );

		xmlNodePtr ndContent = xmlNewText( BAD_CAST value.c_str() );
		xmlAddChild( m_NewChildNode, ndContent );
		
		ACE_OS::free(szOut);
		szOut = NULL;
	}
	catch (...)
	{
		nRetVal = Smt_Fail;	
	}
	
	return nRetVal;
}

/****************************************************************************
函 数 名: ClearTable
参    数:
返回数值: 
功能描述: 
*****************************************************************************/
Smt_Uint Smt_XMLParser::ClearTable()
{
	Smt_Uint nRetVal = Smt_Success;
	try
	{
		xmlNodePtr ndTemp = m_CurrNode->children;
		xmlNodePtr ndTemp2;
		while (NULL != ndTemp )
		{
			if (!xmlStrcmp(ndTemp->name, BAD_CAST "RecRow"))
			{
				ndTemp2 = ndTemp->next;
				xmlUnlinkNode(ndTemp);
				xmlFreeNode(ndTemp);
				ndTemp = ndTemp2;
				continue;
			}
		}
	}
	catch (...)
	{
		nRetVal = Smt_Fail;	
	}
	
	return nRetVal;
}

/****************************************************************************
函 数 名: UpdateTable
参    数:
返回数值: 
功能描述: 保存数据到 XML 文件
*****************************************************************************/

Smt_Uint Smt_XMLParser::UpdateTable()
{		
	Smt_Uint nRetVal = Smt_Success;
	try
	{
		xmlSaveCtxtPtr cTxt;
		cTxt = xmlSaveToFilename( m_FileName.c_str(), "GB2312", XML_SAVE_FORMAT );
		if (cTxt != NULL) 
		{ 
			if (xmlSaveDoc(cTxt, m_XMLDoc) < 0) 
			{
				nRetVal = Smt_Fail;
			} 
			xmlSaveClose(cTxt); 
		}
	}
	catch (...)
	{
		nRetVal = Smt_Fail;	
	}
	
	return nRetVal;
}

/****************************************************************************
函 数 名: MoveFirst
参    数:
返回数值: 
功能描述: 移到某个表的第一条记录
*****************************************************************************/
Smt_Uint Smt_XMLParser::MoveFirst()
{
	if(m_CurrNode != NULL)
		m_CurrNode = m_CurrNode->xmlChildrenNode;

	return Smt_Success;
}

/****************************************************************************
函 数 名: MoveNext
参    数:
返回数值: 
功能描述: 下一条记录
*****************************************************************************/
Smt_Uint Smt_XMLParser::MoveNext()
{
	if(m_CurrNode != NULL)
		m_CurrNode = m_CurrNode->next; 
	return Smt_Success;
}

/****************************************************************************
函 数 名: CodeConvert
参    数:
返回数值: int
功能描述: 代码转换,从一种编码转为另一种编码
*****************************************************************************/
int Smt_XMLParser::CodeConvert(char* from_charset, char* to_charset, char* inbuf,
				 int inlen, char* outbuf, int outlen)
{
	iconv_t cd;
	char** pin = &inbuf;   
	char** pout = &outbuf;

	cd = iconv_open(to_charset,from_charset);   
	if(cd == 0)
		return -1;
	ACE_OS::memset(outbuf,0,outlen);   
	if(iconv(cd,(const char**)pin,(unsigned int *)&inlen,pout,(unsigned int*)&outlen) == -1)
		return -1;   
	iconv_close(cd);
	return 0;   
}

/****************************************************************************
函 数 名: U2G
参    数:
返回数值: int
功能描述: UNICODE码转为GB2312码,成功则返回一个动态分配的char*变量，需要在使用完毕后手动free，失败返回NULL
          nOutLen 加 1 个字节防止出现 0 字节的转换
*****************************************************************************/
char* Smt_XMLParser::U2G(char *inbuf)   
{
	int nOutLen = 2 * ACE_OS::strlen(inbuf) + 1;       
	char* szOut = (char*)ACE_OS::malloc(nOutLen);

	ACE_OS::memset(szOut, 0x0, nOutLen);
	
	if (-1 == CodeConvert("utf-8","gb2312",inbuf,ACE_OS::strlen(inbuf),szOut,nOutLen))
	{
		ACE_OS::free(szOut);
		szOut = NULL;
	}
	return szOut;
}   

/****************************************************************************
函 数 名: G2U
参    数:
返回数值: char
功能描述: GB2312码转为UNICODE码,成功则返回一个动态分配的char*变量，需要在使用完毕后手动free，失败返回NULL
          nOutLen 加 1 个字节防止出现 0 字节的转换
*****************************************************************************/
char* Smt_XMLParser::G2U(char *inbuf)   
{
	int nOutLen = 2 * ACE_OS::strlen(inbuf) + 1;         
	char* szOut = (char*)ACE_OS::malloc(nOutLen);

	ACE_OS::memset(szOut, 0x0, nOutLen);

	if (-1 == CodeConvert("gb2312","utf-8",inbuf,ACE_OS::strlen(inbuf),szOut,nOutLen))
	{
		ACE_OS::free(szOut);
		szOut = NULL;
	}
	return szOut;
}   

/****************************************************************************
函 数 名: GetNodeSet
参    数:
返回数值: xmlXPathObjectPtr
功能描述: 获取Doc的NodeSet,成功则返回一个xmlXPathObjectPtr变量，需要在使用完毕后手动xmlXPathFreeObject，失败返回NULL
*****************************************************************************/
xmlXPathObjectPtr Smt_XMLParser::GetNodeSet(xmlDocPtr doc, const xmlChar *szXpath) 
{
	xmlXPathContextPtr context;							//XPATH上下文指针
	xmlXPathObjectPtr result;							//XPATH对象指针，用来存储查询结果

	try
	{
		context = xmlXPathNewContext(doc);					//创建一个XPath上下文指针
		if (context == NULL) 
		{	
			//printf("context is NULL\n");
			return NULL; 
		}

		result = xmlXPathEvalExpression(szXpath, context);	//查询XPath表达式，得到一个查询结果
		xmlXPathFreeContext(context);						//释放上下文指针
		if (result == NULL) 
		{
			//printf("xmlXPathEvalExpression return NULL\n"); 
			return NULL;  
		}

		if (xmlXPathNodeSetIsEmpty(result->nodesetval))		//检查查询结果是否为空
		{
			xmlXPathFreeObject(result);
			//printf("nodeset is empty\n");
			return NULL;
		}
	}
	catch(...)
	{
	}

	return result;	
}

/****************************************************************************
函 数 名: IsEmpty
参    数:
返回数值: 
功能描述: 记录是否为空
*****************************************************************************/
Smt_Bool Smt_XMLParser::IsEmpty()
{
	if( m_CurrNode == NULL )
		return Smt_BoolTRUE;
	else
		return Smt_BoolFALSE;
}
