#include "posapi.h"
#include "util.h"
#include "appLib.h"
#include "TranProc.h"
#include "lng.h"
#include "ppcomp.h"
#include "FileOper.h"
#include "gzip.h"
#include "xmlParser.h"
#include "xmlProc.h"
#include "SxxCom.h"
#include "setup.h" //add by wuc 2014.4.1
#include "global.h"


#define HOST_IP	"177.71.253.42"//177.71.185.250"//"177.71.248.221"
#define HOST_PORT 6502

static const uchar gzip_header[10] = {0x1f, 0x8b, 8, 0, 0, 0, 0, 0, 0, 0x03};
static uchar bUserCancel = 0; //add by wuc 2014.4.1

#define HEADER_TAG	"header"
#define BODY_TAG	"dados"

extern void ConvertIntpos(char *szInput, char *szOutput, uchar ucMode);
extern int WriteATV001(void);
extern char glIDName[128];
char glTimeStamp[12];
int LoadTimeStamp(uchar *szTimeStamp);
int LoadAID(uchar *szAID);
int LoadKey(uchar *szKey);
int LoadInitPage(uchar *szInitPage);
extern int ProcXMLPage(uchar *szFile);

TransInfo glTransInfo[] =
{
	{ACTIVE, "atualizacaotabelaspinpad"},
	{ADMIN, "administrativo"},
	{PAYMENT, "cartao"},
	{VOID, "cancelamento"},
	{0xff, ""},
};

void GetTransInfo(uchar ucTransType)
{
	uint uiCnt;

	for(uiCnt=0; glTransInfo[uiCnt].ucTransType!=0xff; uiCnt++)
	{
		if(ucTransType == glTransInfo[uiCnt].ucTransType)
		{
			memset(glProcInfo.szService, 0, sizeof(glProcInfo.szService));
			memcpy(glProcInfo.szService, glTransInfo[uiCnt].szService, strlen(glTransInfo[uiCnt].szService));
			break;
		}
	}
}

int BuildHeaderID(uchar *szID)
{	
	uint uiLen = 0;
	uchar szOrigem[16];

	//%02s%04s%03s->%02s%08s%03s, Cielo need to change the size of glSysCtrl.szStoreNum. linzhao 20150311
	sprintf(szOrigem, "%02s%08s%03s", glSysCtrl.szMerchantNum, glSysCtrl.szStoreNum, glSysCtrl.szPosNum);
	uiLen += XML_AddElement(szID, NULL, "processoorigem", szOrigem);
	uiLen += XML_AddElement(szID+uiLen, NULL, "processodestino", glSysCtrl.szDestino);
	if(strlen(glszDeDadosDestino) != 0)
	{
		uiLen += XML_AddElement(szID+uiLen, NULL, "processadordedadosdestino", glszDeDadosDestino);
	}
	if(strlen(glszProcessorede) != 0)
	{
		uiLen += XML_AddElement(szID+uiLen, NULL, "processorede", glszProcessorede);
	}
	/*if(strlen(glszDeDadosrede) != 0)//cielo新需求 add by linzhao 2015.1.23
	{*/
		uiLen += XML_AddElement(szID+uiLen, NULL, "processadordedadosrede", "1");
//	}

	return uiLen;
}

