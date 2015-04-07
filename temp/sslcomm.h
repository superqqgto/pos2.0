#ifndef _SSL_COMM_H
#define _SSL_COMM_H

#define ERR_NO_DISP -1

#ifdef	__cplusplus
extern "C"{
#endif

int Ssl_Txd( char *psTxdData, ulong uiDataLen, ushort uiTimeOutSec );

int Ssl_Rxd( char *psRxeData, ulong uiExpLen, ulong *puiOutLen, ushort UiTimeOutSec );

int Ssl_OnHook( void );

int Ssl_Connect( char *, short, int );

int SSL_PROC(void);

#ifdef __cplusplus
extern "C"}
#endif

#endif    //end of _SSL_COMM_H




