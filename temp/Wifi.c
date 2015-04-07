#include "posapi.h"
#include "posapi_all.h"
#include "applib.h"
#include "util.h"
#include "wifi.h"
#include "setup.h"
#include "lng.h"
#include "global.h"



#ifdef _TERMINAL_DXX_

/********************** Internal macros declaration ************************/
/********************** Internal structure declaration *********************/
/********************** Internal functions declaration *********************/
/********************** Internal variables declaration *********************/
/********************** external reference declaration *********************/

int wnet_socket_id = -1;
int server_socket_id = -1;
int accept_socket_id = -1;

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

int CheckStrValid(char *str)
{
	while((*str)!=0)
	{
		if ((*str) < '0' || (*str) > '9')
		{
			return -1;
		}
		str++;
	}
	return 0;
}

int StringToIp(char *ipstr, char *ip)
{
	char temp[4];
	char *pStr1, *pStr2;
	int i = 0;
	pStr1 = ipstr;
	for (i = 0;i < 3;i++)
	{
		memset(temp, 0, sizeof(temp));
		pStr2 = strstr(pStr1, ".");
		if (pStr2 == NULL)
		{
			return -1;
		}
		memcpy(temp, pStr1, pStr2 - pStr1);
		if (CheckStrValid(temp))
		{
			return -1;
		}
		ip[i] = atoi(temp);
		pStr1 = pStr2 + strlen(".");
	}
	memset(temp, 0, sizeof(temp));
	strcpy(temp, pStr1);
	if (CheckStrValid(temp))
	{
		return -1;
	}
	ip[3] = atoi(temp);
	return 0;
}


//登陆WIFI 热点
int WifiApLogin(void)
{
	int iRet;
	uchar ucTempBuf[20];
	ST_WIFI_AP WifiAp;
	ST_WIFI_PARAM WifiParam;

	memset(&WifiAp, 0, sizeof(ST_WIFI_AP));
	memset(&WifiParam, 0, sizeof(ST_WIFI_PARAM));

	strcpy(WifiAp.Ssid, glSysCtrl.stAppWifiPara.Ssid);
	WifiAp.SecMode = glSysCtrl.stAppWifiPara.SecMode;
	WifiParam.DhcpEnable = glSysCtrl.stAppWifiPara.DhcpEnable;

	if (WifiParam.DhcpEnable == 0)	//DHCP 关闭 
	{
		memset(ucTempBuf, 0, sizeof(ucTempBuf));
		StringToIp(glSysCtrl.szLocalIp, WifiParam.Ip);
		StringToIp(glSysCtrl.szMaskIp, WifiParam.Mask);
		StringToIp(glSysCtrl.szGateIp, WifiParam.Gate);
		StringToIp(glSysCtrl.szDnsIp, WifiParam.Dns);
	}

	if(WifiAp.SecMode == 1)
	{
		strcpy(WifiParam.Wep, glSysCtrl.stAppWifiPara.Wep);
	}
	if(WifiAp.SecMode == 2 || WifiAp.SecMode == 3)
	{
		strcpy(WifiParam.Wpa, glSysCtrl.stAppWifiPara.Wpa);
	}

	iRet = WifiCheck(&WifiAp);
	if(iRet > 0)
	{
		return 0;
	}
	
	iRet = WifiConnectAp(&WifiAp, &WifiParam);
	if(iRet != 0)
	{
		return iRet;
	}

	while (1) 
	{
		iRet = WifiCheck(&WifiAp);
		if (iRet > 0) 
		{
			break;
		}
		else if(iRet == 0)
		{
			continue; 
		}
		else
		{
			return iRet;
		}
	}

	if(glSysCtrl.stAppWifiPara.DhcpEnable)
	{
		NetDevGet(12, glSysCtrl.szLocalIp, glSysCtrl.szMaskIp, glSysCtrl.szGateIp, glSysCtrl.szDnsIp);
	}
	glConnectFlag = 1;
	return 0;
}

