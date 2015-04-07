#include "posapi.h"
#include "appLib.h"
#include "util.h"
#include "FileOper.h"
#include "lng.h"
#include "SxxCom.h"
#include "Vtef.h"
#include "TranProc.h"
#include "Setup.h"
#include "ppcomp.h"
#include "xmlParser.h"
#include "xmlProc.h"
#include "global.h"

uchar glConfiged = 0; //add by wuc 2014.4.1

uchar ValidIPAddress(char *pszIPAddr);
int SetPrinter(void);
int Unlock_App(void);
int ClearSetup(void);
static int FunctionChoose(void);


// 判断是否为数字串
uchar IsNumStr(char *pszStr)
{
	if( pszStr==NULL || *pszStr==0 )
	{
		return FALSE;
	}

	while( *pszStr )
	{
		if( !isdigit(*pszStr++) )
		{
			return FALSE;
		}
	}

	return TRUE;
}

// 输入IP地址
int GetIPAddress(uchar *pszPrompts, uchar bAllowNull, uchar *pszIPAddress)
{
	int iRet = 0;
	uchar	szTemp[20], szDispBuff[20];

	while( 1 )
	{
		ScrCls();
		memset(szTemp, 0, sizeof(szTemp));
		sprintf((char *)szTemp, "%.15s", pszIPAddress);
		sprintf((char *)szDispBuff, "%.16s", pszPrompts);
		PubDispString(szDispBuff, 1|DISP_LINE_CENTER);
		
	#ifdef _TERMINAL_DXX_
		ScrGotoxyEx (0, 96);
		iRet = GetString(szTemp, 0xF5, 0, 15);
		if(iRet)
		{
			return iRet;
		}

		if( bAllowNull && szTemp[1]==0 )
		{
			*pszIPAddress = 0;
			break;
		}
		if( ValidIPAddress((char *)(szTemp+1)) )
		{
			sprintf((char *)pszIPAddress, "%.15s", szTemp+1);
			break;
		}
	#else
		iRet = PubGetString(ALPHA_IN|ECHO_IN, 0, 15, szTemp, COMM_TIMEOUT);
		if(iRet)
		{
			return iRet;
		}
	
		if( bAllowNull && szTemp[0]==0 )
		{
			*pszIPAddress = 0;
			break;
		}
		if( ValidIPAddress((char *)szTemp))
		{
			sprintf((char *)pszIPAddress, "%.15s", szTemp);
			break;
		}
	#endif

		ScrClrBelow(2);
		PubDispString(_T("INVALID IP ADDRESS"), 4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(3);
	}

	return iRet;
}

// 检查IP地址
uchar ValidIPAddress(char *pszIPAddr)
{
	int		i;
	char	*p, *q, szBuf[5+1];

	PubTrimStr((uchar *)pszIPAddr);
	if( *pszIPAddr==0 )
	{
		return FALSE;
	}

	p = strchr(pszIPAddr, ' ');
	if( p!=NULL )
	{
		return FALSE;
	}
	if( strlen(pszIPAddr)>15 )
	{
		return FALSE;
	}

	// 1st --- 3rd  part
	for(q=pszIPAddr, i=0; i<3; i++)
	{
		p = strchr(q, '.');
		if( p==NULL || p==q || p-q>3 )
		{
			return FALSE;
		}
		sprintf(szBuf, "%.*s", (int)(p-q), q);
		if( !IsNumStr(szBuf) || atoi(szBuf)>255 )
		{
			return FALSE;
		}
		q = p + 1;
	}

	// final part
	p = strchr((char *)q, '.');
	if( p!=NULL || !IsNumStr(q) || strlen(q)==0 || strlen(q)>3 || atoi(q)>255 )
	{
		return FALSE;
	}

	return TRUE;
}

// 输入端口
int GetIPPort(uchar *pszPrompts, uchar bAllowNull, uchar *pszPortNo)
{
	int iRet = 0;
	uchar ucMaxLen = 4;
	uchar ucMinLen = 0;
	uchar szFormat[20], szTemp[15], szDispBuff[20];

	ScrCls();
	memset(szTemp, 0, sizeof(szTemp));
	memset(szDispBuff, 0, sizeof(szDispBuff));
	memset(szFormat, 0, sizeof(szFormat));
	sprintf((char *)szFormat, "%%.%ds", ucMaxLen);
	sprintf((char *)szTemp, szFormat, pszPortNo);
	sprintf((char *)szDispBuff, "%.16s", pszPrompts);
	PubDispString(szDispBuff, 1|DISP_LINE_CENTER);
		
	memset(szDispBuff, 0, sizeof(szDispBuff));
	memset(szFormat, 0, sizeof(szFormat));
	sprintf(szFormat, "%%0%ds", ucMaxLen);
#ifdef _TERMINAL_DXX_
	ScrGotoxyEx (0, 96);
	iRet = GetString(szTemp, 0xE5, ucMinLen, ucMaxLen);
	if(iRet)
	{
		return iRet;
	}
	if(szTemp[0] < ucMaxLen)
	{
		szTemp[0] = ucMaxLen;
		strcpy(szDispBuff, szTemp + 1);
		sprintf(szTemp + 1, szFormat, szDispBuff);
	}
	sprintf((char *)pszPortNo, szFormat, szTemp+1);
#else
	iRet = PubGetString(NUM_IN|ECHO_IN, ucMinLen, ucMaxLen, szTemp, COMM_TIMEOUT);
	if(iRet)
	{
		return iRet;
	}
	if(strlen(szTemp) < ucMaxLen)
	{
		strcpy(szDispBuff, szTemp);
		sprintf(szTemp, szFormat, szDispBuff);
	}
	sprintf((char *)pszPortNo, szFormat, szTemp);
#endif

	return iRet;
}

int ConfirmConfig(void) //add by wuc 2014.4.1
{
	uchar ucKey;
	
	ScrCls();
	PubDispString(_T("CONFIRM"), 1|DISP_LINE_CENTER);
	PubDispString(_T("CONFIGURATION?"), 2|DISP_LINE_CENTER);
	PubDispString(_T("1 - YES"), 4|DISP_LINE_LEFT);
	PubDispString(_T("2 - NO"), 5|DISP_LINE_LEFT);
	while(1)
	{
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if(ucKey == KEY1)
		{
			glConfiged = 1;
			break;
		}
		else if(ucKey == KEY2)
		{
			glConfiged = 0;
			break;
		}
		else if(ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return ucKey;
		}
	}

	return 0;
}

int SetMerchantNum(void)
{
	int iRet;
	uchar szTemp[3], szFormat[10], szString[3];
	uchar ucMin = 0;
	uchar ucMax = 2;
	
	ScrCls();
	PubDispString(_T("COMPANY"), 1|DISP_LINE_CENTER);
	
	memset(szTemp, 0, sizeof(szTemp));
	memset(szString, 0, sizeof(szString));
	memset(szFormat, 0, sizeof(szFormat));
	sprintf(szFormat, "%%0%ds", ucMax);
	memcpy(szTemp, glSysCtrl.szMerchantNum, strlen(glSysCtrl.szMerchantNum));
#ifdef _TERMINAL_DXX_
	ScrGotoxyEx(0, 96);
	iRet = GetString(szTemp, 0xF5, ucMin, ucMax);
	if(iRet)
	{
		return iRet;
	}
	if(szTemp[0] < ucMax)
	{
		szTemp[0] = ucMax;
		strcpy(szString, szTemp + 1);
		sprintf(szTemp + 1, szFormat, szString);
	}
	memset(glSysCtrl.szMerchantNum, 0, sizeof(glSysCtrl.szMerchantNum));
	memcpy(glSysCtrl.szMerchantNum, szTemp+1, szTemp[0]);
#else
	iRet = PubGetString(ALPHA_IN|ECHO_IN, ucMin, ucMax, szTemp, COMM_TIMEOUT);
	if(iRet)
	{
		return iRet;
	}
	if(strlen(szTemp) < ucMax)
	{
		strcpy(szString, szTemp);
		sprintf(szTemp, szFormat, szString);
	}
	memset(glSysCtrl.szMerchantNum, 0, sizeof(glSysCtrl.szMerchantNum));
	memcpy(glSysCtrl.szMerchantNum, szTemp, strlen(szTemp));
#endif
	return 0;
}

int SetStoreNum(void)
{
	int iRet;
	uchar szTemp[5], szFormat[10], szString[5];
	uchar ucMin = 0;
	uchar ucMax = 4;
	
	ScrCls();;
	PubDispString(_T("STORE"), 1|DISP_LINE_CENTER);
	
	memset(szTemp, 0, sizeof(szTemp));
	memset(szString, 0, sizeof(szString));
	memset(szFormat, 0, sizeof(szFormat));
	sprintf(szFormat, "%%0%ds", ucMax);
	memcpy(szTemp, glSysCtrl.szStoreNum, strlen(glSysCtrl.szStoreNum));
#ifdef _TERMINAL_DXX_
	ScrGotoxyEx(0, 96);
	iRet = GetString(szTemp, 0xE5, ucMin, ucMax);
	if(iRet)
	{
		return iRet;
	}
	if(szTemp[0] < ucMax)
	{
		szTemp[0] = ucMax;
		strcpy(szString, szTemp + 1);
		sprintf(szTemp + 1, szFormat, szString);
	}
	memset(glSysCtrl.szStoreNum, 0, sizeof(glSysCtrl.szStoreNum));
	memcpy(glSysCtrl.szStoreNum, szTemp+1, szTemp[0]);
#else
	iRet = PubGetString(NUM_IN|ECHO_IN, ucMin, ucMax, szTemp, COMM_TIMEOUT);
	if(iRet)
	{
		return iRet;
	}
	if(strlen(szTemp) < ucMax)
	{
		strcpy(szString, szTemp);
		sprintf(szTemp, szFormat, szString);
	}
	memset(glSysCtrl.szStoreNum, 0, sizeof(glSysCtrl.szStoreNum));
	memcpy(glSysCtrl.szStoreNum, szTemp, strlen(szTemp));
#endif
	return 0;
}

int SetPOSNum(void)
{
	int iRet;
	uchar szTemp[4], szFormat[10], szString[4];
	uchar ucMin = 0;
	uchar ucMax = 3;
	
	ScrCls();;
	PubDispString(_T("PDV"), 1|DISP_LINE_CENTER);
	
	memset(szTemp, 0, sizeof(szTemp));
	memset(szString, 0, sizeof(szString));
	memset(szFormat, 0, sizeof(szFormat));
	sprintf(szFormat, "%%0%ds", ucMax);
	memcpy(szTemp, glSysCtrl.szPosNum, strlen(glSysCtrl.szPosNum));
#ifdef _TERMINAL_DXX_
	ScrGotoxyEx (0, 96);
	iRet = GetString(szTemp, 0xE5, ucMin, ucMax);
	if(iRet)
	{
		return iRet;
	}
	if(szTemp[0] < ucMax)
	{
		szTemp[0] = ucMax;
		strcpy(szString, szTemp + 1);
		sprintf(szTemp + 1, szFormat, szString);
	}
	memset(glSysCtrl.szPosNum, 0, sizeof(glSysCtrl.szPosNum));
	memcpy(glSysCtrl.szPosNum, szTemp+1, szTemp[0]);
#else
	iRet = PubGetString(NUM_IN|ECHO_IN, ucMin, ucMax, szTemp, COMM_TIMEOUT);
	if(iRet)
	{
		return iRet;
	}
	if(strlen(szTemp) < ucMax)
	{
		strcpy(szString, szTemp);
		sprintf(szTemp, szFormat, szString);
	}
	memset(glSysCtrl.szPosNum, 0, sizeof(glSysCtrl.szPosNum));
	memcpy(glSysCtrl.szPosNum, szTemp, strlen(szTemp));
#endif
	return 0;
}

int SetProcessID(void)
{
	int iRet;
	uchar szTemp[5], szFormat[10], szString[5];
	uchar ucMin = 0;
	uchar ucMax = 4;
	
	ScrCls();;
	PubDispString(_T("DESTINATION PROCESS"), 1|DISP_LINE_CENTER);
	
	memset(szTemp, 0, sizeof(szTemp));
	memset(szString, 0, sizeof(szString));
	memset(szFormat, 0, sizeof(szFormat));
	sprintf(szFormat, "%%0%ds", ucMax);
	memcpy(szTemp, glSysCtrl.szDestino, strlen(glSysCtrl.szDestino));
#ifdef _TERMINAL_DXX_
	ScrGotoxyEx (0, 96);
	iRet = GetString(szTemp, 0xE5, ucMin, ucMax);
	if(iRet)
	{
		return iRet;
	}
	if(szTemp[0] < ucMax)
	{
		szTemp[0] = ucMax;
		strcpy(szString, szTemp + 1);
		sprintf(szTemp + 1, szFormat, szString);
	}
	memset(glSysCtrl.szDestino, 0, sizeof(glSysCtrl.szDestino));
	memcpy(glSysCtrl.szDestino, szTemp+1, szTemp[0]);
#else
	iRet = PubGetString(NUM_IN|ECHO_IN, ucMin, ucMax, szTemp, COMM_TIMEOUT);
	if(iRet)
	{
		return iRet;
	}
	if(strlen(szTemp) < ucMax)
	{
		strcpy(szString, szTemp);
		sprintf(szTemp, szFormat, szString);
	}
	memset(glSysCtrl.szDestino, 0, sizeof(glSysCtrl.szDestino));
	memcpy(glSysCtrl.szDestino, szTemp, strlen(szTemp));
#endif
	return 0;
}

int SetRemoteIP(uchar bAllowNull, uchar *pszIP, uchar *pszPort)
{
	int		iRet;

	iRet = GetIPAddress(_T("IP SERVER"), bAllowNull, pszIP);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetIPPort(_T("SERVER PORT"), bAllowNull, pszPort);
	if( iRet!=0)
	{
		return iRet;
	}

	return 0;
}

int SetLocalPort()
{
	int iRet;
	uchar szDisp[32] = "";

	iRet = GetIPPort(_T("PORT"), TRUE, glSysCtrl.szLocalPort);
	if(iRet)
	{
		return iRet;
	}
	
	if(glSysCtrl.bVTEFOpt)
	{
		ScrCls();
		PubDispString(_T("AUTOMATION (ON)"), 1|DISP_LINE_CENTER);
		memset(szDisp, 0, sizeof(szDisp));
		sprintf(szDisp, _T("Local IP: %s"), glSysCtrl.szLocalIp);
		PubDispString(szDisp, 3|DISP_LINE_LEFT);
		memset(szDisp, 0, sizeof(szDisp));
		sprintf(szDisp, _T("Port: %s"), glSysCtrl.szLocalPort);
		PubDispString(szDisp, 4|DISP_LINE_LEFT);
		PubDispString(_T("(PRESS ENTER TO CONTINUE)"), DISP_LINE_CENTER|6);
		PubWaitKey(5);
	}
	
	return iRet;
}

int SetPPOnlineClient(void) //add by wuc 2014.4.1
{
	uchar ucKey;
	
	ScrCls();
	PubDispString(_T("Enable PP Online Client"), 1|DISP_LINE_CENTER);
	PubDispString(_T("1 - YES"), 3|DISP_LINE_LEFT);
	PubDispString(_T("2 - NO"), 4|DISP_LINE_LEFT);
	while(1)
	{
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if(ucKey == KEY1)
		{
			glSysCtrl.bPPOnlineClient = 1;
			break;
		}
		else if(ucKey == KEY2)
		{
			glSysCtrl.bPPOnlineClient = 0;
			break;
		}
		else if(ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return ucKey;
		}
	}

	return 0;
}

int SetPPOnlineAuto(void) //add by wuc 2014.4.1
{
	uchar ucKey;
	
	ScrCls();
	PubDispString(_T("Enable PP Online Automation"), 1|DISP_LINE_CENTER);
	PubDispString(_T("1 - YES"), 3|DISP_LINE_LEFT);
	PubDispString(_T("2 - NO"), 4|DISP_LINE_LEFT);
	while(1)
	{
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if(ucKey == KEY1)
		{
			glSysCtrl.bPPOnlineAuto = 1;
			break;
		}
		else if(ucKey == KEY2)
		{
			glSysCtrl.bPPOnlineAuto = 0;
			break;
		}
		else if(ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return ucKey;
		}
	}

	return 0;
}

int SetDebugOpt(void)
{
	int iRet;
	uchar szDisp[32] = "";
	
	ScrCls();
	if(glSysCtrl.bDebugOpt)
	{
		strcpy(szDisp, _T("DEBUG INFO (ON)"));
	}
	else
	{
		strcpy(szDisp, _T("DEBUG INFO (OFF)"));
	}
	PubDispString(szDisp, 2|DISP_LINE_CENTER);
	PubDispString(_T("1 - ON"), 3|DISP_LINE_LEFT);
	PubDispString(_T("2 - OFF"), 4|DISP_LINE_LEFT);
	
	while(1)
	{
		iRet = PubWaitKey(USER_OPER_TIMEOUT);
		if(iRet == KEY1)
		{
			glSysCtrl.bDebugOpt = 1;
			break;
		}
		else if(iRet == KEY2)
		{
			glSysCtrl.bDebugOpt = 0;
			break;
		}
		else if(iRet == KEYCANCEL || iRet == NOKEY)
		{
			return iRet;
		}
	}

	return ConfirmConfig();
}

int SetGzipMode(void)
{
	uchar ucKey;
	uchar szDisp[32] = "";
	
	ScrCls();
	if(glSysCtrl.bGzipOpt)
	{
		strcpy(szDisp, _T("COMPRESSION (ON)"));
	}
	else
	{
		strcpy(szDisp, _T("COMPRESSION (OFF)"));
	}
	PubDispString(szDisp, 2|DISP_LINE_CENTER);
	PubDispString(_T("1 - ON"), 3|DISP_LINE_LEFT);
	PubDispString(_T("2 - OFF"), 4|DISP_LINE_LEFT);
	while(1)
	{
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if(ucKey == KEY1)
		{
			glSysCtrl.bGzipOpt = 1;
			break;
		}
		else if(ucKey == KEY2)
		{
			glSysCtrl.bGzipOpt = 0;
			break;
		}
		else if(ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return ucKey;
		}
	}

	return ConfirmConfig();
}

int SetTerminalParam(void)
{	
	int iRet = 0;

	iRet = SetMerchantNum();
	if(iRet)
	{
		return iRet;
	}

	iRet = SetStoreNum();
	if(iRet)
	{
		return iRet;
	}

	iRet = SetPOSNum();
	if(iRet)
	{
		return iRet;
	}

	iRet = SetProcessID();
	if(iRet)
	{
		return iRet;
	}

	iRet = SetRemoteIP(TRUE, glSysCtrl.szRemoteIP, glSysCtrl.szRemotePort);
	if(iRet)
	{
		return iRet;
	}

	iRet = SetPPOnlineClient();
	if(iRet)
	{
		return iRet;
	}

	iRet = SetPPOnlineAuto();
	if(iRet)
	{
		return iRet;
	}

	iRet = ConfirmConfig();
	if(iRet)
	{
		return iRet;
	}
	
	return 0;
}

void SetAPNPerfil(WIRELESS_PARAM *pstWlPara, int flag) //add by wuc 2013.12.17
{
	switch(flag)
	{
		case APN_VIVO:
			strcpy(pstWlPara->szAPN, "zap.vivo.com.br");
			strcpy(pstWlPara->szUID, "vivo");
			strcpy(pstWlPara->szPwd, "vivo");
			break;

		case APN_CLARO:
			strcpy(pstWlPara->szAPN, "claro.com.br");
			strcpy(pstWlPara->szUID, "claro");
			strcpy(pstWlPara->szPwd, "claro");
			break;
			
		case APN_TIM:
			strcpy(pstWlPara->szAPN, "tim.br");
			strcpy(pstWlPara->szUID, "tim");
			strcpy(pstWlPara->szPwd, "tim");
			break;
			
		case APN_OI:
			strcpy(pstWlPara->szAPN, "gprs.oi.com.br");
			strcpy(pstWlPara->szUID, "oi");
			strcpy(pstWlPara->szPwd, "oi");
			break;
			
		case APN_CTBC:
			strcpy(pstWlPara->szAPN, "ctbc.br");
			strcpy(pstWlPara->szUID, "ctbc");
			strcpy(pstWlPara->szPwd, "1212");
			break;

		default:
			break;
	}
}

void SetWirelessPara() //modified by wuc 2013.12.17
{
#ifdef _TERMINAL_DXX_
	uchar szTemp[64];
#endif
	int	iRet;
	WIRELESS_PARAM *pstWlPara = NULL;
	SMART_MENU	stSM;
	MENU_ITEM stSlotMenu[] =
	{
		{_T("SETUP"),			-1,		NULL},
		{_T("SLOT1"),			SLOT_1,	NULL},
		{_T("SLOT2"),			SLOT_2,	NULL},
		{"",				0, 		NULL},
	};
	MENU_ITEM stInputMenu[] =
	{
		{_T("SETUP"),			-1,			NULL},
		{_T("APN PERFIL"),		APN_PERFIL,	NULL},
		{_T("APN MANUAL"),		APN_MANUAL,	NULL},
		{"",				0, 			NULL},
	};
	MENU_ITEM stAPNMenu[] =
	{
		{_T("SETUP"),		-1,			NULL},
		{_T("VIVO"),		APN_VIVO,	NULL},
		{_T("CLARO"),		APN_CLARO,	NULL},
		{_T("TIM"),			APN_TIM,	NULL},
		{_T("OI"),			APN_OI,		NULL},
		{_T("CTBC"),		APN_CTBC,	NULL},
		{"",			0, 			NULL},
	};

	//--------------------------------------------------
	ScrCls();
	memset(&stSM, 0, sizeof(stSM));
	PubSmInit(&stSM, stSlotMenu);
	PubSmMask(&stSM, _T("SLOT1"), SM_OFF);	
	PubSmMask(&stSM, _T("SLOT2"), SM_OFF);

	if(1 == glSysCtrl.ucSlotNum)
	{
		PubSmMask(&stSM, _T("SLOT1"), SM_ON);
	}
	else if(2 == glSysCtrl.ucSlotNum)
	{
		PubSmMask(&stSM, _T("SLOT1"), SM_ON);
		PubSmMask(&stSM, _T("SLOT2"), SM_ON);
	}

	iRet = PubSmartMenuEx(&stSM, SM_1ITEM_PER_LINE, USER_OPER_TIMEOUT);
	if (iRet<0)
	{
		return;
	}
	
	if(SLOT_1 == iRet)
	{
		pstWlPara = &glSysCtrl.stWirelessPara1;
	}
	else if(SLOT_2 == iRet)
	{
		pstWlPara = &glSysCtrl.stWirelessPara2;
	}
	glSysCtrl.ucSlot = iRet;
	WlSelSim(glSysCtrl.ucSlot);

	//--------------------------------------------------
	ScrCls();
	memset(&stSM, 0, sizeof(stSM));
	PubSmInit(&stSM, stInputMenu);
	
	iRet = PubSmartMenuEx(&stSM, SM_1ITEM_PER_LINE, USER_OPER_TIMEOUT);
	if (iRet<0)
	{
		return;
	}

	//--------------------------------------------------
	ScrCls();
	if(APN_PERFIL == iRet)
	{
		memset(&stSM, 0, sizeof(stSM));
		PubSmInit(&stSM, stAPNMenu);
		
		iRet = PubSmartMenuEx(&stSM, SM_1ITEM_PER_LINE, USER_OPER_TIMEOUT);
		if (iRet<0)
		{
			return;
		}
		SetAPNPerfil(pstWlPara, iRet);
	}
	else if(APN_MANUAL == iRet)
	{
		PubDispString(_T("SETUP"), 0|DISP_LINE_CENTER);
		ScrClrLine(1, 7);
		PubDispString(_T("APN"), 1|DISP_LINE_LEFT);

#if defined _TERMINAL_DXX_
		ScrGotoxyEx (0, 96);
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, pstWlPara->szAPN, strlen(pstWlPara->szAPN));
		iRet = GetString(szTemp, 0xF5, 0, 32);
		if(iRet != 0 && iRet != 0x0d)
		{
			return;
		}
		
		memset(pstWlPara->szAPN, 0, sizeof(pstWlPara->szAPN));
		memcpy(pstWlPara->szAPN, szTemp+1, szTemp[0]);

		ScrClrLine(1, 7);
		PubDispString(_T("LOGIN NAME"), 1|DISP_LINE_LEFT);

		ScrGotoxyEx (0, 96);
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, pstWlPara->szUID, strlen(pstWlPara->szUID));
		iRet = GetString(szTemp, 0xF5, 0, 32);
		if(iRet != 0 && iRet != 0x0d)
		{
			return;
		}
		
		memset(pstWlPara->szUID, 0, sizeof(pstWlPara->szUID));
		memcpy(pstWlPara->szUID, szTemp+1, szTemp[0]);

		ScrClrLine(1, 7);
		PubDispString(_T("LOGIN PWD"), 1|DISP_LINE_LEFT);

		ScrGotoxyEx (0, 96);
		memset(szTemp, 0, sizeof(szTemp));
		memcpy(szTemp, pstWlPara->szPwd, strlen(pstWlPara->szPwd));
		iRet = GetString(szTemp, 0xF5, 0, 16);
		if(iRet != 0 && iRet != 0x0d)
		{
			return;
		}
		
		memset(pstWlPara->szPwd, 0, sizeof(pstWlPara->szPwd));
		memcpy(pstWlPara->szPwd, szTemp+1, szTemp[0]);
#elif defined _TERMINAL_SXX_
		if( PubGetString(ALPHA_IN|ECHO_IN, 0, 32, pstWlPara->szAPN, USER_OPER_TIMEOUT)!=0 )
		{
			return;
		}

		ScrClrLine(1, 7);
		PubDispString(_T("LOGIN NAME"), 1|DISP_LINE_LEFT);

		if( PubGetString(ALPHA_IN|ECHO_IN, 0, 32, pstWlPara->szUID, USER_OPER_TIMEOUT)!=0 )
		{
			return;
		}

		ScrClrLine(1, 7);
		PubDispString(_T("LOGIN PWD"), 1|DISP_LINE_LEFT);

		if( PubGetString(ALPHA_IN|ECHO_IN, 0, 16, pstWlPara->szPwd, USER_OPER_TIMEOUT)!=0 )
		{
			return;
		}
#endif
	}

	ScrClrLine(1, 7);
	PubDispString(_T("SIM PIN"), 1|DISP_LINE_LEFT);
