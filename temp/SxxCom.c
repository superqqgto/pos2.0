
#include "posapi.h"
#include "posapi_all.h"
#include "posapi_s80.h"
#include "SxxCom.h"
//#include "lng.h"
#include "appLib.h"
#include "util.h"
#include "lng.h"
//#include "terstruc.h"
//#include "extfunc.h"
//#include "glbvar.h"
//#include "ssl.h"
//#include "lan.h"

#define TCPMAXSENDLEN		10240
#define TIMER_TEMPORARY		4
static uchar SocketCheck(int sk);

static int sg_iSocket = -1;

// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpConnect(uchar *pszIP, short sPort, int iTimeoutSec)
{
	int		iRet;
	int	    i;
	struct net_sockaddr stServer_addr;

#ifdef DEBUG_USE
	return 0;
#endif
	/*Try to connect through TCP 3 times,according to the device from S/W department*/
	/* If supporting multi-ip, don't use for loop ,try once,not 3 times, for every ip*/
	for (i=0; i<1; i++)
	{
		if(sg_iSocket >= 0)
		{
			return 0;
		}
		iRet = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
		if (iRet < 0)
		{
			return iRet;	
		}
		sg_iSocket = iRet;

		// set connection timeout
		if (iTimeoutSec < 3)
		{
			iTimeoutSec = 3;
		}
		iRet = Netioctl(sg_iSocket, CMD_TO_SET, iTimeoutSec*1000);//advice from S/W department
		if (iRet != 0)
		{
			return iRet;
		}
		iRet = SockAddrSet(&stServer_addr, pszIP, sPort);
		if (iRet != 0)
		{
			return iRet;
		}

		iRet = NetConnect(sg_iSocket, &stServer_addr, sizeof(stServer_addr));
		if (iRet == 0)
		{
			break;
		}
		NetCloseSocket(sg_iSocket);
		sg_iSocket = -1;
		DelayMs(500);
	}

	return iRet;
}

//Sxx TCP/IP send data
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpTxd(uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec)
{
	int iRet;
	int iSendLen;
	int iSumLen;
	char sBuff[33];

#ifdef DEBUG_USE
	DelayMs(500);
	return 0;
#endif

	iRet = Netioctl(sg_iSocket, CMD_TO_SET, uiTimeOutSec*1000);
	if (iRet < 0)
	{
		return iRet;
	}

	iSumLen = 0;
	while(1)
	{
		if (uiDataLen > TCPMAXSENDLEN)
		{
			iSendLen = TCPMAXSENDLEN;
			uiDataLen = uiDataLen - TCPMAXSENDLEN;
		}
		else
		{
			iSendLen = uiDataLen;
		}	
		iRet = NetSend(sg_iSocket, psTxdData+iSumLen, iSendLen, 0);
		if (iRet < 0)
		{
			ScrClrLine(2, 7);
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

			ScrClrLine(2,7);
		//	PubDispString(sBuff,3|DISP_LINE_LEFT);
			ScrPrint(0, 3, ASCII,sBuff);
			PubWaitKey(5);

			return iRet;
		}
		if (iRet != iSendLen)
		{
			return -1;
		}
		iSumLen = iSumLen + iSendLen;
		if (iSendLen <= TCPMAXSENDLEN)
		{
			break;
		}	
	}
	return 0;
}

//Sxx TCP/IP receive
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpRxd(uchar *psRxdData, ulong uiExpLen, ulong *puiOutLen, ushort uiTimeOutSec)
{
	int   iRet;
	char  sBuff[33];
	int	  iCountTimeout = 0;

#ifdef DEBUG_USE
	DelayMs(200);
	return 0;
#endif
	iRet = 	Netioctl(sg_iSocket, CMD_TO_SET, uiTimeOutSec*1000);
	if (iRet < 0)
	{
		return iRet;
	}

	while(1)
	{
		iRet=NetRecv(sg_iSocket, psRxdData, uiExpLen, 0);
		if (iRet < 0)
		{
			if (iRet==-13) /*try 3 times if error is -13*/
			{
				iCountTimeout++;
				if (iCountTimeout < 3)
				{
					continue;
				}
				else/*already got 3 times error -13*/
				{
					break;
				}
			}
			/*other error code,break*/
			break;
		}
		*puiOutLen = iRet;

		return 0;
	}

	ScrClrLine(2, 7);
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
	case NET_ERR_TIMEOUT:
		strcpy(sBuff,_T("TIMEOUT"));
		break;
	case NET_ERR_VAL:
		strcpy(sBuff,_T("VARIABLE ERR"));
		break;
	case NET_ERR_ARG:
		strcpy(sBuff,_T("PARA ERROR"));
		break;

	default:
		sprintf(sBuff,"ret_rcv=%d",iRet);
		break;
	}

	ScrClrLine(2,7);
	PubDispString(sBuff,3|DISP_LINE_LEFT);
	PubWaitKey(5);

	return iRet;
}

//Sxx TCP/IP close socket
// Shared by GPRS/CDMA/PPP/LAN/WIFI/...
int SxxTcpOnHook(void)
{
	int iRet;

#ifdef DEBUG_USE
	return 0;
#endif

	iRet = NetCloseSocket(sg_iSocket);
	sg_iSocket = -1; 
	return iRet;
}

