#include "applib.h"
#include "posapi.h"
#include "util.h"
#include "ppcomp.h"
#include "gzip.h"
#include "TranProc.h"
#include "xmlParser.h"
#include "lng.h"
#include "SxxCom.h"
#include "global.h"
#include <string.h>

COMM_DATA glSendData, glRecvData;
MSG_DATA glMsgData;
SYS_PROC_INFO glProcInfo;
SYSTEM_CTRL glSysCtrl;
ST_INTPOS_INFO gstIntpos;
PP_COMP glPPComp[32];
uchar glCondition[64];
int glPPCompIdx;
ulong glSeqNo;
uchar glszMessageType[32];
uchar glszDeDadosDestino[16];
uchar glszProcessorede[16];
uchar glszDeDadosrede[16];
uchar glCommMode;
uchar glConnectFlag;
uchar glStartServer;
uchar glAcceptClient;
uchar glScreenLine;
uchar glActivate; //add by wuc 2014.10.11

FuncMag glFunc[] = 
{
	{"TimeStamp",	PPLoadInit, 1, 0},
	{"TabelaAID",		PPLoadAID, 1, 0},
	{"TabelaChaves",	PPLoadKey, 1, 0},
	{"TelaInicialPgto",	PPInitPage, 1, 0},
	{"showpperrormsg", PPShowErrorMsg, 2, 0},
	{"ppgetpin", 		PPGetPIN, 1, 1},
	//{"ppgetcard", 		PPGetCard, 2, 1},
	{"ppgetcard", 		PPGetCard, 1, 1},
	{"ppgoonchip", 	PPGoOnChip, 3, 1},
	{"ppfinishchip",	PP_FinishChip, 2, 1},
	{"ppremovecard",	PPRemoveCard, 1, 0},
	{"ppgetinfo", 		PP_GetInfo, 1, 1},
	{"ppdisplay",		PP_Display, 1, 0},
	{"ppdisplayex",	PP_DisplayEx, 1, 0},
	{"ppgetdukpt",	PP_GetDUKPT, 1, 1},
	{"pptableloadinit",	PP_TableLoadInit, 1, 0},
	{"pptableloadrec",	PP_TableLoadRec, 1, 0},
//	{"pptableloadend",	PP_TableLoadEnd, 1, 0},
	{"ppgettimestamp",	PP_GetTimeStamp, 1, 1},
	{"ppgetkey",		PPGetKey, 1},
	{"ppcheckevent",	PP_CheckEvent, 0, 1},
	{"ppdefinewkpan", PP_DefineWKPAN, 1, 1},  // open by wuc 2014.10.21
	{"ppgenericcmd", PPGenericCmd, 1, 1},  // Cielo need to add new command(PP_GenericCmd).linzhao 2015.2.5
	{"", NULL, 0, 0}
};

