/****************************************************************************
NAME
	SxxCom.h - ����ģ���װ����

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
	��    �ܣ�	TCP/IP �������ݡ�GPRS/CDMA/PPP/LAN/WIFI ����ʹ�á�
	���������
			psTxdData��ָ�����ݻ�����
			uiDataLen�����ݳ���
			iTimeout�����ͳ�ʱʱ�䡣����3�밴3������
	����������
			��
	�� �� �룺	0   �ɹ�
			<0  ʧ��
****************************************************************************/
int SxxTcpTxd(uchar *psTxdData, ushort uiDataLen, ushort uiTimeOutSec);


/****************************************************************************
	��    �ܣ�	TCP/IP �������ݡ�GPRS/CDMA/PPP/LAN/WIFI ����ʹ�á�
	���������
			psRxdData��ָ�����ݽ��ջ�����
			uiExpLen���������ɽ��յ�������ݳ���
			iTimeout�����ճ�ʱʱ�䡣����3�밴3������
			puiOutLen��ʵ���յ����ݵĳ���
	����������
			��
	�� �� �룺	0   �ɹ�
			<0  ʧ��
****************************************************************************/
int SxxTcpRxd(uchar *psRxdData, ulong uiExpLen, ulong *puiOutLen, ushort uiTimeOutSec);

/****************************************************************************
	��    �ܣ�	TCP/IP �ر�socket��GPRS/CDMA/PPP/LAN/WIFI ����ʹ�á�
	���������
			��
	����������
			��
	�� �� �룺	0   �ɹ�
			<0  ʧ��
****************************************************************************/
int SxxTcpOnHook(void);


/****************************************************************************
	Function:		Sϵ�е�DHCPִ��
	Param In:
	ucForceStart	bool,�Ƿ�ǿ��ִ��
	ucTimeOutSec	��ʱʱ��
	Param Out:		none
	Return Code:
	0			OK
	other		fail
****************************************************************************/
int SxxDhcpStart(uchar ucForceStart, uchar ucTimeOutSec);

int SxxTcpConnect(uchar *pszIP, short sPort, int iTimeoutSec);

// Sϵ�е���������
int Dns_Resolve(char *szInURL, char *szOutIP);

int SXXWlInit(WIRELESS_PARAM *pstWlPara);
int SXXWlDial(WIRELESS_PARAM *pstWlPara, int iTimeOut, int iAliveInterval);
int SXXWlSend(uchar *psTxdData, ushort usDataLen, ushort uiTimeOutSec);
int SXXWlRecv(uchar *psRxdData, ulong usExpLen, ulong *pusOutLen, ushort uiTimeOutSec);
int SXXWlCloseTcp(void);
void SXXWlClosePPP(void);

#endif//end of #ifndef _SWIRELESS_H
