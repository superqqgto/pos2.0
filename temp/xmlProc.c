
#include "xmlProc.h"
#include "appLib.h"
#include "xmlParser.h"
#include "util.h"
#include "TranProc.h"
#include "FileOper.h"
#include "global.h"
#include "setup.h"
#include "util.h"
#include "lng.h"
#include "global.h"
//#include "Logo.h" //add by wuc 2014.9.9


extern FuncMag glFunc[];
extern uchar glIDName[128];
extern uchar glCondition[64];
uchar bIsCancel = 0;
uchar gucDispLine;
char gl_szMensagem[64] = "";

static void HeaderProc(uchar *szData);

int GetXmlEleVal(uchar *pszData, uchar *pszeleName, uchar *pszValue)
{
	uchar *pBegin, *pEnd;
	uchar szBegin[16], szEnd[16];

	sprintf(szBegin, "<%s>", pszeleName);
	sprintf(szEnd, "</%s>", pszeleName);
	pBegin = strstr(pszData, szBegin);
	if ( pBegin==NULL )
	{
		return -1;
	}
	pEnd = strstr(pBegin, szEnd);
	if( pEnd==NULL )
	{
		return -1;
	}

	pBegin += strlen(szBegin);
	memset(pszValue, 0, sizeof(pszValue));
	memcpy(pszValue, pBegin, pEnd-pBegin);
	pszValue[pEnd-pBegin] = '\0';

	return pEnd-pszData+strlen(szEnd);
}

//Get value in the <status> and <mensagem>. linzhao 2015.1.26
int StatusProc(uchar *pszData)
{
	uint uiLen;
	uchar szValueStatus[LEN_BODY_DATA+1] = "";

	uiLen = GetXmlEleVal(pszData, "status", szValueStatus);
	GetXmlEleVal(pszData+uiLen, "mensagem", gl_szMensagem);
	if ( 0!=memcmp(szValueStatus, "OK", strlen("OK")) )
	{
		return -1;
	}
	return 0;

}

//get the attr in the <ppcomp>. linzhao 2015.1.26
int XML_GetAttr(uchar *pszPos, uchar *pszAttr_key, uchar *pszAttr_value)
{
	uchar *pBegin, *pEnd;

	if( pszPos==NULL || strlen(pszPos)==0 )
	{
		return -1;
	}
	

	pBegin = strstr(pszPos, "key=");
	if ( pBegin==NULL )
		return -1;
	pBegin = pBegin + strlen("key=") + 1;

	pEnd = strchr(pBegin, '"');
	if ( pEnd==NULL )
		return -1;
	
	memcpy(pszAttr_key, pBegin, pEnd-pBegin);
	pszAttr_key[pEnd-pBegin] = 0;
	if (glSysCtrl.bDebugOpt)
	{
		DebugOutput("%s--%d--%s  Attr:%s\n", __FILE__, __LINE__, __FUNCTION__, pszAttr_key);//linzhao
	}

	pBegin = strstr(pEnd, "value=");
	if ( pBegin==NULL )
		return -1;
	pBegin = pBegin + strlen("value=") + 1;
	pEnd = strchr(pBegin, '"');
	if ( pEnd==NULL)
		return -1;
	memcpy(pszAttr_value, pBegin, pEnd-pBegin);
	pszAttr_value[pEnd-pBegin] = 0;
	pEnd = strstr(pBegin, "/>");

	return pEnd+3-pszPos;
}

//get all the system's para in the ppcomp. linzhao 2015.1.26
int PPCompProc(uchar *pszData)
{
	uchar *pBegin, *pEnd;
	int iRet;
	uchar szEleName[128+1] = "";
	uchar szAttr_key[64] = "";
	uchar szAttr_value[64] = "";
	uchar szValue[LEN_BODY_DATA+1] = "";
	
	pBegin = strstr(pszData, "<ppcomp>");
	pEnd = strstr(pszData, "</ppcomp>");
	if( pBegin==NULL ||pEnd==NULL )
	{
		return -1;
	}

	pBegin += strlen("<ppcomp>");
	while(1)
	{
		memset(szEleName, 0, sizeof(szEleName));
		memset(szAttr_key, 0, sizeof(szAttr_key));
		memset(szAttr_value, 0, sizeof(szAttr_value));
		memset(szValue, 0, sizeof(szValue));

		iRet = XML_GetAttr(pBegin, szAttr_key, szAttr_value);
		if(iRet < 0)
		{
			break;	
		}

		pBegin += iRet;

		if( 0==memcmp(szAttr_key, "empresa", strlen("empresa")))
		{
			memcpy(glSysCtrl.szMerchantNum, szAttr_value, strlen(szAttr_value));
		}
		else if( 0==memcmp(szAttr_key, "loja", strlen("loja")) )
		{
			memcpy(glSysCtrl.szStoreNum, szAttr_value, strlen(szAttr_value));
		}
		else if( 0==memcmp(szAttr_key, "pdv", strlen("pdv")) )
		{
			memcpy(glSysCtrl.szPosNum, szAttr_value, strlen(szAttr_value));
		}
		else if( 0==memcmp(szAttr_key, "endereco_fet", strlen("endereco_fet")) )
		{
			memcpy(glSysCtrl.szRemoteIP, szAttr_value, strlen(szAttr_value));
			memcpy(glSysCtrl.szRemoteIPnoSSL, szAttr_value, strlen(szAttr_value));//linzhao 2015.2.6
		}
		else if( 0==memcmp(szAttr_key, "porta_ip", strlen("porta_ip")) )
		{
			memcpy(glSysCtrl.szRemotePort, szAttr_value, strlen(szAttr_value));
		}
		else if( 0==memcmp(szAttr_key, "processo_destino", strlen("processo_destino")))
		{
			memcpy(glSysCtrl.szDestino, szAttr_value, strlen(szAttr_value));
		}
		else if( 0==memcmp(szAttr_key, "gzip", strlen("gzip")) )
		{

			if ( 0==memcmp(szAttr_key, "true", strlen("true")))
				glSysCtrl.bGzipOpt = 1;
			else
				glSysCtrl.bGzipOpt = 0;
		}
		else if( 0==memcmp(szAttr_key, "telablocante", strlen("telablocante")) )
		{
			if ( 0==memcmp(szAttr_key, "true", strlen("true")))
				glSysCtrl.bTelablocante = 1;
			else
				glSysCtrl.bTelablocante = 0;
		}
		else if( 0==memcmp(szAttr_key, "numero_logico", strlen("numero_logico")))
		{
			memcpy(glSysCtrl.szLogNum, szAttr_value, strlen(szAttr_value));
		}
		if(pBegin == pEnd)
		{
			break;
		}
	}

	return 0;
}