#ifdef _TERMINAL_DXX_
	ScrGotoxyEx (0, 96);
	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, pstWlPara->szSimPin, strlen(pstWlPara->szSimPin));
	iRet = GetString(szTemp, 0xF5, 0, 16);
	if(iRet != 0 && iRet != 0x0d)
	{
		return;
	}
	
	memset(pstWlPara->szSimPin, 0, sizeof(pstWlPara->szSimPin));
	memcpy(pstWlPara->szSimPin, szTemp+1, szTemp[0]);
#else
	if( PubGetString(ALPHA_IN|ECHO_IN, 0, 16, pstWlPara->szSimPin, USER_OPER_TIMEOUT)!=0 )
	{
		return;
	}
#endif
}

int SxxEthSet(void)
{
	int		iRet;
	uchar	ucLocalIpFlg=0, ucMaskFlg=0, ucGatewayFlg=0, ucDNSFlg=0;

	if ((strcmp(glSysCtrl.szLocalIp, "0.0.0.0") == 0) || (strcmp(glSysCtrl.szLocalIp, "") == 0))
	{
		ucLocalIpFlg = 1;
	}
	if ((strcmp(glSysCtrl.szMaskIp, "0.0.0.0") == 0) || (strcmp(glSysCtrl.szMaskIp, "") == 0))
	{
		ucMaskFlg = 1;
	}
	if ((strcmp(glSysCtrl.szGateIp, "0.0.0.0") == 0) || (strcmp(glSysCtrl.szGateIp, "") == 0))
	{
		ucGatewayFlg = 1;
	}
	if ((strcmp(glSysCtrl.szDnsIp, "0.0.0.0") == 0) || (strcmp(glSysCtrl.szDnsIp, "") == 0))
	{
		ucDNSFlg = 1;
	}
	iRet = EthSet(ucLocalIpFlg ? NULL: glSysCtrl.szLocalIp,
				  ucMaskFlg ? NULL : glSysCtrl.szMaskIp, 
				  ucGatewayFlg ? NULL : glSysCtrl.szGateIp, 
				  ucDNSFlg ? NULL : glSysCtrl.szDnsIp);
	if (iRet < 0)
	{
		return iRet;
	}
	return 0;
}

