#ifndef _UTIL_H
#define _UTIL_H

#include "Wifi.h"

#define COMM_TIMEOUT		30
#define USER_OPER_TIMEOUT	30

//#define _TERMINAL_SXX_
#define _TERMINAL_DXX_

//#define DEBUG_USE

#define WIFI_MODE	0x01
#define GPRS_MODE	0x02
#define LAN_MODE	0x04
#define MODEM_MODE	0x08
#define BT_MODE		0x10

#define TCPIP_USE	0
#define GPRS_USE	1
#define WIFI_USE  	2
#define SSL_USE		3

/*****++add by wuc 2013.12.17*****/
#define SLOT_1	0
#define SLOT_2	1

#define APN_PERFIL	0
#define APN_MANUAL	1

#define APN_VIVO	0
#define APN_CLARO	1
#define APN_TIM		2
#define APN_OI		3
#define APN_CTBC	4

#define MENU_TERM	0
#define MENU_COMM	1
#define MENU_GZIP	2
#define MENU_VTEF	3
#define MENU_PRINT	4
#define MENU_DEBUG	5
#define MENU_UNLOCK	6
#define MENU_ACTIVE		7
#define MENU_CLEAR	8
#define MENU_BT		9
#define MENU_SLOT	10
#define MENU_SSL	11//linzhao

#define ELE_NAME_MAX	20
#define XML_DOC_MAX	100
#define XML_INFO_MAX	1024

#define FORMAT_NUM		1	//number
#define FORMAT_AMT		2	//amount
#define FORMAT_STR		3	//string
#define FORMAT_DATE	4	//date
#define FORMAT_STR_SPECIAL 5 //string and special char
#define FORMAT_STR_UPPER_LEFT 6 //string and special char
#define FORMAT_PW		7	//password

// 刷卡/插卡事件
#define CARD_TIMEOUT		0x00	// 超时
#define CARD_SWIPED		0x01	// 刷卡
#define CARD_INSERTED		0x02	// 插卡
#define CARD_KEYIN			0x04	// 手输卡号

#define SKIP_DETECT_ICC		0x20	// 不检查ICC插卡事件 // 防止用户拔卡过快[12/12/2006 Tommy]
#define FALLBACK_SWIPE		0x40	// 刷卡(FALLBACK)
#define SKIP_CHECK_ICC		0x80	// 不检查service code

// Card slot number for customer.
#define ICC_USER		0

#ifdef _TERMINAL_DXX_
#define SCREEN_WIDTH		320
#define SCREEN_HEIGHT		216

#define ROW_9_LINE			9
#define ROW_8_LINE			8
#define ROW_9_HEIGHT		(SCREEN_HEIGHT / ROW_9_LINE)
#define ROW_8_HEIGHT		(SCREEN_HEIGHT / ROW_8_LINE)
#endif

enum
{
	EVENT_NO_KEY,
	EVENT_NUM_KEY,
	EVENT_FUNC_KEY,
	EVENT_WIFI_SHOTCUT, //add by wuc 2014.9.9
	EVENT_TERM_INFO,
	EVENT_SWIPED,
	EVENT_INSERTED
} GetCardEvent;

#define GRAY_COLOR	RGB(144,144,144)
/*****--add by wuc 2013.12.17*****/

#ifdef _TERMINAL_DXX_
#define ScrClrLine		DxxScrClrLine
#define ScrPrint			DxxScrPrint
#define ScrClrBelow(a)	DxxScrClrLine(a, 8)
#define ScrDrawLine		DxxScrDrawLine
#else
#define ScrClrBelow(a)	ScrClrLine(a, 7)
#endif

#define STR_VERSION "POS 2.0 de 20/03/2013"

typedef int (*CallbackFunc)(void*,...);
typedef struct _tagFuncMag{
    	uchar      szFuncName[32];		// func id
   	CallbackFunc pfFunc;	// callback function(if set)
   	uchar ucInputFlag;
   	uchar ucOutputFlag;
}FuncMag;