int BuildHeader(uchar *szHeader, uchar *szMsgType)
{
	uint uiLen = 0;
	uchar szID[256], szSeq[16];
	uchar szData[800];

	memset(szID, 0, sizeof(szID));
	memset(szData, 0, sizeof(szData));
	memset(szSeq, 0, sizeof(szSeq));
	
	BuildHeaderID(szID);

	uiLen = XML_AddElement(szData, NULL, "id", szID);

	if(strcmp(szMsgType, "confirmacao") == 0 || strcmp(szMsgType, "naoconfirmacao") == 0)
	{
		sprintf(szSeq, "%d", 0);
		uiLen += XML_AddElement(szData+uiLen,NULL,  "nroempresa", glSysCtrl.szMerchantNum);
		uiLen += XML_AddElement(szData+uiLen, NULL, "nroloja", glSysCtrl.szStoreNum);
		uiLen += XML_AddElement(szData+uiLen, NULL, "nropdv", glSysCtrl.szPosNum);
		uiLen += XML_AddElement(szData+uiLen, NULL, "tipomensagem", szMsgType);
		uiLen += XML_AddElement(szData+uiLen, NULL, "controlesequencia", szSeq);
		uiLen += XML_AddElement(szData+uiLen, NULL, "versaopos", STR_VERSION);
	}
	else
	{
		if(strcmp(szMsgType, "identificacao"))
		{
			sprintf(szSeq, "%d", glSeqNo);
			uiLen += XML_AddElement(szData+uiLen,NULL,  "nroempresa", glSysCtrl.szMerchantNum);
			uiLen += XML_AddElement(szData+uiLen, NULL, "nroloja", glSysCtrl.szStoreNum);
			uiLen += XML_AddElement(szData+uiLen, NULL, "nropdv", glSysCtrl.szPosNum);
			uiLen += XML_AddElement(szData+uiLen, NULL, "controlesequencia", szSeq);
		//	uiLen += XML_AddElement(szData+uiLen, NULL, "versaopos", STR_VERSION);
		}
		
		uiLen += XML_AddElement(szData+uiLen, NULL, "tipomensagem", szMsgType);
	}
	

	uiLen = XML_AddElement(szHeader, NULL, HEADER_TAG, szData);
	return uiLen;
	
}

int BuildIntPos(uchar *szIntPos)
{
	uchar szData[1024] = "";
	int fd, iLen;
	
	iLen = filesize(FILE_MOBILE_FILE);
	fd = open(FILE_MOBILE_FILE, O_RDWR);
	if(fd < 0)
	{
		return 0;
	}

	seek(fd, 0, SEEK_SET);
	write(fd,szData, iLen);
	close(fd);

	ConvertIntpos(szData, szIntPos, 0);
	return 0;
}


int BuildCampo(uchar *szCampo, uchar *szMsgType, uchar *szService)
{
	uchar szIntPos[128];
	uchar szValue[2][512];
	uchar szOutput[2][512];
	int iLen = 0, i=0;
	uchar szIntPosTemp[2][512] = {"ADM&#xD;&#xA;", ""};

	memset(szValue, 0, sizeof(szValue));
	memcpy(szValue[0], szService, strlen(szService));
	iLen = AppendPostElement(szCampo, "campo", "tipoServico", 0xff, szValue);

	if(fexist(FILE_MOBILE_FILE) >= 0)
	{
		if(strcmp(szMsgType, "confirmacao") && strcmp(szMsgType, "naoconfirmacao"))
		{
			memset(szIntPos, 0, 128);
			BuildIntPos(szIntPos);
			memset(szValue, 0, sizeof(szValue));
			memcpy(szValue[0], szIntPos, strlen(szIntPos));
			iLen += AppendPostElement(szCampo, "intpos", "IntPos", 0xff, szValue);
		}
	}
	
	if(strcmp(szMsgType, "confirmacao") == 0 || strcmp(szMsgType, "naoconfirmacao") == 0)
	{
		memset(szValue, 0, sizeof(szValue));
		strcpy(szValue[0], gstIntpos.szTranNum);
		iLen += AppendPostElement(szCampo, "campo", "nsuTEF", 0xff, szValue);
	}
	
	if(strcmp(szMsgType, "solicitacao") == 0)
	{
		memset(szOutput, 0, sizeof(szOutput));
		i = PP_GetTimeStamp("03", szOutput[0]);
		iLen += AppendPostElement(szCampo, "campo", "PPGetTimeStamp03", i, szOutput);

		//Pos2.0新需求，add by linzhao 2015.1.23
		iLen += AppendPostElement(szCampo, "campo", "IntPos", 0xff, szIntPosTemp);
	}
	
	i = 0;
	while(strlen(glPPComp[i].szFuncID) != 0)
	{
		iLen += AppendPostElement(szCampo, "campo", glPPComp[i].szFuncID, glPPComp[i].uiRespCode, glPPComp[i].szOutput);
		i ++;
	}

	if(ACTIVE == glProcInfo.ucTranType)
	{
		memset(szValue, 0, sizeof(szValue));
		GetPPOnlineAuto(szValue[0]);
		iLen += AppendPostElement(szCampo, "campo", "PPOnlineHabilitadoAutomacao", 0xff, szValue);
		
		memset(szValue, 0, sizeof(szValue));
		GetPPOnlineClient(szValue[0]);
		iLen += AppendPostElement(szCampo, "campo", "PPOnlineHabilitadoClient", 0xff, szValue);
		
		memset(szValue, 0, sizeof(szValue));
		GetCPF_CNPJ(szValue[0]);
		iLen += AppendPostElement(szCampo, "campo", "CPF_CNPJ", 0xff, szValue);
	}

	return iLen;
}