int FuncPPExe(uchar *szFuncName, FuncMag *pstFunc, char *szInput00, char *szInput01, char *szInput02, char *szOutput01, char *szOutput02)
{
	ushort	iCnt;
	
	for(iCnt=0; pstFunc[iCnt].szFuncName[0]!=0; iCnt++)
	{
		if(!strncmp(pstFunc[iCnt].szFuncName, szFuncName, strlen(pstFunc[iCnt].szFuncName)))
		{
			switch(pstFunc[iCnt].ucInputFlag)
			{
				case 0:
					if(pstFunc[iCnt].ucOutputFlag == 0)
					{
						return (*pstFunc[iCnt].pfFunc)(NULL);
					}
					else if(pstFunc[iCnt].ucOutputFlag == 1)
					{
						return (*pstFunc[iCnt].pfFunc)(szOutput01);
					}
					else if(pstFunc[iCnt].ucOutputFlag == 2)
					{
						return (*pstFunc[iCnt].pfFunc)(szOutput01, szOutput02);
					}
					break;
				case 1:
					if(pstFunc[iCnt].ucOutputFlag == 0)
					{
						return (*pstFunc[iCnt].pfFunc)(szInput00);
					}
					else if(pstFunc[iCnt].ucOutputFlag == 1)
					{
						return (*pstFunc[iCnt].pfFunc)(szInput00, szOutput01);
					}
					else if(pstFunc[iCnt].ucOutputFlag == 2)
					{
						return (*pstFunc[iCnt].pfFunc)(szInput00, szOutput01, szOutput02);
					}
					break;
				case 2:
					if(pstFunc[iCnt].ucOutputFlag == 0)
					{
						return (*pstFunc[iCnt].pfFunc)(szInput00, szInput01);
					}
					else if(pstFunc[iCnt].ucOutputFlag == 1)
					{
						return (*pstFunc[iCnt].pfFunc)(szInput00, szInput01,szOutput01);
					}
					else if(pstFunc[iCnt].ucOutputFlag == 2)
					{
						return (*pstFunc[iCnt].pfFunc)(szInput00, szInput01, szOutput01, szOutput02);
					}
					break;
				case 3:
					if(pstFunc[iCnt].ucOutputFlag == 0)
					{
						return (*pstFunc[iCnt].pfFunc)(szInput00, szInput01, szInput02);
					}
					else if(pstFunc[iCnt].ucOutputFlag == 1)
					{
						return (*pstFunc[iCnt].pfFunc)(szInput00, szInput01, szInput02, szOutput01);
					}
					else if(pstFunc[iCnt].ucOutputFlag == 2)
					{
						return (*pstFunc[iCnt].pfFunc)(szInput00, szInput01, szInput02, szOutput01, szOutput02);
					}
					break;
				default:
					break;
			}
		}
	}
	
	return -1;
}

void DxxScrPrint(uchar col,uchar row,uchar mode, char *str,...)
{
	va_list list; 
	char string[256];
	ST_FONT stFont1;
	ST_FONT stFont2;

	memset(string, 0, sizeof(string));
	if(ROW_9_LINE == glScreenLine)
	{
		ScrGotoxyEx (col, row * ROW_9_HEIGHT + (ROW_9_HEIGHT - 24) / 2);
	}
	else if(ROW_8_LINE == glScreenLine)
	{
		ScrGotoxyEx (col, row * ROW_8_HEIGHT + (ROW_8_HEIGHT - 24) / 2);
	}
	va_start(list,str); 
	vsprintf(string, str, list);
	va_end(list); 

	if(mode & REVER)
	{
		ScrAttrSet(1);
	}
	else
	{
		ScrAttrSet(0);
	}

	stFont1.CharSet = CHARSET_WEST;
	stFont1.Bold = 0;
	stFont1.Width = 12;
	stFont1.Height = 24;
	stFont1.Italic = 0;

	stFont2.CharSet = CHARSET_GB2312;
	stFont2.Bold = 0;
	stFont2.Width = 24;
	stFont2.Height = 24;
	stFont2.Italic = 0;
	
	ScrSelectFont(&stFont1, &stFont2);
	
	Lcdprintf("%s",string);
}

void DxxScrClrLine(uchar startline, uchar endline)
{
	if(ROW_9_LINE == glScreenLine)
	{
		ScrClrRect(0, startline * ROW_9_HEIGHT, SCREEN_WIDTH, (endline+1) * ROW_9_HEIGHT);
	}
	else if(ROW_8_LINE == glScreenLine)
	{
		ScrClrRect(0, startline * ROW_8_HEIGHT, SCREEN_WIDTH, (endline+1) * ROW_8_HEIGHT);
	}
}

void DxxScrDrawLine(uchar line, COLORREF LineColor) //add by wuc 2014.4.1
{
	if(ROW_9_LINE == glScreenLine)
	{
		ScrRect(0, line * ROW_9_HEIGHT, SCREEN_WIDTH, line * ROW_9_HEIGHT, LineColor, NO_FILL_COLOR);
	}
	else if(ROW_8_LINE == glScreenLine)
	{
		ScrRect(0, line * ROW_8_HEIGHT, SCREEN_WIDTH, line * ROW_8_HEIGHT, LineColor, NO_FILL_COLOR);
	}
}

