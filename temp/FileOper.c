#include "posapi.h"
#include "appLib.h"
#include "util.h"
#include "lng.h"
#include "ppcomp.h"

uchar FILE_ACTIVE_PAGE[] = "ACTIVE.XML";
uchar FILE_INIT_PAGE[] = "INITPAGE.XML";
uchar FILE_ADMIN_PAGE[] = "ADM.XML";
uchar FILE_VOID_PAGE[] = "CNC.XML";
uchar FILE_SYS_CTRL[] = "SYS_CTRL";
uchar FILE_ATV_FILE[] = "ATIVO.001";
uchar FILE_MOBILE_FILE[] = "MOBILE.001";
uchar FILE_MOBILE_STS[] = "MOBILE.STS";
uchar FILE_UNLOCK_FILE[] = "UNLOCK.001"; //add by wuc 2014.4.1



void SysHalt(void)
{
	ScrCls();
	PubDispString(_T("HALT FOR SAFETY "), DISP_LINE_REVER|DISP_LINE_LEFT);
	PubDispString(_T("PLS RESTART POS "), 4|DISP_LINE_LEFT);
	while(1);
}

int ExistFlowFiles(void)
{
	if (fexist((char *)FILE_INIT_PAGE)<0)
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}
}

// 判断系统参数文件大小
int ValidSysFiles(void)
{
	if ((fexist((char *)FILE_SYS_CTRL)<0) ||
		(filesize((char *)FILE_SYS_CTRL)!=sizeof(SYSTEM_CTRL)) )
	{
		return FALSE;
	}
	
	return TRUE;
}

void RemoveSysFiles(void)
{
	remove((char *)FILE_SYS_CTRL);
}

int SaveSysCtrl(void)
{
	int		iRet;

	// about 60K bytes
	iRet = PubFileWrite(FILE_SYS_CTRL, 0L, &glSysCtrl, sizeof(SYSTEM_CTRL));
	if( iRet!=0 )
	{
		PubTRACE0("SaveSysCtrlAll");
		SysHalt();
		return -1;
	}

	return 0;
}

int GetSysCtrl(void)
{
	int iRet;

	iRet = PubFileRead(FILE_SYS_CTRL, 0L, &glSysCtrl, sizeof(SYSTEM_CTRL));
	if(iRet != 0)
	{
		PubTRACE0("ReadSysCtrlAll");
		SysHalt();
		return -1;
	}

	return 0;
}

int WriteATV001(void)
{
	int fd;
	uchar szBuff[512], szTimeStamp[10+1];
	
	fd = open(FILE_ATV_FILE, O_RDWR|O_CREATE);
	if(fd < 0)
	{
		return 1;
	}

	memset(szBuff, 0, sizeof(szBuff));
	memset(szTimeStamp, 0, sizeof(szTimeStamp));
	PP_GetTimeStamp("03", szTimeStamp);
	sprintf(szBuff, "000-000 = ATV\r\n016-000 = %s\r\n999-999 = 0\r\n", szTimeStamp);
	seek(fd, 0, SEEK_SET);
	write(fd, szBuff, strlen(szBuff));
	close(fd);
	return 0;
}

int WriteMob001(uchar *szData, uint uiLen)
{
	int fd;
	uchar szIn[4096]; //modified by wuc 2014.1.15
	uint i, j;
	
	if(fexist(FILE_MOBILE_FILE) >= 0)
	{
		remove(FILE_MOBILE_FILE);
	}
	
	fd = open(FILE_MOBILE_FILE, O_RDWR|O_CREATE);
	if(fd < 0)
	{
		return 1;
	}

	memset(szIn, 0, sizeof(szIn));
	for(i=j=0; i<uiLen; i++,j++)
	{
		if(szData[j] == 0x5c)
		{
			j++;
			if(szData[j] == 0x72)
			{
				szIn[i] = 0x0d;
			}
			else if(szData[j] == 0x6e)
			{
				szIn[i] = 0x0a;
			}
		}
		else
		{
			szIn[i] = szData[j];
		}
	}
	
	seek(fd, 0, SEEK_SET);
	write(fd, szIn, i);
	close(fd);

	return 0;
}