int BuildPost(uchar *szPost, uchar *szPostID)
{
	uchar szAttr[64];
	int iLen = 0;

	memset(szAttr, 0, 64);
	sprintf(szAttr, "idtela=\"%s\"", szPostID);
	iLen = XML_AddElement(szPost, szAttr, "post", glMsgData.szPost);
	return iLen;
}

int BuildBody(uchar *szBody, uchar *szMsgType)
{
	uint uiLen = 0;
	uchar szPost[LEN_BODY_DATA+20];

	memset(szPost, 0, LEN_BODY_DATA);
	if(strcmp(szMsgType, "identificacao")) //modify by wuc 2014.4.1
	{
		if(strcmp(szMsgType, "confirmacao") && strcmp(szMsgType, "naoconfirmacao"))
		{
			BuildPost(szPost, glIDName);
		}
		else
		{
			XML_AddElement(szPost, NULL, "post", glMsgData.szPost);
		}
	}
	
	uiLen = XML_AddElement(szBody, NULL, BODY_TAG, szPost);
	return uiLen;
}

void put_long(unsigned char *string, unsigned long x, unsigned char cMode) 
{
	if(cMode == 0)
	{
		string[0] = (uchar)(x & 0xff);
		string[1] = (uchar)((x >> 8) & 0xff);
		string[2] = (uchar)((x >> 16) & 0xff);
		string[3] = (uchar)((x >> 24) & 0xff);
	}
	else
	{
		string[0] = (uchar)((x >> 24) & 0xff);
		string[1] = (uchar)((x >> 16) & 0xff);
		string[2] = (uchar)((x >> 8) & 0xff);
		string[3] = (uchar)(x & 0xff);
	}
}

unsigned long get_long(unsigned char *string, unsigned char cMode)
{
	unsigned long lRet;

	if(cMode == 0)
	{
		lRet = (string[3]<<24)+(string[2]<<16)+(string[1]<<8)+string[0];
	}
	else
	{
		lRet = (string[0]<<24)+(string[1]<<16)+(string[2]<<8)+string[3];
	}

	return lRet;
}

int BuildPacket(uchar *szMsgType)
{	
	uchar szTemp[LEN_MAX_COMM_DATA+1] = "";
	int iRet;
	ulong ulOutLen = LEN_MAX_COMM_DATA, ulCRC, ulTempLenth;
	uchar szHttpHeader[] = "PUT / HTTP/1.0\r\nX-Forwarded-Host: mpfdev.cielo.com.br:443\r\nContent-Length:";//linzhao

	BuildHeader(glMsgData.szHeader, szMsgType);
	BuildBody(glMsgData.szBody, szMsgType);

	sprintf(szTemp, "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\r\n<mensagem>\r\n%s\r\n%s\r\n</mensagem>", glMsgData.szHeader, glMsgData.szBody);	
	ulTempLenth = strlen(szTemp);
	sprintf(glSendData.sContent, "%s%d\r\n\r\n%s", szHttpHeader, ulTempLenth, szTemp);//linzhao
	glSendData.ulLength = strlen(glSendData.sContent);

	if(glSysCtrl.bDebugOpt)
	{
		DebugOutput("SEND>>>");
		DebugOutput(glSendData.sContent);
	}
	
	if(glSysCtrl.bGzipOpt)
	{
		memset(szTemp, 0, LEN_MAX_COMM_DATA);
		ulCRC = gzCRC32(glSendData.sContent, glSendData.ulLength);
		iRet = gzCompress(glSendData.sContent, glSendData.ulLength, szTemp, &ulOutLen);
		if(iRet != 0)
		{
			return iRet;
		}

		memset(glSendData.sContent, 0, LEN_MAX_COMM_DATA);
		memcpy(glSendData.sContent+4, gzip_header, 10);
		memcpy(glSendData.sContent+10+4, szTemp, ulOutLen);
		put_long(glSendData.sContent+10+4+ulOutLen, ulCRC, 0);
		put_long(glSendData.sContent+10+4+ulOutLen+4, glSendData.ulLength, 0);
		put_long(glSendData.sContent, 10+4+ulOutLen+4, 1);	//10 gzip header + 4 CRC + 4 origin len + compress len
		glSendData.ulLength = 10+4+4+4+ulOutLen;
	}
	else
	{
		memmove(glSendData.sContent+4, glSendData.sContent, glSendData.ulLength);
		put_long(glSendData.sContent, glSendData.ulLength, 1);
		glSendData.ulLength += 4;
	}
	return 0;
}