void DebugOutput(char *pszFmtStr, ...)
{
	unsigned char	szBuff[LEN_MAX_COMM_DATA+1];
	va_list pVaList;
	ulong nStrLen, i=0;
	uchar ucRet;
	uchar sCurTime[7], szDateTime[22+1];

	if( pszFmtStr==NULL || *pszFmtStr==0 )
	{
		return;
	}

	memset(sCurTime, 0, sizeof(sCurTime));
	memset(szDateTime, 0, sizeof(szDateTime));
	GetTime(sCurTime);
	sprintf((char *)szDateTime, "[%02X%02X-%02X-%02X %02X:%02X:%02X] ",
			(sCurTime[0]>0x80 ? 0x19 : 0x20), sCurTime[0], sCurTime[1],
			sCurTime[2], sCurTime[3], sCurTime[4], sCurTime[5]);
	
	va_start(pVaList, pszFmtStr);
	vsprintf((char*)szBuff, (char*)pszFmtStr, pVaList);
	va_end(pVaList);

	PortOpen(0, "115200,8,n,1");
	PortReset(0);

	nStrLen = strlen(szDateTime);
	for(i=0; i<nStrLen; i++)
	{
		ucRet = PortSend(0, szDateTime[i]);
		if(ucRet != 0x00)
		{
			return;
		}
	}
	
	nStrLen = strlen(szBuff);
	for(i=0; i<nStrLen; i++)
	{
		ucRet = PortSend(0, szBuff[i]);
		if (ucRet != 0x00)
		{
			return;
		}
	}

	PortSend(0, '\x0d');
	PortSend(0, '\x0a');
	PortClose(0);
}

void InitPPComp(void)
{
	int i;

	glPPCompIdx = 0;
	for(i=0; i<32; i++)
	{
		memset(&glPPComp[i], 0x00, sizeof(PP_COMP));
		glPPComp[i].uiRespCode = 0xff;
	}
}

void PPDispStartWait(void)
{
	ScrCls();
	DispPictureBox("Cielologo.jpg", 1, 0, 0);
#ifdef _TERMINAL_DXX_
	PubDispString("CONECTANDO AO SERVIDOR.", DISP_LINE_CENTER|3);
	PubDispString("AGUARDE...", DISP_LINE_CENTER|4);
#else
	PubDispString("CONECTANDO AO", DISP_LINE_CENTER|3);
	PubDispString("SERVIDOR. AGUARDE...", DISP_LINE_CENTER|4);
#endif
}


void PPDispErrMsg(uint iErrNo)
{
	;
}

//add by linzhao, 2015.2.5
int PPGenericCmd(uchar *pszInput, uchar *pszOutput)
{
	int iStat;
	uchar *pszMsgNotify;

	iStat = PP_StartGenericCmd(pszInput);
	if ( PPCOMP_OK!=iStat )
	{
		return iStat;
	}

	kbflush();
	do
	{
		iStat = PP_GenericCmd(pszOutput, pszMsgNotify);
		if(iStat == PPCOMP_NOTIFY)
		{
			ScrClrLine(2, 7);
			PubDispString(pszMsgNotify, DISP_LINE_LEFT|3);
			//do some things
		}
		if( iStat==PPCOMP_PROCESSING && (kbhit()==0 && getkey()==KEYCANCEL) )
		{
			PP_Abort();
			return PPCOMP_CANCEL;
		}
	}while( iStat==PPCOMP_NOTIFY || iStat==PPCOMP_PROCESSING );
	
	return iStat;
}

