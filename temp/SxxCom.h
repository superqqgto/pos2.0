/****************************************************************************
NAME
	SxxCom.h - 无线模块封装函数

DESCRIPTION

REFERENCE

MODIFICATION SHEET:
	MODIFIED   (YYYY.MM.DD)

****************************************************************************/


#ifndef _SWIRELESS_S
#define _SWIRELESS_S

#ifndef uchar
#define uchar   unsigned char
#endif
#ifndef uint
#define uint    unsigned int
#endif
#ifndef ushort
#define ushort  unsigned short
#endif
#ifndef ulong
#define ulong   unsigned long
#endif

#ifndef TRUE
#define TRUE	1
#define FALSE	0
#endif

#include "Util.h"

/****************************************************************************
	功    能：	TCP/IP 发送数据。GPRS/CDMA/PPP/LAN/WIFI 均可使用。
	传入参数：
			psTxdData：指向数据缓冲区
			uiDataLen：数据长度
			iTimeout：发送超时时间。不足3秒按3秒设置
	传出参数：
			无
	返 回 码：	0   成功
			<0  失败
****************************************************************************/
int SxxTcpTxd(uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec);


/****************************************************************************
	功    能：	TCP/IP 接收数据。GPRS/CDMA/PPP/LAN/WIFI 均可使用。
	传入参数：
			psRxdData：指向数据接收缓冲区
			uiExpLen：缓冲区可接收的最大数据长度
			iTimeout：接收超时时间。不足3秒按3秒设置
			puiOutLen：实际收到数据的长度
	传出参数：
			无
	返 回 码：	0   成功
			<0  失败
****************************************************************************/
int SxxTcpRxd(uchar *psRxdData, ulong uiExpLen, ulong *puiOutLen, ushort uiTimeOutSec);

/****************************************************************************
	功    能：	TCP/IP 关闭socket。GPRS/CDMA/PPP/LAN/WIFI 均可使用。
	传入参数：
			无
	传出参数：
			无
	返 回 码：	0   成功
			<0  失败
****************************************************************************/
int SxxTcpOnHook(void);


/****************************************************************************
	Function:		S系列的DHCP执行
	Param In:
	ucForceStart	bool,是否强制执行
	ucTimeOutSec	超时时间
	Param Out:		none
	Return Code:
	0			OK
	other		fail
****************************************************************************/
int SxxDhcpStart(uchar ucForceStart, uchar ucTimeOutSec);

int SxxTcpConnect(uchar *pszIP, short sPort, int iTimeoutSec);

// S系列的域名解析
int Dns_Resolve(char *szInURL, char *szOutIP);

int SXXWlInit(WIRELESS_PARAM *pstWlPara);
int SXXWlDial(WIRELESS_PARAM *pstWlPara, int iTimeOut, int iAliveInterval);
int SXXWlSend(uchar *psTxdData, ushort usDataLen, ushort uiTimeOutSec);
int SXXWlRecv(uchar *psRxdData, ulong usExpLen, ulong *pusOutLen, ushort uiTimeOutSec);
int SXXWlCloseTcp(void);
void SXXWlClosePPP(void);

#endif//end of #ifndef _SWIRELESS_H