typedef struct _tagPPComp{
	uchar szFuncID[128];
	uchar szInput[3][512];
	uint uiRespCode;
	uchar szOutput[2][512];
}PP_COMP;

// 通讯数据
#define	LEN_MAX_COMM_DATA	100000
#define	LEN_HEADER_DATA		1024
#define	LEN_ATTR_DATA			256
#define	LEN_BODY_DATA			95000
typedef struct _tagCOMM_DATA
{
	ulong		ulLength;
	uchar		sContent[LEN_MAX_COMM_DATA+1];
}COMM_DATA;

typedef struct _tagMSG_DATA
{
	uchar szHeader[LEN_HEADER_DATA+1];
	uchar szBody[LEN_BODY_DATA+32];
	uchar szPost[LEN_BODY_DATA+1];
}MSG_DATA;

// GPRS/CDMA configurations
typedef struct _tagWIRELESS_PARAM
{
	uchar		szAPN[64+1];  ///拨号号码 CDMA: #777; GPRS: cmnet
	uchar		szUID[64+1];
	uchar		szPwd[16+1];
	uchar		szSimPin[16+1];     // SIM card PIN
}WIRELESS_PARAM;

typedef struct _tagTCPIP_PARAM
{
	uchar ucDhcp;
}TCPIP_PARAM;

typedef struct _tagSYSTEM_CTRL
{
	uchar szDestino[16];
	uchar szDeDadosDestino[16];
	uchar szProcessorede[16];
	uchar szDeDadosrede[16];
	uchar szMerchantNum[2+1];
	uchar szStoreNum[8+1];
	uchar szPosNum[3+1];
	ulong ulSeq;
	uchar szRemoteIP[15+1];
	uchar szRemotePort[5+1];
	uchar szRemoteIPnoSSL[15+1];
	uchar szRemoteIPwithSSL[15+1];
	uchar bDebugOpt;
	uchar bGzipOpt;
	uchar bVTEFOpt;
	uchar ucWepMode;
	uchar szLocalIp[20+1];
	uchar szGateIp[20+1];
	uchar szMaskIp[20+1];
	uchar szDnsIp[20+1];
	uchar szLocalPort[5+1];
	APP_WIFI_PARA stAppWifiPara;
/*****++modified by wuc 2013.12.17*****/
	uchar ucSlot;
	uchar ucSlotNum;
	WIRELESS_PARAM stWirelessPara1;
	WIRELESS_PARAM stWirelessPara2;
	uchar bUnlock; //0 - lock, 1 - unlock
	uchar bEnableMsg; //0 - disable, 1 - enable
	uchar bPPOnlineClient; //0 - disable, 1 - enable
	uchar bPPOnlineAuto; //0 - disable, 1 - enable
	uchar szTechPsw[8+1];
	uchar szCPFCNPJ[14+1];
	uchar szLogNum[8+1]; //linzhao
	uchar bTelablocante; //linzhao
	uchar bSSL;//linzhao
	uchar ucFirstRun;	//0-not the first run, 1-first run, 2-first run to confirm, linzhao 2015.1.26
	uchar bActivate;	//0 - disable, 1 - enable . linzhao 2015.1.27
/*****--modified by wuc 2013.12.17*****/
	TCPIP_PARAM	stTcpipPara;
	uchar ucCommType;
}SYSTEM_CTRL;
	
typedef struct _tagSYS_PROC_INFO
{
	uchar ucTranType;
#define SWIPE_MODE		0
#define MANUAL_MODE	1
#define INSERT_MODE		2
	uchar ucEntryMode;
	uchar szTranName[64];
	uchar szService[64];
	uchar szAmount[12+1];
	uchar szHeader[3];
	uchar szIdent[3];
}SYS_PROC_INFO;

