#ifndef _SETUP_H_
#define _SETUP_H_

void Function(void);
int GetIPAddress(uchar *pszPrompts, uchar bAllowNull, uchar *pszIPAddress);
//add by wuc 2014.4.1
int ConfirmConfig(void);
int VerifyPassword(void);
int Enterfunctions(uchar *);
void GetPPOnlineAuto(uchar *szAuto);
void GetPPOnlineClient(uchar *szClient);
void GetCPF_CNPJ(uchar *szCPF);
int GetCard(uchar ucMode); //add by wuc 2014.4.1
void LoadDefault(void);
int BuildHeaderUnLock(uchar *szHeader, uchar *szMsgType);//linzhao
int	BuildPacketUnLock(uchar *pszMsgType);//linzhao
int EditLogNum(uchar *pszLogNum);//linzhao
int BuildHeaderIDUnLock(uchar *szID);//linzhao
int FirstAutoConfig(void);
int UnlockComfirmSend(void);
int BuildPacketUnlockConfirm(uchar *);
int	BuildHeaderUnLockConfirm(uchar *, uchar *);
void BlockScrProc(void);
void UnlockScrProc(void);
int EnableSSL(void);
int ConnectTest(void);

extern uchar glConfiged; //add by wuc 2014.4.1

#endif
