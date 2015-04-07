#ifndef _TRANPROC
#define _TRANPROC

#include "util.h"

enum
{
	ACTIVE,
	ADMIN,
	PAYMENT,
	VOID,
	MAX
};

typedef struct _tagTRANS_INFO
{
	uchar ucTransType;
	uchar szService[64+1];
}TransInfo;

int Trans_Payment(void);
int Trans_Admin(void);
int Trans_Void(void);
int Trans_Active(void);
int Trans_Confirm(void);
int Trans_NotConfirm(void);
int TranProcess(uchar *szMsgType);
int BuildCampo(uchar *szCampo, uchar *szMsgType, uchar *szService);

extern char glTimeStamp[12];

#endif
