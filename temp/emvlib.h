/*****************************************************/
/* Emvlib.h                                          */
/* Define the Application Program Interface          */
/* of EMV2 for all PAX POS terminals     		     */
/* Created by ZengYun at 20/01/2005                  */
/*****************************************************/
 

#ifndef _EMV_LIB_H  
#define _EMV_LIB_H 

#define EMV_SCRIPT_PROC_UNIONPAY 1
#define EMV_SCRIPT_PROC_NORMAL 0

#define CLSS_SLOT      0xff
#define CONTACT_SLOT   0x00

//不能通过简单的删除对应的公钥数据来操作回收证书数据
// 否则会令使用该公钥的其他发卡行公钥证书无法使用
#define  MAX_REVOCLIST_NUM    30      // EMVCO要求每个RID必须支持30个回收列表

#define MAX_APP_NUM       100         //EMV库应用列表最多可存储的应用数
#define MAX_APP_ITEMS     17         //由16修改为17

#define MAX_KEY_NUM       64         //monitor平台下EMV库认证中心公钥表最多可存储的公钥数


#define PART_MATCH        0x00       //应用选择匹配标志(部分匹配)
#define FULL_MATCH        0x01       //应用选择匹配标志(完全匹配)

#define EMV_CASH          0x01       //交易类型(现金)
#define EMV_GOODS	      0x02       //交易类型(购物)
#define EMV_SERVICE       0x04       //交易类型(服务)
#define EMV_CASHBACK      0x08       //交易类型(反现)

#define EMV_INQUIRY		  0x10		 //交易类型(查询)
#define EMV_TRANSFER	  0x20		 //交易类型(转账)
#define EMV_PAYMENT		  0x40		 //交易类型(支付)
#define EMV_ADMIN		  0x80		 //交易类型(管理)
#define EMV_CASHDEPOSIT   0x90		 //交易类型(存款)


//交易处理函数返回码定义
#define EMV_OK             0         //成功

#define ICC_RESET_ERR     -1         //IC卡复位失败
#define ICC_CMD_ERR       -2         //IC命令失败
#define ICC_BLOCK         -3         //IC卡锁卡    
   
#define EMV_RSP_ERR       -4         //IC返回码错误
#define EMV_APP_BLOCK     -5         //应用已锁
#define EMV_NO_APP        -6         //卡片里没有EMV应用
#define EMV_USER_CANCEL   -7         //用户取消当前操作或交易
#define EMV_TIME_OUT      -8         //用户操作超时
#define EMV_DATA_ERR      -9         //卡片数据错误
#define EMV_NOT_ACCEPT    -10        //交易不接受
#define EMV_DENIAL        -11        //交易被拒绝
#define EMV_KEY_EXP       -12        //密钥过期

//回调函数或其他函数返回码定义
#define EMV_NO_PINPAD     -13        //没有密码键盘或键盘不可用
#define EMV_NO_PASSWORD   -14        //没有密码或用户忽略了密码输入
#define EMV_SUM_ERR       -15        //认证中心密钥校验和错误
#define EMV_NOT_FOUND     -16        //没有找到指定的数据或元素
#define EMV_NO_DATA       -17        //指定的数据元素没有数据
#define EMV_OVERFLOW      -18        //内存溢出

//读交易日志
#define NO_TRANS_LOG      -19
#define RECORD_NOTEXIST   -20
#define LOGITEM_NOTEXIST  -21

#define ICC_RSP_6985      -22        // GAC中卡片回送6985, 由应用决定是否fallback
#define EMV_FILE_ERR      -24
#define EMV_PARAM_ERR     -30

//因PED接口更新而增加返回值定义
#define EMV_PED_TIMEOUT   0x01   // PED接口返回 PED_RET_ERR_INPUT_TIMEOUT
#define EMV_PED_WAIT      0x02   // PED接口返回 PED_RET_ERR_WAIT_INTERVAL
#define EMV_PED_FAIL      0x03   // PED接口返回其他错误


#define REFER_APPROVE     0x01       //参考返回码(选择批准)
#define REFER_DENIAL      0x02       //参考返回码(选择拒绝)

#define ONLINE_APPROVE    0x00       //联机返回码(联机批准)		
#define ONLINE_FAILED     0x01       //联机返回码(联机失败)	
#define ONLINE_REFER      0x02       //联机返回码(联机参考)
#define ONLINE_DENIAL     0x03       //联机返回码(联机拒绝)
#define ONLINE_ABORT      0x04       //兼容PBOC(交易终止)