int ShowLastAP(void) //add by wuc 2014.9.9
{
	int iRet;

	//读取参数并显示
	kbflush();
	ScrCls(); //add by wuc 2014.1.10
	PubDispString(_T("CONFIGURATIONS"), DISP_LINE_CENTER|0);
	if(strlen(glSysCtrl.stAppWifiPara.Ssid) == 0)
	{
		ScrPrint(0, 1, 0, _T("NO AP CONNECTED")); 
	}
	else
	{
		ScrPrint(0, 1, 0, _T("NETWORK: %s\n"),  glSysCtrl.stAppWifiPara.Ssid);  
		if (glSysCtrl.stAppWifiPara.SecMode == 1) 
		{
			if(glSysCtrl.ucWepMode == 0)
			{	
				ScrPrint(0, 2, 0, _T("MODE:WEP(ASCII)"));
				ScrPrint(0, 3, 0, _T("PASSWORD:%s\n"),  glSysCtrl.stAppWifiPara.Wep);
			}
			else
			{
				ScrPrint(0, 2, 0, _T("MODE:WEP(HEX)"));
				ScrPrint(0, 3, 0, _T("PASSWORD:%02x%02x%02x%02x%02x\n"),  glSysCtrl.stAppWifiPara.Wep[0], glSysCtrl.stAppWifiPara.Wep[1], glSysCtrl.stAppWifiPara.Wep[2], glSysCtrl.stAppWifiPara.Wep[3], glSysCtrl.stAppWifiPara.Wep[4]);
			}
		}
		else if (glSysCtrl.stAppWifiPara.SecMode == 2) 
		{
			ScrPrint(0, 2, 0,  _T("MODE:WPA/WPA2"));
			ScrPrint(0, 3, 0, _T("PASSWORD:%s\n"),  glSysCtrl.stAppWifiPara.Wpa);    
		}
		else if (glSysCtrl.stAppWifiPara.SecMode == 3) 
		{
			ScrPrint(0, 2, 0, _T("MODE:WPA-PSK/WPA2-PSK"));
			ScrPrint(0, 3, 0, _T("PASSWORD:%s\n"),  glSysCtrl.stAppWifiPara.Wpa);
		}
		else
		{
			ScrPrint(0, 2, 0, _T("MODE:NONE"));
		}
		
		if (glSysCtrl.stAppWifiPara.DhcpEnable == 1) 
		{
			ScrPrint(0, 4, 0, _T("DHCP: OPEN"));
		}
		else
		{
			ScrPrint(0, 4, 0, _T("DHCP: CLOSE"));
		}
	}

	PubDispString(_T("(PRESS ENTER TO CONTINUE)"), DISP_LINE_CENTER|6);

	while(1)
	{
		iRet = PubWaitKey(USER_OPER_TIMEOUT);
		if(iRet == KEYCANCEL || iRet == NOKEY)
		{
			return iRet;
		}
		if(iRet == KEYENTER)
		{
			break;
		}
	}

	return 0;
}

//选择WIFI打开模式, 是扫描周围的热点，还是手工输入SSID
int SelectWifiMode(void)
{
	int iRet = 0;
	uchar ucKey;

	while(1)
	{
		kbflush();
		ScrCls();
		PubDispString(_T("WIFI"), DISP_LINE_CENTER|0);
		ScrPrint(0, 1, 0, _T("1 - SEARCH NETWORK"));
		ScrPrint(0, 2, 0, _T("2 - MANUAL")); 

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);

		if (ucKey == KEY1) 
		{
			iRet = ScanWifiAps();
			if (iRet) 
			{
				//add by wuc 2014.3.24
				if(WIFI_ERR_SCAN == iRet)
				{
					WifiClose();
					iRet = WifiOpen();
					if(iRet)
					{
						ScrClrBelow(1);
						ScrPrint(0, 2, 0, "OpenWifi err=%d", iRet); 
						PubWaitKey(5);
						break;
					}
				}
				continue;
			}
			break;
		}

		if (ucKey == KEY2) 
		{
			iRet = ManualWifiAps();
			if (iRet) 
			{
				continue;
			}
			break;
		}

		if (ucKey == KEYCANCEL || ucKey == NOKEY) 
		{
			return ucKey;
		}
	}

	return iRet;
}