int AnaylsePacket(uchar *szPacket, unsigned long ulLen)
{
	int fd;
	int iRet;
	uchar szTemp[LEN_MAX_COMM_DATA+1024];
	ulong ulOutLen = LEN_MAX_COMM_DATA;
	ulong ulCRC, ulTemp;

	memset(szTemp, 0, sizeof(szTemp));

	if(glSysCtrl.bGzipOpt)
	{
		iRet = gzDecompress(szPacket+10, ulLen-18, szTemp, &ulOutLen);
		if(iRet != 0)
		{
			return iRet;
		}
		
	//verify CRC
		ulCRC = gzCRC32(szTemp, ulOutLen);
		ulTemp = get_long(szPacket+ulLen-8, 0);
		if(ulCRC != ulTemp)
		{
			ScrClrBelow(4);
			PubDispString(_T("VERIFY CRC FAILED"), 6|DISP_LINE_CENTER);
			PubWaitKey(5);
			return -1;
		}

	//verfiy origin message length
		ulTemp = get_long(szPacket+ulLen-4, 0);
		if(ulOutLen != ulTemp)
		{
			ScrClrBelow(4);
			PubDispString(_T("VERIFY LENGTH FAILED"), 6|DISP_LINE_CENTER);
			PubWaitKey(5);
			return -1;
		}
	}
	else
	{
		ulOutLen = glRecvData.ulLength;
		memcpy(szTemp,glRecvData.sContent, glRecvData.ulLength);
	}

	if(glSysCtrl.bDebugOpt)
	{
		DebugOutput("RECV<<<");
		DebugOutput(szTemp);
	}

	fd = open("TMP.XML", O_CREATE|O_RDWR);
	if(fd < 0)
	{
		return -1;
	}

	seek(fd, 0, SEEK_SET);
	write(fd, szTemp, ulOutLen);
	close(fd);

	remove(FILE_MOBILE_FILE);

	iRet = ProcXMLPage("TMP.XML");
	if (glSysCtrl.bDebugOpt)
	{
		DebugOutput("%s--%d--%s:iRet:%d \n", __FILE__, __LINE__, __FUNCTION__, iRet);//linzhao
	}
	remove("TMP.XML");
	return iRet;
}

