//***************************************************************************
//      Copyright (C) 2008 Smt Soft Technology Ltd., Co.
//      All rights reserved.
//
//      Project     : BaseLib   	
//      Create Date : 2008.03.26      	
//      Author      : Shenzj    
//      Discription : Smt_XML Code
//        依赖库    : libxml2.lib iconv.lib
//        使用方法  : 不需要build，被包含到其它源代码中,提供从UTF-8到GB2312的内码转换，以及反向的转换
//            1)OpenTable
//            2)AddNewRecord
//            3)PutString  ... ... 
//            4)UpdateTable
//		      5)CloseTable
//***************************************************************************

#ifndef Smt_XML_HH
#define Smt_XML_HH

// disable any warning
#pragma warning(disable: 4267)
#pragma warning(disable: 4996)

#include "Smt_BaseType.h"
#include "Smt_BaseObject.h"

using namespace Smt_Type;

//使用libxml2需要的一些文件
#include <libxml/parser.h>
#include <libxml/tree.h>
#include <libxml/xpath.h>
#include <libxml/xmlsave.h>
#include <iconv.h>

#pragma comment(lib, "libxml2.lib")
#pragma comment(lib, "iconv.lib")

/////////////////////////////////////////////////////////////////////////////
// Smt_XMLParser  code
class Smt_XMLParser
{
public:
	Smt_XMLParser( Smt_String filename );
	virtual ~Smt_XMLParser();

public:
	Smt_Uint   OpenTable(Smt_String tlbname );
	Smt_Uint   CloseTable();
	Smt_String GetPropString(Smt_String fieldname );
	Smt_Uint   GetPropUint(Smt_String fieldname );
	Smt_Uint   PutPropString(Smt_String fieldname, Smt_String value );
	Smt_Uint   PutPropUint(Smt_String fieldname, Smt_Uint value );
	Smt_Uint   PutContentString(Smt_String value );
	Smt_Uint   AddNewRecord();
	Smt_Uint   UpdateTable();
	Smt_Uint   MoveFirst();
	Smt_Uint   MoveNext();
	Smt_Uint   ClearTable();
	Smt_Bool   IsEmpty();

public:
	Smt_String m_FileName;
	xmlDocPtr    m_XMLDoc;                //定义解析文档指针 
	xmlNodePtr   m_CurrNode ;             //定义结点指针(需要它为了在各个结点间移动) 
	xmlXPathObjectPtr m_XPathObj;         //XPath对象
	xmlNodePtr   m_NewChildNode ;         //当前节点的新增子节点

	char* U2G(char *inbuf);
	char* G2U(char *inbuf);
	int CodeConvert(char* from_charset, char* to_charset, char* inbuf, int inlen, char* outbuf, int outlen);
	xmlXPathObjectPtr GetNodeSet(xmlDocPtr doc, const xmlChar *szXpath);
};

#endif // end file