//手工设置WIFI 热点
int ManualWifiAps(void)
{
	int iRet, iSecMode;
	uchar ucSsid[100], ucTempBuf[100];

	//SSID
	memset(ucSsid, 0, sizeof(ucSsid));
	iRet = GetWifiSsid(ucSsid);
	if (iRet) 
	{
		return iRet;
	}
	strcpy(glSysCtrl.stAppWifiPara.Ssid, ucSsid);

	//加密方式
	iSecMode = 0;
	iRet = GetWifiSecMode(ucSsid, (uchar *)&iSecMode);
	if (iRet) 
	{
		return iRet;
	}
	if (iSecMode == 0)
	{
		glSysCtrl.stAppWifiPara.SecMode = 0;
		return 0;
	}
	else
	{
		glSysCtrl.stAppWifiPara.SecMode = iSecMode;
	}

	//WIFI 密码
	memset(ucTempBuf, 0, sizeof(ucTempBuf));
	iRet = GetWifiKey(ucSsid, iSecMode, ucTempBuf);
	if(iRet)
	{
		return iRet;
	}
	
	if (iSecMode == 1)	//WEP
	{
		strcpy(glSysCtrl.stAppWifiPara.Wep, ucTempBuf);
	}
	else
	{
		strcpy(glSysCtrl.stAppWifiPara.Wpa, ucTempBuf);
	}

	while(1)
	{
		iRet = ConfirmConfig();
		if(iRet)
		{
			break;
		}

		if(glConfiged)
		{
			glConfiged = 0;
			break;
		}
		else
		{
			iRet = GetWifiKey(ucSsid, 1, ucTempBuf);
			if (iRet) 
			{
				break;
			}
			strcpy(glSysCtrl.stAppWifiPara.Wep, ucTempBuf);
			continue;
		}
	}

	return iRet;
}

//扫描周围的热点
int ScanWifiAps(void)
{
	int i, iRet, iApCount;
	uchar ucKey, ucTempBuf[100];
	ST_WIFI_AP stWifiAps[16];

	ScrClrBelow(1);
	ScrPrint(0, 1, 0, _T("SCANNING AP...")); //scanning ap

	if(strlen(glSysCtrl.stAppWifiPara.Ssid) != 0)
	{
		WifiDisconAp();
	}

	iApCount = 8;
	memset(&stWifiAps, 0, sizeof(stWifiAps));
	iRet = WifiScanAps(&stWifiAps[0], iApCount);
	if(iRet == 0)
	{
		ScrClrBelow(1);
		ScrPrint(0, 2, 0, _T("NO AVAILABLE AP"));
		getkey();
		return WIFI_ERR_NULL;
	}
	if(iRet < 0)
	{
		ScrClrBelow(1);
		ScrPrint(0, 2, 0, "ScanAps err=%d", iRet); 
		getkey();
		return WIFI_ERR_SCAN;
	}

	ScrCls();
	PubDispString(_T("SEARCH NETWORK"), DISP_LINE_CENTER|0);  //please select
	
	//显示搜索到的热点
	for (i=0; i<iRet; i++) 
	{
		ScrPrint(0, (uchar)(i + 1), 0, "%d - %s",i, stWifiAps[i].Ssid);
	}

	while (1) 
	{
		kbflush();
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);

		if (ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			return ucKey; //modify by wuc 2014.4.1
		}
	
		i = ucKey - 0x30;
		if(i<0 || i>iRet)
		{
			continue;
		}
		
		break;
	}

	strcpy(glSysCtrl.stAppWifiPara.Ssid, stWifiAps[i].Ssid);
	
	//判断是否需要密码
	if(stWifiAps[i].SecMode == 0)	//无加密
	{
		glSysCtrl.stAppWifiPara.SecMode = 0;
	}
	else if(stWifiAps[i].SecMode == 1)	//WEP加密
	{
		glSysCtrl.stAppWifiPara.SecMode = 1;
		memset(ucTempBuf, 0, sizeof(ucTempBuf));
		iRet = GetWifiKey(stWifiAps[i].Ssid, stWifiAps[i].SecMode, ucTempBuf);
		if(iRet)
		{
			return iRet;
		}
		strcpy(glSysCtrl.stAppWifiPara.Wep, ucTempBuf);
	}
	else if(stWifiAps[i].SecMode == 2)	//WPA/WPA2加密 TKIP
	{
		glSysCtrl.stAppWifiPara.SecMode = 2;
		memset(ucTempBuf, 0, sizeof(ucTempBuf));
		iRet = GetWifiKey(stWifiAps[i].Ssid, stWifiAps[i].SecMode, ucTempBuf);
		if(iRet)
		{
			return iRet;
		}
		strcpy(glSysCtrl.stAppWifiPara.Wpa, ucTempBuf);
	}
	else if(stWifiAps[i].SecMode == 3)	//WPA-PSK/WPA2-PSK加密 AES
	{
		glSysCtrl.stAppWifiPara.SecMode = 3;
		memset(ucTempBuf, 0, sizeof(ucTempBuf));
		iRet = GetWifiKey(stWifiAps[i].Ssid, stWifiAps[i].SecMode, ucTempBuf);
		if(iRet)
		{
			return iRet;
		}
		strcpy(glSysCtrl.stAppWifiPara.Wpa, ucTempBuf);
	}
	else
	{
		ScrClrRect(0, 24, 319, 215);
		ScrGotoxyEx(0, 24);
		Lcdprintf("%s\n", stWifiAps[i].Ssid);
		Lcdprintf(_T("UNSUPPORTED MODE\n"));
		getkey();
		return WIFI_ERR_CANCEL;
	}
	
	return 0;
}