int AutoGetLocalIP(void)
{
	int		iRet;
	long        lTcpState;

	ScrClrLine(2, 7);
	PubDispString(_T("GETTING IP..."), 4|DISP_LINE_LEFT);
	iRet = SxxDhcpStart(FALSE, 30);
	if (iRet==0)
	{
		iRet = EthGet(glSysCtrl.szLocalIp, glSysCtrl.szMaskIp, glSysCtrl.szGateIp, glSysCtrl.szDnsIp, &lTcpState);
		if (iRet == 0)
		{
			glSysCtrl.stTcpipPara.ucDhcp = 1;
		}
	}

	return iRet;
}

int GetIpLocalSettings(void)
{
	int		iRet;

	iRet = GetIPAddress(_T("LOCAL IP"), TRUE, glSysCtrl.szLocalIp);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetIPAddress(_T("IP MASK"), TRUE, glSysCtrl.szMaskIp);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetIPAddress(_T("GATEWAY IP"), TRUE, glSysCtrl.szGateIp);
	if( iRet!=0 )
	{
		return iRet;
	}
	
	iRet = GetIPAddress(_T("DNS"), TRUE, glSysCtrl.szDnsIp);
	if( iRet!=0 )
	{
		return iRet;
	}

	return 0;
}


void SetTcpIpPara(void)
{
	int		iRet;
	uchar	ucKey;
	uchar	szDispBuff[32];
	long lTcpState;

	while (1)
	{
		ScrClrLine(2, 7);
		iRet = DhcpCheck();
		if (iRet==0)
		{
			sprintf((char *)szDispBuff, _T("DHCP: OK"));
		}
		else
		{
			sprintf((char *)szDispBuff, _T("DHCP: STOPPED"));
		}
	
		ucKey = PubSelectOneTwo((char *)szDispBuff, (uchar *)_T("START"), (uchar *)_T("STOP"));
		if (ucKey==KEYCANCEL)
		{
			return;
		}
		else if (ucKey==KEYENTER)
		{
			continue;
		}
		else if (ucKey==KEY1)
		{
			glSysCtrl.stTcpipPara.ucDhcp = 1;

			ScrClrLine(2, 7);
			PubDispString(_T("GETTING IP..."), 4|DISP_LINE_LEFT);
			if (SxxDhcpStart(FALSE, 30)==0)
			{
				iRet = EthGet(glSysCtrl.szLocalIp, glSysCtrl.szMaskIp, glSysCtrl.szGateIp, glSysCtrl.szDnsIp, &lTcpState);
				
				ScrClrLine(2, 7);
				PubDispString(_T("LOCAL IP"), 2|DISP_LINE_LEFT);
				PubDispString(glSysCtrl.szLocalIp, 6|DISP_LINE_RIGHT);
				getkey();
				
				ScrClrLine(2, 7);
				PubDispString(_T("IP MASK"), 2|DISP_LINE_LEFT);
				PubDispString(glSysCtrl.szMaskIp, 6|DISP_LINE_RIGHT);
				getkey();

				ScrClrLine(2, 7);
				PubDispString(_T("GATEWAY IP"), 2|DISP_LINE_LEFT);
				PubDispString(glSysCtrl.szGateIp, 6|DISP_LINE_RIGHT);
				getkey();

				ScrClrLine(2, 7);
				PubDispString(_T("DNS"), 2|DISP_LINE_LEFT);
				PubDispString(glSysCtrl.szDnsIp, 6|DISP_LINE_RIGHT);
				getkey();

				return;
			}
		}
		else if (ucKey==KEY2)
		{
			glSysCtrl.stTcpipPara.ucDhcp = 0;
			break;
		}
	}

	// Manual setup
	if (iRet == 0)
	{
		DhcpStop();
	}

	iRet = EthGet(glSysCtrl.szLocalIp, glSysCtrl.szMaskIp, glSysCtrl.szGateIp, glSysCtrl.szDnsIp, &lTcpState);
	if( iRet!=0 )
	{
		return;
	}

	iRet = GetIpLocalSettings();
	if( iRet!=0 )
	{
		return;
	}

	iRet = SxxEthSet();
	if (iRet < 0)
	{
		ScrClrLine(2, 7);
		PubDispString(_T("SET STATIC IP"), 2|DISP_LINE_LEFT);
		PubDispString(_T("FAILED."),       4|DISP_LINE_LEFT);
		PubBeepErr();
		PubWaitKey(USER_OPER_TIMEOUT);
		return;
	}

	return;
}