int TranProcess(uchar *szMsgType)
{
	int iRet;
	int iRetry=0;
#ifndef DEBUG_USE
	ulong ulExpLen, ulLen, ulTempLen;
#else
	int fd;
#endif

	if ( 1==glSysCtrl.ucFirstRun)
	{
		BuildPacketUnLock(szMsgType);
	}
	else if (2==glSysCtrl.ucFirstRun)
	{
		BuildPacketUnlockConfirm(szMsgType);
	}
	else
	{
		BuildPacket(szMsgType);
	}
	PPDispStartWait();

	for(iRetry=0; iRetry<3; iRetry++)
	{
		iRet = CommConnect();
		if(iRet == 0xff)
		{
			ScrClrBelow(2);
			PubDispString(_T("CONNECT CANCELLED"), DISP_LINE_CENTER|4);
			PubWaitKey(10);
			if(glProcInfo.ucTranType == ACTIVE) //modify by wuc 2014.4.1
			{
				return iRet;
			}
			bUserCancel = 1;
			return iRet;
		}
		if(iRet == 0)
		{
			break;
		}
	}

	if(iRet < 0)
	{
		ScrClrBelow(2);
		PubDispString(_T("CONNECT FAIL"), DISP_LINE_CENTER|4);
		PubWaitKey(10);
		return iRet;
	}

	iRetry = 0;

#ifdef DEBUG_USE
	glRecvData.ulLength = filesize("DEBUG.XML");
	fd = open("DEBUG.XML", O_RDWR);
	if(fd < 0)
	{
		return -1;
	}

	seek(fd, 0, SEEK_SET);
	memset(glRecvData.sContent, 0x00, glRecvData.ulLength);
	read(fd, glRecvData.sContent, glRecvData.ulLength);
	close(fd);

	iRet = AnaylsePacket(glRecvData.sContent, glRecvData.ulLength);
	if(glProcInfo.ucTranType == ACTIVE)
	{
		PP_TableLoadEnd();
	}
#else
	while(1)
	{
		iRet = CommSendMsg(glSendData.sContent, (ushort)glSendData.ulLength, COMM_TIMEOUT);
		DebugOutput("%s--%d--%s ,CommSendMsg iRet:%d, glSend.ulLength:%d\n", __FILE__, __LINE__, __FUNCTION__, iRet, glSendData.ulLength);//linzhao
		if(iRet != 0)
		{
			CommDisconnect();
			ScrClrBelow(2);
			PubDispString(_T("SEND FAIL"), DISP_LINE_CENTER|3);
			PubWaitKey(10);
			return iRet;
		}

		if(strcmp(szMsgType, "identificacao") == 0 || strcmp(szMsgType, "confirmacao") == 0
			|| strcmp(szMsgType, "naoconfirmacao") == 0)	//no need waiting for host response
		{
			return 0;
		}
	
		ulExpLen = 4;
		iRet = CommRecvMsg(glRecvData.sContent, ulExpLen, &glRecvData.ulLength, COMM_TIMEOUT);
		DebugOutput("%s--%d--%s , CommRecvMsg, iRet:%d, glRecv.ulLength:%d\n", __FILE__, __LINE__, __FUNCTION__, iRet, glRecvData.ulLength);//linzhao
		if(iRet == 0)
		{
			break;
		}
		else //modify by wuc 2014.9.9
		{
			/*iRetry ++;
			if(iRetry >=3 )
			{
				CommDisconnect();
				ScrClrBelow(2);
				PubDispString(_T("RECV TIMEOUT"), DISP_LINE_CENTER|3);
				PubWaitKey(10);
				return iRet;
			}
			continue;*/
			CommDisconnect();
			ScrClrBelow(2);
			PubDispString(_T("RECV TIMEOUT"), DISP_LINE_CENTER|3);
			PubWaitKey(10);
			return iRet;
		}
	}

WAIT_RESPONSE:
	ulExpLen = get_long(glRecvData.sContent, 1);
	memset(glRecvData.sContent, 0, sizeof(glRecvData.sContent));
	glRecvData.ulLength = 0;
	if(ulExpLen > 4096)
	{
		ulTempLen = 4096;
	}
	else
	{
		ulTempLen = ulExpLen;
	}

	if(glSysCtrl.bDebugOpt)
	{
		DebugOutput("RECV LENGTH = %ld", ulExpLen);
	}
	while(1)
	{
		iRet = CommRecvMsg(glRecvData.sContent+glRecvData.ulLength, ulTempLen, &ulLen, COMM_TIMEOUT);
		if(iRet != 0)
		{
			CommDisconnect();
			ScrClrBelow(2);
			PubDispString(_T("RECV DATA FAIL"), DISP_LINE_CENTER|3);
			PubWaitKey(10);
			return iRet;
		}

		glRecvData.ulLength += ulLen;
		if(glRecvData.ulLength >= ulExpLen)
		{
			break;
		}
		if(ulExpLen-glRecvData.ulLength <4096)
		{
			ulTempLen = ulExpLen-glRecvData.ulLength;
		}
	}

	iRet = AnaylsePacket(glRecvData.sContent, glRecvData.ulLength);
	if(glProcInfo.ucTranType == ACTIVE)
	{
		PP_TableLoadEnd();
	}

	if(strstr(glIDName, "Processando") != 0)
	{
		memset(&glRecvData, 0, sizeof(COMM_DATA));
		iRet = CommRecvMsg(glRecvData.sContent, 4, &glRecvData.ulLength, COMM_TIMEOUT);
		if(iRet != 0)
		{
			CommDisconnect();
			ScrClrBelow(2);
			PubDispString(_T("RECV TIMEOUT"), DISP_LINE_CENTER|3);
			PubWaitKey(10);
			return iRet;
		}
		goto WAIT_RESPONSE;
	}
#endif

	return iRet;
}

