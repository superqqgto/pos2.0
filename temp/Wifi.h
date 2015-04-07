/****************************************************************************
NAME
	wifi.h - ʵ��Dxx WIFI ͨѶ

DESCRIPTION
	1. ��ģ���װ��DXX WIFI��ͨѶ,��װ��Ӧ�Ľӿ�

REFERENCE

MODIFICATION SHEET:
	MODIFIED   (YYYY.MM.DD)
	Renjl		2013.01.14		- created
	
****************************************************************************/

#ifndef _WIFI_H
#define _WIFI_H

#include "posapi.h"
#define	WIFI_ERR_START	-1000	//Ӧ�ò�WIFIģ�鷵�صĴ�������ʼֵ
#define WIFI_ERR_END	-1100	//Ӧ�ò�WIFIģ�鷵�صĴ��������ֵ
#define WIFI_OK			0		//WIFI ���سɹ�

#define WIFI_ERR_NULL	WIFI_ERR_START-2	//δ�ҵ��ȵ�
#define WIFI_ERR_SCAN	WIFI_ERR_START-3	//�����ȵ�ʧ��

#define WIFI_ERR_CANCEL	WIFI_ERR_START-1	//�ͻ�ȡ��
#define WIFI_ERR_INPUT	WIFI_ERR_START-4	//�ͻ�����ʧ��
#define TCPMAXSENDLEN		10240 

extern int wnet_socket_id;
extern int server_socket_id;
extern int accept_socket_id;

typedef struct{
   
	uchar Ssid[SSID_MAXLEN];	/* ��������AP��SSID */
	int SecMode;				/* ��ȫģʽ */
	int Rssi;					/* ��������AP���ź�ǿ�� */
	int DhcpEnable;				/* DHCPʹ�ܣ�0-�رգ�1-���� */
	uchar Ip[IPLEN];			/* ��̬IP*/
	uchar Mask[IPLEN];			/* ���� */
	uchar Gate[IPLEN];			/* ���� */
	uchar Dns[IPLEN];			/* DNS */
	uchar Wep[KEY_WEP_LEN];		/* wep���� */
	uchar Wpa[KEY_WPA_MAXLEN];	/* WPA��������*/
	uchar reserved[256];		/*Ԥ��*/
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