#define AC_AAC       0x00
#define AC_TC        0x01
#define AC_ARQC      0x02
#define AC_AAC_HOST  0x03 //used for VisaOnLine


#ifdef WIN32
#pragma warning(disable:4103)
#pragma pack(1)//设定为1字节对齐
// #else
// 	__attribute__((__packed__))
#endif

typedef struct{
	unsigned char AppName[33];       //本地应用名，以'\x00'结尾的字符串
	unsigned char AID[17];           //应用标志
	unsigned char AidLen;            //AID的长度
    unsigned char SelFlag;           //选择标志(PART_MATCH 部分匹配  FULL_MATCH 全匹配)      
	unsigned char Priority;          //优先级标志
	unsigned char TargetPer;         //目标百分比数
	unsigned char MaxTargetPer;      //最大目标百分比数
 	unsigned char FloorLimitCheck;   //是否检查最低限额
	unsigned char RandTransSel;      //是否进行随机交易选择
	unsigned char VelocityCheck;     //是否进行频度检测
    unsigned long FloorLimit;        //最低限额
	unsigned long Threshold;         //阀值
 	unsigned char TACDenial[6];      //终端行为代码(拒绝)
	unsigned char TACOnline[6];      //终端行为代码(联机)
	unsigned char TACDefault[6];     //终端行为代码(缺省)
    unsigned char AcquierId[6];      //收单行标志
	unsigned char dDOL[256];         //终端缺省DDOL
	unsigned char tDOL[256];         //终端缺省TDOL
	unsigned char Version[3];        //应用版本
	unsigned char RiskManData[10];   //风险管理数据
}EMV_APPLIST;

typedef struct{
	unsigned char aucAppPreName[17];  //应用优先选择名称，以'\0'结尾
	unsigned char aucAppLabel[17];  //应用标签，以'\0'结尾
	unsigned char aucIssDiscrData[244]; //tag 'BF0C'数据：1个字节的长度字节+'BF0C'最大222个字节，以'\0'结尾 
	                                    //tag '73'数据：1个字节的长度字节+'73'最大242个字节，以'\0'结尾
	unsigned char aucAID[17]; //卡片AID，以'\0'结尾
	unsigned char ucAidLen; //卡片AID长度
}APPLABEL_LIST;

typedef struct {
	unsigned char RID[5];            //应用注册服务商ID
	unsigned char KeyID;             //密钥索引
	unsigned char HashInd;           //HASH算法标志
	unsigned char ArithInd;          //RSA算法标志
	unsigned char ModulLen;          //模长度
	unsigned char Modul[248];        //模
	unsigned char ExponentLen;       //指数长度
	unsigned char Exponent[3];       //指数
	unsigned char ExpDate[3];        //有效期(YYMMDD)
	unsigned char CheckSum[20];      //密钥校验和
}EMV_CAPK;

typedef struct{
	unsigned char MerchName[256];    //商户名
	unsigned char MerchCateCode[2];  //商户类别码(没要求可不设置)
	unsigned char MerchId[15];       //商户标志(商户号)
	unsigned char TermId[8];         //终端标志(POS号)
	unsigned char TerminalType;      //终端类型
	unsigned char Capability[3];     //终端性能
	unsigned char ExCapability[5];   //终端扩展性能
	unsigned char TransCurrExp;      //交易货币代码指数
	unsigned char ReferCurrExp;      //参考货币指数
	unsigned char ReferCurrCode[2];  //参考货币代码
	unsigned char CountryCode[2];    //终端国家代码
	unsigned char TransCurrCode[2];  //交易货币代码
	unsigned long ReferCurrCon;      //参考货币代码和交易代码的转换系数(交易货币对参考货币的汇率*1000)
	unsigned char TransType;         //当前交易类型(EMV_CASH EMV_GOODS EMV_SERVICE EMV_GOODS&EMV_CASHBACK EMV_SERVICE&EMV_CASHBACK)
	unsigned char ForceOnline;       //商户强制联机(1 表示总是联机交易)
	unsigned char GetDataPIN;        //密码检测前是否读重试次数
	unsigned char SurportPSESel;     //是否支持PSE选择方式
}EMV_PARAM;

// 不能通过简单的删除对应的公钥数据来操作回收证书数据
// 否则会令使用该公钥的其他发卡行公钥证书无法使用 
typedef  struct 
{
	unsigned char   ucRid[5];  		        // RID
	unsigned char   ucIndex;   		        // CA公钥索引
	unsigned char   ucCertSn[3];            // 发卡行证书序列号
}EMV_REVOCLIST;