void InitTrans(void)
{
	GetTransInfo(glProcInfo.ucTranType);
	memset(&glMsgData, 0, sizeof(MSG_DATA));
	memset(&glSendData, 0, sizeof(COMM_DATA));
	memset(&glRecvData, 0, sizeof(COMM_DATA));
	memset(glProcInfo.szAmount, '0', 12);
	glSeqNo = 0;
	memset(glszMessageType, 0, 32);
	memset(glszDeDadosDestino, 0, 16);
	memset(glszProcessorede, 0, 16);
	memset(glszDeDadosrede, 0, 16);
	InitPPComp();
	glProcInfo.ucEntryMode = SWIPE_MODE;
	glActivate = 0; //add by wuc 2014.10.11
}

/******************++add by wuc 2014.4.1******************/
int Trans_Cancellation(void)
{
	int iRet;
	uchar szBuff[]="000-000 = CNC\r\n001-000 = 1\r\n999-999 = 0\r\n";
	uchar szABTCmd[256];

	//cielo新需求， 增加ABT命令
	if ( 1==glSysCtrl.bVTEFOpt )
	{
		g_bAbort = 1;
		AppendIntPos("000-000", "ABT", szABTCmd);
		AppendIntPos("000-001", gstIntpos.szIdentification, szABTCmd);
		AppendIntPos("000-002", glSysCtrl.szCPFCNPJ, szABTCmd);
		AppendIntPos("999-999", gstIntpos.szTrailer, szABTCmd);

		VTEF_Txd(szABTCmd, strlen(szABTCmd), 30);
	}
	PPRemoveCard("    OPERACAO       CANCELADA    ");
	iRet = WriteMob001(szBuff, strlen(szBuff));

	return iRet;
}

int Trans_Confirm(void)
{
	int iRet;
	
	InitTrans();
	BuildCampo(glMsgData.szPost, "confirmacao", glProcInfo.szService);
	iRet = TranProcess("confirmacao");
	CommDisconnect();

	return iRet;
}

int Trans_NotConfirm(void)
{
	int iRet;
	
	InitTrans();
	BuildCampo(glMsgData.szPost, "naoconfirmacao", glProcInfo.szService);
	iRet = TranProcess("naoconfirmacao");
	CommDisconnect();

	return iRet;
}
/******************--add by wuc 2014.4.1******************/

int Trans_Payment(void) //modify by wuc 2014.4.1
{
	int iRet;
	
	ScrCls();
	DispPictureBox("Cielologo.jpg", 1, 0, 0);
	glProcInfo.ucTranType = PAYMENT;
	InitTrans();
	iRet = ProcXMLPage(FILE_INIT_PAGE);
	DebugOutput("%s-%d-%s, g_bAbort:%d\n", __FILE__, __LINE__, __FUNCTION__, g_bAbort);//linzhao
	if(iRet)
	{
		if(-2 == iRet)
		{
			ScrClrBelow(2);
			PubDispString(_T("USER CANCELLED"), DISP_LINE_CENTER|4);
			PubWaitKey(3);
			Trans_Cancellation();
			return 0;
		}
		else if(-3 == iRet) //add by wuc 2014.10.11
		{
			glActivate = 0;
			ScrClrBelow(2);
			PubDispString(_T("TABLE EXPIRE"), DISP_LINE_CENTER|4);
			PubWaitKey(3);
			Trans_Active();
			return 0;
		}
		else
		{
			return iRet;
		}
	}

	iRet = TranProcess("identificacao");
	DebugOutput("%s-%d-%s, g_bAbort:%d\n", __FILE__, __LINE__, __FUNCTION__, g_bAbort);//linzhao
	if(bUserCancel)
	{
		bUserCancel = 0;
		Trans_Cancellation();
		CommDisconnect();
		return 0;
	}
	if(iRet)
	{
		CommDisconnect();
		return iRet;
	}

	while(1)
	{
		memset(&glMsgData, 0, sizeof(MSG_DATA));
		memset(&glSendData, 0, sizeof(COMM_DATA));
		memset(&glRecvData, 0, sizeof(COMM_DATA));
		BuildCampo(glMsgData.szPost, "solicitacao", glProcInfo.szService);
		iRet = TranProcess("solicitacao");
		if(bUserCancel)
		{
			bUserCancel = 0;
			Trans_Cancellation();
			CommDisconnect();
			return 0;
		}
		if(iRet)
		{
			if(!glSysCtrl.bVTEFOpt)
			{
				memset(&glMsgData, 0, sizeof(MSG_DATA));
				memset(&glSendData, 0, sizeof(COMM_DATA));
				memset(&glRecvData, 0, sizeof(COMM_DATA));
				BuildCampo(glMsgData.szPost, "naoconfirmacao", glProcInfo.szService);
				iRet = TranProcess("naoconfirmacao");
			}
			CommDisconnect();
			return iRet;
		}

		if(strcmp(glszMessageType, "respostafinal") == 0)
		{
			if(!glSysCtrl.bVTEFOpt)
			{
				memset(&glMsgData, 0, sizeof(MSG_DATA));
				memset(&glSendData, 0, sizeof(COMM_DATA));
				memset(&glRecvData, 0, sizeof(COMM_DATA));
				BuildCampo(glMsgData.szPost, "confirmacao", glProcInfo.szService);
				iRet = TranProcess("confirmacao");
			}
			break;
		}
	}

	CommDisconnect();

	return 0;
}