//输入WIFI 密码
int GetWifiKey(uchar *psName, int iSecMode, uchar *psOut)
{
	uchar ucRet;
	uchar ucTempBuf[100];
	uchar ucWEPMode;

	if (iSecMode < 1 || iSecMode > 3) 
	{
		return 0;
	}

	ScrCls();
	PubDispString(psName, DISP_LINE_CENTER|1);
	
	if(iSecMode == 1)
	{
		PubDispString(_T("SELECT WEP MODE"), DISP_LINE_CENTER|2);
		ScrPrint(0, 3, 0, _T("1. ASCII"));
		ScrPrint(0, 4, 0, _T("2. HEX"));
		while(1)
		{
			ucRet = PubWaitKey(USER_OPER_TIMEOUT);
			if(ucRet == KEY1)
			{
				ucWEPMode = 0;
				break;
			}
			else if(ucRet == KEY2)
			{
				ucWEPMode = 1;
				break;
			}
			else if(ucRet == KEYCANCEL || ucRet == NOKEY)
			{
				return ucRet;
			}
		}
		ScrClrBelow(2);
	}
	
	PubDispString(_T("TYPE PASSWORD:"), DISP_LINE_CENTER|2);
	ScrGotoxyEx (0, 96);
	memset(ucTempBuf, 0, sizeof(ucTempBuf));
	if (iSecMode == 1)	//WEP 
	{
		if(glSysCtrl.ucWepMode == 0)	//ascii
		{
			strcpy(ucTempBuf, glSysCtrl.stAppWifiPara.Wep);
		}
		else		//hex
		{
			PubBcd2Asc0(glSysCtrl.stAppWifiPara.Wep, 5, ucTempBuf);
		}
		ucRet = GetString(ucTempBuf, 0xF5, (uchar)(ucWEPMode==0 ? 5:10), (uchar)(ucWEPMode==0 ? 5:10));
		glSysCtrl.ucWepMode = ucWEPMode;
	}
	else	//WPA
	{
		strcpy(ucTempBuf, glSysCtrl.stAppWifiPara.Wpa);
		ucRet = GetString(ucTempBuf, 0xF5, 1, KEY_WPA_MAXLEN);
	}

	if (ucRet == 0x00)
	{
		if((iSecMode == 1) && (ucWEPMode == 1))
		{
			PubAsc2Bcd(&ucTempBuf[1], ucTempBuf[0], psOut);
		}
		else
		{
			memcpy(psOut, &ucTempBuf[1], ucTempBuf[0]);
		}
		return 0;
	}
	if (ucRet == 0x0D) 
	{
		return 0;
	}

	return ucRet;
}

//输入WIFI SSID
int GetWifiSsid(uchar *psOut)
{
	uchar ucRet;
	uchar ucTempBuf[100];

	memset(ucTempBuf, 0, sizeof(ucTempBuf));
	strcpy(ucTempBuf, glSysCtrl.stAppWifiPara.Ssid);

	ScrCls();
	PubDispString(ucTempBuf, DISP_LINE_CENTER|1);
	PubDispString(_T("SSID:"), DISP_LINE_CENTER|2);
	ScrGotoxyEx (0, 96);
	ucRet = GetString(ucTempBuf, 0xF5, 0, KEY_WPA_MAXLEN);
	if (ucRet)
	{
		return ucRet;
	}

	memcpy(psOut, &ucTempBuf[1], ucTempBuf[0]);
	return 0;
}

//输入WIFI 加密方式
int GetWifiSecMode(uchar *psName, uchar *psOut)
{
	uchar ucKey;

	ScrCls();
	PubDispString(_T("ENCRYPTION"), DISP_LINE_CENTER|1);
	ScrPrint(0, 3, 0, _T("1 - NONE"));
	ScrPrint(0, 4, 0, _T("2 - WEP"));
	ScrPrint(0, 5, 0, _T("3 - WPA/WPA2"));
	ScrPrint(0, 6, 0, _T("4 - WPA-PSK/WPA2-PSK"));

	do {
		ucKey = PubWaitKey(USER_OPER_TIMEOUT);
	} while(ucKey!=KEY1 && ucKey!=KEY2 && ucKey!=KEY3 && ucKey!=KEY4 && ucKey!=KEYCANCEL);

	if (ucKey == KEY1) 
	{
		*psOut = 0;	
	}

	if (ucKey == KEY2) 
	{
		*psOut = 1;	
	}
	if (ucKey == KEY3) 
	{
		*psOut = 2;	
	}

	if (ucKey == KEY4) 
	{
		*psOut = 3;	
	}

	if (ucKey == KEYCANCEL || ucKey == NOKEY) 
	{
		return ucKey;
	}

	return 0;
}

