#include "posapi.h"
#include "appLib.h"
#include "util.h"
#include "lng.h"
#include "ppcomp.h"
#include "fileoper.h"
#include "Vtef.h"
#include "util.h"
#include "TranProc.h"
#include "xmlParser.h"
#include "global.h"

#ifdef _TERMINAL_DXX_
#define TCPMAXSENDLEN		10240 

#define INTPOS_MOB_STS 0
#define INTPOS_MOB_001 1

uchar g_bAbort = 0;

int VTEF_Rxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, uint *puiOutLen)
{
	int ret;

#ifdef DEBUG_USE
	return 0;
#endif
	
	ret = NetRecv(accept_socket_id, psRxdData,  uiExpLen, 0);
	if(ret > 0)
	{
		*puiOutLen = ret;
		return 0;    
	}
	else
	{
		if(ret == 0) ret = 0xff;
		return ret;   
	}
}

int VTEF_Txd(uchar *szData, uint uiDataLen, ushort uiTimeOutSec)
{
	int iRet;
	int iSendLen;
	int iSumLen;

#ifdef DEBUG_USE
	DelayMs(500);
	return 0;
#endif

	//Netioctl(accept_socket_id, CMD_TO_SET, uiTimeOutSec*1000);   // 系统默认为2秒
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
		iRet = NetSend(accept_socket_id, szData+iSumLen, iSendLen, 0);
		if (iRet < 0)
		{
		/*	DxxScrClrLine(4, 18);
			switch (iRet)
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
			default:
				sprintf(sBuff,"ret_send=%d",iRet);
				break;
			}

			DxxScrClrLine(4,18);
			PubDispString(sBuff,4|DISP_LINE_LEFT);
			PubWaitKey(5);
*/			
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

int VTEF_PackData(void)
{
	int fd;
	uchar szData[1024];

	memset(szData, 0, 1024);

	if(fexist(FILE_MOBILE_STS) >= 0)
	{
		remove(FILE_MOBILE_STS);
	}
	
	fd = open(FILE_MOBILE_STS, O_CREATE|O_RDWR);
	if(fd < 0)
	{
		return 1;
	}
	
	AppendIntPos("000-000", gstIntpos.szHeader, szData);
	AppendIntPos("001-000", gstIntpos.szIdentification, szData);
	AppendIntPos("032-000", glSysCtrl.szCPFCNPJ, szData);//linzhao
	AppendIntPos("999-999", gstIntpos.szTrailer, szData);
	seek(fd, 0, SEEK_SET);
	write(fd, szData, strlen(szData));
	close(fd);
	return 0;
}

int VTEF_UnpackData(void)
{
	int fd;
	uchar szData[1024];
	uint uiLen;

	if(fexist(FILE_MOBILE_FILE) < 0)
	{
		return 1;
	}
	
	uiLen = filesize(FILE_MOBILE_FILE);
	
	fd = open(FILE_MOBILE_FILE, O_RDWR);
	if(fd < 0)
	{
		return 1;
	}

	seek(fd, 0, SEEK_SET);
	read(fd, szData, uiLen);
	close(fd);

	memset(&gstIntpos, 0x00, sizeof(ST_INTPOS_INFO));
	memset(gstIntpos.szHeader, 0, 4);
	SearchIntPos("000-000", gstIntpos.szHeader, szData);
	memset(gstIntpos.szIdentification, 0, 11);
	SearchIntPos("001-000", gstIntpos.szIdentification, szData);
	memset(gstIntpos.szFiscalDoc, 0, 13);
	SearchIntPos("002-000", gstIntpos.szFiscalDoc, szData);
	memset(gstIntpos.szTotalValue, 0, 13);
	SearchIntPos("003-000", gstIntpos.szTotalValue, szData);
	memset(gstIntpos.szCoin, 0, 2);
	SearchIntPos("004-000", gstIntpos.szCoin, szData);
	memset(gstIntpos.szCMC7, 0, 71);
	SearchIntPos("005-000", gstIntpos.szCMC7, szData);
	memset(gstIntpos.szPersonType, 0, 2);
	SearchIntPos("006-000", gstIntpos.szPersonType, szData);
	memset(gstIntpos.szPersonDoc, 0, 17);
	SearchIntPos("007-000", gstIntpos.szPersonDoc, szData);
	memset(gstIntpos.szNetworkName, 0, 13);
	SearchIntPos("010-000", gstIntpos.szNetworkName, szData);
	memset(gstIntpos.szTranNum, 0, 13);
	SearchIntPos("012-000", gstIntpos.szTranNum, szData);
	memset(gstIntpos.szReceiptDate, 0, 9);
	SearchIntPos("022-000", gstIntpos.szReceiptDate, szData);
	memset(gstIntpos.szReceiptHour, 0, 7);
	SearchIntPos("023-000", gstIntpos.szReceiptHour, szData);
	memset(gstIntpos.szFinish, 0, 31);
	SearchIntPos("027-000", gstIntpos.szFinish, szData);
	memset(gstIntpos.szCNPJ, 0, sizeof(gstIntpos.szCNPJ));//linzhao
	SearchIntPos("032-000", gstIntpos.szCNPJ, szData);
	memset(gstIntpos.szTrailer, 0, 2);
	SearchIntPos("999-999", gstIntpos.szTrailer, szData);
	
	return 0;
}

int Mobile2POS(void)
{
	int iRet;
	uchar szBuff[1024+1];
	uint uiLen = 0;

	memset(szBuff, 0, 1024+1);
	iRet = VTEF_Rxd(szBuff, 100, 0, &uiLen);

	if(strlen(szBuff) > 0)
	{
		if(glSysCtrl.bDebugOpt)
		{
			DebugOutput("READ DATA = [%s]", szBuff);
		}

		WriteMob001(szBuff, strlen(szBuff));
	}

	return iRet;
}


int ModifyMob001_CRT(void)
{
	int ifd, iLen;
	char szBuf[1024*10], szBufTemp[1024*10];
	char *pBegin, *pEnd;

	if (fexist(FILE_MOBILE_FILE)<0)
		return -1;

	iLen = filesize(FILE_MOBILE_FILE);
	ifd = open(FILE_MOBILE_FILE, O_RDWR);
	memset(szBuf, 0, sizeof(szBuf));
	seek(ifd, 0, SEEK_SET);
	read(ifd,szBuf, iLen);

	pBegin = strstr(szBuf, "001-000 =");
	if (NULL==pBegin)
	{
		return -1;
	}
	pEnd = strstr(szBuf, "999-999 =");
	if (NULL==pEnd)
	{
		return -1;
	}
	memset(szBufTemp, 0, sizeof(szBufTemp));
	memcpy(szBufTemp, pBegin, pEnd-pBegin);

	memset(szBuf, 0, sizeof(szBuf));
	seek(ifd, 0, SEEK_SET);
	write(ifd, "\0", iLen);

	sprintf(szBuf, "000-000 = CRT\n%s999-999 = 0", szBufTemp);
//	DebugOutput("%s--%d--%s\n szBuf:%s", __FILE__, __LINE__, __FUNCTION__, szBuf);
	seek(ifd, 0, SEEK_SET);
	write(ifd, szBuf, strlen(szBuf));

	close(ifd);

	return 0;
}

int POS2Mobile(uchar ucType)
{
	int fd;
	uchar szBuff[1024*10];
	uint uiLen;

	if(ucType == INTPOS_MOB_STS)
	{
		if(fexist(FILE_MOBILE_STS) < 0)
		{
			return 0;
		}

		uiLen = filesize(FILE_MOBILE_STS);
		fd = open(FILE_MOBILE_STS, O_RDWR);
	}
	else if(ucType == INTPOS_MOB_001)
	{
		if(fexist(FILE_MOBILE_FILE) < 0)
		{
			return 0;
		}
		
		uiLen = filesize(FILE_MOBILE_FILE);
		fd = open(FILE_MOBILE_FILE, O_RDWR);
	}

	memset(szBuff, 0, sizeof(szBuff));
	seek(fd, 0, SEEK_SET);
	read(fd, szBuff, uiLen);
	close(fd);

	if(ucType == INTPOS_MOB_STS)
	{
		remove(FILE_MOBILE_STS);
	}
	if(glSysCtrl.bDebugOpt)
	{
		DebugOutput("SEND DATA = [%s]", szBuff);
	}
	return VTEF_Txd(szBuff, uiLen, 30);
}

void VTEFMain(void)
{
	int iRet = 0;
	static int s_iCrtCnt = 0;

	if(Mobile2POS() == 0)
	{
		VTEF_UnpackData();
		//Cielo: if pos received "CRT" continuously,pos must send comfirm to server then proccess the "CRT"
		//linzhao 2015.2.5
		if ( 0==strncmp(gstIntpos.szHeader, "CRT", 3) && 0!=s_iCrtCnt )
		{
			Trans_Confirm();
		}
		else
		{
			s_iCrtCnt = 0;
		}

		if(strncmp(gstIntpos.szHeader, "ATV", 3) == 0)
		{
			if(fexist(FILE_ATV_FILE) < 0)
			{
				iRet = Trans_Active();
			}

			if(iRet == 0)
			{
				VTEF_PackData();
				POS2Mobile(INTPOS_MOB_STS);
			}
		}
		else if(strncmp(gstIntpos.szHeader, "ADM", 3) == 0 ||
			    	strncmp(gstIntpos.szHeader, "CRT", 3) == 0 ||
			    	strncmp(gstIntpos.szHeader, "CNC", 3) == 0)
		{
			VTEF_PackData();
			POS2Mobile(INTPOS_MOB_STS);
			if(strncmp(gstIntpos.szHeader, "ADM", 3) == 0)
			{
				iRet = Trans_Admin();
			}
			else if(strncmp(gstIntpos.szHeader, "CRT", 3) == 0)
			{
				iRet = Trans_Payment();
				ModifyMob001_CRT();//Cielo need to modify the format.linzhao 2015.2.5
				s_iCrtCnt++;
			}
			else if(strncmp(gstIntpos.szHeader, "CNC", 3) == 0)
			{
				iRet = Trans_Void();
			}
			
			if(0==iRet && 0==g_bAbort)//linzhao
			{
				POS2Mobile(INTPOS_MOB_001);
				remove(FILE_MOBILE_FILE);
			}
		}
		else if(strncmp(gstIntpos.szHeader, "CNF", 3) == 0)
		{
			Trans_Confirm();
		}
		else if(strncmp(gstIntpos.szHeader, "NCN", 3) == 0)
		{
			Trans_NotConfirm();
		}
/*		else
		{
			VTEF_PackData();
			POS2Mobile(INTPOS_MOB_STS);
		}
*/ //linzhao

		g_bAbort = 0;//linzhao
		ScrCls();
		DispPictureBox("Cielologo.jpg", 1, 0, 0);
		PubDispString(_T("WELCOME USE POS2.0"), DISP_LINE_CENTER|3);
		PubDispString(_T("WAITING..."), DISP_LINE_CENTER|4);
	}
}

#endif