int Trans_Admin(void)//modify by wuc 2014.4.1
{
	int iRet;

	ScrCls();
	DispPictureBox("Cielologo.jpg", 1, 0, 0);
	glProcInfo.ucTranType = ADMIN;
	InitTrans();
	iRet = ProcXMLPage(FILE_ADMIN_PAGE);
	if(iRet)
	{
		if(-2 == iRet) 
		{
			ScrClrBelow(2);
			PubDispString(_T("USER CANCELLED"), DISP_LINE_CENTER|4);
			PubWaitKey(3);
			Trans_Cancellation();
			return 0;
		}
		else if(-3 == iRet) //add by wuc 2014.10.11
		{
			glActivate = 0;
			ScrClrBelow(2);
			PubDispString(_T("TABLE EXPIRE"), DISP_LINE_CENTER|4);
			PubWaitKey(3);
			Trans_Active();
			return 0;
		}
		else
		{
			return iRet;
		}
	}

	iRet = TranProcess("identificacao");
	if(bUserCancel)
	{
		bUserCancel = 0;
		Trans_Cancellation();
		CommDisconnect();
		return 0;
	}
	if(iRet)
	{
		CommDisconnect();
		return iRet;
	}

	while(1)
	{
		memset(&glMsgData, 0, sizeof(MSG_DATA));
		memset(&glSendData, 0, sizeof(COMM_DATA));
		memset(&glRecvData, 0, sizeof(COMM_DATA));
		BuildCampo(glMsgData.szPost, "solicitacao", glProcInfo.szService);
		iRet = TranProcess("solicitacao");
		if(bUserCancel)
		{
			bUserCancel = 0;
			Trans_Cancellation();
			CommDisconnect();
			return 0;
		}
		if(iRet)
		{
			if(!glSysCtrl.bVTEFOpt)
			{
				memset(&glMsgData, 0, sizeof(MSG_DATA));
				memset(&glSendData, 0, sizeof(COMM_DATA));
				memset(&glRecvData, 0, sizeof(COMM_DATA));
				BuildCampo(glMsgData.szPost, "naoconfirmacao", glProcInfo.szService);
				iRet = TranProcess("naoconfirmacao");
			}
			CommDisconnect();
			return iRet;
		}
		
		if(strcmp(glszMessageType, "respostafinal") == 0)
		{
			if(!glSysCtrl.bVTEFOpt)
			{
				memset(&glMsgData, 0, sizeof(MSG_DATA));
				memset(&glSendData, 0, sizeof(COMM_DATA));
				memset(&glRecvData, 0, sizeof(COMM_DATA));
				BuildCampo(glMsgData.szPost, "confirmacao", glProcInfo.szService);
				iRet = TranProcess("confirmacao");
			}
			break;
		}
	}

	CommDisconnect();
	return iRet;
}