int SetCommInfo(void)
{
	int	iRet = 0;
	SMART_MENU	stSmDownMode;
	MENU_ITEM stCommMenu[] =
	{
		{_T("CONNECTION TYPE"),		-1,			NULL},
		{_T("TCPIP"),					TCPIP_USE,	NULL},
		{_T("GPRS"),					GPRS_USE,	NULL},
		{_T("WIFI"),					WIFI_USE,	NULL},
		{"",							0, 			NULL},
	};// This menu does not provide translation

	//--------------------------------------------------
	ScrCls();
	memset(&stSmDownMode, 0, sizeof(stSmDownMode));
	PubSmInit(&stSmDownMode, stCommMenu);
	PubSmMask(&stSmDownMode, _T("TCPIP"), SM_OFF);	
	PubSmMask(&stSmDownMode, _T("GPRS"), SM_OFF);
	PubSmMask(&stSmDownMode, _T("WIFI"), SM_OFF);

	if(glCommMode&LAN_MODE)
	{
		PubSmMask(&stSmDownMode, _T("TCPIP"), SM_ON);
	}
	if(glCommMode&GPRS_MODE)
	{
		PubSmMask(&stSmDownMode, _T("GPRS"), SM_ON);
	}
	if(glCommMode&WIFI_MODE)
	{
		PubSmMask(&stSmDownMode, _T("WIFI"), SM_ON);
	}

	iRet = PubSmartMenuEx(&stSmDownMode, SM_1ITEM_PER_LINE, USER_OPER_TIMEOUT);
	if (iRet<0)
	{
		return iRet;
	}
	
	ScrCls();
	ScrPrint(0,0,0, _T("SETUP"));

	glSysCtrl.ucCommType = iRet;
	if(glSysCtrl.ucCommType == WIFI_USE) //modified by wuc 2014.9.9
	{
		ShowLastAP();

		iRet = WifiConfig();
	}
	else if(glSysCtrl.ucCommType == GPRS_USE)
	{
		SetWirelessPara(); //modified by wuc 2013.12.17
		ScrCls();
		PubDispString(_T("INIT COMM..."), DISP_LINE_CENTER|4);
		/************++modified by wuc 2013.12.17************/
		if(SLOT_1 == glSysCtrl.ucSlot)
		{
			iRet = SXXWlInit(&glSysCtrl.stWirelessPara1);
		}
		else if(SLOT_2 == glSysCtrl.ucSlot)
		{
			iRet = SXXWlInit(&glSysCtrl.stWirelessPara2);
		}
		/************--modified by wuc 2013.12.17************/
	}
	else if(glSysCtrl.ucCommType == TCPIP_USE)
	{
		ScrCls();
		SetTcpIpPara();
		ScrCls();
		PubDispString(_T("INIT COMM..."), DISP_LINE_CENTER|4);
		if (glSysCtrl.stTcpipPara.ucDhcp)
		{
			iRet = AutoGetLocalIP();
		}
		else
		{
			if (ValidIPAddress(glSysCtrl.szLocalIp))
			{
				iRet = SxxEthSet();
			}
		}
	}

	return iRet;
}

#ifdef _TERMINAL_DXX_
int SetVTEF(void)
{
	int iRet;
	uchar ucKey, szDisp[32] = "";
	
	ScrCls();
	if(glSysCtrl.bVTEFOpt)
	{
		strcpy(szDisp, _T("AUTOMMATION (ON)"));
	}
	else
	{
		strcpy(szDisp, _T("AUTOMMATION (OFF)"));
	}
	PubDispString(szDisp, 2|DISP_LINE_CENTER);
	PubDispString(_T("1 - ON"), 3|DISP_LINE_LEFT);
	PubDispString(_T("2 - OFF"), 4|DISP_LINE_LEFT);
	while(1)
	{
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if(ucKey == KEY1)
		{
			glSysCtrl.bVTEFOpt = 1;
			iRet = SetLocalPort();
			if(iRet)
			{
				return iRet;
			}
			break;
		}
		else if(ucKey == KEY2)
		{
			glSysCtrl.bVTEFOpt = 0;
			if(accept_socket_id >= 0)
			{
				NetCloseSocket(accept_socket_id);
				accept_socket_id = -1;
			}
			glStartServer = glAcceptClient = 0;
			break;
		}
		else if(ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return ucKey;
		}
	}

	return ConfirmConfig();
}
#endif

/*********************++add by wuc 2014.1.2**********************/
void SetSlot(void)
{
	uchar ucKey, szDisp[32] = "";
	
	while(1)
	{
		ScrCls();
		if(SLOT_1 == glSysCtrl.ucSlot)
		{
			strcpy(szDisp, _T("SELECTED SLOT1"));
		}
		else if(SLOT_2 == glSysCtrl.ucSlot)
		{
			strcpy(szDisp, _T("SELECTED SLOT2"));
		}
		
		PubDispString(szDisp, 2|DISP_LINE_LEFT);
		PubDispString(_T("1. SLOT1"), 3|DISP_LINE_LEFT);
		if(2 == glSysCtrl.ucSlotNum)
		{
			PubDispString(_T("2. SLOT2"), 4|DISP_LINE_LEFT);
		}
		
		while(1)
		{
			ucKey = PubWaitKey(USER_OPER_TIMEOUT);
			if(KEY1 == ucKey)
			{
				PubDispString(_T("CHANGING SLOT..."), 6|DISP_LINE_CENTER);
				
				glSysCtrl.ucSlot = SLOT_1;
				WlSelSim(glSysCtrl.ucSlot);
				SXXWlInit(&glSysCtrl.stWirelessPara1);

				ScrClrLine(6, 7);
				break;
			}
			else if(KEY2 == ucKey && 2 == glSysCtrl.ucSlotNum)
			{
				PubDispString(_T("CHANGING SLOT..."), 6|DISP_LINE_CENTER);
				
				glSysCtrl.ucSlot = SLOT_2;
				WlSelSim(glSysCtrl.ucSlot);
				SXXWlInit(&glSysCtrl.stWirelessPara2);

				ScrClrLine(6, 7);
				break;
			}
			else if(KEYCANCEL == ucKey || NOKEY == ucKey)
			{
				return;
			}
		}
	}
}
/*********************--add by wuc 2014.1.2**********************/