//get card
//int PPGetCard(uchar *szInput1, uchar *szInput2, uchar *szOutput)
int PPGetCard(uchar *szInput1, uchar *szOutput)
{
	int iStat;
	uchar szIn[42] = "";
	uchar szDateTime[8] = "";
	uchar szTimeStamp[11] = "";
	uchar  szMsgNotify[64] = "";

	GetTime(szDateTime);

	memcpy(szIn, szInput1, 4);
	memcpy(szIn+4, glProcInfo.szAmount, 12);

	sprintf(szIn+16, "%02X%02X%02X%02X%02X%02X", szDateTime[0], szDateTime[1], szDateTime[2], szDateTime[3], szDateTime[4], szDateTime[5]);
	PP_GetTimeStamp("03", szTimeStamp);
	memcpy(szIn+28, szTimeStamp, 10);
	memcpy(szIn+38, "00", 2);
	memcpy(szIn+40, "0", 1);
	szIn[41] = '\0';

	iStat = PP_StartGetCard(szIn);
	DebugOutput("%s--%d--%s, PP_StartGetCard:%d\n", __FILE__, __LINE__, __FUNCTION__, iStat);//linzhao
	if(iStat != PPCOMP_OK)
	{
		return iStat;
	}

	kbflush();
	do
	{
		iStat = PP_GetCard(szOutput, szMsgNotify);
		DebugOutput("%s--%d--%s, PP_GetCard:%d\n", __FILE__, __LINE__, __FUNCTION__, iStat);//linzhao
		if(iStat == PPCOMP_PROCESSING && (kbhit() == 0 && getkey() == KEYCANCEL))
		{
			PP_Abort();
			return PPCOMP_CANCEL;
		}
		if(iStat == PPCOMP_MCDATAERR)
		{
			PP_Display("ERROR");//need confirm the display message.
		}

		if(iStat == PPCOMP_CANCEL || iStat == PPCOMP_TIMEOUT)
		{
			glProcInfo.ucEntryMode = MANUAL_MODE;
			strcpy(glCondition, "ppgetcard cancelado");
		}

	}while(iStat == PPCOMP_PROCESSING || iStat == PPCOMP_MCDATAERR ||iStat == PPCOMP_NOTIFY);

	return /*PPCOMP_OK*/ iStat;//linzhao
}

//get pin
int PPGetPIN(uchar *szInput, uchar *szOutput)
{
	int iStat;
	uchar szMsgNotify[32];

	iStat = PP_StartGetPIN(szInput);
	if(iStat != PPCOMP_OK)
	{
		return iStat;
	}

	kbflush();
	do
	{
		iStat = PP_GetPIN(szOutput, szMsgNotify);
		if(iStat == PPCOMP_NOTIFY)
		{
			//do some things
		}
		if(iStat == PPCOMP_PROCESSING && (kbhit()==0 && getkey()==KEYCANCEL))
		{
			PP_Abort();
			return PPCOMP_CANCEL;
		}
	}while(iStat == PPCOMP_NOTIFY || iStat == PPCOMP_PROCESSING);

	return iStat;
}

//chip processing
int PPGoOnChip(uchar *szInput1, uchar *szInput2, uchar *szInput3, uchar *szOutput)
{
	int iStat;
	uchar szMsgNotify[32];
	
	glProcInfo.ucEntryMode = INSERT_MODE;
	iStat = PP_StartGoOnChip(szInput1, szInput2, szInput3);
	if(iStat != PPCOMP_OK)
	{
		return iStat;
	}

	kbflush();
	do
	{
		iStat = PP_GoOnChip(szOutput, szMsgNotify);
		if(iStat == PPCOMP_NOTIFY)
		{
			//do some things
		}
		if(iStat == PPCOMP_PROCESSING && (kbhit()==0 && getkey()==KEYCANCEL))
		{
			PP_Abort();
			return PPCOMP_CANCEL;
		}
	}while(iStat == PPCOMP_NOTIFY || iStat == PPCOMP_PROCESSING);

	return iStat;
}

int PPRemoveCard(uchar *szInput)
{
	int iStat;
	uchar szMsgNotify[32];

	iStat = PP_StartRemoveCard(szInput);
	if(iStat != PPCOMP_OK)
	{
		return iStat;
	}
	
	kbflush();
	do
	{
		iStat = PP_RemoveCard(szMsgNotify);
		if(iStat == PPCOMP_NOTIFY)
		{
			//do some things
		}
		if(iStat == PPCOMP_PROCESSING && (kbhit()==0 && getkey()==KEYCANCEL))
		{
			PP_Abort();
			return PPCOMP_CANCEL;
		}
	}while(iStat == PPCOMP_NOTIFY || iStat == PPCOMP_PROCESSING);

	return iStat;
}

//get key
int PPGetKey(void)
{
	int iStat;

	iStat = PP_StartGetKey();
	if(iStat != PPCOMP_OK)
	{
		return iStat;
	}

	do
	{
		iStat = PP_GetKey();
		if(iStat == PPCOMP_PROCESSING && (kbhit()==0 && getkey()==KEYCANCEL))
		{
			PP_Abort();
			return PPCOMP_CANCEL;
		}
	}while(iStat == PPCOMP_PROCESSING);

	return iStat;
}


