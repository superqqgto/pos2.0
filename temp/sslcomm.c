
#include "global.h"

/********************** Internal macros declaration ************************/
#define SSLMAXSENDLEN		4000
#define MAX_PEM_ELN			2048*3

#define CER_FILE			"cielo.crt"
/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/
static int SSLSysTime(SYSTEM_TIME_T *date);
static void GetRandom(unsigned char *pucDataOut);
static int SSLRandom(unsigned char *buf, int len);
static int SSLNetOpen(char *remote_addr, short remote_port, short local_port, long flag);
static int SSLNetSend(int net_hd, void *buf, int size);
static int SSLNetRecv(int net_hd, void *buf, int size);
static int SSLNetClose(int net_hd);
static int OurServCertAck(CERT_INVAL_CODE reason);
static int ReadPemFile(uchar *pszAppFileName, uchar *psCA, uchar *psCert, uchar *psPrivatekey);

/********************** Internal variables declaration *********************/
static SSL_NET_OPS stSslNetOps=
{
	SSLNetOpen,
	SSLNetSend,
	SSLNetRecv,
	SSLNetClose
};

static SSL_SYS_OPS stSslSysOps=
{
	SSLSysTime,
	SSLRandom,
	NULL,
	NULL
};

static int iSSLSocket = -1;
static int s_iSocket = -1;//linzhao

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/


int Ssl_Connect( char *szRemoteIP, short RemotePort, int TimeOutSec )
{
	int iRet, iEvent;
	int length;
	SSL_BUF_T  pos_ca;
	SSL_BUF_T  pos_cert;
	SSL_BUF_T  pos_privatekey;
	SSL_BUF_T  pos_certs[2];

	char pos_ca_pem[2048];
	char pos_cert_pem[2048];
	char pos_privatekey_pem[2048];

//	char pos_ca_buf[2048];
	char pos_cert_buf[2048];
//	char pos_privatekey_buf[2048];

	//linzhao 0331
	DebugOutput("%s--%d iSSLSocket:%d\n", __FILE__, __LINE__, iSSLSocket);
	if (iSSLSocket>=0)
	{
/*		iEvent = Netioctl(iSSLSocket, CMD_TO_GET, 0);
		DebugOutput("%s--%d iEvent:%d\n", __FILE__, __LINE__, iEvent);
		if (iEvent&(SOCK_EVENT_CONN|SOCK_EVENT_READ|SOCK_EVENT_WRITE))
		{
			return 0;
		}
		*/
		return 0;
	}
	SslSetNetOps( &stSslNetOps );	//Let SSL module know how to perform TCP/IP operations(open, send, receive and close)
	DebugOutput("%s--%d,SslSetNetOps\n", __FILE__, __LINE__);
	
	memset( &stSslSysOps, 0, sizeof(SSL_SYS_OPS) );
	stSslSysOps.ServCertAck = OurServCertAck;
	SslSetSysOps( &stSslSysOps );	//Define some system related call back functions to use by SSL module

//*
//  begin: 证书验证
	length = 0;
	memset(pos_ca_pem, 0, sizeof(pos_ca_pem));
	memset(pos_cert_pem, 0, sizeof(pos_cert_pem));

	memset(pos_privatekey_pem, 0, sizeof(pos_privatekey_pem));
	iRet = ReadPemFile(CER_FILE, pos_ca_pem, pos_cert_pem, pos_privatekey_pem);
	DebugOutput("%s--%d--%s, ReadPemFile iRet:%d\n", __FILE__, __LINE__, __FUNCTION__, iRet);
	if (iRet == 0)
	{
		pos_ca.size = 0;
		pos_ca.ptr = NULL;

		length = 0;
		memset(pos_cert_buf, 0, sizeof(pos_cert_buf));
		length = SslDecodePem(pos_cert_pem, strlen(pos_cert_pem), pos_cert_buf, sizeof(pos_cert_buf));
		if (length <= 0)
		{
			ScrPrint(0, 6, 1, _T("ssl Cert wrong!"));
			PubWaitKey(30);
			return ERR_NO_DISP;
		}
		pos_cert.size = length;
		pos_cert.ptr = pos_cert_buf;

		pos_privatekey.size = 0;
		pos_privatekey.ptr = NULL;
	}


//  end: for 证书验证
//*/

	iRet = SslCreate();
	if ( iRet < 0 )
	{
		return iRet;
	}
	iSSLSocket = iRet;
	DebugOutput("%s--%d--%s, iSSLSocket:%d\n", __FILE__, __LINE__, __FUNCTION__, iSSLSocket);

	pos_certs[0] = pos_cert;
	pos_certs[1] = pos_ca;
	iRet = SslCertsSet(iSSLSocket, 0, 0, NULL, pos_certs, 1, &pos_privatekey);
	DebugOutput("%s--%d--%s, SslCertsSet iRet:%d\n", __FILE__, __LINE__, __FUNCTION__, iRet);
	if (iRet < 0)
	{
		ScrPrint(0, 6, 1, "SslCertsSet=%d", iRet);
		PubWaitKey(30);
		return iRet;
	}

	iRet = SslConnect( iSSLSocket, szRemoteIP, RemotePort, 0, 0 );
	DebugOutput("%s--%d--%s, sslConnect:[%s:%d]=[%d], iSSLSocket:%d\n", __FILE__, __LINE__, __FUNCTION__, szRemoteIP, RemotePort, iRet, iSSLSocket);
	//you wen ti
	if ( iRet != 0 &&  0==SslClose( iSSLSocket ))//linzhao
	{
		DebugOutput("%s--%d, iSSLSocket:%d, s_iSocket:%d\n", __FILE__, __LINE__, iSSLSocket, s_iSocket);

		iSSLSocket = -1;
		s_iSocket = -1;//linzhao
	}
	DebugOutput("%s--%d--%s, iSSLSocket:%d\n", __FILE__, __LINE__, __FUNCTION__, iSSLSocket);
	return iRet;
}