//进入开启或关闭SSL界面，linzhao
int EnterSSl(void)
{
	uchar  ucKey;
	uchar szDnsResult[32];

	ScrCls();
	DispPictureBox("Cielologo.jpg", 1, 0, 0);
	PubDispString(_T("1 - Enable SSL" ), 3|DISP_LINE_LEFT );
	PubDispString(_T("2 - Disable SSL" ), 4|DISP_LINE_LEFT );

	while (1)
	{
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);

		if (KEY1==ucKey)
		{
			glSysCtrl.bSSL = 1;
			glSysCtrl.ucCommType = SSL_USE;

			ScrClrLine(2, 7);
			PubDispString(_T("Waiting..."), DISP_LINE_CENTER|4);
//			DnsResolve("mpfdev.cielo.com.br/desenv", szDnsResult, sizeof(szDnsResult));
//			DebugOutput("%s--%d--%s, iRet:%d, szDnsResult:%s\n", __FILE__, __LINE__, __FUNCTION__, iRet,szDnsResult);
//			DnsResolve("mpfdev.cielo.com.br/homolog", szDnsResult, sizeof(szDnsResult));
//			DebugOutput("%s--%d--%s, iRet:%d, szDnsResult:%s\n", __FILE__, __LINE__, __FUNCTION__, iRet,szDnsResult);
			strcpy(glSysCtrl.szRemoteIPwithSSL, "201.18.41.206");
			memcpy(glSysCtrl.szRemoteIP, glSysCtrl.szRemoteIPwithSSL, strlen(glSysCtrl.szRemoteIPwithSSL));
			glSysCtrl.ucFirstRun = 1;
			TranProcess("testecomunicacao");
			glSysCtrl.ucFirstRun = 0;

			break;
		}
		else if (KEY2==ucKey)
		{
			glSysCtrl.bSSL = 0;
			glSysCtrl.ucCommType = WIFI_USE;
			memcpy(glSysCtrl.szRemoteIP, glSysCtrl.szRemoteIPnoSSL, strlen(glSysCtrl.szRemoteIPnoSSL));
			DebugOutput("%s--%d--%s, ipssl:%s, ipnossl:%s\n", __FILE__, __LINE__, __FUNCTION__, glSysCtrl.szRemoteIPwithSSL, glSysCtrl.szRemoteIPnoSSL);
			glSysCtrl.ucFirstRun = 0;
			break;
		}
		else if (KEYCANCEL==ucKey || NOKEY==ucKey)
		{

			return ucKey;
		}
	}

	return ConfirmConfig();
}

void Function(void) //modified by wuc 2014.1.2
{
	int iRet;
	SMART_MENU	stSM;
#if defined _TERMINAL_DXX_
	MENU_ITEM stMainMenu[] =
	{
		{_T("TECHNICAL FUNCTIONS"),			-1,				NULL},
		{_T("TERMINAL"),			MENU_TERM,		NULL},
		{_T("COMMUNICATION"),	MENU_COMM,		NULL},
		{_T("COMPRESSION"),		MENU_GZIP,		NULL},
		{_T("AUTOMMATION"),		MENU_VTEF,		NULL},
		//{_T("PRINTER INFO"),	MENU_PRINT,		NULL},
		//{_T("DEBUG INFO"),		MENU_DEBUG,	NULL},
		{_T("UNLOCKED"),			MENU_UNLOCK,	NULL},
		{_T("ACTIVATION"),		MENU_ACTIVE,	NULL},
		{_T("Enable SSL"),		MENU_SSL,	NULL},
		{_T("CLEAR SETUP"),		MENU_CLEAR,		NULL},
		{_T("SET BLUETOOTH"),	MENU_BT,		NULL},
		{_T("SET SLOT"),			MENU_SLOT,		NULL},
		{"",						0, 				NULL},
	};
#elif defined _TERMINAL_SXX_
	MENU_ITEM stMainMenu[] =
	{
		{_T("TECHNICAL FUNCTIONS"),			-1,				NULL},
		{_T("TERMINAL"),			MENU_TERM,		NULL},
		{_T("COMMUNICATION"),	MENU_COMM,		NULL},
		{_T("DEBUG INFO"),		MENU_DEBUG,	NULL},
		{_T("COMPRESSION"),		MENU_GZIP,		NULL},
		{_T("UNLOCKED"),			MENU_UNLOCK,	NULL},
		{_T("ACTIVATION"),		MENU_ACTIVE,	NULL},
		{_T("SET SLOT"),			MENU_SLOT,		NULL},
		{"",						0, 				NULL},
	};
#endif

	while(1)
	{
		GetSysCtrl(); //add by wuc 2014.4.1
		
		ScrCls();
		memset(&stSM, 0, sizeof(stSM));
		PubSmInit(&stSM, stMainMenu);
#ifdef _TERMINAL_DXX_
		PubSmMask(&stSM, _T("SET BLUETOOTH"), SM_OFF);
#endif
		PubSmMask(&stSM, _T("ACTIVATION"), SM_OFF);
		PubSmMask(&stSM, _T("SET SLOT"), SM_OFF);
		//linzhao
#ifdef _SSL_ONLY_
		PubSmMask(&stSM, _T("Enable SSL"), SM_OFF);
#endif

#ifdef _TERMINAL_DXX_
		if(glCommMode&BT_MODE)
		{
			PubSmMask(&stSM, _T("SET BLUETOOTH"), SM_ON);
		}
#endif
		if(glSysCtrl.bUnlock)
		{
			PubSmMask(&stSM, _T("ACTIVATION"), SM_ON);
		}
		
		if(glCommMode&GPRS_MODE)
		{
			PubSmMask(&stSM, _T("SET SLOT"), SM_ON);
		}

		iRet = PubSmartMenuEx(&stSM, SM_1ITEM_PER_LINE, USER_OPER_TIMEOUT);
		if (iRet < 0)
		{
			break;
		}
		
		if(MENU_TERM == iRet)
		{
			SetTerminalParam();
			if(glConfiged)
			{
				glConfiged = 0;
			}
			else
			{
				continue;
			}
		}
		else if(MENU_COMM == iRet)
		{
			SetCommInfo();
		}
		else if(MENU_DEBUG == iRet)
		{
			SetDebugOpt();
			if(glConfiged)
			{
				glConfiged = 0;
			}
			else
			{
				continue;
			}
		}
		else if(MENU_GZIP == iRet)
		{
			SetGzipMode();
			if(glConfiged)
			{
				glConfiged = 0;
			}
			else
			{
				continue;
			}
		}
#ifdef _TERMINAL_DXX_
		else if(MENU_VTEF == iRet)
		{
			SetVTEF();
			if(glConfiged)
			{
				glConfiged = 0;
			}
			else
			{
				continue;
			}
		}
#endif
		else if(MENU_PRINT == iRet)
		{
			SetPrinter();
			if(glConfiged)
			{
				glConfiged = 0;
			}
			else
			{
				continue;
			}
		}
		else if(MENU_UNLOCK == iRet)
		{
			Unlock_App();
			if(glConfiged)
			{
				glConfiged = 0;
			}
			else
			{
				continue;
			}
		}
		else if(MENU_ACTIVE == iRet)
		{
			Trans_Active();
		}
		else if(MENU_CLEAR == iRet)
		{
			ClearSetup();
			if(glConfiged)
			{
				glConfiged = 0;
			}
			else
			{
				continue;
			}
		}
		else if(MENU_SLOT == iRet)
		{
			SetSlot();
		}
		//Cielo need add SSL.linzhao
		else if (MENU_SSL==iRet)
		{
			EnterSSl();
		}

		SaveSysCtrl();
	}
}

/**********************++add by wuc 2014.4.1**********************/
void SetSlotNum(void)
{
	if(-210 != WlSelSim(SLOT_1))
	{
		glSysCtrl.ucSlotNum = 1;
	}
	
	if(-210 != WlSelSim(SLOT_2))
	{
		glSysCtrl.ucSlotNum = 2;
	}
}

void LoadDefault(void)
{
	//strcpy(glSysCtrl.szDestino, "1000");
	//strcpy(glSysCtrl.szMerchantNum, "10");
	//strcpy(glSysCtrl.szStoreNum, "12");
	//strcpy(glSysCtrl.szPosNum, "83");
	glSysCtrl.ulSeq = 0;
#ifdef _SSL_ONLY_
	strcpy(glSysCtrl.szRemoteIP, "201.18.41.206");//cielo_ssl, linzhao
	strcpy(glSysCtrl.szRemotePort, "443");//cielo_ssl, linzhao
#else
	strcpy(glSysCtrl.szRemoteIP, "54.94.211.249");//177.71.185.250");//linzhao
	strcpy(glSysCtrl.szRemotePort, "7001");//linzhao
#endif
//	strcpy(glSysCtrl.szRemoteIP, "192.168.11.200");//177.71.185.250");//cielo,linzhao
//	strcpy(glSysCtrl.szRemotePort, "6502");//cielo,linzhao

	//strcpy(glSysCtrl.szLocalPort, "123");
	glSysCtrl.bDebugOpt = 1;//linzhao original:0
	glSysCtrl.ucFirstRun = 1;//linzhao
	glSysCtrl.bActivate = 0; //linzhao
	glSysCtrl.bGzipOpt = 0;
	glSysCtrl.bVTEFOpt = 0;
	glSysCtrl.stAppWifiPara.DhcpEnable = 1;
	//strcpy(glSysCtrl.stAppWifiPara.Ssid, "pax-2");
	//strcpy(glSysCtrl.stAppWifiPara.Wpa, "paxsz123");
	glSysCtrl.stAppWifiPara.SecMode = 3;
/***********++modified by wuc 2013.12.17***********/
	glSysCtrl.ucSlot = SLOT_1;
	if(glCommMode&GPRS_MODE) //add by wuc 2014.1.10
	{
		SetSlotNum();
	}
	//strcpy(glSysCtrl.stWirelessPara1.szAPN, "CMNET");
	//strcpy(glSysCtrl.stWirelessPara2.szAPN, "CMNET");
	glSysCtrl.bUnlock = 0;
	glSysCtrl.bEnableMsg = 1;
	glSysCtrl.bPPOnlineClient = 1;
	glSysCtrl.bPPOnlineAuto = 1;
	strcpy(glSysCtrl.szTechPsw, "20122012");
/***********--modified by wuc 2013.12.17***********/
	PP_TableLoadInit("030000000000");
	SaveSysCtrl();
}