uchar SocketCheck(int sk)
{
	int event;
	if (sk < 0) 
	{
		return RET_TCPCLOSED;
	}

	event = Netioctl(sk, CMD_EVENT_GET, 0);
	if (event < 0)
	{
		NetCloseSocket(sk);
		return RET_TCPCLOSED;
	}	

	if (event&(SOCK_EVENT_CONN|SOCK_EVENT_WRITE|SOCK_EVENT_READ))
	{
		return RET_TCPOPENED;
	}
	else if (event&(SOCK_EVENT_ERROR))
	{
		NetCloseSocket(sk);
		return RET_TCPCLOSED;
	}

	return RET_TCPOPENING;
}

int SxxDhcpStart(uchar ucForceStart, uchar ucTimeOutSec)
{
	int	iRet = 0;

	#ifdef DEBUG_USE
	return 0;
#endif
	if (ucForceStart && (DhcpCheck()==0))
	{
		DhcpStop();
	}

	if (ucForceStart || (DhcpCheck()!=0))
	{
		iRet = DhcpStart();
		if (iRet < 0)
		{
			return iRet;
		}

		TimerSet(TIMER_TEMPORARY, (ushort)(ucTimeOutSec*10));
		while (TimerCheck(TIMER_TEMPORARY)!=0)
		{
			DelayMs(200);
			iRet = DhcpCheck();
			if (iRet==0)
			{
				return 0;
			}
		}

		return iRet;
	}

	return 0;
}

// Shawn 20101206 add DNS
int Dns_Resolve(char *szInURL, char *szOutIP)
{
 	int dnsResult, i;
							
	// DNS , try 3 times
	for (i = 0; i < 3 && dnsResult != 0; i++)
	{
		dnsResult = DnsResolve(szInURL, szOutIP, sizeof(szOutIP));
	}
	if (dnsResult != 0)
	{
		return dnsResult;
	}

	return 0;
}

//显示信号强度
void SXXWlDispSignal(void)
{
	uchar	ucRet, ucLevel;
	
	
	ucRet = WlGetSignal(&ucLevel);
	if( ucRet!=RET_OK )
	{
		ScrSetIcon(ICON_SIGNAL, CLOSEICON);
		return;
	}
	
	ScrSetIcon(ICON_SIGNAL, (uchar)(5-ucLevel));
}


//初始化无线模块
int SXXWlInit(WIRELESS_PARAM *pstWlPara)
{
	int iRet;

	iRet = WlInit(pstWlPara->szSimPin);
	if (iRet == -212)	//模块已初始化
	{
		iRet = 0;
	}

	SXXWlDispSignal();

	return iRet;
}

//先判断PPP链路是否联通，若没有联通先进行PPP链接，成功后再进行TCP连接；若已联通则直接进行TCP连接
int SXXWlDial(WIRELESS_PARAM *pstWlPara, int iTimeOut, int iAliveInterval)
{
	int		iRet;
	int		iRetryTime;

	SXXWlDispSignal();

#ifdef DEBUG_USE
	return 0;
#endif
	if (iTimeOut<1)
	{
		iTimeOut = 1;
	}

	// ********** Full-dial **********

	// ********** Check PPP connection **********
/*	TimerSet(TIMER_TEMPORARY, (ushort)(iTimeOut*10));
	while (TimerCheck(TIMER_TEMPORARY)!=0)
	{
		iRet = WlPppCheck();

		if (iRet == 0)
		{
			goto TCPCONNECT;
		}
	}*/

	// ********** Take PPP dial action **********
	iRetryTime = 3;
	while(iRetryTime--)
	{
		iRet = WlPppLogin(pstWlPara->szAPN, pstWlPara->szUID, pstWlPara->szPwd, 0xFF, iTimeOut*1000, iAliveInterval);
		if (iRet != 0)
		{
			DelayMs(100);
			continue;
		}

		iRet = WlPppCheck();
		if (iRet == 0)
		{
			break;
		}
	}

	if (iRetryTime <= 0 && iRet != 0)
	{
		return iRet;
	}

	// ********** Connect IP **********
//TCPCONNECT:	
	iRet = SocketCheck(sg_iSocket);  //come from R&D, tom
//	ScrPrint(0, 7, ASCII, "tang[SocketCheck(%i)]",iRet); DelayMs(1000);
	if (iRet == RET_TCPOPENED)
	{
		return 0;
	}

	iRet = SxxTcpConnect(glSysCtrl.szRemoteIP, (short)atoi(glSysCtrl.szRemotePort), iTimeOut);
	if (iRet < 0)
	{
		return iRet;
	}

	return 0;
}

//发送数据
int SXXWlSend(uchar *psTxdData, ushort usDataLen, ushort uiTimeOutSec)
{
	return SxxTcpTxd(psTxdData, usDataLen, uiTimeOutSec);
}

//接收数据
int SXXWlRecv(uchar *psRxdData, ulong usExpLen, ulong *pusOutLen, ushort uiTimeOutSec)
{
	DelayMs(200);
	return SxxTcpRxd(psRxdData, usExpLen, pusOutLen, uiTimeOutSec);
}

//关闭TCP连接
int SXXWlCloseTcp(void)
{
	int iRet;

	iRet = NetCloseSocket(sg_iSocket);
	if (iRet < 0)
	{
		return iRet;
	}
	
	sg_iSocket = -1;
	return 0;
}

//关闭PPP链路
void SXXWlClosePPP(void)
{
	WlPppLogout(); 
	return;
}