int Ssl_Txd( char *psTxdData, ulong uiDataLen, ushort uiTimeOutSec )
{
	int iRet;
	int iSendLen;
	int iSumLen;

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
		iRet = SslSend( iSSLSocket, psTxdData+iSumLen, iSendLen );
		DebugOutput("%s--%d--%s, sslSend: iRet:%d, lenth:%d\n", __FILE__, __LINE__, __FUNCTION__, iRet, iSendLen);
		if (iRet < 0)
		{
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

int Ssl_Rxd( char *psRxdData, ulong uiExpLen,  ulong *puiOutLen, ushort UiTimeOutSec)
{
	int iRet;
	
	TimerSet(TIMER_TEMPORARY, (ushort)(UiTimeOutSec*10));
	iRet = 0;

	while (1)
	{
		if( TimerCheck(TIMER_TEMPORARY)==0 )	// 检查定时器
		{
			if( iRet>0 )	// 已经读取到数据
			{
				*puiOutLen = iRet;
				return 0;
			}
			return 0xff;
		}

		iRet = SslRecv( iSSLSocket, psRxdData, uiExpLen );
		if (iRet > 0)
		{
			*puiOutLen = iRet;
			return 0;
		}
	}

}

int Ssl_OnHook( void )
{
	int iRet;
	

	DebugOutput("%s--%d, sslClose start\n", __FILE__, __LINE__);
	iRet = SslClose( iSSLSocket );
	//linzhao
	if (0==iRet)
	{
		DebugOutput("%s--%d, sslClose=0\n", __FILE__, __LINE__);
		iSSLSocket = -1;
		s_iSocket = -1;//linzhao
		return iRet;
	}
	else if ( iRet < 0 )
	{
		return iRet;
	}

}

static int SSLSysTime(SYSTEM_TIME_T *date)
{
#define BCD2I(v) ((((v)>>4)&0xf)*10+((v)&0xf))
	unsigned char t[7];
	GetTime(t);
	date->year = 2000+BCD2I(t[0]);
	date->month = BCD2I(t[1]);
	date->day = BCD2I(t[2]);
	date->hour = BCD2I(t[3]);
	date->min = BCD2I(t[4]);
	date->sec = BCD2I(t[5]);
	date->zone.hour = 0;
	date->zone.min = 0;
	return 0;
}

static void GetRandom(unsigned char *pucDataOut)
{
	int i;
	for(i=0; i<8; i++)
		pucDataOut[i] = i;
}

static int SSLRandom(unsigned char *buf, int len)
{
	unsigned char value[8];
	int i;
	while(len>0)
	{
		GetRandom(value);
		for(i=0; i<8&&len>0; i++,len--, buf++)
		{
			*buf = value[i];
		}
	}
	return 0;
}


static int SSLNetOpen(char *remote_addr, short remote_port, short local_port, long flag)
{
	int iErrMsg, iRet; 
	NET_SOCKADDR stSocketAddr;

	if(s_iSocket>= 0)
	{
		DebugOutput("%s--%d, s_iSocket:%d\n", __FILE__, __LINE__, s_iSocket);
		return s_iSocket;
	}
	

	s_iSocket = NetSocket(NET_AF_INET, NET_SOCK_STREAM, 0);
	DebugOutput("%s--%d--%s, s_iSocket:%d\n", __FILE__, __LINE__, __FUNCTION__, s_iSocket);//linzhao
	if (s_iSocket < 0)
	{
		return s_iSocket;	
	}
	SockAddrSet(&stSocketAddr, remote_addr, remote_port);
	iErrMsg = NetConnect(s_iSocket, &stSocketAddr, sizeof(stSocketAddr));	
	DebugOutput("%s--%d--%s, iErrMsg:%d\n", __FILE__, __LINE__, __FUNCTION__, iErrMsg);//linzhao
	if (iErrMsg < 0)
	{
		iRet = NetCloseSocket(s_iSocket);	
		DebugOutput("%s--%d, iRet;%d, s_iSocket:%d\n", __FILE__, __LINE__, iRet, s_iSocket);
		//linzhao
		//if (0==iRet)
		//	s_iSocket = -1;
		switch (iErrMsg)
		{
		case NET_ERR_TIMEOUT:
			return -ERR_SSL_TIMEOUT;
		case NET_ERR_RST:
			return -ERR_SSL_NET;
		default:
			return iErrMsg;
		}
	}
	return s_iSocket;
}

int SSLNetSend(int net_hd, void *buf, int size)
{
	int iRet;
	iRet  = NetSend(net_hd, buf, size, 0);
	DebugOutput("%s--%d--%s,net_hd:%d, iRet:%d\n buf:%s", 
				__FILE__, __LINE__, __FUNCTION__, net_hd, iRet, buf);//linzhao
	if (iRet < 0)
	{
		switch (iRet)
		{
		case NET_ERR_TIMEOUT:
			return -ERR_SSL_TIMEOUT;
		default:
			return iRet;
			
		}
	}
	
	return iRet;
}

static int SSLNetRecv(int net_hd, void *buf, int size)
{
	int iRet;
	iRet = NetRecv(net_hd, buf, size, 0);
//	DebugOutput("%s--%d--%s,net_hd:%d, iRet:%d\n buf:%s", 
//				__FILE__, __LINE__, __FUNCTION__, net_hd, iRet, buf);//linzhao
	if (iRet < 0)
	{
		switch (iRet)
		{
		case NET_ERR_TIMEOUT:
			return -ERR_SSL_TIMEOUT;
		default:
			return iRet;
		}
	}
	
	return iRet;
}

static int SSLNetClose(int net_hd)
{
	int iRet;
	iRet = NetCloseSocket(net_hd);
	DebugOutput("%s--%d--%s, iRet:%d\n", __FILE__, __LINE__, __FUNCTION__,  iRet);//linzhao
	
	return 0;
}

static int OurServCertAck(CERT_INVAL_CODE reason)
{
	//ScrPrint(0, 6, 1, "CertAck err=%d\n",reason);
	return 0;
}

static int ReadPemFile(uchar *pszAppFileName, uchar *psCA, uchar *psCert, uchar *psPrivatekey)
{
	int		fd, iRet, iTempLen;
	uchar	*ptr1, *ptr2 ;
	uchar ucBuf[MAX_PEM_ELN+1]; 
	uchar pos_ca_buf[2048];
	uchar pos_cer_buf[2048];
	uchar pos_privatekey_buf[2048];

	if (pszAppFileName == NULL)
	{
		return -1;
	}
	//打开文件
	fd = open(pszAppFileName, O_RDWR);
	if (fd < 0)
	{	
		return fd;
	}
	
	//读取文件
	memset(ucBuf, 0, sizeof(ucBuf));
	iRet = read(fd, ucBuf, MAX_PEM_ELN);
	if (iRet < 0)
	{
		ScrClrLine(6,9);
		ScrPrint(0, 6, 1, "READ FILE ERR:%d", iRet);
		PubWaitKey(30);
		close(fd);
		return iRet;	
	}	
	close(fd);

	//Get CERTIFICATE
	if ((ptr1 = strstr(ucBuf, "-----BEGIN CERTIFICATE-----")) && 
		(ptr2 = strstr(ucBuf, "-----END CERTIFICATE-----")) )
	{
		memset(pos_cer_buf, 0, sizeof(pos_cer_buf));
		iTempLen = strlen(ptr1) - strlen(ptr2) - strlen("-----BEGIN CERTIFICATE-----\n");
		memcpy(pos_cer_buf, ptr1+strlen("-----BEGIN CERTIFICATE-----\n"), iTempLen);
	}
	else
	{
		ScrClrLine(6,9);
		ScrPrint(0, 6, 1, "UNFOUND CERT");
		PubWaitKey(30);

		return -2;	
	}	

  	memset(pos_privatekey_buf, 0, sizeof(pos_privatekey_buf));
  	memset(pos_ca_buf, 0, sizeof(pos_ca_buf));

	//out data
	memcpy(psCA, pos_ca_buf, strlen(pos_ca_buf));
	memcpy(psCert, pos_cer_buf, strlen(pos_cer_buf));
	memcpy(psPrivatekey, pos_privatekey_buf, strlen(pos_privatekey_buf));

	return 0;
}

int SSL_PROC(void)
{
	int iRet;
	ST_WIFI_AP WifiAp;

	iRet = WifiCheck(&WifiAp);
	if (iRet<=0)
	{
		iRet = WifiApLogin();
		if (iRet<0)
			return iRet;
	}

	iRet = Ssl_Connect(glSysCtrl.szRemoteIP, (short)atoi(glSysCtrl.szRemotePort), COMM_TIMEOUT);
	DebugOutput("%s--%d--%s, iRet=%d\n", __FILE__, __LINE__, __FUNCTION__, iRet);

	return iRet;
}