//设置WIFI 本地IP参数
int SetWifiLocalIpPara(void)
{
	int iRet = 0;
	uchar ucKey;

	while(1)
	{
		kbflush();
		ScrCls();
		PubDispString(_T("PROTOCOL"), DISP_LINE_CENTER|0);
		ScrPrint(0, 1, 0, _T("1 - DHCP"));
		ScrPrint(0, 2, 0, _T("2 - MANUAL"));

		ucKey = PubWaitKey(USER_OPER_TIMEOUT);

		if (ucKey == KEY1) 
		{
			glSysCtrl.stAppWifiPara.DhcpEnable= 1;
			glConfiged = 1;
			break;
		}

		if (ucKey == KEY2) 
		{
			glSysCtrl.stAppWifiPara.DhcpEnable = 0;
			iRet = ManualWifiIps();
			if (iRet) 
			{
				return iRet;
			}
			
			iRet = ConfirmConfig();
			if(iRet)
			{
				return iRet;
			}
			break;
		}

		if (ucKey == KEYCANCEL || ucKey == NOKEY) 
		{
			return ucKey;
		}

		/*if(ucKey == KEYENTER)
		{
			break;
		}*/
	}

	return iRet;
}

//设置WIFI 静态IP参数
int ManualWifiIps(void)
{
	int iRet;

	iRet = GetIPAddress(_T("IP ADDRESS"), TRUE, glSysCtrl.szLocalIp);
	if( iRet!=0 )
	{
		return iRet;
	}

	iRet = GetIPAddress(_T("SUB-NETWORK MASK"), TRUE, glSysCtrl.szMaskIp);
	if( iRet!=0 )
	{
		return iRet;
	}
	
	iRet = GetIPAddress(_T("STANDARD GATEWAY"), TRUE, glSysCtrl.szGateIp);
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

int wifiConnect(uchar *inIp, uchar *inPort)
{
	int i = 0;
	int ret;
	int trans_port;
	int event;
	int ret_num;
	char  sBuff[33];
	ST_WIFI_AP WifiAp;
	struct net_sockaddr server_addr;

	for (i = 0;i < 1; i++) // change 5 to 1 by wuc 2014.9.2
	{
		ret = WifiCheck(&WifiAp);
		if (ret < 0)
		{
			ret = WifiApLogin();
			if (ret != 0 && i != 4)
			{
				continue;
			}
			else if (ret != 0 && i == 4)
			{
				return ret;
			}
			else
			{
				break;
			}
		}
		else if (ret == 0)
		{
			DelayMs(500);
			continue;
		}
		else
		{
			break;
		}
	}
	
	trans_port = atoi(inPort);

	//如果TCP仍然连接，就不用再连接
	if(wnet_socket_id >= 0)
	{

		event = Netioctl(wnet_socket_id, CMD_EVENT_GET, 0);

		if(event&(SOCK_EVENT_CONN|SOCK_EVENT_WRITE|SOCK_EVENT_READ))
		{
			return 0;
		}
	}

	if(wnet_socket_id >= 0)
	{
		NetCloseSocket(wnet_socket_id);
		wnet_socket_id = -1;
	}

	ret = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
	if(ret < 0)
	{
		return -1;
	}

	wnet_socket_id = ret;

	/*ret = Netioctl(wnet_socket_id, CMD_TO_SET, 30*1000);//advice from S/W department
	if (ret != 0)
	{
		DebugOutput("ret = [%d]", ret);
		return ret;
	}*/

	SockAddrSet(&server_addr, (char*)inIp, (short)trans_port);
	
	ret_num = NetConnect(wnet_socket_id, &server_addr, sizeof(server_addr));
	if(ret_num < 0)
	{
		switch (ret_num)
		{
			case NET_ERR_ABRT:
			case NET_ERR_CONN:
			case NET_ERR_IF://linzhao
				strcpy(sBuff,_T("CONN FAIL"));
				break;
			case NET_ERR_RST:
				strcpy(sBuff,_T("CONN RESET"));
				break;
			case NET_ERR_CLSD:
				strcpy(sBuff,_T("CONN CLOSE"));
				break;
			case NET_ERR_RTE:
				strcpy(sBuff,_T("ROUTE ERR"));
				break;
			case NET_ERR_ISCONN:
				strcpy(sBuff,_T("CONN EXIST"));
				break;
			case NET_ERR_TIMEOUT:
				strcpy(sBuff,_T("CONN TIMOUT"));
				break;
			case NET_ERR_PASSWD:
				strcpy(sBuff,_T("PASSWORD ERR"));
				break;
			case NET_ERR_MODEM:
				strcpy(sBuff,_T("PPP FAIL"));
				break;
			case NET_ERR_LINKDOWN:
			case NET_ERR_LOGOUT:
			case NET_ERR_PPP:
				strcpy(sBuff,_T("RETRY PPP"));
				break;
			default:
				strcpy(sBuff,_T("UNKNOW ERR"));
				break;
		}

		ScrClrBelow(2);
		PubDispString(sBuff, DISP_LINE_LEFT|2);
		//Cielo要求去除返回码， linzhao
//		sprintf(sBuff, _T("code=%d "), ret_num);
//		PubDispString(sBuff, DISP_LINE_LEFT|3);
		PubWaitKey(2);

		NetCloseSocket(wnet_socket_id);
		wnet_socket_id = -1;
		return -1;
	}
	else
		return 0;
}

int Wifi_Connect()
{
	int ret;

#ifdef DEBUG_USE
	return 0;
#endif
	
	ret = wifiConnect(glSysCtrl.szRemoteIP, glSysCtrl.szRemotePort);
	if(glSysCtrl.bDebugOpt)
	{
		DebugOutput("%s--%d, wifi connect[%s:%s] = [%d]", __FILE__, __LINE__, glSysCtrl.szRemoteIP, glSysCtrl.szRemotePort, ret);
	}
	return ret;
}

void WifiHook()
{
	int ret;
#ifdef DEBUG_USE
	return;
#endif
	ret = NetCloseSocket(wnet_socket_id);
	if(ret == 0)
	{
		wnet_socket_id = -1;
	}
	return;
}

uchar Ap_Connect()
{
	int i ,j;
	int iApCount;
	int iRet;
	
	ST_WIFI_AP WifiAp;
	ST_WIFI_AP stWifiAps[20];
	ST_WIFI_PARAM WifiParam;

	memset(&WifiAp, 0, sizeof(ST_WIFI_AP));
	memset(&WifiParam, 0, sizeof(ST_WIFI_PARAM));
	strcpy(WifiAp.Ssid, glSysCtrl.stAppWifiPara.Ssid);
	WifiAp.SecMode = glSysCtrl.stAppWifiPara.SecMode;
	WifiParam.DhcpEnable = glSysCtrl.stAppWifiPara.DhcpEnable;

	if (WifiParam.DhcpEnable == 0)	//DHCP 关闭 
	{
		StringToIp(glSysCtrl.szLocalIp, WifiParam.Ip);
		StringToIp(glSysCtrl.szMaskIp, WifiParam.Mask);
		StringToIp(glSysCtrl.szGateIp, WifiParam.Gate);
		StringToIp(glSysCtrl.szDnsIp, WifiParam.Dns);
	}

	if(WifiAp.SecMode == 1)
	{
		strcpy(WifiParam.Wep, glSysCtrl.stAppWifiPara.Wep);
	}
	if(WifiAp.SecMode == 2 || WifiAp.SecMode == 3)
	{
		strcpy(WifiParam.Wpa, glSysCtrl.stAppWifiPara.Wpa);
	}
	for (i=0; i<5; i++)
	{
		iRet = WifiCheck(&WifiAp);
		if (iRet < 0)
		{
			iApCount = 15;
			iRet = WifiScanAps(&stWifiAps[0],iApCount);
			for (j= 0; j<iRet; j++)
			{
				if (memcmp(glSysCtrl.stAppWifiPara.Ssid, stWifiAps[j].Ssid,strlen(glSysCtrl.stAppWifiPara.Ssid)) == 0)
				{
					break;
				}
			}
			if (j != iRet && memcmp(glSysCtrl.stAppWifiPara.Ssid, stWifiAps[j].Ssid,strlen(glSysCtrl.stAppWifiPara.Ssid)) == 0 )
			{
				iRet = WifiConnectAp(&stWifiAps[j],&WifiParam);
				if (iRet != 0)
				{
					return -1;
				}
				break;
			}
			else continue;
		}
	}
	while (1) 
	{
		iRet = WifiCheck(&WifiAp);
		if (iRet > 0) 
		{
			break;
		}
		else if(iRet == 0)
		{
			continue; 
		}
		else
		{
			return iRet;
		}
	}
	return 0;
}

////发送数据
int WifiNetTcpTxd(uchar *psTxdData, ushort usDataLen, ushort uiTimeOutSec)
{
	int iRet;
	int iSendLen;
	int iSumLen;
	uchar sBuff[128];

#ifdef DEBUG_USE
	DelayMs(500);
	return 0;
#endif
//	Netioctl(wnet_socket_id, CMD_TO_SET, uiTimeOutSec*1000);   // 系统默认为2秒
	iSumLen = 0;
	while(1)
	{
		if (usDataLen > TCPMAXSENDLEN)
		{
			iSendLen = TCPMAXSENDLEN;
			usDataLen = usDataLen - TCPMAXSENDLEN;
		}
		else
		{
			iSendLen = usDataLen;
		}
		iRet = NetSend(wnet_socket_id, psTxdData+iSumLen, iSendLen, 0);
		{
			DebugOutput("%s--%d--%s, netsend : iRet  = %d\n", __FILE__, __LINE__, __FUNCTION__, iRet);
		}//linzhao
		if (iRet < 0)
		{
			memset(sBuff, 0, sizeof(sBuff));
			switch (iRet)
			{
			case NET_ERR_MEM:
				strcpy(sBuff,_T("MEM ERROR"));
				break;
			case NET_ERR_BUF:
				strcpy(sBuff,_T("BUF ERROR"));
				break;
			case NET_ERR_CLSD:
				strcpy(sBuff,_T("CONNE CLOSED"));
				break;
			case NET_ERR_CONN:
				strcpy(sBuff,_T("CONNE NOT EST"));
				break;
			case NET_ERR_LINKDOWN:
				strcpy(sBuff,_T("LINK DOWN"));
				break;
			default:
				sprintf(sBuff,"ret_send=%d",iRet);
				break;
			}

			DxxScrClrLine(2, 7);
			PubDispString(sBuff,4|DISP_LINE_CENTER);
			PubWaitKey(5);
			return iRet;
		}

		iSumLen = iSumLen + iSendLen;
		if (iSendLen <= TCPMAXSENDLEN)
		{
			break;
		}	
	}
	return 0;
}

//接收数据
int WifiNetTcpRxd( uchar * RxData,ulong ExpLen,ulong * RxLen,ushort ms)
{
	int ret;

#ifdef DEBUG_USE
	return 0;
#endif

/*	ret = Netioctl(wnet_socket_id, CMD_TO_SET, ms*1000);//设置超时
	if(ret < 0)
		return ret;
	*/
	TimerSet(TIMER_TEMPORARY, (ushort)(ms*10));
	ret = 0;
	
	while(1)
	{
		if( TimerCheck(TIMER_TEMPORARY)==0 )	// 检查定时器
		{
			if( ret>0 )	// 已经读取到数据
			{
				break;
			}
			return 0xff;
		}
		
		ret = NetRecv(wnet_socket_id, RxData,  ExpLen, 0);
		if(ret > 0)
		{
			*RxLen = ret;
			return 0;    
		}
	//else
	//{
		/*	DxxScrClrLine(4, 18);
			
			switch (ret)
			{
				case NET_ERR_MEM:
					strcpy(sBuff,("MEM ERROR"));
					break;
				case NET_ERR_BUF:
					strcpy(sBuff,("BUF ERROR"));
					break;
				case NET_ERR_CLSD:
					strcpy(sBuff,("CONNE CLOSED"));
					break;
				case NET_ERR_CONN:
					strcpy(sBuff,("CONNE NOT EST"));
					break;
				case NET_ERR_LINKDOWN:
					strcpy(sBuff,("LINK DOWN"));
					break;
				case NET_ERR_TIMEOUT:
					strcpy(sBuff,("TIMEOUT"));
					break;
				case NET_ERR_VAL:
					strcpy(sBuff,("VARIABLE ERR"));
					break;
				case NET_ERR_ARG:
					strcpy(sBuff,("PARA ERROR"));
					break;

				default:
					sprintf(sBuff,"ret_rcv=%d",ret);
					ret = 0xff;
					break;
			}

			DxxScrClrLine(4,18);
			PubDispString(sBuff,4|DISP_LINE_CENTER);
			PubWaitKey(5);
	*/
		//if(ret == 0) ret = 0xff;
		//return ret;   
	//}
		}
	return 0;
}

int WifiStartServer(void)
{
	int iRet;
	struct net_sockaddr server_addr;
	
	if(server_socket_id >= 0)
	{
		NetCloseSocket(server_socket_id);
		server_socket_id = -1;
	}

	iRet = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
	if(iRet < 0)
	{
		return iRet;
	}

	server_socket_id = iRet;

	iRet = SockAddrSet(&server_addr, glSysCtrl.szLocalIp, (short)atoi(glSysCtrl.szLocalPort));
	if(iRet < 0)
	{
		return iRet;
	}
	
	iRet = NetBind(server_socket_id, &server_addr, sizeof(server_addr));
	if(iRet < 0)
	{
		return iRet;
	}
	
	iRet = NetListen(server_socket_id, 5);
	if(glSysCtrl.bDebugOpt)
	{
		DebugOutput("listen start = [%d]", iRet);
	}
	return iRet;
}

int WifiAccept(void)
{
	int iRet;
	struct net_sockaddr stSocketAddr;
	socklen_t len;
	uchar szIp[15+1];
	ushort usPort;

	memset(&stSocketAddr, 0, sizeof(struct net_sockaddr));
	memset(szIp, 0, sizeof(szIp));
	iRet = SockAddrGet(&stSocketAddr, szIp, &usPort);
	if(iRet < 0)
	{
		return iRet;
	}

	iRet = NetAccept(server_socket_id, &stSocketAddr, &len);
	if(iRet < 0)
	{
		return iRet;
	}
	accept_socket_id = iRet;

	iRet = Netioctl(accept_socket_id, CMD_IO_SET, 1);
	return iRet;
}

int WifiStatus(void)
{
	int iRet;

	iRet  = Netioctl(accept_socket_id, CMD_EVENT_GET, 0);
	if(iRet == SOCK_EVENT_ERROR)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

//连接WIFI
int WifiConfig(void) //add by wuc 2014.9.9
{
	int iRet;
	
/*	iRet = SelectWifiMode();
	if(iRet)
	{
		return iRet;
	}
*/
	
	while(1)
	{
		//linzhao
		iRet = SelectWifiMode();
		if(iRet)
		{
			break;
		}

		iRet = SetWifiLocalIpPara();
		if(iRet)
		{
			break;
		}

		if(glConfiged)
		{
			glConfiged = 0;
		}
		else
		{
			iRet = ManualWifiAps();
			if (iRet) 
			{
				break;
			}
		}
		
		ScrCls();
		PubDispString(_T("CONNECT..."), DISP_LINE_CENTER|3);	//CONNECTING
		iRet = WifiApLogin();
		if(iRet == 0)
		{
			ScrCls();
			PubDispString(_T("WIFI CONNECTED"), DISP_LINE_CENTER|3);
			PubWaitKey(3);
			break;
		}
		else
		{
			ScrCls();
			PubDispString(_T("CONNECT FAIL"), DISP_LINE_CENTER|3);	//CONNECT FAIL
			PubWaitKey(3);
			continue;
		}
	}

	return iRet;
}

#else
int WifiApLogin(void)
{
	return 0;
}

int Wifi_Connect()
{
	return 0;
}

int WifiNetTcpTxd(uchar *psTxdData, ushort usDataLen, ushort uiTimeOutSec)
{
	return 0;
}

int WifiNetTcpRxd( uchar * RxData,ulong ExpLen,ulong * RxLen,ushort ms)
{
	return 0;
}

void WifiHook()
{
	return;
}
int WifiStartServer(void)
{
	return 0;
}

int WifiAccept(void)
{
	return 0;
}

int WifiStatus(void)
{
	return 0;
}

int ManualWifiAps(void)
{
	return 0;
}

int SetWifiLocalIpPara(void)
{
	return 0;
}

int SelectWifiMode(void)
{
	return 0;
}
#endif

//connect wifi at the first fun. linzhao 2015.1.26
void WifiPrompt(void)
{
	ST_WIFI_AP WifiAp;

	while( WifiCheck(&WifiAp)<0 )	
	{
		ScrCls();
		PubDispString(_T("Configure WIFI network"), DISP_LINE_CENTER|3);
		PubDispString(_T("Press F2"), DISP_LINE_CENTER|4);
		if ( KEYF2==getkey() && 0==WifiConfig() )
		{
#ifdef _SSL_ONLY_
			glSysCtrl.ucCommType = SSL_USE;
#else
			glSysCtrl.ucCommType = WIFI_USE;
#endif
			break;
		}
	}
}

// end of file



