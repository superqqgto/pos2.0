#ifndef _FILEOPER_H
#define _FILEOPER_H

extern uchar FILE_ACTIVE_PAGE[];
extern uchar FILE_ADMIN_PAGE[];
extern uchar FILE_VOID_PAGE[];
extern uchar FILE_SYS_CTRL[];
extern uchar FILE_INIT_PAGE[];
extern uchar FILE_ATV_FILE[];
extern uchar FILE_MOBILE_STS[];
extern uchar FILE_MOBILE_FILE[];

int ExistSysFiles(void);
int ValidSysFiles(void);
void RemoveSysFiles(void);
int SaveSysCtrl(void);
int GetSysCtrl(void);

int WriteMob001(uchar *szData, uint uiLen);
#endif