typedef struct  
{
	unsigned char ucUseTermAIPFlg;   //是否基于卡片AIP进行风险管理,0-用卡片AIP,1-用终端AIP,默认为0
	unsigned char aucTermAIP[2];    //终端是否强制进行风险管理，byte1-bit4为1：强制进行风险管理；byte1-bit4为0：不进行风险管理。默认两个字节全为0。
	unsigned char ucBypassAllFlg;         //whether bypass all other pin when one pin has been bypassed 1-Yes, 0-No
}EMV_EXTMPARAM;

// MCK 配置参数结构
typedef struct 
{
	unsigned char  ucBypassPin; //0-不支持，1－支持，默认支持
	unsigned char  ucBatchCapture;  // 0-online data capture, 1-batch capture
	void *pvoid;
}EMV_MCKPARAM;

// for PBOC 小额支付
// 设置终端电子现金相关参数
typedef struct 
{
	unsigned char ucECTSIFlg;   // TSI存在? 1-存在 电子现金终端支持指示器 （EC Terminal Support Indicator）
	unsigned char ucECTSIVal;   // 电子现金终端支持指示器 = 1,支持
	unsigned char ucECTTLFlg;   // TTL存在? 1-存在 电子现金终端交易限额（EC Terminal Transaction Limit）
	unsigned long ulECTTLVal;   // 电子现金终端交易限额
}EMV_TMECPARAM;

// add for clss pboc
typedef struct  
{
    unsigned long  ulAmntAuth;     // 授权金额(unsigned long), 若为返现, 则该金额需包括ulAmntOther的金额
    unsigned long  ulAmntOther;    // 其他金额(unsigned long) 
    unsigned long  ulTransNo;      // 交易序列计数器(4 BYTE)
	unsigned char  ucTransType;    // 交易类型'9C', 0-消费/服务 1-现金/返现
	unsigned char  aucTransDate[4]; // 交易日期 YYMMDD
	unsigned char  aucTransTime[4]; // 交易时间 HHMMSS
}Clss_TransParam;

typedef struct{
    int MaxLen;
    unsigned short Tag;
    unsigned short Attr;
	unsigned short usTemplate[2];// 所属的模版，没有则为0
	unsigned char ucSource;// 三个取值EMV_SRC_TM，EMV_SRC_ICC，EMV_SRC_ISS
}ELEMENT_ATTR;


//密钥和应用管理函数定义
int  EMVCoreInit(void);
void EMVGetParameter(EMV_PARAM *tParam);
void EMVSetParameter(EMV_PARAM *tParam);
int  EMVGetTLVData(unsigned short Tag, unsigned char *DataOut, int *OutLen);
int  EMVSetTLVData(unsigned short Tag, unsigned char *DataIn, int DataLen);
int  EMVGetScriptResult(unsigned char *Result, int *RetLen);

//密钥和应用管理函数定义
int  EMVAddApp(EMV_APPLIST *App);
int  EMVGetApp(int Index, EMV_APPLIST *App);
int  EMVDelApp(unsigned char *AID, int AidLen);
int  EMVDelAllApp(void);
int  EMVModFinalAppPara(EMV_APPLIST *ptEMV_APP);
int  EMVGetFinalAppPara(EMV_APPLIST *ptEMV_APP);
int  EMVGetLabelList(APPLABEL_LIST  *ptAppLabel, int *pnAppNum); //add by nt 20091218

int  EMVAddCAPK(EMV_CAPK  *capk );
int  EMVGetCAPK(int Index, EMV_CAPK  *capk );
int  EMVDelCAPK(unsigned char KeyID, unsigned char *RID);
int  EMVCheckCAPK(unsigned char *KeyID, unsigned char *RID);

// 因无法由应用程序绕过证书列表问题而增加接口
int  EMVAddRevocList(EMV_REVOCLIST *pRevocList);
int  EMVDelRevocList(unsigned char ucIndex, unsigned char *pucRID);
void EMVDelAllRevocList(void);

//交易处理函数定义
int  EMVAppSelect(int Slot,unsigned long TransNo);
int  EMVReadAppData(void);
int  EMVCardAuth(void);
int  EMVStartTrans(unsigned long ulAuthAmt, unsigned long ulCashBackAmt, unsigned char *pACType); //for Visa Online
int  EMVCompleteTrans(int nCommuStatus, unsigned char *paucScript, int *pnScriptLen, unsigned char *pACType); //for Visa Online

