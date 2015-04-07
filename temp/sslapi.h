/*
 *  SSL API 
 *  Author: jhsun
 *  Date: 2007-6-20
 *  Version: 2.0
 *  History:
 *  07-06-20 v1.0 jhsun create the file 
 *  08-04-29 v2.0 jhsun 
 *     1.增加了对证书的支持
 *     2.增加读取系统时间接口
 *     3.增加随机数接口
    090309 sunJH
    1. SslGetVer接口用于获取版本信息
    2. 取消兼容1.0 version API
091022 sunJH
1. 增加自签名错误码
091221 sunJH
增加SslDecodePem，SslDecodeCertPubkey和SslDecodePrivKey
3个接口；
 */
#ifndef _SSL_API_H_
#define _SSL_API_H_

#ifdef  __cplusplus
extern "C" {
#endif

/*
 * SSL Error code
 */
 
#define ERR_AGAIN     2/* retry again */ 
#define ERR_SSL_BASE  100
#define ERR_SSL_PKT       (3+ERR_SSL_BASE)/* packet is too small or too big*/
#define ERR_SSL_CT        (4+ERR_SSL_BASE)/* the content type is error */
#define ERR_SSL_HT        (5+ERR_SSL_BASE)/* the handshake is error */
#define ERR_SSL_VER       (6+ERR_SSL_BASE)/* the version is error */
#define ERR_SSL_MEM       (7+ERR_SSL_BASE)/* NO MEM */
#define ERR_SSL_NET       (8+ERR_SSL_BASE)/* net error */
#define ERR_SSL_HANDLE    (9+ERR_SSL_BASE)/* the handle does not exist */
#define ERR_SSL_TIMEOUT   (10+ERR_SSL_BASE)/* time out */
#define ERR_SSL_CONNECTED (11+ERR_SSL_BASE)/* connected already */
#define ERR_SSL_CONNECT   (12+ERR_SSL_BASE)/* not connect */
#define ERR_SSL_CLOSED    (13+ERR_SSL_BASE)/* closed */
#define ERR_SSL_MD        (14+ERR_SSL_BASE)/* message digest fail */
#define ERR_SSL_OPS       (15+ERR_SSL_BASE)/* no net operations */
#define ERR_SSL_DEC       (16+ERR_SSL_BASE)/* decrypt fail */
#define ERR_SSL_CERT      (17+ERR_SSL_BASE)/* bad cert */
#define ERR_SSL_MAC       (18+ERR_SSL_BASE)/* mac function not support now! */
#define ERR_SSL_PKEY      (19+ERR_SSL_BASE)/* we only support RSA */
#define ERR_SSL_TIME      (20+ERR_SSL_BASE)/* cert validity */
#define ERR_SSL_CERTREQ   (21+ERR_SSL_BASE)/* cert req */
#define ERR_SSL_CERTNUM   (22+ERR_SSL_BASE)/* Cert Num is too big */
#define ERR_SSL_NOPRIVKEY (23+ERR_SSL_BASE)/* No  Private Key*/
#define ERR_SSL_CRL       (24+ERR_SSL_BASE)/* Cert Revoked List*/      
#define ERR_SSL_SIGN      (25+ERR_SSL_BASE)/* BAD SIGN */  
#define ERR_SSL_ISSUER      (26+ERR_SSL_BASE)/* no cert for the issuer */  
#define ERR_SSL_APP_CANCEL (27+ERR_SSL_BASE) /* 由于无法验证证书导致用户cancel*/
#define ERR_SSL_BADPRIVKEY (28+ERR_SSL_BASE)/* bad private key */
#define ERR_SSL_INITCERT   (29+ERR_SSL_BASE)/* Init CertInfo fail */
#define ERR_SSL_LIBVER     (30+ERR_SSL_BASE)/* 库版本号不一致 */
#define ERR_SSL_SELF_SIGN  (31+ERR_SSL_BASE)/* 禁止自签名证书 */
#define ERR_SSL_ILLEGAL_PARA  (32+ERR_SSL_BASE)/* 接收包的参数非法 */
#define ERR_SSL_NA        (100+ERR_SSL_BASE)/* unknown error */

/*
 * SslCreate:
 *    Create SSL Context
 * RETURN:
 *    if succeed, return ssl handle(>=0);
 *    if fail, return error code(<0)
 * ERRORS:
 *    SslCreate call fail, if
 *    [ERR_SSL_MEM]    Insufficient buffer space is available.The SSL Context
 *                              cannot be created until sufficient resources are freed.
 *    [ERR_SSL_OPS]     Before use ssl, you must call SslSetNetOps to set operations.
 */
int  SslCreate(void);

/*
 * SslConnect:
 *    Connect SSL(HTTPS) Server
 * PARAM:
 *    s     SSL Handle
 *    remote_addr     ssl server ip addr, such as "219.142.89.66"
 *    remote_port     ssl server port(host byte order), default is 443
 *    local_port        local host port; if it is 0, SSL will select a port >1024;
 *      flag                NOT USED now, must set be 0
 * RETURN:
 *    if succeed, return 0;otherwise, return erro code(<0)
 * ERRORS:
 *    SslConnect call fail, if
 *    [ERR_SSL_HANDLER]  ssl handle does not exist
 *    [ERR_SSL_NET]          the network connecting server is error
 *    [ERR_SSL_TIMEOUT]   time out
 *    [ERR_SSL_PKT]          receive illegal packet which is either too small or too big
 *    [ERR_SSL_CT]            receive illegal packet the content type is error
 *    [ERR_SSL_HT]            handshake procotol is error
 *    [ERR_SSL_VER]          SSL Server Version is too old, Only Support SSLv3.0,TLSv1.0
 *    [ERR_SSL_MEM]          Insufficient buffer space is available
 *    [ERR_SSL_CONNECTED] connected already
 */
int SslConnect(int s, char *remote_addr, short remote_port, short local_port, long flag);

/*
 * SslSend
 *    send data to remote host
 * PARAM:
 *    s      SSL Handle
 *    buf   pointer to input data  
 *    size  the size of input data in byte, MUST be <= 4000
 * RETURN:
 *    The call returns the number of characters sent, or <0 if an error occurred
 * ERRORS:
 *    SslSend call fail, if
 *    [ERR_SSL_HANDLER]  ssl handle does not exist
 *    [ERR_SSL_NET]          the network connecting server is error
 *    [ERR_SSL_TIMEOUT]   time out
 *    [ERR_SSL_CONNECT]  before use SslSend, you must call SslConnect
 *    [ERR_SSL_CLOSED]     ssl is closed
 *    [ERR_SSL_MEM]          size > 4000
 */
int SslSend(int s, void *buf, int size);

/*
 * SslSend
 *   receive data from remote host
 * PARAM:
 *    s      SSL Handle
 *    buf   pointer to output data  
 *    size  the size of buffer, in byte
 * RETURN:
 *    The call returns the number of characters received, or <0 if an error occurred
 * ERRORS:
 *    SslRecv call fail, if
 *    [ERR_SSL_HANDLER]  ssl handle does not exist
 *    [ERR_SSL_NET]          the network connecting server is error
 *    [ERR_SSL_TIMEOUT]   time out
 *    [ERR_SSL_CONNECT]  before use SslRecv, you must call SslConnect
 *    [ERR_SSL_CLOSED]     ssl is closed
 *    [ERR_SSL_MD]            message digest fail
 */
int SslRecv(int s, void *buf, int size);

/*
 * SslClose
 *   close ssl connect
 * PARAM:
 *    s      SSL Handle
 * RETURN:
 *    The call returns 0, or <0 if an error occurred
 * ERRORS:
 *    SslClose call fail, if
 *    [ERR_SSL_HANDLER]  ssl handle does not exist
 */
int SslClose(int s);

/*
 * The Network Operate Sets
 */
typedef struct ssl_net_ops_s
{
	int (*net_open)(char *remote_addr, short remote_port, short local_port, long flag);
	int (*net_send)(int net_hd, void *buf, int size);
	int (*net_recv)(int net_hd, void *buf, int size);
	int (*net_close)(int net_hd);
}SSL_NET_OPS;

/*
 * SslSetNetOps
 *    tell ssl how to operate TCP/IP network(open, send, rcv and close), 
 * Param:
 *    ops      operate set, such as net_open, net_send, net_recv and net_close etc.
 *                 net_open      open a net connection from local_port to remote_addr:remote_port,
 *                                     if succeed, return net handle;otherwise, return < 0
 *                 net_send      send data in buf to remote host, 
 *                                     returns the number of characters sent, or <0 if an error occurred
 *                 net_recv       receive data from remote host,
 *                                     returns the number of characters received, or <0 if an error occurred
 */
void SslSetNetOps(SSL_NET_OPS *ops);

/*
** 时区格式
** ex1: 时区为-8:30,则hour=-8, min = 30
** ex2: 时区为+9:00,则hour=-9, min = 0
**/
typedef struct time_zone
{
	short hour;/* <0表示-, >0表示+ */
	short min;
}TIME_ZONE;

/*
** 系统日期格式
**/
typedef struct system_time
{
	short year;/* 2000~2050 */
	short month;/* 1~12 */ 
	short day;/* 1~31 */
	short hour;/* 0~23 */
	short min;/* 0~59 */
	short sec;/* 0~59 */
	TIME_ZONE zone;/* 时区*/
}SYSTEM_TIME_T;

/*
** 证书无效原因
**/
typedef enum 
{
	CERT_BAD = 1, /* 不能成功识别的证书格式*/
	CERT_TIME,/* 证书有效时间已过*/
	CERT_CRL,/* 证书已撤销*/
	CERT_SIGN,/* 证书签名无效*/
	CERT_CA_TIME,/* CA证书已过期*/
	CERT_CA_CRL,/* CA证书已撤销*/
	CERT_CA_SIGN,/* CA证书签名无效*/
	CERT_MAC, /* 系统不能支持签名对应的MAC算法*/
	CERT_MEM,/* 系统内存资源不够不能处理*/
	CERT_ISSUER,/* 找不到证书的发布者*/
	CERT_SELF_SIGN,
	CERT_NA=100,/* 未知原因*/
}CERT_INVAL_CODE;

/*
**跟系统相关的操作集合
**/
typedef struct ssl_sys_ops_s
{
	/* ReadSysTime: 读取系统日期接口，
	**   这是用于验证证书的有效时间；
	**   注意：如果该接口为NULL，
	**   则SSL会忽略证书时间的有效性；
	**   t:保存获取的时间,
	**   返回值: 成功返回0,失败返回<0
	**/
	int (*ReadSysTime)(SYSTEM_TIME_T *t/* OUT */);
	/* Random: 产生随机数接口，
	**   产生的结果需要符合PCI标准；
	**   注意：如果该接口为NULL，
	**   则SSL会自动产生随机数，但是不能符合PCI标准；
	**   buf: 保存随机数的值
	**   len: buf的空间大小
	**  返回值: 成功返回0,失败返回<0
	**/
	int (*Random)(unsigned char *buf/* OUT */, int len);
	/* Time: 获取以秒为单位的时间,
	** 注意：如果该接口为NULL，
	** 则SSL获取的时间值为一个固定的,
	** 这会降低SSL的安全性；
	** 相当于系统调用time(NULL)*/
	unsigned long (*Time)(unsigned long *);
	/* ServCertAck: 无法验证服务器证书的有效性的后续处理
	**    reason: 证书无效的原因
	**    返回值: 0表示忽略证书是否有效,继续连接服务器;
	**                      <0表示断开连接
	**/
	int (*ServCertAck)(CERT_INVAL_CODE reason);
}SSL_SYS_OPS;

/*
** 设置跟系统相关的操作
**  Read_SysTime: 读取系统时间,成功返回0,失败返回<0
**  Random:产生随机数,成功返回0,失败返回<0
**  Time:以秒为单位的时间,形同time(NULL) API
**/
void SslSetSysOps(SSL_SYS_OPS *ops);

typedef struct ssl_buf_s
{
	void *ptr;
	int  size;
}SSL_BUF_T;

/*
** SslCertsSet: 设置跟证书相关的信息
**    s: 对应的socket 句柄,
**    cert_chains:可信任的证书列表,
**            用于验证服务器端的证书;
**    cert_chains_num: 证书个数;
**    crl:撤销列表
**    local_certs: 本地使用的证书,
**            第一个必须为终端使用的本地证书,
**            其他为CA证书(用于验证本地证书);
**    local_privatekey:本地使用的证书对应的私钥
** 注意事项:
**    1.证书,crl,以及private key这些信息都采用Binary格式存储
**       如果为其他格式,要先转换
**    2.使用顺序: SslCertsSet  ---> SslConnect 
**      
**/
int SslCertsSet(int s, SSL_BUF_T *cert_chains, int cert_chains_num,
		SSL_BUF_T *crl, 
		SSL_BUF_T *local_certs, int local_certs_num,
		SSL_BUF_T *local_privatekey);

void SslGetVer(char str[10]);

/*
int SslDecodePem(char *BufIn, int BufInSize, char *BufOut, int BufOutSize)
Param:
BufIn   PEM Buffer Pointer
BufInSize  the size of BufIn
BufOut         BIN Buffer Pointer
BufOutSize  the size of BufOut
Return:
>0       succeed, No error
<0       Fail, Please see error code
Note:
The size of BufOut must be bigger than the size of BufIn
*/
int SslDecodePem(char *BufIn, int BufInSize, char *BufOut, int BufOutSize);

/*
SslDecodeCertPubkey从证书里面抽取RSA Public Key
*/
int SslDecodeCertPubkey(SSL_BUF_T *CertBuf, SSL_BUF_T *Module, SSL_BUF_T *Exp);

/*
SslDecodePrivKey
*/
int SslDecodePrivKey(SSL_BUF_T *PrivKey, SSL_BUF_T *Module, SSL_BUF_T *Exp);

/*
SslVerifyCert:验证证书有效性
cert: 等待被验证的证书
cert_chains: 证书链
*/
int SslVerifyCert(SSL_BUF_T *cert, 
			SSL_BUF_T *cert_chains, 
			int cert_chains_num);

#ifdef  __cplusplus
}
#endif

#endif/* _SSL_API_H_ */