//linzhao
int TelablocanteProc(uchar *file)
{

	uchar *pBegin, *pEnd;
	uchar sInput[512], sOutput[512];

	if ( NULL==file)
		return -2;
	pBegin = strstr(file, "<Telablocante>");
	if ( NULL==pBegin)
		return TAG_NOT_FOUND;

	pBegin += strlen("<Telablocante>") + 1;
	pEnd = strstr(pBegin, "</Telablocante>");
	if ( NULL==pEnd)
		return TAG_NOT_FOUND;

	DebugOutput("%s--%d--%s \n", __FILE__, __LINE__, __FUNCTION__ );//linzhao
	memcpy(sInput, pBegin, pEnd-pBegin);
	sInput[pEnd-pBegin] = 0;

	Base64Decode(sInput, sOutput, strlen(sInput));
	DebugOutput("%s--%d--%s--sOutput:%s \n", __FILE__, __LINE__, __FUNCTION__, sOutput);//linzhao

	if ( 0==memcmp(sOutput, "false", strlen("false")))
		return 0;
	else
		return -1;
}

int ProcXMLPage(uchar *szFile)
{
	char file[1024*100];
	int fd;
	long file_size, read_size;
	char *temp1;
	char *temp2;
	int iRet;
	
	file_size = filesize(szFile);
	fd = open(szFile, O_RDWR);
	if (fd < 0)
	{
		ScrClrBelow(2);
		PubDispString(_T("UNSUPPORTED"), DISP_LINE_CENTER|4);
		PubWaitKey(USER_OPER_TIMEOUT);
		return -1;
	}

	memset(file, 0, sizeof(file));
	read_size = read(fd, file, file_size);
	if (fd >= 0)
	{
		close(fd);	
	}
	if(read_size != file_size)
	{
		return -1;
	}
	InitPPComp();
	//Auto configurate at the first fun.linzhao, 2015.1.16
	if ( 1==glSysCtrl.ucFirstRun )
	{
		if (  0==StatusProc(file) && 0==PPCompProc(file))
		{
			glSysCtrl.ucFirstRun++;
			return 0;
		}
		else
			return -1;
	}

	//add by linzhao, maybe want analyze BASE64
//	iRet = TelablocanteProc(file);
//	DebugOutput("%s--%d--%s :iRet :%d  \n", __FILE__, __LINE__, __FUNCTION__, iRet);//linzhao
//	if ( iRet==TELABLOCANTE_FALSE )
//		return iRet;
	//add end linzhao

	HeaderProc(file);

	if(glActivate) //add by wuc 2014.10.11
	{
		return -3;
	}
	
	memset(glIDName, 0, sizeof(glIDName));
	temp1 = FormProc(file, glIDName);
	if(temp1 == NULL)
	{
		if(bIsCancel  )
		{
			bIsCancel = 0;
		}
		return -2; //modify by wuc 2014.4.1
	}
	
	while (temp1 != NULL)
	{
		temp2 = temp1;
		temp1 = FormProc(temp2, NULL);
	}

	if(bIsCancel )
	{
		bIsCancel = 0;
		return -2; //modify by wuc 2014.4.1
	}
	return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
static int LabelProc(char * const pszBegin, char *pszEnd)
{
	tag_label stLabel[MAX_TAG_NUM];
	int i,j;
	uchar iTotalLen, iIndex, bFlag=0;
	uchar szDispLine[32];

	memset(&stLabel, 0, sizeof(tag_label)*MAX_TAG_NUM);
	ParseLabel(&stLabel[0], pszBegin, pszEnd);
	i = 0;
	gucDispLine = 2;
	while(stLabel[i].label_begin != NULL && stLabel[i].label_begin<pszEnd)
	{
		i++;
		ParseLabel(&stLabel[i], stLabel[i-1].label_end, pszEnd);
	}
	
 	iTotalLen = iIndex = i;
	for(i=0; i<iTotalLen; i++)
	{
		for(j=i+1, bFlag=0; j<iIndex; j++)
		{
			if(strncmp(stLabel[i].szID, stLabel[j].szID, (strlen(stLabel[i].szID)-1)) == 0)
			{
				memset(szDispLine, 0, sizeof(szDispLine));
				sprintf(szDispLine, "%s %s", stLabel[i].szText, stLabel[j].szText);
				gucDispLine = DispLabel(szDispLine, gucDispLine, DISP_LINE_LEFT);
				iTotalLen -= 1;
				bFlag = 1;
				break;
			}
		}

		if(!bFlag)
		{
			gucDispLine = DispLabel(stLabel[i].szText, gucDispLine, DISP_LINE_CENTER);
		}
	}
	
	return 0;
}

static int ListProc(char *pszBegin, char *pszEnd)
{
	tag_list stList;
	tag_option stOption[MAX_TAG_NUM];
	char *p, szListID[64];
	char szLine[128] = "";
	char szSetFlag[20];
	uchar iIndex;
	int i, iRet;

	memset(szListID, 0, 64);
	memset(&stList, 0, sizeof(tag_list));
	iRet = ParseList(&stList, pszBegin, pszEnd);
	if(iRet != 0)
	{
		return 0;
	}

	p = strstr(stList.list_begin, "id=\"");
	if((p != NULL) && (p<stList.list_end))
	{
		p += strlen("id=\"");
		i = 0;
		do
		{
			szListID[i++] = *(p++);
		}while(*p!='"');
	}
	if(stList.list_begin!=NULL && stList.list_end<pszEnd)
	{
		ParseOption(&stOption[0], stList.list_begin, stList.list_end);
		i = 0;
		while(stOption[i].option_begin!=NULL && stOption[i].option_begin<pszEnd)
		{	
			++i;
			ParseOption(&stOption[i], stOption[i-1].option_end, stList.list_end);
		}

		SetScreenLine(ROW_8_LINE);
		ScrClrBelow(gucDispLine);

		for(iIndex=0; iIndex<i; iIndex++)
		{
			memset(szLine, 0, sizeof(szLine));
			sprintf(szLine, "%02d.%s", iIndex+1, stOption[iIndex].szText);
			PubDispString(szLine, (ushort)(DISP_LINE_LEFT|gucDispLine+iIndex));
			ScrDrawLine(gucDispLine+iIndex, GRAY_COLOR); //add by wuc 2014.4.1
		}
		ScrDrawLine(gucDispLine+iIndex, GRAY_COLOR); //add by wuc 2014.4.1
		SetScreenLine(ROW_9_LINE);
		
		kbflush();
		while (1)
		{	
			iIndex = PubWaitKey(USER_OPER_TIMEOUT);
			if((iIndex >= KEY1) && (iIndex <= (i+0x30)))
			{
				strcpy(szSetFlag, stOption[iIndex-0x31].szSetFlag);
				strcpy(glPPComp[glPPCompIdx].szFuncID, szListID);
				strcpy(glPPComp[glPPCompIdx].szOutput[0], stOption[iIndex-0x31].szValue);
				glPPCompIdx ++;
				break;
			}
			if(iIndex == KEYCANCEL || iIndex == NOKEY)
			{
				bIsCancel = 1;
				return -1;
			}
			/*if(iIndex == KEYF1 || iIndex == KEYATM1)
			{
				if(0 == Enterfunctions())
				{
					Function();
				}
				ScrCls();
				bIsCancel = 1;
				return -1;
			}*/ //modify by wuc 2014.4.1
		}// while (1
		
		if (strcmp(szSetFlag, "") != 0) 
		{
			strcpy(glSetFormFlag, szSetFlag);
		}
	}

	return 0;
}

static int TextboxProc(char *pszBegin, char *pszEnd)
{
	tag_textbox stTxtbox;
	int iRet;
	char szTextInput[32] = "";

	memset(&stTxtbox, 0, sizeof(tag_textbox));
	iRet = ParseTextBox(&stTxtbox, pszBegin, pszEnd);
	if(iRet != 0)
	{
		return 0;
	}
	
	DebugOutput("%s--%d\n, szID:%s, iFormat:%d, szFormat:%s, szOptional:%s, szText:%s, imaxlen:%d, iMinlen:%d\n", 
				__FILE__, __LINE__, stTxtbox.szID, stTxtbox.iFormat, stTxtbox.szFormat,
				stTxtbox.szOpcional, stTxtbox.szText, stTxtbox.iMaxLen, stTxtbox.iMinLen);//linzhao
	if (stTxtbox.textbox_begin!=NULL && stTxtbox.textbox_begin<pszEnd)
	{
		if(memcmp(stTxtbox.szID, "Confirmacao", 11) == 0)
		{
			while(1)
			{
				iRet = PubWaitKey(USER_OPER_TIMEOUT);
				if(iRet == KEYENTER)
				{
					szTextInput[0] = '1';
					break;
				}
				else if(iRet == KEYCANCEL || iRet == NOKEY)
				{
					szTextInput[0] = '0';
					break;
				}
			}
			DebugOutput("%s--%d\n", __FILE__, __LINE__);//linzhao
				
		}
		else
		{
			memset(szTextInput, 0, sizeof(szTextInput));
			iRet = DispTextBox(&stTxtbox, szTextInput);
			DebugOutput("%s--%d, iRet:%d\n", __FILE__, __LINE__, iRet);//linzhao
			if(iRet != 0)
			{
				bIsCancel = 1;
				return -1;
			}
		}
		DebugOutput("%s--%d, stTxtbox.szID:%s\n", __FILE__, __LINE__, stTxtbox.szID);
		strcpy(glPPComp[glPPCompIdx].szFuncID, stTxtbox.szID);
		strcpy(glPPComp[glPPCompIdx].szOutput[0], szTextInput);
		glPPCompIdx ++;
	}

	
	return 0;
}

static int RadbuttProc(char *pszBegin, char *pszEnd)
{
	tag_radbut stRadbutt[MAX_TAG_NUM];
	
	int i,iTotal;
	uchar iIndex;
	int iRet;
	char szSetFlag[20];

	memset(&stRadbutt, 0, sizeof(tag_radbut)*MAX_TAG_NUM);
	iRet = ParseRadButt(&stRadbutt[0], pszBegin, pszEnd);
	if(iRet != 0)
	{
		return 0;
	}
	i = 0;
	if (stRadbutt[i].radbut_begin!=NULL && stRadbutt[i].radbut_begin<pszEnd)
	{
		while (stRadbutt[i].radbut_begin!=NULL && stRadbutt[i].radbut_begin<pszEnd)
		{
			++i;
			ParseRadButt(&stRadbutt[i], stRadbutt[i-1].radbut_end, pszEnd);
		}

		iTotal = i;
		SetScreenLine(ROW_8_LINE);
		ScrClrBelow(gucDispLine);

		for(iIndex=0; iIndex<iTotal; iIndex++)
		{
			PubDispString(stRadbutt[iIndex].szText, (ushort)(DISP_LINE_LEFT|(gucDispLine+iIndex)));
			ScrDrawLine(gucDispLine+iIndex, GRAY_COLOR); //add by wuc 2014.4.1
		}
		ScrDrawLine(gucDispLine+iIndex, GRAY_COLOR); //add by wuc 2014.4.1
		SetScreenLine(ROW_9_LINE);

		kbflush();
		while (1)
		{	
			iRet = PubWaitKey(USER_OPER_TIMEOUT);
			if(iRet == KEYCANCEL || iRet == NOKEY)
			{
				bIsCancel = 1;
				return -1;
			}
			else
			{
				for(i=0; i<iTotal; i++)
				{
					iIndex = atoi(stRadbutt[i].szSelectNo) + 0x30;
					if(iRet == iIndex)
					{
						strcpy(szSetFlag, stRadbutt[i].szSetFlag);
						stRadbutt[i].bIsSelect = TRUE;
						break;
					}
				}

				if(i<iTotal)
				{
					break;
				}
			}
		}
		
		if (strcmp(szSetFlag, ""))
		{
			strcpy(glSetFormFlag, szSetFlag);
		}
		
		for(i=0; i<iTotal; i++)
		{
			strcpy(glPPComp[glPPCompIdx].szFuncID, stRadbutt[i].szID);
			if(stRadbutt[i].bIsSelect == TRUE)
			{
				strcpy(glPPComp[glPPCompIdx].szOutput[0], "True");
			}
			else
			{
				strcpy(glPPComp[glPPCompIdx].szOutput[0], "False");
			}
			glPPCompIdx ++;	
		}
	}

	return 0;
}

//add activate judage conditon: rc="01" is fail, no this is ok.(just for activation). linzhao 2015.1.27
static int MsgboxActivateJudge(tag_msgbox *pstMsgbox )
{
	uchar *pBegin, *pEnd, szText[8];

	if (glProcInfo.ucTranType==ACTIVE)
	{
		pBegin = strstr(pstMsgbox->msgbox_begin, "rc=");
		if (pBegin==NULL)
		{
			return 0;
		}
		pBegin = pBegin + strlen("rc=") + 1;
		pEnd = strchr(pBegin, '\"');
		if (pEnd==NULL)
		{
			return 0;
		}
		memcpy(szText, pBegin, pEnd-pBegin);
		szText[pEnd-pBegin] = 0;
		DebugOutput("%s--%d--%s, szText:%s\n", __FILE__, __LINE__, __FUNCTION__, szText);
		if ( 0==strcmp("01", szText) )
		{
			glSysCtrl.bActivate = 0;
		}
		else
		{
			glSysCtrl.bActivate = 1;
		}
	}

	return 0;
}
static int MsgboxProc(char * const pszBegin, char *pszEnd)
{
	tag_msgbox stMsgbox;
	int iRet;

	memset(&stMsgbox, 0, sizeof(tag_msgbox));
	iRet = ParseMsgBox(&stMsgbox, pszBegin, pszEnd);
	if(iRet != 0)
	{
		return 0;
	}
	
	MsgboxActivateJudge(&stMsgbox);//linzhao 2015.1.27

	if(strlen(stMsgbox.szText) != 0)
	{
		DispMsgBox(stMsgbox.szText);
	}

	return 0;
}

static int PicboxProc(char *const pszBegin, char *pszEnd)
{
	tag_picbox stPicbox;

	memset(&stPicbox, 0, sizeof(tag_picbox));
	ParsePicBox(&stPicbox, pszBegin, pszEnd);
	if(strlen(stPicbox.szFile) != 0)
	{
		DispPictureBox(stPicbox.szFile, 2, 0, 0);
	}

	return 0;
}

void ConvertIntpos(char *szInput, char *szOutput, uchar ucMode)
{
	int iLen;
	char *p1, *p2,  *pOut;

	iLen = strlen((char *)szInput);
	p1 = p2 = szInput;
	pOut = szOutput;

	if(ucMode == 0)
	{
		while(iLen > (p1-p2))
		{
			if(*p1 == 0x0d)
			{
				*(pOut++) = '&';
				*(pOut++) = '#';
				*(pOut++) = 'x';
				*(pOut++) = 'D';
				*(pOut++) = ';';
				p1 ++;
			}
			else if(*p1 == 0x0a)
			{
				*(pOut++) = '&';
				*(pOut++) = '#';
				*(pOut++) = 'x';
				*(pOut++) = 'A';
				*(pOut++) = ';';
				p1 ++;
			}
			else
				*(pOut++) = *(p1++);
		}
	}
	else if(ucMode == 1)
	{
		while(iLen > (p1-p2))
		{
			if(*p1 == '&')
			{
				if(*(p1+1) == '#' && *(p1+2) == 'x' && *(p1+3) == 'D' && *(p1+4) == ';')
				{
					*pOut = 0x0d;
					p1 += 5;
				}
				else if(*(p1+1) == '#' && *(p1+2) == 'x' && *(p1+3) == 'A' && *(p1+4) == ';')
				{
					*pOut = 0x0a;
					p1 += 5;
				}
				else if(*(p1+1) == 'q' && *(p1+2) == 'u' && *(p1+3) == 'o' && *(p1+4) == 't' && *(p1+5) == ';')
				{
					*pOut = '"';
					p1 += 6;
				}
				pOut += 1;
				
			}
			else
				*(pOut++) = *(p1++);
		}
	}
}

char *IntposProc(char *pszBegin, char *pszEnd)
{
	tag_intpos stIntpos;
	int iLine = 0, i=0;
	uchar *p1, *p2, *p, szBuff[4096]=""; //modified by wuc 2014.1.15
	int iRet;
	ST_FONT font1;
	

	memset(&stIntpos, 0, sizeof(tag_intpos));
	iRet = ParseIntPos(&stIntpos, pszBegin, pszEnd);
	if(iRet != 0)
	{
		return NULL;
	}

	if(glSysCtrl.bVTEFOpt)
	{
		ConvertIntpos(stIntpos.szData, szBuff, 1);
		WriteMob001(szBuff, strlen(szBuff));
		p1 = strstr(stIntpos.szData, "030-000 = ");
		if(p1 != NULL)
		{
			p1 += strlen("030-000 = ");
			p2 = strstr(p1, "&#xD;&#xA;");
			if(p2 != NULL)
			{
				memcpy(szBuff, p1, p2-p1);
				szBuff[p2-p1] = 0;
				ScrClrBelow(2);
				PubDispString(szBuff, 4|DISP_LINE_CENTER);
				PubWaitKey(2);
			}
		}
		
		goto END;
	}
	else //add by wuc 2014.4.1
	{
		ConvertIntpos(stIntpos.szData, szBuff, 1);
		memset(gstIntpos.szTranNum, 0x00, sizeof(gstIntpos.szTranNum));
		SearchIntPos("012-000", gstIntpos.szTranNum, szBuff);
	}

	if(!glSysCtrl.bEnableMsg) //add by wuc 2014.4.1
	{
		goto END;
	}

	memset(szBuff, 0, sizeof(szBuff));
	font1.CharSet = 0x01;
	font1.Width   = 8;
	font1.Height  = 16;
	font1.Bold    = 0;
	font1.Italic  = 0;

	
//COMPLETE RECEIPT
	p1 = strstr(stIntpos.szData, "028-000 = ");
	if(p1 != NULL)
	{
		p1 += strlen("028-000 = ");
		p2 = strstr(p1, "&#xD;&#xA;");
		if(p2 != NULL)
		{
			memcpy(szBuff, p1, p2-p1);
			szBuff[p2-p1] = 0;
			iLine = atoi(szBuff);
		}


		p = p2+strlen("&#xD;&#xA;");
		if(iLine != 0)
		{
			if(glProcInfo.ucTranType == PAYMENT && glProcInfo.ucEntryMode!= INSERT_MODE)
			{
				ScrClrBelow(2);
				PubDispString(_T("Transaction Accepted"), 4|DISP_LINE_CENTER);
				PubWaitKey(2);
			}
			PrnInit();
			PrnSelectFont(&font1,NULL);
			PrnDoubleHeight(1, 1);
			PrnLeftIndent(60);
			PrnLogo(sCielo_PB_Logo); //add by wuc 2014.9.9
			PrnLeftIndent(0);
			for(i=0; i<iLine; i++)
			{
				memset(szBuff, 0, sizeof(szBuff));
				p1 = strstr(p, "029");
				if(p1 == NULL)
				{
					break;
				}
				p1 += 10; //"029-xxx = "
				p1 += strlen("&quot;");
				p2 = strstr(p1, "&quot;");
				if(p2 != NULL)
				{
					memcpy(szBuff, p1, p2-p1);
					szBuff[p2-p1] = 0;
					if(strncmp(szBuff, "................................", 32) == 0)
					{
						PrnStr("\n\n\n\n\n\n\n\n");
						StartPrinter();
						PrnInit();
						PrnSelectFont(&font1,NULL);
						PrnDoubleHeight(1, 1);
						PrnLeftIndent(60);
						PrnLogo(sCielo_PB_Logo); //add by wuc 2014.9.9
						PrnLeftIndent(0);
						ScrClrBelow(2);
						PubDispString(_T("PRINT SECOND VIA?"), 4|DISP_LINE_CENTER);
						while(1)
						{
							iRet = PubWaitKey(USER_OPER_TIMEOUT);
							if(iRet == KEYENTER || iRet == NOKEY)
							{
								break;
							}
							else if(iRet == KEYCANCEL)
							{
								goto NEXT1;
							}
						}
					}
					else
					{
						PrnStr(szBuff);
						PrnStr("\n");
					}
				}
				p = p2+strlen("&quot;&#xD;&#xA;");
			}
			PrnStr("\n\n\n\n\n\n\n\n");
			StartPrinter();
		}
	}

NEXT1: //remove by wuc 2014.9.9
//SHORT RECEIPT
	p1 = strstr(stIntpos.szData, "710");
	if(p1 != NULL)
	{
		p1 += 10; //"710-xxx = "
		p2 = strstr(p1, "&#xD;&#xA;");
		if(p2 != NULL)
		{
			memcpy(szBuff, p1, p2-p1);
			szBuff[p2-p1] = 0;
			iLine = atoi(szBuff);
		}

		if(iLine != 0)
		{
			if(glProcInfo.ucTranType == PAYMENT && glProcInfo.ucEntryMode!= INSERT_MODE)
			{
				ScrClrBelow(2);
				PubDispString(_T("Transaction Accepted"), 4|DISP_LINE_CENTER);
				PubWaitKey(2);
			}
			p = p2+strlen("&#xD;&#xA;");
			PrnInit();
			PrnSelectFont(&font1,NULL);
			PrnDoubleHeight(1, 1);
			PrnLeftIndent(60);
			PrnLogo(sCielo_PB_Logo); //add by wuc 2014.9.9
			PrnLeftIndent(0);
			for(i=0; i<iLine; i++)
			{
				memset(szBuff, 0, sizeof(szBuff));
				p1 = strstr(p, "711");
				if(p1 == NULL)
				{
					break;
				}
				p1 += 10; //"711-xxx = "
				p1 += strlen("&quot;");
				p2 = strstr(p1, "&quot;");
				if(p2 != NULL)
				{
					memcpy(szBuff, p1, p2-p1);
					szBuff[p2-p1] = 0;
					if(strcmp(szBuff, "................................") == 0)
					{
						PrnStr("\n\n\n\n\n\n\n\n");
						StartPrinter();
						PrnInit();
						PrnSelectFont(&font1,NULL);
						PrnDoubleHeight(1, 1);
						PrnLeftIndent(60);
						PrnLogo(sCielo_PB_Logo); //add by wuc 2014.9.9
						PrnLeftIndent(0);
						ScrClrBelow(2);
						PubDispString(_T("PRINT SECOND VIA?"), 4|DISP_LINE_CENTER);
						while(1)
						{
							iRet = PubWaitKey(USER_OPER_TIMEOUT);
							if(iRet == KEYENTER || iRet == NOKEY)
							{
								break;
							}
							else if(iRet == KEYCANCEL)
							{
								goto NEXT2;
							}
						}
					}
					else
					{
						PrnStr(szBuff);
						PrnStr("\n");
					}
				}
				p = p2 + strlen("&quot;&#xD;&#xA;");
			}
			PrnStr("\n\n\n\n\n\n\n\n");
			StartPrinter();
		}
	}

NEXT2: //remove by wuc 2014.9.9
//RECEIPT DIFFRENTIATED FOR CUSTOMER
	p1 = strstr(stIntpos.szData, "712");
	if(p1 != NULL)
	{
		p1 += 10; //"712-xxx = "
		p2 = strstr(p1, "&#xD;&#xA");
		if(p2 != NULL)
		{
			memcpy(szBuff, p1, p2-p1);
			szBuff[p2-p1] = 0;
			iLine = atoi(szBuff);
		}

		if(iLine != 0)
		{
			if(glProcInfo.ucTranType == PAYMENT && glProcInfo.ucEntryMode!= INSERT_MODE)
			{
				ScrClrBelow(2);
				PubDispString(_T("Transaction Accepted"), 4|DISP_LINE_CENTER);
				PubWaitKey(2);
			}
			p = p2+strlen("&#xD;&#xA;");
			PrnInit();
			PrnSelectFont(&font1,NULL);
			PrnDoubleHeight(1, 1);
			PrnLeftIndent(60);
			PrnLogo(sCielo_PB_Logo); //add by wuc 2014.9.9
			PrnLeftIndent(0);
			for(i=0; i<iLine; i++)
			{
				memset(szBuff, 0, sizeof(szBuff));
				p1 = strstr(p, "713");
				if(p1 == NULL)
				{
					break;
				}
				p1 += 10; //"713-xxx = "
				p1 += strlen("&quot;");
				p2 = strstr(p1, "&quot;");
				if(p2 != NULL)
				{
					memcpy(szBuff, p1, p2-p1);
					szBuff[p2-p1] = 0;
					if(strcmp(szBuff, "................................") == 0)
					{
						PrnStr("\n\n\n\n\n\n\n\n");
						StartPrinter();
						PrnInit();
						PrnSelectFont(&font1,NULL);
						PrnDoubleHeight(1, 1);
						PrnLeftIndent(60);
						PrnLogo(sCielo_PB_Logo); //add by wuc 2014.9.9
						PrnLeftIndent(0);
						ScrClrBelow(2);
						PubDispString(_T("PRINT SECOND VIA?"), 4|DISP_LINE_CENTER);
						while(1)
						{
							iRet = PubWaitKey(USER_OPER_TIMEOUT);
							if(iRet == KEYENTER || iRet == NOKEY)
							{
								break;
							}
							else if(iRet == KEYCANCEL)
							{
								goto NEXT3;
							}
						}
					}
					else
					{
						PrnStr(szBuff);
						PrnStr("\n");
					}
				}
				p = p2 + strlen("&quot;&#xD;&#xA;");
			}
			PrnStr("\n\n\n\n\n\n\n\n");
			StartPrinter();
		}
	}

NEXT3:
//RECEIPT DIFFERENTIATED FOR MERCHANT
	p1 = strstr(stIntpos.szData, "714");
	if(p1 != NULL)
	{
		p1 += 10; //"710-xxx = "
		p2 = strstr(p1, "&#xD;&#xA");
		if(p2 != NULL)
		{
			memcpy(szBuff, p1, p2-p1);
			szBuff[p2-p1] = 0;
			iLine = atoi(szBuff);
		}

		if(iLine != 0)
		{
			if(glProcInfo.ucTranType == PAYMENT && glProcInfo.ucEntryMode!= INSERT_MODE)
			{
				ScrClrBelow(2);
				PubDispString(_T("Transaction Accepted"), 4|DISP_LINE_CENTER);
				PubWaitKey(2);
			}
			p = p2+strlen("&#xD;&#xA;");
			PrnInit();
			PrnSelectFont(&font1,NULL);
			PrnDoubleHeight(1, 1);
			PrnLeftIndent(60);
			PrnLogo(sCielo_PB_Logo); //add by wuc 2014.9.9
			PrnLeftIndent(0);
			for(i=0; i<iLine; i++)
			{
				memset(szBuff, 0, sizeof(szBuff));
				p1 = strstr(p, "715");
				if(p1 == NULL)
				{
					break;
				}
				p1 += 10; //"715-xxx = "
				p1 += strlen("&quot;");
				p2 = strstr(p1, "&quot;");
				if(p2 != NULL)
				{
					memcpy(szBuff, p1, p2-p1);
					szBuff[p2-p1] = 0;
					if(strcmp(szBuff, "................................") == 0)
					{
						PrnStr("\n\n\n\n\n\n\n\n");
						StartPrinter();
						PrnInit();
						PrnSelectFont(&font1,NULL);
						PrnDoubleHeight(1, 1);
						PrnLeftIndent(60);
						PrnLogo(sCielo_PB_Logo); //add by wuc 2014.9.9
						PrnLeftIndent(0);
						ScrClrBelow(2);
						PubDispString(_T("PRINT SECOND VIA?"), 4|DISP_LINE_CENTER);
						while(1)
						{
							iRet = PubWaitKey(USER_OPER_TIMEOUT);
							if(iRet == KEYENTER || iRet == NOKEY)
							{
								break;
							}
							else if(iRet == KEYCANCEL)
							{
								goto END;
							}
						}
					}
					else
					{
						PrnStr(szBuff);
						PrnStr("\n");
					}
				}
				p = p2 + strlen("&quot;&#xD;&#xA;");
			}
			PrnStr("\n\n\n\n\n\n\n\n");
			StartPrinter();
		}
	}

END:
	return (stIntpos.ppintpos_end);
	
}
char *CompProc(char *pszBegin, char *pszEnd)
{
	tag_comp stComp;

	memset(&stComp, 0, sizeof(tag_comp));

	if(ParseComp(&stComp, pszBegin, pszEnd) != 0)
	{
		return NULL;
	}
	
	if (stComp.comp_begin == NULL)
	{
		return NULL;
	}
	else
	{
		if(strcmp(stComp.szCondition, glCondition) != 0)
		{
			return NULL;
		}
		
		if (TRUE == stComp.bClrScr)
		{
			ScrCls();
		}

		PicboxProc(stComp.comp_begin, stComp.comp_end);
		LabelProc(stComp.comp_begin, stComp.comp_end);
		if(ListProc(stComp.comp_begin, stComp.comp_end) != 0)
		{
			return NULL;
		}
		if(TextboxProc(stComp.comp_begin, stComp.comp_end) != 0)
		{
			return NULL;
		}
		if(RadbuttProc(stComp.comp_begin, stComp.comp_end) != 0)
		{
			return NULL;
		}
		MsgboxProc(stComp.comp_begin, stComp.comp_end);
	}

	return (stComp.comp_end);
	//return 0;
}

char *ppProc(char *pszBegin, char *pszEnd)
{
	tag_ppcomp stPP;
	uchar szEleName[128+1] = "";
	uchar szAttr[LEN_BODY_DATA+1] = "";
	uchar szValue[LEN_BODY_DATA+1] = "";
	uchar szOutput[2][512];
	int iValueLen = 0, iRet=0, iFlag=0;
	ELEMENT_ATTR_XML ele_attr;
	uchar *p;
	
	memset(&stPP, 0, sizeof(tag_ppcomp));
	if(ParsePPcomp(&stPP, pszBegin, pszEnd) != 0)
	{
		return NULL;
	}
	
	p = stPP.ppcomp_begin+strlen("<ppcomp>");

	while(1)
	{
		iRet = XML_GetElement(p, szEleName, szAttr, szValue, &iValueLen);
		if(iRet < 0)
		{
			break;
		}
		p += iRet;
		iFlag = 0;
		if(strlen(szAttr) != 0)
		{
			memset(&ele_attr, 0x00, sizeof(ELEMENT_ATTR_XML));
			ParseElementAttr(szAttr, &ele_attr);
			if((strlen(ele_attr.szFlag) != 0) && (strcmp(ele_attr.szFlag, glSetFormFlag) != 0))
			{
				continue;
			}
			iFlag = 1;
		}
		
		if(iFlag)	//if has attribute
		{	
			memset(szOutput, 0, sizeof(szOutput));
			iRet = FuncPPExe(szEleName, glFunc, ele_attr.szInput[0], ele_attr.szInput[1], ele_attr.szInput[2], szOutput[0], szOutput[1]);
			//DebugOutput("%s-%d-%s,szEleName:%s, els.szInput[0]:%s, els.szInput[1]:%s,, szInput[2]:%s, szOutput[0]:%s\n, szOutput[1]:%s\n", __FILE__, __LINE__, __FUNCTION__, szEleName, ele_attr.szInput[0], ele_attr.szInput[1], ele_attr.szInput[2], szOutput[0], szOutput[1]);//linzhao
			if(iRet >= 0)
			{
				strcpy(glPPComp[glPPCompIdx].szFuncID, ele_attr.szID);
				memcpy((uchar *)glPPComp[glPPCompIdx].szInput, (uchar *)ele_attr.szInput, 3*sizeof(ele_attr.szInput[0]));
				memcpy((uchar *)glPPComp[glPPCompIdx].szOutput, (uchar *)szOutput, 2*sizeof(szOutput[0]));
				glPPComp[glPPCompIdx].uiRespCode = iRet;
				glPPCompIdx++;
			}
			
		}
		else
		{
			FuncPPExe(szEleName, glFunc, szValue, "", "", "", "");
		}
	}

	//pszBegin = stPP.ppcomp_end+strlen("</ppcomp>");
	return (stPP.ppcomp_end);
}

int FlagProc(char *pszFlag)
{
	char *pszTempStart;
	char *pszTempEnd;
	char szFlag[64];
	
	if (strcmp(pszFlag, "") != 0)
	{
		pszTempStart = pszFlag;
		pszTempEnd = strchr(pszFlag, ';');
		
		while(NULL != pszTempEnd)
		{
			memset(szFlag, 0, sizeof(szFlag));
			memcpy(szFlag, pszTempStart, pszTempEnd-pszTempStart);
			szFlag[pszTempEnd-pszTempStart] = '\0';
			if (strcmp(szFlag, glSetFormFlag) == 0)
			{
				return TRUE;
			}
			else
			{
				pszTempStart = pszTempEnd + 1;
				pszTempEnd = strchr(pszTempStart, ';');
			}
		}
		
		if (NULL == pszTempEnd)
		{
			memset(szFlag, 0, sizeof(szFlag));
			strcpy(szFlag, pszTempStart);
			if (strcmp(szFlag, glSetFormFlag) != 0)
			{
				return FALSE;  // flag does not match
			}
		}		
	}// if(NULL!=FLAG && NULL!=pszFlag)
	
	return TRUE;
}

char *FormProc(char *xml_data, char *szID)
{
	tag_form stForm;
	uchar *szBegin, *szEnd;
	uchar *pCompRet, *pPPRet, *pIntposRet;
	
	int bFormProc;

	memset(glCondition, 0, 64);
	if (xml_data == NULL)
	{
		return NULL;
	}

	memset(&stForm, 0, sizeof(tag_form));
	ParseForm(xml_data, &stForm);
	if (stForm.form_begin == NULL || stForm.form_end == NULL)
	{
		return NULL;
	}
	else
	{
		if(szID != NULL)
		{
			strcpy(szID, stForm.szID);
		}
		bFormProc = FlagProc(stForm.szFormFlag);
		if (FALSE == bFormProc)
		{
			return stForm.form_end;		// if flags are not match, then tag<form> should not be processed
		}

		szBegin = stForm.form_begin;
		szEnd = stForm.form_end;
		while(1)
		{
			pCompRet = CompProc(szBegin, szEnd);
			if(bIsCancel )
			{
				return NULL;
			}
			if(pCompRet != NULL)
			{
				szBegin = pCompRet;
			}

			pPPRet = ppProc(szBegin, szEnd);
			if(pPPRet != NULL)
			{
				szBegin = pPPRet;
			}

			pIntposRet = IntposProc(szBegin, szEnd);
			if(pIntposRet != NULL)
			{
				szBegin = pIntposRet;
			}
			
			if(pCompRet == NULL && pPPRet == NULL && pIntposRet == NULL)
			{
				break;
			}
		};
	}

	return stForm.form_end;
}

int digitProc(uchar *pBegin, uchar *pEnd, uchar *pOutput)
{
	if (pEnd-pBegin==1)
	{
		*pOutput = '0';
		memcpy(pOutput+1, pBegin, pEnd-pBegin);
	}
	else 
	{
		memcpy(pOutput, pBegin, pEnd-pBegin);
	}
	*(pOutput+2) = 0;

	return 0;
}

//报文日期时间格式：mm/dd/yyyy, 所以须更改为POS对应格式. add by linzhao
int DateFormatChange(char *pDate, int iLen, char *pOutput)
{
	uchar ucYear[4], ucMonth[4], ucDay[4];
	char *pBegin, *pEnd;
	uchar ucTemp[16];

	if (NULL==pDate || 0==strlen(pDate))
	{
		return -1;
	}
	pBegin = pDate;
	pEnd = strchr(pBegin, '/');
	if (NULL==pEnd)
	{
		return -1;
	}
	digitProc(pBegin, pEnd, ucTemp);
	strcpy(ucMonth, ucTemp);

	pBegin = pEnd + 1;
	pEnd = strchr(pBegin, '/');
	if (NULL==pEnd)
	{
		return -1;
	}
	digitProc(pBegin, pEnd, ucTemp);
	strcpy(ucDay, ucTemp);

	pBegin = pEnd + 1;
	pEnd = pDate + iLen;
	memcpy(ucTemp, pBegin, pEnd-pBegin);
	ucTemp[pEnd-pBegin] = 0;
	strcpy(ucYear, ucTemp);

	sprintf(pOutput, "%s%s%s", ucYear, ucMonth, ucDay);

	return 0;
}

int CurrentTimeFormatChange(uchar *pValue, int iLen, uchar *pOutput)
{
	uchar ucHour[8], ucMin[8], ucSec[8], ucTemp[16];
	uchar *pBegin, *pEnd;
	int iHour;

	if (NULL==pValue || 0==strlen(pValue))
	{
		return -1;
	}
	pBegin = pValue;
	pEnd = strchr(pBegin, ':');
	if (NULL==pEnd)
	{
		return -1;
	}
	digitProc(pBegin, pEnd, ucTemp);
	if ( NULL!=strstr(pValue, "PM") )
	{
		iHour = atoi(ucTemp);
		if (iHour<12)
		{
			iHour += 12;
			sprintf(ucTemp,"%d", iHour);
		}
		else
		{
			strcpy(ucTemp, "00");
		}
	}
	strcpy(ucHour, ucTemp);

	pBegin = pEnd + 1;
	pEnd = strchr(pBegin, ':');
	if (NULL==pEnd)
	{
		return -1;
	}
	memcpy(ucTemp, pBegin, pEnd-pBegin);
	ucTemp[pEnd-pBegin] = 0;
	strcpy(ucMin, ucTemp);

	pBegin = pEnd + 1;
	pEnd = pValue + iLen;
	memcpy(ucTemp, pBegin, 2);
	ucTemp[2] = 0;
	strcpy(ucSec, ucTemp);

	sprintf(pOutput, "%s%s%s", ucHour, ucMin, ucSec);

	return 0;
}

void IDProc(uchar *szData)
{
	uchar *pBegin;
	int iRet;
	uchar szEleName[128+1] = "";
	uchar szAttr[LEN_BODY_DATA+1] = "";
	uchar szValue[LEN_BODY_DATA+1] = "";
	int iValueLen;
	uchar szDate[32], szTimeset[64], szTimesetBCD[64];
	uchar szCurTime[32];

	pBegin = szData;
	
	while(1)
	{
		iRet = XML_GetElement(pBegin, szEleName, szAttr, szValue, &iValueLen);
		if(iRet < 0)
		{
			break;
		}

		pBegin += iRet;
		if(memcmp(szEleName, "processadordedadosorigem", strlen("processadordedadosorigem")) == 0)
		{
			memcpy(glszDeDadosDestino, szValue, iValueLen);
		}
		else if(memcmp(szEleName, "processorede", strlen("processorede")) == 0)
		{
			memcpy(glszProcessorede, szValue, iValueLen);
		}
		else if(memcmp(szEleName, "processadordedadosrede", strlen("processadordedadosrede")) == 0)
		{
			memcpy(glszDeDadosrede, szValue, iValueLen);
		}
		//CIELO需要增加时间设置,add by linzhao
		else if( 0==memcmp(szEleName, "dataservidor", strlen("dataservidor")) )
		{
			//2/9/2015 format
			//DateFormatChange(szValue, iValueLen, szDate);
			//DD/MM/YYYY format
			sprintf(szDate, "%2.2s%2.2s%2.2s", szValue+8, szValue+3, szValue);
		}
		else if( 0==memcmp(szEleName, "horaservidor", strlen("horaservidor")))
		{
			//HH:MM:SS AM format 
			//CurrentTimeFormatChange(szValue, iValueLen, szCurTime);
			//sprintf(szTimeset, "%s%s", szDate+2, szCurTime);
			//HH:MM:SS  format
			sprintf(szTimeset, "%s%2.2s%2.2s%2.2s", szDate, szValue, szValue+3, szValue+6);

		}
	}
	if ( 12==strlen(szTimeset) )
	{
		PubAsc2Bcd(szTimeset, strlen(szTimeset), szTimesetBCD);
		SetTime(szTimesetBCD);
	}

}

void HeaderProc(uchar *szData)
{
	uchar *pBegin, *pEnd;
	int iRet;
	uchar szEleName[128+1] = "";
	uchar szAttr[LEN_BODY_DATA+1] = "";
	uchar szValue[LEN_BODY_DATA+1] = "";
	int iValueLen;
	
	pBegin = strstr(szData, "<header>");
	pEnd = strstr(szData, "</header>");
	if(pBegin == NULL)
	{
		return;
	}

	pBegin += strlen("<header>");
	while(1)
	{
		memset(szEleName, 0, sizeof(szEleName));
		memset(szAttr, 0, sizeof(szAttr));
		memset(szValue, 0, sizeof(szValue));
		iRet = XML_GetElement(pBegin, szEleName, szAttr, szValue, &iValueLen);
		if(iRet < 0)
		{
			break;
		}

		pBegin += iRet;
		//linzhao
		/*if (memcmp(szEleName, ""))
		{
		}*/
		if(memcmp(szEleName, "id", 2) == 0)
		{
			IDProc(szValue);
		}
		else if(memcmp(szEleName, "controlesequencia", strlen("controlesequencia")) == 0)
		{
			if(strlen(szValue) != 0)
			{
				glSeqNo = atol(szValue);
			}
		}
		else if(memcmp(szEleName, "tipomensagem", strlen("tipomensagem")) == 0)
		{
			memcpy(glszMessageType, szValue, iValueLen);
		}
		else if(memcmp(szEleName, "extra_proximaacao", strlen("extra_proximaacao")) == 0) //add by wuc 2014.10.11
		{
			if(strcmp(szValue, "ativacao") == 0)
			{
				glActivate = 1;
			}
		}
		if(pBegin == pEnd)
		{
			break;
		}
	}

	SaveSysCtrl();

}
////////////////////////////////////////////////////////////////////////////////////////


// end of file