int SetMessage(void)
{
	uchar ucKey;
	
	ScrCls();
	if(glSysCtrl.bEnableMsg)
	{
		PubDispString(_T("HABILITAR MENSAGENS"), 1|DISP_LINE_CENTER);
		PubDispString(_T("'IMPRIMINDO' E 'IMPRIME"), 2|DISP_LINE_CENTER);
		PubDispString(_T("2 degree VIA' (ON):"), 3|DISP_LINE_CENTER);
	}
	else
	{
		PubDispString(_T("HABILITAR MENSAGENS"), 1|DISP_LINE_LEFT);
		PubDispString(_T("'IMPRIMINDO'\nE 'IMPRIME"), 2|DISP_LINE_LEFT);
		PubDispString(_T("2 degree VIA' (OFF)"), 3|DISP_LINE_LEFT);
	}
	PubDispString(_T("1 - ON"), 4|DISP_LINE_LEFT);
	PubDispString(_T("2 - OFF"), 5|DISP_LINE_LEFT);
	
	while(1)
	{
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if(ucKey == KEY1)
		{
			glSysCtrl.bEnableMsg = 1;
			break;
		}
		else if(ucKey == KEY2)
		{
			glSysCtrl.bEnableMsg = 0;
			break;
		}
		else if(ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return ucKey;
		}
	}

	return ConfirmConfig();
}

int SetPrinter(void)
{
	int iRet;
	uchar ucKey;
	
	ScrCls();
	PubDispString(_T("PRINTER"), 1|DISP_LINE_CENTER);
	PubDispString(_T("1 - MASSAGES"), 2|DISP_LINE_LEFT);
	while(1)
	{
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if(ucKey == KEY1)
		{
			iRet = SetMessage();
			if(iRet)
			{
				return iRet;
			}
			break;
		}
		else if(ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return ucKey;
		}
	}

	return 0;
}

int FunctionChoose(void)
{
	while (1)
	{
		ScrCls();
		DispPictureBox("Cielologo.jpg", 1, 0, 0);
		PubDispString(_T("1-Input the CNPJ OR CPF"), 2|DISP_LINE_CENTER);
		PubDispString(_T("2-Test Connection"), 3|DISP_LINE_CENTER);

		if ( KEY1==getkey())
		{
			return 0;
		}
		else if ( KEY2==getkey() )
		{
			ConnectTest();
		}
	}
}

//show a input box when user inputer numbers. linzhao 2015.1.26
int EditCPF(uchar *szCPF)
{
	int iRet ;
	uchar ucUserInput;
	
	ScrCls();
	DispPictureBox("Cielologo.jpg", 1, 0, 0);
	PubDispString(_T("REPORT THE CPF OR CNPJ"), 2|DISP_LINE_CENTER);
	PubDispString(_T("REGISTERED"), 3|DISP_LINE_CENTER);

	ScrGotoxyEx(58, 98);
	ScrDrawRect(55, 96, 265, 126);
#ifdef _TERMINAL_DXX_
	kbflush();
	iRet = getkey();
	if ( KEYF1==iRet && 0==Enterfunctions(&ucUserInput))
	{
		if ('2'==ucUserInput)
		{
			Function();
		}
		else if ('0'==ucUserInput)
		{
			ConnectTest();
		}
	}
	else if (KEYF2==iRet)
	{
		WifiConfig();
		SaveSysCtrl();
	}
	else if (iRet>=KEY0 && iRet<=KEY9)
	{
		szCPF[0] = iRet;
		iRet = GetString(szCPF, 0xE5, 14, 14); 
	}
	
#else
	iRet = PubGetString(NUM_IN|ECHO_IN, 14, 14, szCPF, COMM_TIMEOUT);
	if(iRet)
	{
		return iRet;
	}
#endif

	return iRet;
}

int ConfirmUnlock(void)
{
	uchar ucKey;
	
	ScrCls();
	PubDispString(_T("CONFIRM CPF/CNPJ?"), 1|DISP_LINE_CENTER);
	PubDispString(glSysCtrl.szCPFCNPJ, 2|DISP_LINE_CENTER);
	PubDispString(_T("1 - YES"), 3|DISP_LINE_LEFT);
	PubDispString(_T("2 - NO"), 4|DISP_LINE_LEFT);
	
	while(1)
	{
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if(ucKey == KEY1)
		{
			glSysCtrl.bUnlock = 1;
			glConfiged = 1;
			break;
		}
		else if(ucKey == KEY2)
		{
			ScrCls();
			PubDispString(_T("Configure the Terminal"), 4|DISP_LINE_CENTER);
			PubWaitKey(3);
			glConfiged = 0;
			return KEYCANCEL;
		}
		else if(ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return ucKey;
		}
	}

	return 0;
}

//Build ID in Header of XML for unlock. linzhao 2015.1.26
int BuildHeaderIDUnLock(uchar *szID)
{
	uint uiLen = 0;

	uiLen += XML_AddElement(szID, NULL, "processoorigem", "100003035" );
	uiLen += XML_AddElement(szID+uiLen, NULL, "processodestino", /*"1009"*/"7000");

	return uiLen;
}

//Build Header of XML for unlock. linzhao 2015.1.26
int BuildHeaderUnLock(uchar *szHeader, uchar *szMsgType)
{
	uint uiLen = 0;
	uchar szID[256], szSeq[16];
	uchar szData[800], szOutput[512], sMacAddr[6], szMacAddr[32], szMacAddrTemp[32], szWifiName[32];
	int iRet;

	memset(szID, 0, sizeof(szID));
	memset(szData, 0, sizeof(szData));
	memset(szSeq, 0, sizeof(szSeq));

	BuildHeaderIDUnLock(szID);

	uiLen = XML_AddElement(szData, NULL, "id", szID);
	sprintf(szSeq, "%d", 0);
	uiLen += XML_AddElement(szData+uiLen, NULL, "controlesequencia", szSeq);
	uiLen += XML_AddElement(szData+uiLen, NULL, "tipomensagem", szMsgType);
	uiLen += XML_AddElement(szData+uiLen,NULL,  "numero_logico", glSysCtrl.szLogNum);

	memset(szOutput, 0, sizeof(szOutput));
	iRet = EthMacGet(sMacAddr);
	//Mac addr temporary handle, because POS API can't get Mac addr properly in D200/D210
	sprintf(szMacAddrTemp, "%x", sMacAddr);
	if ( 0==iRet)
	{
		sprintf(szMacAddr, "%.2s.%.2s.%.2s.%.2s.%.2s.%.2s",
				szMacAddrTemp, szMacAddrTemp+2,szMacAddrTemp+4, 
				szMacAddrTemp+6, szMacAddrTemp+8, szMacAddrTemp+10);
	}
	else
	{
		sprintf(szMacAddr, "%.2s.%.2s.%.2s.%.2s",
				szMacAddrTemp, szMacAddrTemp+2,szMacAddrTemp+4, szMacAddrTemp+6);
	}

	//wifi名字格式处理
	sprintf(szWifiName, "%-s%*s", glSysCtrl.stAppWifiPara.Ssid, 31-strlen(glSysCtrl.stAppWifiPara.Ssid), " ");

	sprintf(szOutput, "W%s%s", szMacAddr, szWifiName);
	uiLen += XML_AddElement(szData+uiLen, NULL, "info_network", szOutput);
	
	memset(szOutput, 0, sizeof(szOutput));
	iRet = PP_GetDUKPT("201", szOutput);
	if (0!=iRet)
	{
		sprintf(szOutput,"%d", iRet);
		uiLen += XML_AddElement(szData+uiLen, NULL, "info_dukpt_des", szOutput);
	}
	else
	{
		uiLen += XML_AddElement(szData+uiLen, NULL, "info_dukpt_des", szOutput);
	}

	memset(szOutput, 0, sizeof(szOutput));
	iRet = PP_GetDUKPT("301", szOutput);
	if (0!=iRet)
	{
		sprintf(szOutput,"%d", iRet);
		uiLen += XML_AddElement(szData+uiLen, NULL, "info_dukpt_3des", szOutput);
	}
	else
	{
		uiLen += XML_AddElement(szData+uiLen, NULL, "info_dukpt_3des", szOutput);	
	}

	memset(szOutput, 0, sizeof(szOutput));
	iRet = PP_GetInfo("00", szOutput);
	if ( 0!=iRet )
	{
		return iRet;
	}
	uiLen += XML_AddElement(szData+uiLen, NULL, "info00", szOutput);

	memset(szOutput, 0, sizeof(szOutput));
	iRet = PP_GetInfo("03", szOutput);
	if ( 0!=iRet )
	{
		return iRet;
	}
	uiLen += XML_AddElement(szData+uiLen, NULL, "info03", szOutput);

	uiLen = XML_AddElement(szHeader, NULL, "header", szData);
	if (glSysCtrl.bDebugOpt)
	{
		DebugOutput("%s--%d--%s. szHeader : %s\n", __FILE__, __LINE__, __FUNCTION__, szHeader);
	}
	return uiLen;
}

//Build the xml of unlock confirm's header. linzhao 2015.1.26
int	BuildHeaderUnLockConfirm(uchar *pszHeader, uchar *pszMsgType)
{
	uint uiLen = 0;
	uchar szIDContent[128] = "" ;

	memset(pszHeader, 0, sizeof(pszHeader));
	BuildHeaderIDUnLock(szIDContent);
	uiLen += XML_AddElement(pszHeader+uiLen, NULL, "id", szIDContent);
	uiLen += XML_AddElement(pszHeader+uiLen, NULL, "nroempresa", glSysCtrl.szMerchantNum);
	uiLen += XML_AddElement(pszHeader+uiLen, NULL, "nroloja", glSysCtrl.szStoreNum);
	uiLen += XML_AddElement(pszHeader+uiLen, NULL, "nropdv", glSysCtrl.szPosNum);
	uiLen += XML_AddElement(pszHeader+uiLen, NULL, "tipomensagem", pszMsgType);
	uiLen += XML_AddElement(pszHeader+uiLen, NULL, "controlesequencia","0");
	uiLen += XML_AddElement(pszHeader+uiLen, NULL, "versaopos","POS 2.0 de 15/06/2012");

	return 0;
}