typedef struct _TAGINTPOSIFNO
{
	uchar szHeader[3+1];
	uchar szIdentification[10+1];
	uchar szFiscalDoc[12+1];
	uchar szTotalValue[12+1];
	uchar szCoin[1+1];
	uchar szCMC7[70+1];
	uchar szPersonType[1+1];
	uchar szPersonDoc[16+1];
	uchar szNetworkName[12+1];
	uchar szTranStatus[3+1];
	uchar szTranType[2+1];
	uchar szTranNum[12+1];
	uchar szTranAuthCode[6+1];
	uchar szTranLotNum[10+1];
	uchar szHostTimeStamp[10+1];
	uchar szLocalTimeStamp[10+1];
	uchar ucInstallmentType;
	uchar szInstallmentNum[2+1];
	uchar szInstallmentExp[8+1];
	uchar szInstallmentValue[12+1];
	uchar szInstallmentNSU[12+1];
	uchar szReceiptDate[8+1];
	uchar szReceiptHour[6+1];
	uchar szPredate[8+1];
	uchar szCancelTranNum[12+1];
	uchar szCancelTranDate[10+1];
	uchar szFinish[30+1];
	uchar szReceiptLine[3+1];
	uchar szReceiptData[40+1];
	uchar szOperatorText[40+1];
	uchar szClientText[40+1];
	uchar szAuth[20+1];
	uchar szTrailer[1+1];
	uchar szCNPJ[14+1];//linzhao
}ST_INTPOS_INFO;


#define PRN_CANCEL		0xA1

extern COMM_DATA glSendData, glRecvData;
extern MSG_DATA glMsgData;
extern SYS_PROC_INFO glProcInfo;
extern SYSTEM_CTRL glSysCtrl;
extern PP_COMP glPPComp[32];
extern int glPPCompIdx;
extern ulong glSeqNo;
extern uchar glszMessageType[32];
extern uchar glszDeDadosDestino[16];
extern uchar glszProcessorede[16];
extern uchar glszDeDadosrede[16];
extern ST_INTPOS_INFO gstIntpos;

extern uchar glCommMode;
extern uchar glConnectFlag;
extern uchar glStartServer;
extern uchar glAcceptClient;
extern uchar glActivate; //add by wuc 2014.10.11

void InitPPComp(void);

void PPDispStartWait(void);

//int PPGetCard(uchar *szInput1, uchar *szInput2, uchar *szOutput);
int PPGetCard(uchar *szInput1, uchar *szOutput);
int PPGetPIN(uchar *szInput, uchar *szOutput);
int PPGoOnChip(uchar *szInput1, uchar *szInput2, uchar *szInput3, uchar *szOutput);
int PPRemoveCard(uchar *szInput);
int PPGetKey();
void PPDispErrMsg(uint iErrNo);
int PPLoadInit(uchar *szTimestamp);
int PPLoadAID(uchar *szAID);
int PPLoadKey(uchar *szKey);
int PPInitPage(uchar *szInitPage);
int PPShowErrorMsg(uchar *szInput00, uchar *szInput01);
void DxxScrPrint(uchar col,uchar row,uchar mode, char *str,...);
void DxxScrClrLine(uchar startline, uchar endline);
void DxxScrDrawLine(uchar line, COLORREF LineColor); //add by wuc 2014.4.1
void SetScreenLine(uchar ucLine); //add by wuc 2014.4.1
void AppendIntPos(uchar *szSerial, uchar *szValue, uchar *szOut);
void SearchIntPos(uchar *szSerial, uchar *szValue, uchar *szIntPos);

int StartPrinter(void);
int FuncPPExe(uchar *szFuncName, FuncMag *pstFunc, char *szInput00, char *szInput01, char *szInput02, char *szOutput01, char *szOutput02);


void DebugOutput(char *pszFmtStr, ...);
void DispPrnError(uchar ucErrCode);

int CommConnect(void);
int CommSendMsg(uchar *szMsg, ushort usLen, ushort usTimeout);
int CommRecvMsg(uchar *szMsg, ulong ulExpLen, ulong *RxLen, ushort usTimeout);
void CommDisconnect(void);
int Base64Decode(unsigned char *szInput, unsigned char *szOutput, unsigned int uiInputLen);
int PPGenericCmd(uchar *pszInput, uchar *pszOutput);
#endif