//读日志时调用
//增加参数 ucFlg, 用于确定是否加入锁定的应用
int  EMVAppSelectForLog(int Slot, unsigned char ucFlg);
int  EMVAddIccTag(ELEMENT_ATTR tEleAttr[], int nAddNum);

//回调函数定义
int  cEMVWaitAppSel(int TryCnt, EMV_APPLIST List[], int AppNum);  
int  cEMVInputAmount(unsigned long *AuthAmt, unsigned long *CashBackAmt);
int  cEMVGetHolderPwd(int TryFlag, int RemainCnt, unsigned char *pin);
int  cEMVReferProc(void);
int  cEMVUnknowTLVData(unsigned short Tag, unsigned char *dat, int len);
void cEMVAdviceProc(void);
void cEMVVerifyPINOK(void);
int  cEMVSetParam(void); //因应用需求，增加返回值处理
// 标准非接PBOC回调，接触式可将此回调函数实现为空函数
unsigned char  cEMVPiccIsoCommand(unsigned char cid,APDU_SEND *ApduSend,APDU_RESP *ApduRecv);

//证件认证(PBOC2.0)
int  cCertVerify(void);
unsigned char cEMVSM2Verify(unsigned char *paucPubkeyIn,unsigned char *paucMsgIn,int nMsglenIn,
							   unsigned char *paucSignIn, int nSignlenIn);
unsigned char cEMVSM3(unsigned char *paucMsgIn, int nMsglenIn,unsigned char *paucResultOut);

//读交易日志(PBOC2.0)

int ReadLogRecord(int RecordNo);
int GetLogItem(unsigned short Tag, unsigned char *TagData, int *TagLen);

//读圈存日志（PBOC3.0）
int EMVReadSingleLoadLog(int nRecordNoIn);
int EMVGetSingleLoadLogItem(unsigned short usTagIn, unsigned char *paucDataOut, int *pnLenOut);

int EMVReadAllLoadLogs(unsigned char *paucLogDataOut, int *pnLenOut);
int EMVGetLogData(unsigned char *paucLogDataOut, int *pnLenOut);

// 获取参数标志,如打印签名标志
int EMVGetParamFlag(unsigned char ucParam, int *pnFlg);

// 读电子现金余额时调用
int EMVGetCardECBalance(unsigned long *plBalance);
int EMVSetTmECParam(EMV_TMECPARAM *tParam);

// 设置MCK配置中的参数，用于将MCK配置设置传给内核
int EMVSetMCKParam(EMV_MCKPARAM *pMCKParam);
int EMVGetMCKParam(EMV_MCKPARAM *pMCKParam);// 获取内核的MCK参数

// 获取EMV 库发布版本号及发布日期信息
int EMVReadVerInfo(char *paucVer);

//增加设置是否支持advice的标志
void EMVSetConfigFlag(int nConfigflag);

void EMVSetScriptProcMethod(uchar ucScriptMethodIn);
// 初始化EMV数据元值（内核设置默认值）
void EMVInitTLVData(void);

// 标准非接PBOC专用接口
// 设置非接PBOC最终选择以及GPO数据到内核中
int EMVSwitchClss(Clss_TransParam *ptTransParam,unsigned char *pucSelData, int nSelLen, uchar *pucGPOData, int nGPOLen);

// 设置大金额到内核
int EMVSetAmount(unsigned char * szAuthAmount, unsigned char * szCashBackAmount);

int EMVGetVerifyICCStatus(unsigned char *pucSWA, unsigned char *pucSWB);
int  EMVCoreVersion(void);

int  EMVProcTrans(void);

//设置(关闭)EMV－CORE的调试状态
void EMVSetDebug(int EnableFlag);
void EMVGetICCStatus(unsigned char *SWA, unsigned char *SWB);
int  cEMVOnlineProc(unsigned char *RspCode, unsigned char *AuthCode, int *AuthCodeLen, unsigned char *IAuthData, int *IAuthDataLen, unsigned char *Script, int *ScriptLen);

// 设置是否使用PCI 脱机PIN操作接口
int EMVSetPCIModeParam(unsigned char ucPciMode, unsigned char *pucExpectPinLen,unsigned long ulTimeoutMs);

// 清除EMV 库中保存的交易日志信息（用于终端风险管理中的最低限额检查功能）
int EMVClearTransLog(void);

#endif