//Build the XML of confirmation for unlock . linzhao 2015.1.26
int BuildPacketUnlockConfirm(uchar *pszMsgType)
{
	uchar  szHeader[1024];
	uchar szTemp[LEN_MAX_COMM_DATA+1] = "";
	int iRet;
	ulong ulOutLen = LEN_MAX_COMM_DATA, ulCRC, ulTempLenth;
	uchar szHttpHeader[] = "PUT / HTTP/1.0\r\nX-Forwarded-Host: mpfdev.cielo.com.br:443\r\nContent-Length:";
	BuildHeaderUnLockConfirm(szHeader, pszMsgType);

	memset(glMsgData.szHeader, 0, sizeof(glMsgData.szHeader));
	XML_AddElement(glMsgData.szHeader, NULL, "header", szHeader);

	sprintf(szTemp, "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\r\n<mensagem>\r\n%s\r\n</mensagem>", glMsgData.szHeader);
	ulTempLenth = strlen(szTemp);
	sprintf(glSendData.sContent, "%s%d\r\n\r\n%s", szHttpHeader, ulTempLenth, szTemp);	
	glSendData.ulLength = strlen(glSendData.sContent);
	
	if(glSysCtrl.bDebugOpt)
	{
		DebugOutput("SEND>>>");
		DebugOutput(glSendData.sContent);
		DebugOutput("glSendData.length:%d\n", glSendData.ulLength);
	}
	memmove(glSendData.sContent+4, glSendData.sContent, glSendData.ulLength);
	put_long(glSendData.sContent, glSendData.ulLength, 1);
	glSendData.ulLength += 4;

	return 0;
}

//Build Unlock xml to send.linzhao 2015.1.26
int	BuildPacketUnLock(uchar *pszMsgType)
{
	uchar szTemp[LEN_MAX_COMM_DATA+1] = "";
	int iRet;
	ulong ulOutLen = LEN_MAX_COMM_DATA, ulCRC, ulTempLenth;
	uchar szHttpHeader[] = "PUT / HTTP/1.0\r\nX-Forwarded-Host: mpfdev.cielo.com.br:443\r\nContent-Length:";

	BuildHeaderUnLock(glMsgData.szHeader, pszMsgType);

	sprintf(szTemp, "<?xml version=\"1.0\" encoding=\"iso-8859-1\" ?>\r\n<mensagem>\r\n%s\r\n</mensagem>", glMsgData.szHeader);
	ulTempLenth = strlen(szTemp);
	sprintf(glSendData.sContent, "%s%d\r\n\r\n%s", szHttpHeader, ulTempLenth, szTemp);	
	glSendData.ulLength = strlen(glSendData.sContent);
	
	if(glSysCtrl.bDebugOpt)
	{
		DebugOutput("SEND>>>");
		DebugOutput(glSendData.sContent);
		DebugOutput("glSendData.length:%d\n", glSendData.ulLength);
	}
	memmove(glSendData.sContent+4, glSendData.sContent, glSendData.ulLength);
	put_long(glSendData.sContent, glSendData.ulLength, 1);
	glSendData.ulLength += 4;

	return 0;
}