int PPLoadInit(uchar *szTimestamp)
{
	uchar szIn[13] = "";
	int iRet;
	
	memcpy(szIn, "03", 2);
	memcpy(szIn+2, szTimestamp, 10);
	szIn[12] = '\0';

	iRet = PP_TableLoadInit(szIn);
	memcpy(glTimeStamp, szTimestamp, 10);
	return iRet;
}

int PPLoadAID(uchar *szAID)
{
	uchar *p, *p1, *p2;
	int iRet;
	uchar szBuff[512] = "";

	p = szAID;
	while(1)
	{
		memset(szBuff, 0, sizeof(szBuff));
		p1 = strchr(p, 0x0d);
		p2 = strchr(p, 0x0a);
		if(p2 == NULL)
		{
			return -1;
		}
		if(p1 != NULL)
		{
			*p1 = 0;
		}
		else
		{
			*p2 = 0;
		}

		memcpy(szBuff, "01", 2);
		memcpy(szBuff+2, p, strlen(p));
		iRet = PP_TableLoadRec(szBuff);
		if(iRet)
		{
			return iRet;
		}

		p = p2+1;
	}

	return iRet;
}

int PPLoadKey(uchar *szKey)
{
	uchar *p, *p1, *p2;
	int iRet;
	uchar szBuff[700] = "";
	
	p = szKey;
	while(1)
	{
		memset(szBuff, 0, sizeof(szBuff));
		p1 = strchr(p, 0x0d);
		p2 = strchr(p, 0x0a);
		if(p2 == NULL)
		{
			return -1;
		}
		if(p1 != NULL)
		{
			*p1 = 0;
		}
		else
		{
			*p2 = 0;
		}
		
		memcpy(szBuff, "01", 2);
		memcpy(szBuff+2, p, strlen(p));
		iRet = PP_TableLoadRec(szBuff);
		if(iRet)
		{
			return iRet;
		}
		p = p2+1;
	}

	return 0;
}

int PPInitPage(uchar *szInitPage)
{
	uchar szBuff[LEN_BODY_DATA];
	int fd;
	int iLen;
	
	iLen = strlen(szInitPage);
	memset(szBuff, 0, LEN_BODY_DATA);
	PubAsc2Bcd(szInitPage, iLen, szBuff);

	fd = open("INITPAGE.XML", O_CREATE|O_RDWR);
	if(fd < 0)
	{
		return -1;
	}

	seek(fd, 0L, SEEK_SET);
	write(fd, szBuff, (iLen/2));
	close(fd);

	return 0;
}

int PPShowErrorMsg(uchar *szInput00, uchar *szInput01)
{
	uchar iLen;
	
	ScrClrBelow(4);
	iLen = 4;
	iLen = DispLabel(szInput00, iLen, DISP_LINE_CENTER);
	iLen = DispLabel(szInput01, iLen, DISP_LINE_CENTER);
	PubWaitKey(USER_OPER_TIMEOUT);

	return 0;
}

void DispPrnError(uchar ucErrCode)
{
	switch( ucErrCode )
	{
	case PRN_BUSY:
		PubDispString(_T("PRINTER BUSY"), 6|DISP_LINE_CENTER);
		break;

	case PRN_PAPEROUT:
		PubDispString(_T("OUT OF PAPER"), 6|DISP_LINE_CENTER);
		break;

	case PRN_WRONG_PACKAGE:
		PubDispString(_T("PRN DATA ERROR"), 6|DISP_LINE_CENTER);
		break;

	case PRN_FAULT:
		PubDispString(_T("PRINTER ERROR"), 6|DISP_LINE_CENTER);
		break;

	case PRN_NOFONTLIB:
		PubDispString(_T("FONT MISSING"), 6|DISP_LINE_CENTER);
		break;

	case PRN_OUTOFMEMORY:
		PubDispString(_T("PRN OVERFLOW"), 6|DISP_LINE_CENTER);
		break;

	default:
		PubDispString(_T("PRINT FAILED"), 6|DISP_LINE_CENTER);
		break;
	}
	
	PubBeepErr();
}

