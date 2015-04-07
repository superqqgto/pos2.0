/****************************************************************************
NAME
	wifi.h - 实现Dxx WIFI 通讯

DESCRIPTION
	1. 本模块封装了DXX WIFI的通讯,封装相应的接口

REFERENCE

MODIFICATION SHEET:
	MODIFIED   (YYYY.MM.DD)
	Renjl		2013.01.14		- created
	
****************************************************************************/

#ifndef _WIFI_H
#define _WIFI_H

#include "posapi.h"
#define	WIFI_ERR_START	-1000	//应用层WIFI模块返回的错误码起始值
#define WIFI_ERR_END	-1100	//应用层WIFI模块返回的错误码结束值
#define WIFI_OK			0		//WIFI 返回成功

#define WIFI_ERR_NULL	WIFI_ERR_START-2	//未找到热点
#define WIFI_ERR_SCAN	WIFI_ERR_START-3	//搜索热点失败

#define WIFI_ERR_CANCEL	WIFI_ERR_START-1	//客户取消
#define WIFI_ERR_INPUT	WIFI_ERR_START-4	//客户输入失败
#define TCPMAXSENDLEN		10240 

extern int wnet_socket_id;
extern int server_socket_id;
extern int accept_socket_id;

typedef struct{
   
	uchar Ssid[SSID_MAXLEN];	/* 搜索到的AP的SSID */
	int SecMode;				/* 安全模式 */
	int Rssi;					/* 搜索到的AP的信号强度 */
	int DhcpEnable;				/* DHCP使能，0-关闭，1-开启 */
	uchar Ip[IPLEN];			/* 静态IP*/
	uchar Mask[IPLEN];			/* 掩码 */
	uchar Gate[IPLEN];			/* 网关 */
	uchar Dns[IPLEN];			/* DNS */
	uchar Wep[KEY_WEP_LEN];		/* wep密码 */
	uchar Wpa[KEY_WPA_MAXLEN];	/* WPA连接密码*/
	uchar reserved[256];		/*预留*/
}APP_WIFI_PARA;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

	int WifiApLogin(void);
	int SetWifiPara(void);
	int ShowLastAP(void); //add by wuc 2014.9.9
	int ManualWifiAps(void);
	int ScanWifiAps(void);
	int GetWifiKey(uchar *psName, int iSecMode, uchar *psOut);
	int GetWifiSsid(uchar *psOut);
	int GetWifiSecMode(uchar *psName, uchar *psOut);
	int ManualWifiIps(void);
	int Wifi_Connect();
	uchar Ap_Connect();
	int WifiNetTcpTxd(uchar *psTxdData, ushort usDataLen, ushort uiTimeOutSec);
	int WifiNetTcpRxd( uchar * RxData,ulong ExpLen,ulong * RxLen,ushort ms);
	void WifiHook();
	int WifiStartServer(void);
	int WifiAccept(void);
	int WifiStatus(void);
	int WifiApCheck(void);
	int WifiApRelogin(void);
	int WifiConfig(void); //add by wuc 2014.9.9
	void WifiPrompt(void);//linzhao
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* _WIFI_H */