//comfirm of unlock xml to send, no need to receive  response. linzhao 2015.1.26 note:ready to delete
int UnlockComfirmSend()
{
	int iRet, iRetry;

	BuildPacketUnlockConfirm("confirmacao");
	PPDispStartWait();

	for(iRetry=0; iRetry<3; iRetry++)
	{
		iRet = CommConnect();
		if(iRet == 0xff)
		{
			ScrClrBelow(2);
			PubDispString(_T("CONNECT CANCELLED"), DISP_LINE_CENTER|4);
			PubWaitKey(10);
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

	iRet = CommSendMsg(glSendData.sContent, (ushort)glSendData.ulLength, COMM_TIMEOUT);
	if(iRet != 0)
	{
		CommDisconnect();
		ScrClrBelow(2);
		PubDispString(_T("SEND FAIL"), DISP_LINE_CENTER|3);
		PubWaitKey(10);
		return iRet;
	}

	return 0;
}
//show a box when user enter logical number.linzhao 2015.1.23
int EditLogNum(uchar *pszLogNum)
{
	int iRet;
	uchar ucUserInput;

	ScrCls();
	DispPictureBox("Cielologo.jpg", 1, 0, 0);
	PubDispString(_T("Insert the logical number"), DISP_LINE_LEFT|2);
	
#ifdef _TERMINAL_DXX_
	ScrDrawRect(100, 96, 220, 126);
	ScrGotoxyEx(105, 98);
	kbflush();
	iRet = getkey();
	if ( KEYF1==iRet && 0==Enterfunctions(&ucUserInput))
	{
		if ('2'==ucUserInput)
		{
			Function();
		}
		else if ('0'==ucUserInput)
		{
			ConnectTest();
		}

	}
	else if (KEYF2==iRet)
	{
		WifiConfig();
		SaveSysCtrl();
	}
	else if (iRet>=KEY0 && iRet<=KEY9)
	{
		pszLogNum[0] = iRet;
		iRet = GetString(pszLogNum, 0xE5, 8, 8);
	}
		
#endif

	return iRet;
}

//Unlock for auto config.linzhao 2015.1.26
void UnlockScrProc(void)
{
	uchar szTemp[16];//linzhao
	int iRet;

	while (1)
	{
		memset(szTemp, 0, sizeof(szTemp));
		iRet = EditLogNum(szTemp);
		if (iRet)
		{
			continue;
		}
		memcpy(glSysCtrl.szLogNum, szTemp+1, sizeof(glSysCtrl.szLogNum));
		iRet = TranProcess("desbloqueio");
		if (glSysCtrl.bDebugOpt)
		{
			DebugOutput("%s--%d--%s : iRet:%d \n", __FILE__, __LINE__, __FUNCTION__, iRet);//linzhao
		}
		if ( 0==iRet )
		{
			//CommDisconnect();//20150330
			ScrClrLine(2, 7);
			PubDispString(gl_szMensagem, DISP_LINE_CENTER|4);
			PubWaitKey(3);
			iRet = TranProcess("confirmacao");
			DelayMs(2000);//linzhao, for test
			//UnlockComfirmSend();
			//20150330
			if ( 0==iRet )
				CommDisconnect();//20150330
			glSysCtrl.bUnlock = 1;
			glSysCtrl.ucFirstRun = 0;
			glConfiged = 1;
			SaveSysCtrl();
			break;
		}
		ScrClrLine(2, 7);
		PubDispString(gl_szMensagem, DISP_LINE_CENTER|4);
		PubWaitKey(3);
	}

	return;
}

//Process of Block screen: enter number and activate. linzhao 2015.1.26
void BlockScrProc(void)
{
	int iRet;
	uchar szTemp[16];

	while (1)
	{
		memset(szTemp, 0, sizeof(szTemp));
		if( 0==EditCPF(szTemp))
		{
#ifdef _TERMINAL_DXX_
			memcpy(glSysCtrl.szCPFCNPJ, szTemp + 1, 14);
#else
			memcpy(glSysCtrl.szCPFCNPJ, szTemp, 14);
#endif
			iRet = Trans_Active();
			if ( 0==iRet )
				break;
		}
	}


}

//Ceilo requirement: Auto configurate at first run. linzhao 2015.1.26
int FirstAutoConfig()
{
	if (0!=glSysCtrl.ucFirstRun)
	{
		UnlockScrProc();
	}
	BlockScrProc();

	return 0;
}

int Unlock_App(void)
{
	int iRet = 0;
	uchar szTemp[16];

	memset(szTemp, 0, sizeof(szTemp));
	memcpy(szTemp, glSysCtrl.szCPFCNPJ, 14);
	iRet = EditCPF(szTemp);
	if(iRet)
	{
		return iRet;
	}
#ifdef _TERMINAL_DXX_
	memcpy(glSysCtrl.szCPFCNPJ, szTemp + 1, 14);
#else
	memcpy(glSysCtrl.szCPFCNPJ, szTemp, 14);
#endif

	iRet = ConfirmUnlock();
	if(!iRet)
	{
		Trans_Active();
	}

	return iRet;
}

int VerifyPassword(void)
{
	uchar szTemp[10];
	int iRet;

	while(1)
	{
		ScrCls();
		PubDispString(_T("TECHNICAL PASSWORD"), 1|DISP_LINE_LEFT);
		memset(szTemp, 0, 10);
#ifdef _TERMINAL_DXX_
		ScrGotoxyEx(0, 96);
		iRet = GetString(szTemp, 0x6D, 8, 8);
		if(iRet)
		{
			return iRet;
		}

		if(memcmp(glSysCtrl.szTechPsw, szTemp + 1, 8))
		{
			ScrClrBelow(2);
			PubDispString(_T("INVALID PASSWORD"), 4|DISP_LINE_LEFT);
			PubWaitKey(5);
			continue;
		}
#else
		iRet = PubGetString(NUM_IN|PASS_IN, 8, 8, szTemp, COMM_TIMEOUT);
		if(iRet)
		{
			return iRet;
		}

		if(memcmp(glSysCtrl.szTechPsw, szTemp, 8))
		{
			ScrClrBelow(2);
			PubDispString(_T("INVALID PASSWORD"), 4|DISP_LINE_LEFT);
			PubWaitKey(5);
			continue;
		}
#endif
		
		break;
	}

	return iRet;
}

//cielo要求增加按0来测试连接  linzhao
int Enterfunctions(uchar *pucOutput)
{
	uchar szTemp[4];
	int iRet;

	while(1)
	{
		ScrCls();
		PubDispString(_T("FUNCTION:"), 1|DISP_LINE_LEFT);
		memset(szTemp, 0, sizeof(szTemp));
#ifdef _TERMINAL_DXX_
		ScrGotoxyEx(0, 96);
		iRet = GetString(szTemp, 0xE5, 1, 1);
		if(iRet)
		{
			return iRet;
		}

/*		if(memcmp(szTemp+1, "2", 1))
		{
			ScrClrBelow(2);
			PubDispString(_T("INVALID FUNCTION"), 4|DISP_LINE_LEFT);
			PubWaitKey(5);
			continue;
		}
*/
		if (0==memcmp(szTemp+1, "2", 1) || 0==memcmp(szTemp+1, "0", 1))
		{
			*pucOutput = szTemp[1];
			break;
		}
		else
		{
			ScrClrBelow(2);
			PubDispString(_T("INVALID FUNCTION"), 4|DISP_LINE_LEFT);
			PubWaitKey(5);
			continue;
		}
#else
		iRet = PubGetString(NUM_IN|ECHO_IN, 1, 2, szTemp, COMM_TIMEOUT);
		if(iRet)
		{
			return iRet;
		}

		if(memcmp(szTemp, "2", 1))
		{
			ScrClrBelow(2);
			PubDispString(_T("INVALID FUNCTION"), 4|DISP_LINE_LEFT);
			PubWaitKey(5);
			continue;
		}
#endif
		break;
	}

	iRet = VerifyPassword();
	return iRet;
}

void GetPPOnlineAuto(uchar *szAuto)
{
	if(glSysCtrl.bPPOnlineAuto)
	{
		strcpy(szAuto, "true");
	}
	else
	{
		strcpy(szAuto, "false");
	}
}

void GetPPOnlineClient(uchar *szClient)
{
	if(glSysCtrl.bPPOnlineClient)
	{
		strcpy(szClient, "true");
	}
	else
	{
		strcpy(szClient, "false");
	}
}

void GetCPF_CNPJ(uchar *szCPF)
{
	memcpy(szCPF, glSysCtrl.szCPFCNPJ, 14);
}

// 用户输入事件检测(按键/刷卡/插卡)
int DetectCardEvent(uchar ucMode)
{
	//磁头上电、打开、清缓冲
	if( ucMode & CARD_SWIPED )
	{
		MagClose();
		MagOpen();
		MagReset();
	}

	TimerSet(TIMER_TEMPORARY, (ushort)(5*10));
	
	kbflush();
	while( 1 )
	{
		if( 0 == TimerCheck(TIMER_TEMPORARY) )	// 检查定时器
		{
			return CARD_TIMEOUT;
		}
		
		if( PubKeyPressed() )
		{
			return CARD_KEYIN;		// 有按键事件
		}
		
		if( (ucMode & CARD_SWIPED) && (MagSwiped()==0) )
		{
			return CARD_SWIPED;		// 有刷卡事件
		}
		
		if( (ucMode & CARD_INSERTED) )
		{
			if( ucMode & SKIP_DETECT_ICC )
			{
				return CARD_INSERTED;	// 有插入IC卡事件
			}
			else if( IccDetect(ICC_USER)==0 )
			{
				return CARD_INSERTED;	// 有插入IC卡事件
			}
		}
	}
}

int GetCard(uchar ucMode)
{
	int		iEventID;
	uchar	ucKey;

	while( 1 )
	{
		iEventID = DetectCardEvent(ucMode);
		if( iEventID==CARD_TIMEOUT )
		{
			return EVENT_NO_KEY;
		}
		else if( iEventID==CARD_KEYIN )
		{
			ucKey = getkey();
			if( ucMode & CARD_KEYIN )
			{
				if( ucKey==KEYCANCEL || ucKey==KEYENTER  )
				{
					continue;
				}
				else if( ucKey>KEY0 && ucKey<=KEY9 )
				{
					return EVENT_NUM_KEY;
				}
				else if( ucKey==KEY0 )
				{
					return EVENT_TERM_INFO;
				}
			#ifdef _TERMINAL_DXX_
				else if( ucKey==KEYF1 )
			#else
				else if( ucKey==KEYATM1 )
			#endif
				{
					return EVENT_FUNC_KEY;
				}
			#ifdef _TERMINAL_DXX_ //add by wuc 2014.9.9
				else if( ucKey==KEYF2 )
			#else
				else if( ucKey==KEYATM2 )
			#endif
				{
					return EVENT_WIFI_SHOTCUT;
				}
			}
		}
		else if( iEventID==CARD_SWIPED )
		{
			return EVENT_SWIPED;
		}
		else if( iEventID==CARD_INSERTED )
		{
			return EVENT_INSERTED;
		}
	}
}

int ClearSetup(void)
{
	uchar ucKey;
	
	ScrCls();
	PubDispString(_T("WANT TO ZERO CONFIGURATION"), 2|DISP_LINE_CENTER);
	PubDispString(_T("1 - YES"), 3|DISP_LINE_LEFT);
	PubDispString(_T("2 - NO"), 4|DISP_LINE_LEFT);
	while(1)
	{
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
		if(ucKey == KEY1)
		{
			memset(&glSysCtrl, 0, sizeof(glSysCtrl));
			LoadDefault();
			glConfiged = 1;
			break;
		}
		else if(ucKey == KEY2)
		{
			glConfiged = 0;
			break;
		}
		else if(ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return ucKey;
		}
	}

	return 0;
}
int BuildCampo_test(uchar *szCampo, uchar *szMsgType, uchar *szService )
{
	uchar  szOutput[2][512];
	uchar sMacAddr[6], szMacAddr[32], szMacAddrTemp[32], szWifiName[32];
	int iRet;
	uint uiLen = 0;

	memset(szOutput, 0, sizeof(szOutput));
	memcpy(szOutput[0], szService, strlen(szService));
	uiLen = AppendPostElement(szCampo, "campo", "tipoServico", 0xff, szOutput);

	memset(szOutput, 0, sizeof(szOutput));
	iRet = PP_GetInfo("00", szOutput[0]);
	if ( 0!=iRet )
	{
		return iRet;
	}
	uiLen += AppendPostElement(szCampo, "campo", "info00", 0xff, szOutput);

	memset(szOutput, 0, sizeof(szOutput));
	iRet = PP_GetInfo("03", szOutput[0]);
	if ( 0!=iRet )
	{
		return iRet;
	}
	uiLen += AppendPostElement(szCampo, "campo", "info03", 0xff, szOutput);

	memset(szOutput, 0, sizeof(szOutput));
	iRet = PP_GetDUKPT("201", szOutput[0]);
	if (0!=iRet)
	{
		sprintf(szOutput[0],"%d", iRet);
		uiLen += AppendPostElement(szCampo, "campo", "info_dukpt_des", iRet, szOutput);
	}
	else
	{
		uiLen += AppendPostElement(szCampo, "campo", "info_dukpt_des", 0xff, szOutput);
	}

	memset(szOutput, 0, sizeof(szOutput));
	iRet = PP_GetDUKPT("301", szOutput[0]);
	if (0!=iRet)
	{
		sprintf(szOutput[0],"%d", iRet);
		uiLen += AppendPostElement(szCampo, "campo", "info_dukpt_3des", iRet, szOutput);
	}
	else
	{
		uiLen += AppendPostElement(szCampo, "campo", "info_dukpt_3des", 0xff, szOutput);
	}

	memset(szOutput, 0, sizeof(szOutput));
	iRet = EthMacGet(sMacAddr);
	//Mac addr temporary handle, because POS API can't get Mac addr properly in D200/D210
	sprintf(szMacAddrTemp, "%x", sMacAddr);
	if ( 0==iRet)
	{
		sprintf(szMacAddr, "%.2s.%.2s.%.2s.%.2s.%.2s.%.2s",
			szMacAddrTemp, szMacAddrTemp+2,szMacAddrTemp+4, 
			szMacAddrTemp+6, szMacAddrTemp+8, szMacAddrTemp+10);
	}
	else
	{
		sprintf(szMacAddr, "%.2s.%.2s.%.2s.%.2s",
			szMacAddrTemp, szMacAddrTemp+2,szMacAddrTemp+4, szMacAddrTemp+6);
	}

	//wifi名字格式处理
	sprintf(szWifiName, "%-s%*s", glSysCtrl.stAppWifiPara.Ssid, 31-strlen(glSysCtrl.stAppWifiPara.Ssid), " ");

	sprintf(szOutput[0], "W%s%s", szMacAddr, szWifiName);
	uiLen += AppendPostElement(szCampo, "campo", "info_network", 0xff, szOutput);

	memset(szOutput, 0, sizeof(szOutput));
	strcpy(szOutput[0], "POS 2.0 de 15/01/2015");
	uiLen += AppendPostElement(szCampo, "campo", "versaopos", 0xff, szOutput);
	if (glSysCtrl.bDebugOpt)
	{
		DebugOutput("%s--%d--%s. szCampo: %s\n", __FILE__, __LINE__, __FUNCTION__, szCampo);
	}
	return uiLen;
}

//Cielo : add the test connection.//linzhao
int ConnectTest()
{
	int iRet;

	iRet = TranProcess("identificacao");
	DebugOutput("%s--%d--%s, iRet:%d\n", __FILE__, __LINE__, __FUNCTION__, iRet);
	if(iRet)
	{
		CommDisconnect();
		return iRet;
	}
	memset(&glMsgData, 0, sizeof(MSG_DATA));
	memset(&glSendData, 0, sizeof(COMM_DATA));
	memset(&glRecvData, 0, sizeof(COMM_DATA));
	BuildCampo_test(glMsgData.szPost, "solicitacao", "testecomunicacao");
	iRet = TranProcess("solicitacao");
	if (0!=iRet)
	{
		CommDisconnect();
		ScrClrBelow(2);
		PubDispString(_T("CONNECTION FAILED"), 4|DISP_LINE_LEFT);
		PubWaitKey(3);
	}

	return 0;
}
/**********************--add by wuc 2014.4.1**********************/