int StartPrinter(void)
{
	uchar	ucRet, ucKey;

	ScrClrBelow(2);
	PubDispString(_T("PRINTING..."), 4|DISP_LINE_CENTER);
	PrnStart();
	while( 1 )
	{
		ucRet = PrnStatus();
		if( ucRet==PRN_OK || ucRet==PRN_CANCEL )
		{
			return (ucRet==PRN_OK ? 0 : 1);
		}
		else if( ucRet==PRN_BUSY )
		{
			DelayMs(500);
			continue;
		}
		ScrClrBelow(4);
		DispPrnError(ucRet);
		if( ucRet!=PRN_PAPEROUT)
		{
			PubWaitKey(3);
			break;
		}

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if( ucKey==KEYCANCEL || ucKey==NOKEY )
		{
			break;
		}
		ScrClrBelow(2);
		PubDispString(_T("PRINTING..."), 4|DISP_LINE_CENTER);
		PrnStart();
	}

	return 0;
}

int CommConnect(void)
{
	int iRet;
	
	if(glSysCtrl.ucCommType == WIFI_USE)
	{
		iRet = Wifi_Connect();
		if(iRet < 0) //add by wuc 2014.4.1
		{
			return iRet;
		}
	}
	else if(glSysCtrl.ucCommType == GPRS_USE)
	{
		/********************++modified by wuc 2013.12.17********************/
		if(SLOT_1 == glSysCtrl.ucSlot)
		{
			iRet = SXXWlDial(&glSysCtrl.stWirelessPara1, COMM_TIMEOUT, 3600);
			if(iRet < 0) //add by wuc 2014.4.1
			{
				return iRet;
			}
		}
		else if(SLOT_2 == glSysCtrl.ucSlot)
		{
			iRet = SXXWlDial(&glSysCtrl.stWirelessPara2, COMM_TIMEOUT, 3600);
			if(iRet < 0) //add by wuc 2014.4.1
			{
				return iRet;
			}
		}
		/********************--modified by wuc 2013.12.17********************/
	}
	else if(glSysCtrl.ucCommType == TCPIP_USE)
	{
		SxxDhcpStart(1, COMM_TIMEOUT);
		iRet = SxxTcpConnect(glSysCtrl.szRemoteIP, (short)atoi(glSysCtrl.szRemotePort), COMM_TIMEOUT);
		if(iRet < 0) //add by wuc 2014.4.1
		{
			return iRet;
		}
	}
	else if (glSysCtrl.ucCommType==SSL_USE)//add by linzhao, 2015.2.5
	{
		iRet = SSL_PROC();
		DebugOutput("%s--%d--%s, SSL_PROC, iRet=%d\n", __FILE__, __LINE__, __FUNCTION__, iRet);
		if(iRet < 0) 
		{
			return iRet;
		}
	}

	// 是否按过取消
	if ((kbhit()==0) && (getkey()==KEYCANCEL))
	{
		return 0xff;
	}
	
	return iRet;
}

int CommSendMsg(uchar *szMsg, ushort usLen, ushort usTimeout)
{
	int iRet = -1;
	
	if(glSysCtrl.ucCommType == WIFI_USE)
	{
		iRet = WifiNetTcpTxd(szMsg, usLen, usTimeout);
	}
	else if(glSysCtrl.ucCommType == GPRS_USE)
	{
		iRet = SXXWlSend(szMsg, usLen, usTimeout);
	}
	else if(glSysCtrl.ucCommType == TCPIP_USE)
	{
		iRet = SxxTcpTxd(szMsg, usLen, usTimeout);
	}
	else if (glSysCtrl.ucCommType == SSL_USE)
	{
		iRet = Ssl_Txd(szMsg, usLen, usTimeout);
	}

	return iRet;
}

