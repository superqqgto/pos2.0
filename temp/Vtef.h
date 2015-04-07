#ifndef _VTEF_H
#define _VTEF_H

int VTEF_Rxd(uchar *psRxdData, ushort uiExpLen, ushort uiTimeOutSec, uint *puiOutLen);
int VTEF_Txd(uchar *szData, uint uiDataLen, ushort uiTimeOutSec);
int VTEF_PackData(void);
int VTEF_UnpackData(void);
int Mobile2POS(void);
int POS2Mobile(uchar ucType);
void VTEFMain(void);

extern uchar g_bAbort;
#endif
