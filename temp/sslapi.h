/*
 *  SSL API 
 *  Author: jhsun
 *  Date: 2007-6-20
 *  Version: 2.0
 *  History:
 *  07-06-20 v1.0 jhsun create the file 
 *  08-04-29 v2.0 jhsun 
 *     1.�����˶�֤���֧��
 *     2.���Ӷ�ȡϵͳʱ��ӿ�
 *     3.����������ӿ�
    090309 sunJH
    1. SslGetVer�ӿ����ڻ�ȡ�汾��Ϣ
    2. ȡ������1.0 version API
091022 sunJH
1. ������ǩ��������
091221 sunJH
����SslDecodePem��SslDecodeCertPubkey��SslDecodePrivKey
3���ӿڣ�
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
#define ERR_SSL_APP_CANCEL (27+ERR_SSL_BASE) /* �����޷���֤֤�鵼���û�cancel*/
#define ERR_SSL_BADPRIVKEY (28+ERR_SSL_BASE)/* bad private key */
#define ERR_SSL_INITCERT   (29+ERR_SSL_BASE)/* Init CertInfo fail */
#define ERR_SSL_LIBVER     (30+ERR_SSL_BASE)/* ��汾�Ų�һ�� */
#define ERR_SSL_SELF_SIGN  (31+ERR_SSL_BASE)/* ��ֹ��ǩ��֤�� */
#define ERR_SSL_ILLEGAL_PARA  (32+ERR_SSL_BASE)/* ���հ��Ĳ����Ƿ� */
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
** ʱ����ʽ
** ex1: ʱ��Ϊ-8:30,��hour=-8, min = 30
** ex2: ʱ��Ϊ+9:00,��hour=-9, min = 0
**/
typedef struct time_zone
{
	short hour;/* <0��ʾ-, >0��ʾ+ */
	short min;
}TIME_ZONE;

/*
** ϵͳ���ڸ�ʽ
**/
typedef struct system_time
{
	short year;/* 2000~2050 */
	short month;/* 1~12 */ 
	short day;/* 1~31 */
	short hour;/* 0~23 */
	short min;/* 0~59 */
	short sec;/* 0~59 */
	TIME_ZONE zone;/* ʱ��*/
}SYSTEM_TIME_T;

/*
** ֤����Чԭ��
**/
typedef enum 
{
	CERT_BAD = 1, /* ���ܳɹ�ʶ���֤���ʽ*/
	CERT_TIME,/* ֤����Чʱ���ѹ�*/
	CERT_CRL,/* ֤���ѳ���*/
	CERT_SIGN,/* ֤��ǩ����Ч*/
	CERT_CA_TIME,/* CA֤���ѹ���*/
	CERT_CA_CRL,/* CA֤���ѳ���*/
	CERT_CA_SIGN,/* CA֤��ǩ����Ч*/
	CERT_MAC, /* ϵͳ����֧��ǩ����Ӧ��MAC�㷨*/
	CERT_MEM,/* ϵͳ�ڴ���Դ�������ܴ���*/
	CERT_ISSUER,/* �Ҳ���֤��ķ�����*/
	CERT_SELF_SIGN,
	CERT_NA=100,/* δ֪ԭ��*/
}CERT_INVAL_CODE;

/*
**��ϵͳ��صĲ�������
**/
typedef struct ssl_sys_ops_s
{
	/* ReadSysTime: ��ȡϵͳ���ڽӿڣ�
	**   ����������֤֤�����Чʱ�䣻
	**   ע�⣺����ýӿ�ΪNULL��
	**   ��SSL�����֤��ʱ�����Ч�ԣ�
	**   t:�����ȡ��ʱ��,
	**   ����ֵ: �ɹ�����0,ʧ�ܷ���<0
	**/
	int (*ReadSysTime)(SYSTEM_TIME_T *t/* OUT */);
	/* Random: ����������ӿڣ�
	**   �����Ľ����Ҫ����PCI��׼��
	**   ע�⣺����ýӿ�ΪNULL��
	**   ��SSL���Զ���������������ǲ��ܷ���PCI��׼��
	**   buf: �����������ֵ
	**   len: buf�Ŀռ��С
	**  ����ֵ: �ɹ�����0,ʧ�ܷ���<0
	**/
	int (*Random)(unsigned char *buf/* OUT */, int len);
	/* Time: ��ȡ����Ϊ��λ��ʱ��,
	** ע�⣺����ýӿ�ΪNULL��
	** ��SSL��ȡ��ʱ��ֵΪһ���̶���,
	** ��ή��SSL�İ�ȫ�ԣ�
	** �൱��ϵͳ����time(NULL)*/
	unsigned long (*Time)(unsigned long *);
	/* ServCertAck: �޷���֤������֤�����Ч�Եĺ�������
	**    reason: ֤����Ч��ԭ��
	**    ����ֵ: 0��ʾ����֤���Ƿ���Ч,�������ӷ�����;
	**                      <0��ʾ�Ͽ�����
	**/
	int (*ServCertAck)(CERT_INVAL_CODE reason);
}SSL_SYS_OPS;

/*
** ���ø�ϵͳ��صĲ���
**  Read_SysTime: ��ȡϵͳʱ��,�ɹ�����0,ʧ�ܷ���<0
**  Random:���������,�ɹ�����0,ʧ�ܷ���<0
**  Time:����Ϊ��λ��ʱ��,��ͬtime(NULL) API
**/
void SslSetSysOps(SSL_SYS_OPS *ops);

typedef struct ssl_buf_s
{
	void *ptr;
	int  size;
}SSL_BUF_T;

/*
** SslCertsSet: ���ø�֤����ص���Ϣ
**    s: ��Ӧ��socket ���,
**    cert_chains:�����ε�֤���б�,
**            ������֤�������˵�֤��;
**    cert_chains_num: ֤�����;
**    crl:�����б�
**    local_certs: ����ʹ�õ�֤��,
**            ��һ������Ϊ�ն�ʹ�õı���֤��,
**            ����ΪCA֤��(������֤����֤��);
**    local_privatekey:����ʹ�õ�֤���Ӧ��˽Կ
** ע������:
**    1.֤��,crl,�Լ�private key��Щ��Ϣ������Binary��ʽ�洢
**       ���Ϊ������ʽ,Ҫ��ת��
**    2.ʹ��˳��: SslCertsSet  ---> SslConnect 
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
SslDecodeCertPubkey��֤�������ȡRSA Public Key
*/
int SslDecodeCertPubkey(SSL_BUF_T *CertBuf, SSL_BUF_T *Module, SSL_BUF_T *Exp);

/*
SslDecodePrivKey
*/
int SslDecodePrivKey(SSL_BUF_T *PrivKey, SSL_BUF_T *Module, SSL_BUF_T *Exp);

/*
SslVerifyCert:��֤֤����Ч��
cert: �ȴ�����֤��֤��
cert_chains: ֤����
*/
int SslVerifyCert(SSL_BUF_T *cert, 
			SSL_BUF_T *cert_chains, 
			int cert_chains_num);

#ifdef  __cplusplus
}
#endif

#endif/* _SSL_API_H_ */