int CommRecvMsg(uchar *szMsg, ulong ulExpLen, ulong *RxLen, ushort usTimeout)
{
	int iRet = -1;

	DebugOutput("%s--%d, ucCommType:%d\n", __FILE__, __LINE__, glSysCtrl.ucCommType);
	if(glSysCtrl.ucCommType == WIFI_USE)
	{
		iRet = WifiNetTcpRxd(szMsg, ulExpLen, RxLen, usTimeout);
	}
	else if(glSysCtrl.ucCommType == GPRS_USE)
	{
		iRet = SXXWlRecv(szMsg, ulExpLen, RxLen, usTimeout);
	}
	else if(glSysCtrl.ucCommType == TCPIP_USE)
	{
		iRet = SxxTcpRxd(szMsg, ulExpLen, RxLen, usTimeout);
	}
	else if ( glSysCtrl.ucCommType==SSL_USE )
	{
		iRet = Ssl_Rxd(szMsg, ulExpLen, RxLen, usTimeout);
	}

	return iRet;
}

void CommDisconnect(void)
{
	if(glSysCtrl.ucCommType == WIFI_USE)
	{
		WifiHook();
	}
	else if(glSysCtrl.ucCommType == GPRS_USE)
	{
		SXXWlCloseTcp();
	}
	else if(glSysCtrl.ucCommType == TCPIP_USE)
	{
		SxxTcpOnHook();
	}
	else if ( glSysCtrl.ucCommType==SSL_USE )
	{
		Ssl_OnHook();
	}
}

uchar GetTerminalType(void)
{
	uchar szInfo[128];
	uchar ucInfo;

	memset(szInfo, 0, sizeof(szInfo));
	GetTermInfo(szInfo);
	ucInfo = szInfo[0];
	return ucInfo;
}

/*******************++add by wuc 2014.4.1*********************/
void AppendIntPos(uchar *szSerial, uchar *szValue, uchar *szOut)
{
	uchar *p;

	p = szOut+strlen(szOut);
	
	if (0==strcmp("999-999", szSerial))//fix the format error.linzhao
	{
		sprintf(szValue, "%d", 0);
	}
	sprintf(p, "%s = %.*s\r\n", szSerial, strlen(szValue), szValue);
}

void SearchIntPos(uchar *szSerial, uchar *szValue, uchar *szIntPos)
{
	uchar *p = NULL;
	uchar *p1 = NULL;
	
	p = strstr(szIntPos, szSerial);
	if(p != NULL)
	{
		p += (strlen(szSerial)+strlen(" = "));
		p1 = strchr(p, 0x0d);
		if(p1 != NULL)
		{
			memcpy(szValue, p, p1-p);
		}
		else
		{
			p1 = strchr(p, 0x0a);
			if(p1 != NULL)
			{
				memcpy(szValue, p, p1-p);
			}
		}
	}
}

void SetScreenLine(uchar ucLine)
{
	glScreenLine = ucLine;
}
/*******************--add by wuc 2014.4.1*********************/

int APP_iTestCancel (void) //add by wuc 2014.10.21
{
	return 0;
}

//Added by linzhao 2015.1.14
static char base64_alphabet[] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/="};
int Base64Decode(unsigned char *szInput, unsigned char *szOutput, unsigned int uiInputLen)
{
	unsigned char szCode[5];
	unsigned int i,j,k,m;
	unsigned char szASC[4];
	for(i = 0,j=0; i<uiInputLen; i+=4, j+=3)
	{
		memset(szCode, 0, 5);
		szCode[0] = szInput[i];
		szCode[1] = szInput[i+1];
		szCode[2] = szInput[i+2];
		szCode[3] = szInput[i+3];
		for(k=0; k<4; k++)
		{
			for(m=0; m<65; m++)
			{
				if(szCode[k] == base64_alphabet[m])
				{
					szASC[k] = m;
					break;
				}
			}
		}
		if((szCode[2] == '=') && (szCode[3] == '='))
		{
			szOutput[j] = (szASC[0]<<2) + ((szASC[1]<<2)>>6);
		}
		else if(szCode[3] == '=')
		{
			szOutput[j] = (szASC[0]<<2) + ((szASC[1]<<2)>>6);
			szOutput[j+1] = (szASC[1]<<4) + ((szASC[2]<<2)>>4);
		}
		else
		{
			szOutput[j] = (szASC[0]<<2) + ((szASC[1]<<2)>>6);
			szOutput[j+1] = (szASC[1]<<4) + ((szASC[2]<<2)>>4);
			szOutput[j+2] = (szASC[2]<<6) + szASC[3];
		}
	}
	return (j+3);
}