int Trans_Void(void) //modify by wuc 2014.4.1
{
	int iRet;

	ScrCls();
	DispPictureBox("Cielologo.jpg", 1, 0, 0);
	glProcInfo.ucTranType = VOID;
	InitTrans();
	iRet = ProcXMLPage(FILE_VOID_PAGE);
	if(iRet)
	{
		if(-2 == iRet)
		{
			ScrClrBelow(2);
			PubDispString(_T("USER CANCELLED"), DISP_LINE_CENTER|4);
			PubWaitKey(3);
			Trans_Cancellation();
			return 0;
		}
		else if(-3 == iRet) //add by wuc 2014.10.11
		{
			glActivate = 0;
			ScrClrBelow(2);
			PubDispString(_T("TABLE EXPIRE"), DISP_LINE_CENTER|4);
			PubWaitKey(3);
			Trans_Active();
			return 0;
		}
		else
		{
			return iRet;
		}
	}

	iRet = TranProcess("identificacao");
	if(bUserCancel)
	{
		bUserCancel = 0;
		Trans_Cancellation();
		CommDisconnect();
		return 0;
	}
	if(iRet)
	{
		CommDisconnect();
		return iRet;
	}

	while(1)
	{
		memset(&glMsgData, 0, sizeof(MSG_DATA));
		memset(&glSendData, 0, sizeof(COMM_DATA));
		memset(&glRecvData, 0, sizeof(COMM_DATA));
		BuildCampo(glMsgData.szPost, "solicitacao", glProcInfo.szService);
		iRet = TranProcess("solicitacao");
		if(bUserCancel)
		{
			bUserCancel = 0;
			Trans_Cancellation();
			CommDisconnect();
			return 0;
		}
		if(iRet)
		{
			if(!glSysCtrl.bVTEFOpt)
			{
				memset(&glMsgData, 0, sizeof(MSG_DATA));
				memset(&glSendData, 0, sizeof(COMM_DATA));
				memset(&glRecvData, 0, sizeof(COMM_DATA));
				BuildCampo(glMsgData.szPost, "naoconfirmacao", glProcInfo.szService);
				iRet = TranProcess("naoconfirmacao");
			}
			CommDisconnect();
			return iRet;
		}
		
		if(strcmp(glszMessageType, "respostafinal") == 0)
		{
			if(!glSysCtrl.bVTEFOpt)
			{
				memset(&glMsgData, 0, sizeof(MSG_DATA));
				memset(&glSendData, 0, sizeof(COMM_DATA));
				memset(&glRecvData, 0, sizeof(COMM_DATA));
				BuildCampo(glMsgData.szPost, "confirmacao", glProcInfo.szService);
				iRet = TranProcess("confirmacao");
			}
			break;
		}
	}

	CommDisconnect();
	return iRet;
}

int Trans_Active(void)
{
	int iRet;

	ScrCls();	
	DispPictureBox("Cielologo.jpg", 1, 0, 0);
	glProcInfo.ucTranType = ACTIVE;
	glSysCtrl.bActivate = 1;//linzhao
	InitTrans();
	iRet = ProcXMLPage(FILE_ACTIVE_PAGE);
	DebugOutput("%s--%d--%s, iRet:%d\n", __FILE__, __LINE__, __FUNCTION__, iRet);
	if(iRet)
	{
		return iRet;
	}

	iRet = TranProcess("identificacao");
	DebugOutput("%s--%d--%s, iRet:%d\n", __FILE__, __LINE__, __FUNCTION__, iRet);
//	CommDisconnect();//linzhao 0331
	if(iRet)
	{
		CommDisconnect();
		return iRet;
	}

	memset(&glMsgData, 0, sizeof(MSG_DATA));
	memset(&glSendData, 0, sizeof(COMM_DATA));
	memset(&glRecvData, 0, sizeof(COMM_DATA));
	BuildCampo(glMsgData.szPost, "solicitacao", glProcInfo.szService);

	iRet = TranProcess("solicitacao");
	ScrClrBelow(1);

/*	if(iRet)
	{
		PubDispString(_T("ACTIVE FAIL"), 4|DISP_LINE_CENTER);
	}
	else
	{
		WriteATV001();
		PubDispString(_T("ACTIVE SUCCESS"), 4|DISP_LINE_CENTER);
	}
*/
	//add by linzhao
	if (1==glSysCtrl.bActivate && 0==iRet)
	{
		SaveSysCtrl();
		WriteATV001();
		PubDispString(_T("ACTIVE SUCCESS"), 4|DISP_LINE_CENTER);
	}
	else
	{
		glSysCtrl.bActivate = 0;
		iRet = -1;
		SaveSysCtrl();
		PubDispString(_T("ACTIVE FAIL"), 4|DISP_LINE_CENTER);
	}
	//add end
	PubWaitKey(5);
	CommDisconnect();

	return iRet;
}

