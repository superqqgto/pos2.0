
#include "posapi.h"
#include "applib.h"
#include "util.h"
#include "lng.h"

/********************** Internal macros declaration ************************/
#define TIMER_TEMPORARY         4   // Temporary timer(Shared by different modules)
#ifdef _POS_DEBUG
#define LEN_DBGDATA         1024    // Maximum debug data length
#define MAX_CHARS           5       // Max characters per line in debug usage
#define CHARS_PERLINE       21      // Characters per line (6*8 english font)
#define MAX_LOG_MSG         (((LEN_DBGDATA+MAX_CHARS-1)/MAX_CHARS)*CHARS_PERLINE)
#endif /* _POS_DEBUG */

#define LEN_GETSTRING		512	// for PubGetString() use

// Icons
#ifndef ICON_PHONE
#define ICON_PHONE	   1	// phone 电话
#define ICON_SIGNAL	   2	// wireless signal 信号
#define ICON_PRINTER   3	// printer 打印机
#define ICON_ICCARD	   4	// smart card IC卡
#define ICON_LOCK      5	// lock 锁
#define ICON_SPEAKER   6	// speeker 扬声器
#define ICON_BATTERY   6	// Battery 电池图标
#define ICON_UP        7	// up 向上
#define ICON_DOWN      8	// down 向下
#define CLOSEICON	   0	// Action: blackout icon 关闭图标[针对所有图标]
#define OPENICON       1    // Action: lit icon 显示图标[针对打印机、IC卡、锁、电池、向上、向下]
#endif
/********************** Internal structure declaration *********************/

/********************** Internal functions declaration *********************/
static void GetBaseName(uchar *pszFullPath, uchar *pszBaseName);
static void MapChar2Index(uchar ch, int *piRow, int *piCol);
static void ShowWorkBuf(uchar ucMode, uchar *pszWorkBuff, int iLen);
static uchar IsLeapYear(unsigned long ulYear);

//static int UnPackElement(FIELD_ATTR *pAttr, uchar *pusIn, uchar *pusOut,uint *puiInLen);
static uchar ScrSend(uchar *psData, ushort nDataLen, ushort nStartLine);
static uchar PortSendstr(uchar ucChannel, uchar *psStr, ushort nStrLen);
static uchar UnitSend(uchar *psData, ushort nDataLen, uchar ucDevice, uchar ucMode, ushort nStartLine);
static void disp_text(unsigned char *mess,unsigned char line,unsigned char loc,
                 unsigned char inkey,unsigned char back);
static int prepare_screen_text(SMART_MENU * sm, int start_index, int hilight, unsigned int mode, unsigned char with_disp, int * scr_item_num,  int * itemList);
static int GetNextHiliIndex(SMART_MENU *sm, int hilight_index);
static int GetPrevHiliIndex(SMART_MENU *sm, int hilight_index);
static int GetFirstHiliIndex(SMART_MENU *pstSmartMenu);

/********************** Internal variables declaration *********************/
#ifdef _POS_DEBUG
static uchar glszDbgInfo[MAX_LOG_MSG+128];
#endif

const uint uiConstMonthday[12]=
{
	0,		
	31,  //1 
	31+28,  //2 
	31+28+31,  //3 
	31+28+31+30,  //4 
	31+28+31+30+31,  //5 
	31+28+31+30+31+30,  //6 
	31+28+31+30+31+30+31,  //7 
	31+28+31+30+31+30+31+31,  //8 
	31+28+31+30+31+30+31+31+30,  //9 
	31+28+31+30+31+30+31+31+30+31,  //10
	31+28+31+30+31+30+31+31+30+31+30,  //11
};     
static uchar sgMinLineNo;

// Alpha table for PubGetString
static char *pszAlphaTbl[10] =
{
	"0,*# ~`!@$%^&-+=(){}[]<>_|\\:;\"\'?/",
	"1QZ.", "2ABC", "3DEF",
	"4GHI", "5JKL", "6MNO",
	"7PRS", "8TUV", "9WXY",
};

/********************** external reference declaration *********************/

/******************>>>>>>>>>>>>>Implementations<<<<<<<<<<<<*****************/

//将源串与目标串异或后存入目标串(两个串长度相等)
//Source string XOR object string, then store the result in object string.(The two string are same length)
void PubXor(uchar *psVect1, uchar *psVect2, uint uiLength, uchar *psOut)
{
	uint   uiCnt;

	if ((psVect1 == NULL) || (psVect2 == NULL) || (psOut == NULL))
	{
		return;
	}

	memcpy(psOut, psVect1, uiLength); 

	for(uiCnt = 0; uiCnt < uiLength; uiCnt++)
	{
		psOut[uiCnt] ^= psVect2[uiCnt];
	}
}

//将二进制源串分解成双倍长度可读的16进制串,0x12AB-->"12AB"
//Convert BIN string to readable HEX string, which have double length of BIN string. 0x12AB-->"12AB"
void PubBcd2Asc(uchar *psIn, uint uiLength, uchar *psOut)
{
	static const uchar ucHexToChar[16] = {"0123456789ABCDEF"};
	uint   uiCnt;

	if ((psIn == NULL) || (psOut == NULL))
	{
		return;
	}

	for(uiCnt = 0; uiCnt < uiLength; uiCnt++)
	{
		psOut[2*uiCnt]   = ucHexToChar[(psIn[uiCnt] >> 4)];
		psOut[2*uiCnt + 1] = ucHexToChar[(psIn[uiCnt] & 0x0F)];
	}
}

// 同vOneTwo()函数,并在目标串后添一 '\0'
//Similar with function PubOne2Two(), and add '\0' at the end of object string
void PubBcd2Asc0(uchar *psIn, uint uiLength, uchar *pszOut)
{
	if ((psIn == NULL) || (pszOut == NULL))
	{
		return;
	}

	PubBcd2Asc(psIn, uiLength, pszOut);
	pszOut[2*uiLength] = 0;
}

//将可读的16进制串合并成其一半长度的二进制串,"12AB"-->0x12AB
//Convert readable HEX string to BIN string, which only half length of HEX string. "12AB"-->0x12AB
void PubAsc2Bcd(uchar *psIn, uint uiLength, uchar *psOut)
{
	uchar   tmp;
	uint    i;

	if ((psIn == NULL) || (psOut == NULL))
	{
		return;
	}

	for(i = 0; i < uiLength; i += 2)
	{
		tmp = psIn[i];
		if( tmp > '9' )
		{
			tmp = (uchar)toupper((int)tmp) - 'A' + 0x0A;
		}
		else
		{
			tmp &= 0x0F;
		}
		psOut[i / 2] = (tmp << 4);

		tmp = psIn[i+1];
		if( tmp > '9' )
		{
			tmp = toupper((char)tmp) - 'A' + 0x0A;
		}
		else
		{
			tmp &= 0x0F;
		}
		psOut[i/2] |= tmp;
	}
}

//将数字串转变成长整数,相当于atol(),但是该函数不要求零字符结束
//Convert number string to long integer, similar to atol(). This function don't request end with '\0'.
ulong PubAsc2Long(uchar *psString, uint uiLength, ulong *pulOut)
{
	uchar    szBuff[15+1];
	ulong    ulTmp;

	sprintf((char *)szBuff, "%.*s", uiLength <= 15 ? uiLength : 15, psString);
	ulTmp =  (ulong)atol((char*)szBuff);
	if (pulOut != NULL)
	{
		*pulOut = ulTmp;
	}
	return ulTmp;
}

//整型数转换成高位在前的字符串表示(按照网络字节格式存放)
//Convert integer to string which high bit at the front. (Store it according to network byte format)
void PubLong2Char(ulong ulSource, uint uiTCnt, uchar *psTarget)
{
	uint    i;

	for(i = 0; i < uiTCnt; i++)
	{
		psTarget[i] = (uchar)(ulSource >> (8 * (uiTCnt - i - 1)));
	}
}

//高位在前的字符串表示转换为整型数
//Convert the character string with high bit in the front to integer
ulong PubChar2Long(uchar *psSource, uint uiSCnt, ulong *pulTarget)
{
	uint    i;
	ulong ulTmp;

	if (psSource == NULL)
	{
		return 0;
	}

	ulTmp = 0L;
	for(i = 0; i < uiSCnt; i++)
	{
		ulTmp |= ((ulong)psSource[i] << 8 * (uiSCnt - i - 1));
	}

	if (pulTarget != NULL)
	{
		*pulTarget = ulTmp;
	}
	return ulTmp;
}

//将整数转换为BCD格式的串存放,1234 --> 0x12 0x34
//Converted integer to BCD string,1234 --> 0x12 0x34
void PubLong2Bcd(ulong ulSource, uint uiTCnt, uchar *psTarget)
{
	uchar    szFmt[30], szBuf[30];

	sprintf((char*)szFmt, "%%0%dlu", uiTCnt*2);
	sprintf((char*)szBuf, (char *)szFmt, ulSource);
	if( psTarget != NULL )
	{
		PubAsc2Bcd(szBuf, uiTCnt * 2, psTarget);
	}
}

//转换BCD串为整数
//Convert BCD string to integer
ulong PubBcd2Long(uchar *psSource, uint uiSCnt, ulong *pulTarget)
{
	uchar   szBuf[30];
	ulong   ulTmp;

	if ((psSource == NULL))
	{
		return 0;
	}

	PubBcd2Asc0(psSource, uiSCnt, szBuf);
	ulTmp = (ulong)atol((char *)szBuf);
	if( pulTarget != NULL )
	{
		*pulTarget = ulTmp;
	}

	return ulTmp;
}

//将字符串转换为大写
//Convert the character string to capital
void PubStrUpper(uchar *pszString)
{
	if ((pszString == NULL))
	{
		return;
	}

	while( *pszString )
	{
		*pszString = toupper((char)*pszString);
		pszString++;
	}
}

//将字符串转换为小写
//Convert the character string to lowercase
void PubStrLower(uchar *pszString)
{
	if ((pszString == NULL))
	{
		return;
	}

	while( *pszString )
	{
		*pszString = (uchar)tolower((int)*pszString);
		pszString++;
	}
}

//删除字符串中的空白字符(包括,空格/回车/换行/TAB字符)
//Delete the characters include blank,enter,newline,TAB in the string
void PubTrimStr(uchar *pszString)
{
#define ISSPACE(ch) ( ((ch) == ' ')  || ((ch) == '\t') || \
	((ch) == '\n') || ((ch) == '\r') )

	uchar *p, *q;

	if( !pszString || !*pszString )
	{
		return;
	}

	p = &pszString[strlen((char*)pszString) - 1];
	while(( p > pszString) && ISSPACE(*p) )
	{
		*p-- = 0;
	}
	if( (p == pszString) && ISSPACE(*p) )
	{
		*p = 0;
	}

	for(p = pszString; *p && ISSPACE(*p); p++);
	if( p != pszString )
	{
		q = pszString;
		while( *p )
		{
			*q++ = *p++;
		}
		*q = 0;
	}
#undef ISSPACE
}


//删除字符串两端的特定字符
//Delete the specified character in string
void PubTrimSpcStr(uchar *pszString, uchar ucSpcChar)
{
	uchar *p, *q;
	
	if( !pszString || !*pszString )
	{
		return;
	}
	
	p = &pszString[strlen((char*)pszString) - 1];
	while( (p > pszString) && ((*p) == ucSpcChar))
	{
		*p-- = 0;
	}
	if( (p == pszString) && ((*p) == ucSpcChar) )
	{
		*p = 0;
	}
	
	for(p = pszString; *p && ((*p) == ucSpcChar); p++);
	if( p != pszString )
	{
		q = pszString;
		while( *p )
		{
			*q++ = *p++;
		}
		*q = 0;
	}
}

//删除字符串右边连续的特定字符
//Delete the specified continuous characters on the right of the string
void PubTrimTailChars(uchar *pszString, uchar ucRemoveChar)
{
	int		i, iLen;
	
	if( !pszString || !*pszString )
    	{
        	return;
    	}
	
	iLen = strlen((char *)pszString);
	for(i=iLen-1; i>=0; i--)
	{
		if( pszString[i]!=ucRemoveChar )
		{
			break;
		}
		pszString[i] = 0;
	}
}

// 删除前导字符
// delete the specified continuous characters on the left of the string
void PubTrimHeadChars(uchar *pszString, uchar ucRemoveChar)
{
	uchar	*p;
	
	if( !pszString || !*pszString )
    	{
        	return;
    	}
	
	for(p=pszString; *p && *p==ucRemoveChar; p++);
	if( p!=pszString )
	{
		strcpy(pszString, p);
	}
}

// 填充前导字符,以便达到uiTargetLen长度
// Pad leading charactes till specific length.
void PubAddHeadChars( uchar *pszString, uint uiTargetLen, uchar ucAddChar )
{
	uint	uiLen;

	if (pszString == NULL)
	{
		return;
	}
	
	uiLen = (uint)strlen((char *)pszString);
	if( uiLen>=uiTargetLen )
	{
		return;
	}
	
	memmove(pszString+uiTargetLen-uiLen, pszString, uiLen+1);
	memset(pszString, ucAddChar, uiTargetLen-uiLen);
}

// 不区分大小写的字符串比较函数
//Compare two strings, non case-sensitive.
int PubStrNoCaseCmp(uchar *pszStr1, uchar *pszStr2)
{
	if((pszStr1 == NULL) || (pszStr2 == NULL))
	{
		return -1;
	}

	while( *pszStr1 && *pszStr2 )
	{
		if( toupper((char)*pszStr1) != toupper((char)*pszStr2) )
		{
			return (toupper((char)*pszStr1) - toupper((char)*pszStr2));
		}
		pszStr1++;
		pszStr2++;
	}
	if( !*pszStr1 && !*pszStr2 )
	{
		return 0;
	}
	if( !*pszStr1 )
	{
		return -1;
	}

	return 1;
}

//反转字符串
//reverse the string
void PubStrReverse(uchar *pszStringInOut)
{
	int     i, j, iLength;
	uchar    ucTmp;

	if (pszStringInOut == NULL)
	{
		return;
	}

	iLength = strlen((char*)pszStringInOut);
	for(i = 0,j = iLength - 1; i < iLength / 2; i++,j--)
	{
		ucTmp        = pszStringInOut[i];
		pszStringInOut[i] = pszStringInOut[j];
		pszStringInOut[j] = ucTmp;
	}
}


//取字节的高四位
//get the high 4 bit of the byte
uchar PubHigh4Bit(uchar ucInChar)
{
	return (ucInChar / 16);
}


//取字节的低四位
//get the low 4 bit of the byte
uchar PubLow4Bit(uchar ucInChar)
{
	return (ucInChar & 0x0F);
}

//两个ASC字符串相乘,结果放在pszProduct里
//multiply one ASC string by another
void PubAscMul(uchar *pszFaciend, uchar *pszMultiplier, uchar *pszProduct)
{
	uchar	*p, ucTemp, ucCarryBit, szBuff[100+1];
	uint	uiFaciLen, uiMulLen, uiProdPos, uiCnt;
	
	if ((pszFaciend == NULL) || (pszMultiplier == NULL) || (pszProduct == NULL))
	{
		return;
	}
	
	uiFaciLen = strlen((char *)pszFaciend);
	uiMulLen  = strlen((char *)pszMultiplier);
	
	PubASSERT( uiFaciLen+uiMulLen<=100 );
	sprintf((char *)szBuff, "%0*ld", uiFaciLen+uiMulLen, 0L);
	
	for(uiProdPos=0; uiFaciLen>0; uiFaciLen--,uiProdPos++)
	{
		ucCarryBit = 0;
		p = &szBuff[uiProdPos];
		
		for(uiCnt=uiMulLen; uiCnt>0; uiCnt--)
		{
			ucTemp = (pszFaciend[uiFaciLen-1] & 0x0F) * (pszMultiplier[uiCnt-1] & 0x0F) +
				ucCarryBit + (*p & 0x0F);
			*p++   = (ucTemp % 10) + '0';
			ucCarryBit = ucTemp / 10;
		}
		if( ucCarryBit!=0 )
		{
			*p++ = ucCarryBit + '0';
		}
	}
	PubTrimTailChars(szBuff, '0');
	PubStrReverse(szBuff);
	
	if( szBuff[0]==0 )
	{
		sprintf((char *)szBuff, "0");
	}
	if( pszProduct!=NULL )
	{
		sprintf((char *)pszProduct, "%s", szBuff);
	}
}

//长度相等的两个ASC字符串相加
// Addition of two equal length ASCII string(length specified, no need '\0' ending)
void PubAscAdd(uchar *psAddend1, uchar *psAddend2, uint uiLen, uchar *pszSum)
{
	uchar	*pszResult, ucCarryBit, ucTemp, szBuff[100+1];
	
	if ((psAddend1 == NULL) || (psAddend2 == NULL) || (pszSum == NULL) || (uiLen > 100))
	{
		return;
	}

	ucCarryBit = 0;
	pszResult  = szBuff;
	while( uiLen>0 )
	{
		ucTemp = (psAddend1[uiLen-1] & 0x0F) + (psAddend2[uiLen-1] & 0x0F) + ucCarryBit;
		*pszResult++ = (ucTemp % 10) + '0';
		ucCarryBit   = (ucTemp>9) ? 1 : 0;
		uiLen--;
	}
	if( ucCarryBit!=0 )
	{
		*pszResult++ = '1';
	}
	*pszResult = 0;
	
	PubStrReverse(szBuff);
	if( pszSum!=NULL )
	{
		sprintf((char *)pszSum, "%s", szBuff);
	}
}

//长度相等的两个ASC字符串相减(psMinuend>psSubtrahend)
// Subtraction of two equal length ASCII string(length specified, no need '\0' ending)
void PubAscSub(uchar *psMinuend, uchar *psSubtrahend, uint uiLen, uchar *pszResult)
{
	uchar	*pszOut, ucCarryBit, ucTemp, szBuff[100+1];
	
	if ((psMinuend == NULL) || (psSubtrahend == NULL) || (pszResult == NULL) || (uiLen > 100))
	{
		return;
	}

	pszOut     = szBuff;
	ucCarryBit = 0;
	while( uiLen>0 )
	{
		ucTemp = (psMinuend[uiLen-1] & 0x0F) - (psSubtrahend[uiLen-1] & 0x0F) - ucCarryBit + 10;
		*pszOut++  = (ucTemp % 10) + '0';
		ucCarryBit = (psMinuend[uiLen-1]<psSubtrahend[uiLen-1]+ucCarryBit) ? 1 : 0;
		uiLen--;
	}
	*pszOut = 0;
	
	PubStrReverse(szBuff);
	if( pszResult!=NULL )
	{
		sprintf((char *)pszResult, "%s", szBuff);
	}
}

//ASC字符串减1,结果放在原串里,有返回值
//subtract 1 from the ASC string, the result store in that ASC string
uchar PubAscDec(uchar *psAscStr, uint uiStrLen)
{
	if ((psAscStr == NULL) || (uiStrLen == 0))
	{
		return 1;
	}
	
	psAscStr += uiStrLen - 1;
	while(--(*psAscStr) < '0')
	{
		*psAscStr-- = '9';
		if(--uiStrLen == 0)
		{
			return 1;
		}
	}
	return 0;
}


//ASC字符串加1,结果放在原串里,有返回值
//Add 1 to the ASC string, the result store in that ASC string
uchar PubAscInc(uchar *psAscStr, uint uiStrLen)
{
	if ((psAscStr == NULL) || (uiStrLen == 0))
	{
		return 1;
	}

	psAscStr += uiStrLen - 1;
	while(++(*psAscStr) > '9')
	{
		*psAscStr-- = '0';
		if(--uiStrLen == 0)
		{
			return 1;
		}
	}
	
	return 0;
}


//长度相等的两个BCD串相加，结果放在psBcdStr1里
//Addition of 2 equal length BCD string. length specified and no need '\0' ending. fill leading 0 if necessary.
void PubBcdAdd(uchar *psAddend1, uchar *psAddend2, uint uiLen, uchar *psResult)
{
	uchar	sAdd1[100+1], sAdd2[100];
	
	if ((psAddend1 == NULL) || (psAddend2 == NULL) || (psResult == NULL) || (uiLen > 50))
	{
		return;
	}

	PubASSERT( uiLen<=50 );
	PubBcd2Asc(psAddend1, uiLen, sAdd1);
	PubBcd2Asc(psAddend2, uiLen, sAdd2);
	PubAscAdd(sAdd1, sAdd2, uiLen*2, sAdd1);
	PubAsc2Bcd(sAdd1, uiLen*2, psResult);
}


//长度相等的两个BCD串相减，结果放在psAddend1里
//subtraction of 2 equal length BCD string. length specified and no need '\0' ending. fill leading 0 if necessary.
void PubBcdSub(uchar *psMinuend, uchar *psSubtrahend, uint uiLen, uchar *psResult)
{
	uchar	sMinuend[100+1], sSubtrahend[100];
	
	if ((psMinuend == NULL) || (psSubtrahend == NULL) || (psResult == NULL) || (uiLen > 50))
	{
		return;
	}

	PubASSERT( uiLen<=50 );
	PubBcd2Asc(psMinuend, uiLen, sMinuend);
	PubBcd2Asc(psSubtrahend, uiLen, sSubtrahend);
	PubAscSub(sMinuend, sSubtrahend, uiLen*2, sMinuend);
	PubAsc2Bcd(sMinuend, uiLen*2, psResult);
}


//BCD字符串减1,结果放在原串里,有返回值
//subtracted from the BCD string, the result store in that BCD string
uchar PubBcdDec(uchar *psBcdStr, uint uiStrLen)
{
	if ((psBcdStr == NULL) || (uiStrLen == 0))
	{
		return 1;
	}

	psBcdStr += uiStrLen - 1;
	while(--(*psBcdStr) == 0xff)
	{
		*psBcdStr = 0x99;
		psBcdStr--;
		if(--uiStrLen == 0)
		{
			return 1;
		}
	}
	
	if(((*psBcdStr) & 0x0f) > 9)
	{
		*psBcdStr -= 0x06;
	}
	return 0;
}


//BCD字符串加1,结果放在原串里,有返回值
//Add 1 to the BCD string, the result store in that BCD string
uchar PubBcdInc(uchar *psBcdStr, uint uiStrLen)
{
	if ((psBcdStr == NULL) || (uiStrLen == 0))
	{
		return 1;
	}

	psBcdStr += uiStrLen - 1;
	while(++(*psBcdStr) > 0x99)
	{
		*psBcdStr = 0;
		psBcdStr--;
		if((--uiStrLen) == 0)
		{
			return 1;
		}
	}
	
	if(((*psBcdStr) & 0x0f) > 9)
	{
		*psBcdStr += 0x06;
	}
	return 0;
}


/*********************************************************************************************/
/*********************************************************************************************/


void PubConvAmount(uchar *pszPrefix, uchar *pszIn, uchar ucDeciPos, uchar ucIgnoreDigit, uchar *pszOut, uchar ucMisc)
{
	int		ii, iInLen, iIntegerLen;
	uchar	bSeparator, bNegative, szBuff[40+1], szTemp[40];
	uchar	*pRead, *pWr;
	
	PubASSERT(pszIn!=NULL && pszOut!=NULL && ucDeciPos<4);

	if ((pszIn == NULL) || (pszOut == NULL) || (strlen((char*)pszIn) > 40))
	{
		return;
	}
	
	sprintf((char *)szBuff, "%.40s", pszIn);
	PubTrimHeadChars(szBuff, '0');
	iInLen = strlen((char *)szBuff);		//get input amount length
	if (ucIgnoreDigit && iInLen>ucIgnoreDigit)	// trim ignored digits.
	{
		iInLen -= ucIgnoreDigit;
		szBuff[iInLen] = 0;
	}
	pRead  = szBuff;
	pWr = pszOut;
	
	bSeparator = ucMisc & GA_SEPARATOR;		//whether use ',' to separate huge amount
	bNegative  = ucMisc & GA_NEGATIVE;		//whether display negative amount
	
	if (pszPrefix!=NULL && *pszPrefix!=0)
	{
		pWr += sprintf((char *)pWr, "%.3s ", pszPrefix);
		//		pWr += MIN(strlen((char *)pszPrefix), 4);
	}
	if (bNegative)
	{
		*(pWr++) = '-';
	}
	
	// before decimal point
	if (iInLen>ucDeciPos)	// if value>1
	{
		iIntegerLen = iInLen - ucDeciPos;
		ii = iIntegerLen;
		while (ii--)
		{
			*(pWr++) = *(pRead++);
			if (bSeparator && (ii%3==0) && ii)
			{
				*(pWr++) = ',';
			}
		}
	}
	else
	{
		*(pWr++) = '0';
	}
	
	// decimal point and afterwards
	if (ucDeciPos!=0)
	{
		sprintf((char *)szTemp, "%.*s", ucDeciPos, pRead);
		PubAddHeadChars(szTemp, ucDeciPos, '0');
		sprintf((char *)pWr, ".%s", szTemp);
		//		sprintf((char *)pWr, ".%0*s", ucDeciPos, pRead);
		pWr += (ucDeciPos+1);
	}
	else
	{
		*(pWr++) = 0;
	}
	
	// Process suffix (if any)
	//...
}

// Check if any key pressed.
uchar PubKeyPressed(void)
{
	return (kbhit()==0x00);
}

// Wait key within specific timeout. no time limit if iWaitTime<0
uchar PubWaitKey(short iWaitTime)
{
	uchar   ucKey, bChkTimer;
	
	if (iWaitTime>=0)
	{
		bChkTimer = TRUE;
		TimerSet(TIMER_TEMPORARY, (short)(iWaitTime*10));
	}
	else
	{
		bChkTimer = FALSE;
	}
	
	while (1)
	{
		if (PubKeyPressed())
		{
			ucKey = getkey();
			break;
		}
		else if (bChkTimer && TimerCheck(TIMER_TEMPORARY)==0)
		{
			ucKey = NOKEY;
			break;
		}
	}   // end of while (1)
	
	return ucKey;
}


////////////////////////////////////// Smart Menu 智能菜单 //////////////////////////////////////




int PubSmartMenuEx(SMART_MENU *pstSmartMenu, uint uiMode, int iTimeout)
{
	int i;
	int iStartIndex;
	int iEndIndex;
	uchar ucKey;    
	int ScrIndex[10];	// 记录每一屏的第一个菜单项在sm->content中的索引    
	int iCurScr = 0;	// Current Screen sequence 当前的菜单所在的屏数   
	int iHilightIndex;	// Current highlighted item 当前的高亮项目索引
	int iRet;    
	uchar ucStartKey;
	int iScrItemNum;
	int itemList[16];
	int iTargetIdx;

	for(i = 0; i<16; i++)
	{
		itemList[i] = 0;
	}

	if (pstSmartMenu == NULL)
	{
		return -1;
	}

	if(pstSmartMenu->pstContent == NULL) 
	{
		PubShowTwoMsg(2, "SMART MENU", "NOT INIT'ED");
		getkey();
		return -1;
	}

	if(uiMode & SM_ZERO_START)
	{
		ucStartKey = KEY0;
	}
	else
	{
		ucStartKey = KEY1;
	}

	//Process title 处理标题
	if(pstSmartMenu->pstContent[0].iValue < 0) 
	{
		iStartIndex = 1;
	}
	else
	{
		iStartIndex = 0;
	}

	//Locate first available item 找到第一个有效的菜单项
	iStartIndex = GetFirstHiliIndex(pstSmartMenu);
	if(iStartIndex < 0) 
	{
		return -1;
	}

	iHilightIndex = iStartIndex;

	//Find the first menu item in each screen. 确定每一屏的第一个菜单项的索引
	for(i = 0; i < 10; i++)
	{
		ScrIndex[i] = -1;
	}

	ScrIndex[0] = iStartIndex;
	i = 1;
	while(iStartIndex < pstSmartMenu->iNum) 
	{
		iEndIndex = prepare_screen_text(pstSmartMenu, iStartIndex, iHilightIndex, uiMode, 0, &iScrItemNum, itemList);
		ScrIndex[i++] = iEndIndex;
		iStartIndex = iEndIndex;
	}

	iStartIndex = ScrIndex[0];
	iHilightIndex =iStartIndex;
	iCurScr = 0;
	while(1) 
	{
		iStartIndex = ScrIndex[iCurScr];
		iEndIndex = prepare_screen_text(pstSmartMenu, iStartIndex, iHilightIndex, uiMode, 1, &iScrItemNum, itemList);

		if(uiMode & SM_ARROW_SEL) 
		{
			if(iHilightIndex > ScrIndex[0]) 
			{
				ScrSetIcon(MENU_ICON_UP, MENU_OPENICON);
			}
			else 
			{
				ScrSetIcon(MENU_ICON_UP, MENU_CLOSEICON);
			}

			if((ScrIndex[iCurScr + 1] >= pstSmartMenu->iNum )
				&& (iHilightIndex == iStartIndex + iScrItemNum -1 ))
			{
				ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
			}
			else 
			{
				ScrSetIcon(MENU_ICON_DOWN, MENU_OPENICON);
			}
		}
		else 
		{
			if(iCurScr > 0) 
			{
				ScrSetIcon(MENU_ICON_UP, MENU_OPENICON);
			}
			else 
			{
				ScrSetIcon(MENU_ICON_UP, MENU_CLOSEICON);
			}

			if(ScrIndex[iCurScr + 1] >= pstSmartMenu->iNum )
			{
				ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
			}
			else 
			{
				ScrSetIcon(MENU_ICON_DOWN, MENU_OPENICON);
			}
		}

		if (iTimeout==0)
		{
			ucKey = PubWaitKey(-1);
		}
		else
		{
			ucKey = PubWaitKey((short)iTimeout);
		}

		if (ucKey == KEYCANCEL || ucKey == NOKEY)
		{
			ScrSetIcon(MENU_ICON_UP,   MENU_CLOSEICON);
			ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
			return -1;
		}

		if(uiMode & SM_ARROW_SEL) 
		{
			if(ucKey == KEYENTER) 
			{
				if(pstSmartMenu->pstContent[iHilightIndex].pstSubMenu) 
				{
					iRet = PubSmartMenuEx(pstSmartMenu->pstContent[iHilightIndex].pstSubMenu, uiMode, iTimeout);
					if(uiMode & SM_DIRECT_RETURN) 
					{
						ScrSetIcon(MENU_ICON_UP,   MENU_CLOSEICON);
						ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
						return iRet;
					}
					else if(iRet > 0)
					{
						ScrSetIcon(MENU_ICON_UP,   MENU_CLOSEICON);
						ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
						return iRet;
					}
				}
				else 
				{
					ScrSetIcon(MENU_ICON_UP,   MENU_CLOSEICON);
					ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
					return pstSmartMenu->pstContent[iHilightIndex].iValue;
				}
			}
			else if(ucKey == KEYUP) 
			{
				iHilightIndex = GetPrevHiliIndex(pstSmartMenu, iHilightIndex);
				//If Still can show 本屏还能显示
				if(iHilightIndex >= iStartIndex) 
				{
					continue;
				}
				//本屏不能显示
				else 
				{
					if(iCurScr > 0) 
					{
						for(i = iCurScr; i > 0; i++) 
						{
							if(iHilightIndex >= ScrIndex[i -1] && iHilightIndex < ScrIndex[i]) 
							{
								iCurScr = i - 1;
								break;
							}
						}
					}
					continue;
				}
			}
			else if(ucKey == KEYDOWN) 
			{
				iHilightIndex = GetNextHiliIndex(pstSmartMenu, iHilightIndex);

				for(i = iCurScr + 1; i < 10; i++) 
				{
					if(ScrIndex[i] == -1) 
					{
						iCurScr = i -1;
						break;
					}
					if(iHilightIndex >= ScrIndex[i -1] && iHilightIndex < ScrIndex[i]) 
					{
						iCurScr = i - 1;
						break;
					}   
				}
				continue;
			}
		}
		else 
		{
			if(ucKey == KEYUP) 
			{
				if(iCurScr > 0) 
				{
					iCurScr--;
				}

				iStartIndex = ScrIndex[iCurScr];
			}
			else if(ucKey == KEYDOWN || ucKey == KEYENTER) 
			{
				if(iEndIndex < pstSmartMenu->iNum) 
				{
					iCurScr++;
					iStartIndex = iEndIndex;    
				}
			}
			else if(ucKey >= ucStartKey && ucKey < ucStartKey + iScrItemNum ) 
			{
				iTargetIdx = itemList[ucKey - ucStartKey];
				if(pstSmartMenu->pstContent[iTargetIdx].pstSubMenu)
				{
					iRet = PubSmartMenuEx(pstSmartMenu->pstContent[iTargetIdx].pstSubMenu, uiMode, iTimeout);
					if(uiMode & SM_DIRECT_RETURN) 
					{
						ScrSetIcon(MENU_ICON_UP,   MENU_CLOSEICON);
						ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
						return iRet;
					}
					else if(iRet > 0)
					{
						ScrSetIcon(MENU_ICON_UP,   MENU_CLOSEICON);
						ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
						return iRet;
					}
				}
				else 
				{
					ScrSetIcon(MENU_ICON_UP,   MENU_CLOSEICON);
					ScrSetIcon(MENU_ICON_DOWN, MENU_CLOSEICON);
					return pstSmartMenu->pstContent[iTargetIdx].iValue;
				}
			}
		}
	}	
}

/********************************************************************************
 * Input Parameter: sm - Samrt menu pointer 智能菜单指针
 *        
 * Output Parameter: 无
 * Return: The first available menu. 第一有效菜单项，入无则返回-1
 *          
 ********************************************************************************/
static int GetFirstHiliIndex(SMART_MENU *pstSmartMenu)
{
	int i = 0;
	int j;
	if(pstSmartMenu->pstContent->iValue < 0)
	{
		i = 1;
	}

	for(j = i; j < pstSmartMenu->iNum; j++) 
	{
		if(pstSmartMenu->sMask[j] == SM_ON) 
		{
			return j;
		}
	}

	return -1;
}

/********************************************************************************
 * 函数名: GetNextHiliIndex
 * 功  能: 返回下一个高亮显示的菜单项的索引
 * 入  参: sm - 智能菜单指针
 *         hilight_index - 当前高亮显示的菜单的索引
 * 出  参: 无
 * 返回值: 下一个高亮显示的菜单项的索引，如果没有下一个可供高亮显示的菜单
 *          则返回当前的高亮菜单索引
 ********************************************************************************/
static int GetNextHiliIndex(SMART_MENU *pstSmartMenu, int iHilightIndex)
{
	int i = 0;

	for(i = iHilightIndex+1; i < pstSmartMenu->iNum; i++) 
	{
		if(pstSmartMenu->sMask[i] == SM_ON) 
		{
			return i;
		}
	}
	return iHilightIndex;
}

/********************************************************************************
 * 函数名: GetNextHiliIndex
 * 功  能: 返回上一个高亮显示的菜单项的索引
 * 入  参: sm - 智能菜单指针
 *         hilight_index - 当前高亮显示的菜单的索引
 * 出  参: 无
 * 返回值: 上一个高亮显示的菜单项的索引，如果没有上一个可供高亮显示的菜单
 *          则返回当前的高亮菜单索引
 ********************************************************************************/
static int GetPrevHiliIndex(SMART_MENU *pstSmartMenu, int iHilightIndex)
{
	int iLow = 0;
	int i;

	if(pstSmartMenu->pstContent[0].iValue < 0)
	{
		iLow = 1;
	}

	if(iHilightIndex == iLow) 
	{
		return iHilightIndex;
	}

	for(i = iHilightIndex -1; i >= iLow; i--) 
	{
		if(pstSmartMenu->sMask[i] == SM_ON) 
		{
			return i;
		}
	}

	return iHilightIndex;
}

void PubSmInit(SMART_MENU *pstSmartMenu, MENU_ITEM *pstMenuItem)
{
	int i=0;

	if (pstSmartMenu == NULL || pstMenuItem == NULL)
	{
		return;
	}

	pstSmartMenu->pstContent = pstMenuItem;
	pstSmartMenu->iNum = 0;
	for(i = 0; i < SM_MAX_LINE; i++) 
	{
		if (pstMenuItem[i].pszText == NULL)
		{
			return;
		}

		if(strcmp(pstMenuItem[i].pszText, "") == 0) 
		{
			break;
		}

		pstSmartMenu->sMask[i] = SM_ON;
		pstSmartMenu->iNum = i+1;
	}
}


void PubSmMask(SMART_MENU *pstSmartMenu, const char *pszName, int iFlag)
{
	int i=0;

	if (pstSmartMenu == NULL || pszName == NULL || pstSmartMenu->pstContent == NULL)
	{
		return;
	}

	for(i = 0; i < SM_MAX_LINE; i++) 
	{
		if(strcmp(pstSmartMenu->pstContent[i].pszText, "") == 0) 
		{
			break;
		}

		if(strcmp(pstSmartMenu->pstContent[i].pszText, pszName) == 0) 
		{
			if (iFlag != SM_OFF)
			{
				iFlag = SM_ON;
			}
			pstSmartMenu->sMask[i] = iFlag;
			break;
		}
	}
}

// Prepare the current screen in smart menu
// 返回下一屏第一个菜单项在sm->content中的索引（如果下一屏已经没有菜单，则返回sm->num）
// 参数 pstSmartMenu - 菜单
//      iStartIndex - 本屏第一个菜单项在sm->content中的索引
//      iHilightIndex - 需要高亮显示的菜单项在sm->content中的索引（只在SM_ARROW_SEL模式下有效）
//      uiMode-显示模式
//      ucWithDisp - 1 显示 0 不显示，只计算
//      piScrItemNum  本屏的菜单项个数，用于输出
static int prepare_screen_text(SMART_MENU *pstSmartMenu, int iStartIndex, int iHilightIndex, 
							   uint uiMode, uchar ucWithDisp, int *piScrItemNum, int * itemList) //modified by wuc 2013.12.17
{
	int iCurLine = 0;
	int iNumIndex;
	int i;
	int iCurItem;
	int iFlag = 0;
	char buf[SM_LINE_WIDTH * 2 +1];
	char buf1[SM_LINE_WIDTH * 2 +1];
	int iHilightLine;
	int iEndIndex;
	char menu_text[SM_ASCII_MAX_LINE][SM_LINE_WIDTH + 1];
	ushort nDispMode;
	ushort nReverFlag;
	int k = 0;

	memset(buf, 0x00, sizeof(buf));  
	memset(buf1, 0x00, sizeof(buf)); 

	*piScrItemNum = 0;

	for(i = 0; i < SM_ASCII_MAX_LINE; i++) 
	{
		memset(menu_text[i], 0, sizeof(menu_text[i]));
	}

	//Title 标题
	if(pstSmartMenu->pstContent[0].iValue < 0) 
	{
		if (strlen(pstSmartMenu->pstContent[0].pszText) > SM_LINE_WIDTH)
		{
			memcpy(menu_text[0], pstSmartMenu->pstContent[0].pszText, SM_LINE_WIDTH);
		}
		else
		{
			strcpy(menu_text[0], pstSmartMenu->pstContent[0].pszText);
		}
		iCurLine = 1;
	}

	if(uiMode & SM_ZERO_START)
	{
		iNumIndex = 0;
	}
	else
	{
		iNumIndex = 1;
	}

	iCurItem = iStartIndex;

	iEndIndex = pstSmartMenu->iNum;

	for(i = iCurItem; i < pstSmartMenu->iNum; i++) 
	{	
		if(pstSmartMenu->sMask[i] == SM_OFF) 
		{
			continue;
		}

		if(i == iHilightIndex)
		{
			iHilightLine = iCurLine;
		}

		itemList[k++] = i;

		if(uiMode & SM_ARROW_SEL)
		{
			sprintf(buf, "%s", pstSmartMenu->pstContent[i].pszText);
		}
		else
		{
			sprintf(buf, "%d.%s", iNumIndex, pstSmartMenu->pstContent[i].pszText);
		}

		/*处理超长的菜单项,但未考虑半个汉字问题:-(*/
		if(strlen(buf) > SM_LINE_WIDTH) 
		{
			buf[SM_LINE_WIDTH] = 0;         
		}

		iNumIndex++;

		//使用方向键选择，强制每行一条
		if((uiMode & SM_1ITEM_PER_LINE) || (uiMode & SM_ARROW_SEL)) 
		{
			strcpy(menu_text[iCurLine], buf);
			(*piScrItemNum) ++;
			iCurLine++;
			if(iCurLine == SM_ASCII_MAX_LINE) 
			{
				iEndIndex = i+1;
				break;
			}
		}
		else 
		{
			if(iFlag == 0) 
			{ //cur_line行当前无内容
				strcpy(menu_text[iCurLine], buf);
				(*piScrItemNum) ++;
				iFlag = 1;
			}
			else 
			{          //cur_line已有内容
				if(strlen(buf) + strlen(menu_text[iCurLine]) <= SM_LINE_WIDTH)
				{//要加入的项可以放入当前行
					strcpy(buf1, menu_text[iCurLine]);
					memset(menu_text[iCurLine], ' ', SM_LINE_WIDTH);
					memcpy(menu_text[iCurLine], buf1, strlen(buf1));
					memcpy(menu_text[iCurLine]+SM_LINE_WIDTH-strlen(buf), buf, strlen(buf));
					menu_text[iCurLine][SM_LINE_WIDTH] = 0;
					iCurLine++;
					(*piScrItemNum) ++;
					if(iCurLine == SM_ASCII_MAX_LINE)
					{
						iEndIndex = i+1;
						break;
					}
					iFlag = 0;
				}
				else
				{ //另起一行
					if(iCurLine == SM_ASCII_MAX_LINE - 1)
					{
						iEndIndex = i;
						break;
					}
					iCurLine++;
					strcpy(menu_text[iCurLine], buf);
					(*piScrItemNum) ++;
					iFlag = 1;
				}
			}
		}
	}

	if(ucWithDisp) 
	{
		for(i = 0; i < SM_ASCII_MAX_LINE; i++) 
		{
			if(pstSmartMenu->pstContent[0].iValue < 0 && i ==0) 
			{	
				nDispMode = 0;
				if (uiMode & SM_MIDDLE_TITLE)
				{
					nDispMode |= DISP_LINE_CENTER;
				}

				PubDispString(menu_text[i],(ushort)((i)|nDispMode));
			}
			else
			{
				nDispMode = 0;
				if (uiMode & SM_MIDDLE_ITEM)
				{
					nDispMode |= DISP_LINE_CENTER;
				}

				if ((uchar)((i == iHilightLine) && (uiMode & SM_ARROW_SEL)))
				{
					nReverFlag = DISP_PART_REVER;
				}
				else
				{
					nReverFlag = 0;
				}

				PubDispString(menu_text[i],(ushort)((i)|nDispMode|nReverFlag));
			}
		}
	}

	//返回下一个有效菜单索引，如果没有就返回sm->num
	for(i = iEndIndex ; i < pstSmartMenu->iNum; i++) 
	{
		if(pstSmartMenu->sMask[i] == SM_ON)
		{
			break;
		}
	}

	return i;
}


uchar PubYesNo(ushort nWaitTime)
{
	uchar   ucTmp;
	
	while (1)
	{
		ucTmp = PubWaitKey(nWaitTime);
		if (ucTmp==KEYENTER)
		{
			return  0;
		}
		if (ucTmp==KEYCANCEL || ucTmp==NOKEY)
		{
			return  0xFF;
		}
	}
}

uchar PubGetAmount(uchar *pszPrefix, uchar ucDeciPos,
				   uchar ucMinLen, uchar ucMaxLen, uchar *pszData,
				   uchar ucTimeout, uchar ucMisc)
{
	uchar	*pszNowChar, ucKeyVal;//, bSeparator, bNegative
	uchar	szBuff[20], szDispBuf[20], szDispBuf2[30];//, szTrunBuf[10];	//buffer to display
	int		iTotalLen;
	
	if (ucDeciPos>5 || ucMinLen>ucMaxLen || ucMaxLen>12 ||pszData==NULL)
	{
		return 0xFE;		//parameter illegal
	}
	
	memset(szBuff, 0, sizeof(szBuff));
	memset(szDispBuf, 0, sizeof(szDispBuf));
	memset(szDispBuf2, 0, sizeof(szDispBuf2));
	
	//bSeparator = ucMisc & GA_SEPARATOR;		//whether use ',' to separate huge amount
	//bNegative  = ucMisc & GA_NEGATIVE;		//whether display negative amount
	pszNowChar = szBuff;
	
	while (1)
	{
		/* 显示 */
		memset(szDispBuf2, 0, sizeof(szDispBuf2));
		if (pszPrefix!=NULL && strlen((char *)pszPrefix)>0)
		{
			sprintf((char *)szDispBuf2, "%.4s", (char *)pszPrefix);
		}
		
		//protocol
		PubConvAmount(NULL, szBuff, ucDeciPos, 0, szDispBuf, ucMisc);

		DebugOutput("%s--%d, szBuff:%s, szDispBuf:%s, szDispBuf2:%s\n",
					__FILE__, __LINE__, szBuff, szDispBuf, szDispBuf2);
		
		iTotalLen = strlen((char *)szDispBuf) + strlen((char *)szDispBuf2);
		if (iTotalLen < 17)
		{
			strcat((char *)szDispBuf2, (char *)szDispBuf);
			ScrPrint(0, 3, ASCII, "%16.16s", szDispBuf2);
		}
		else
		{
			strcat((char *)szDispBuf2, (char *)szDispBuf);
			ScrPrint(0, 3, ASCII, "%21.21s", szDispBuf2);
		}
		
		/* 处理按键 */
		if (ucTimeout==0)
		{
			ucKeyVal = getkey();
		}
		else
		{
			ucKeyVal = PubWaitKey(ucTimeout);
		}
		
		switch(ucKeyVal)
		{
		case KEY0:
			if (pszNowChar==szBuff || pszNowChar>=szBuff+ucMaxLen)
			{
				PubBeepErr();
			}
			else
			{
				*pszNowChar++ = ucKeyVal;
				*pszNowChar   = 0;
			}
			break;
		case KEY1:
		case KEY2:
		case KEY3:
		case KEY4:
		case KEY5:
		case KEY6:
		case KEY7:
		case KEY8:
		case KEY9:
			if (pszNowChar>=szBuff+ucMaxLen)
			{
				PubBeepErr();
			}
			else
			{
				*pszNowChar++ = ucKeyVal;
				*pszNowChar   = 0;
			}
			break;
#ifdef _P80_
		case KEYCLEAR:
			pszNowChar  = szBuff;
			*pszNowChar = 0;
			break;
			
		case KEYF5:	// backspace
#else
		case KEYCLEAR:
#endif
			if (pszNowChar<=szBuff)
			{
				PubBeepErr();
			}
			else
			{
				*(--pszNowChar) = 0;
			}
			break;
			
		case KEYENTER:
			if (pszNowChar>=szBuff+ucMinLen)
			{
				*pszData = 0;
				strcpy((char *)pszData, (char *)szBuff);
				if (strlen((char *)szBuff)==0)
				{
					strcpy((char *)pszData, "0");
				}
				return 0;
			}
			else
			{
				PubBeepErr();
			}
			break;
		case KEYCANCEL:
		case NOKEY:
			return 0xFF;

		case KEYATM1:
		case KEYATM2:
		case KEYATM3:
		case KEYATM4:
			return ucKeyVal;
			
		default:
			PubBeepErr();
			break;
		}
	}
}


uchar PubGetBoolParam(uchar *pszPrompt, uchar ucLine, uchar *pucResult)
{
	uchar	ucKey, ucFlag;
	
	if (pszPrompt!=NULL && *pszPrompt!=0)
	{
		ScrClrLine(2, NUM_MAXLINENO);
		ScrPrint(0, ucLine, CFONT, "%.*s", NUM_MAXZHCHARS, (char *)pszPrompt);
	}
	
	ucFlag = *pucResult;
	while(1)
	{
		PubShowMsg(NUM_MAXLINENO-1, (uchar *)(ucFlag ? "[YES]" : "[NO]"));
		ucKey = PubWaitKey(60);
		if (ucKey==KEYENTER)
		{
			*pucResult = ucFlag;
			return 0;
		}
		if (ucKey==KEYCANCEL || ucKey==NOKEY)
		{
			return 0xFF;
		}
		if ((ucKey==KEYCLEAR)
			|| (ucKey==KEYALPHA)
			|| (ucKey>=KEY0 && ucKey<=KEY9))
		{
			ucFlag = !ucFlag;
		}
	}   // end of while(1)
}

void MapChar2Index(uchar ch, int *piRow, int *piCol)
{
	int		i;
	char	*p;
	
	for(i=0; i<sizeof(pszAlphaTbl)/sizeof(pszAlphaTbl[0]); i++)
	{
		for(p=pszAlphaTbl[i]; *p; p++)
		{
			if( *p==toupper(ch) )
			{
				*piRow = i;
				*piCol = p-pszAlphaTbl[i];
				break;
			}
		}
	}
	DebugOutput("%s--%d\n", __FILE__, __LINE__);//linzhao
}

void ShowWorkBuf(uchar ucMode, uchar *pszWorkBuff, int iLen)
{
	int		iCnt, iLeftBytes;
	uchar	ucLineNo, sTemp[32];
	
	memset(sTemp, '*', sizeof(sTemp));
	ucLineNo = 3;
	ScrClrBelow(ucLineNo);
	for(iCnt=1; iCnt<=3; iCnt++)
	{
		if( (iCnt*NUM_MAXZHCHARS)<=iLen )
		{
			iLeftBytes = NUM_MAXZHCHARS;
			if( ucMode & PASS_IN )
			{
				ScrPrint(0, ucLineNo, ASCII, "%.*s", iLeftBytes, sTemp);
			}
			else
			{
				ScrPrint(0, ucLineNo, ASCII, "%.*s", iLeftBytes, pszWorkBuff + iLen - iCnt*NUM_MAXZHCHARS);
			}

		}
		else
		{
			iLeftBytes = iLen - (iCnt-1)*NUM_MAXZHCHARS;
			if( iLeftBytes<=0 && iCnt>1 )
			{
				break;
			}
			if( ucMode & PASS_IN )
			{
				ScrPrint(0, ucLineNo, ASCII, "%*.*s", NUM_MAXZHCHARS, iLeftBytes, sTemp);
			}
			else
			{
				ScrPrint(0, ucLineNo, ASCII, "%*.*s", NUM_MAXZHCHARS, iLeftBytes, pszWorkBuff);
			}

			break;
		}
		ucLineNo += 2;
	}
}

// Get string/numeric string
uchar PubGetString(uchar ucMode, uchar ucMinlen, uchar ucMaxlen, uchar *pszOut, uchar ucTimeOut)
{
	uchar	ucKey, bClearInData, szWorkBuff[LEN_GETSTRING+1];
	int		iLen, iIndex, iLastKey;
	
	if (ucMaxlen < ucMinlen)
	{
		return 0xFF;
	}
	
	PubASSERT( (ucMode & NUM_IN) || (ucMode & PASS_IN) || (ucMode & ALPHA_IN) );
	PubASSERT( !((ucMode & CARRY_IN) && (ucMode & ECHO_IN)) );
	if( (ucMode & CARRY_IN) || (ucMode & ECHO_IN) )
	{
		sprintf((char *)szWorkBuff, "%.*s", LEN_GETSTRING, pszOut);
	}
	else
	{
		memset(szWorkBuff, 0, sizeof(szWorkBuff));
	}
	
	DebugOutput("%s--%d, szWorkbuff:%s", __FILE__, __LINE__, szWorkBuff);//linzhao
	iLen = strlen((char *)szWorkBuff);
	if( iLen>0 )
	{
		MapChar2Index(szWorkBuff[iLen-1], &iLastKey, &iIndex);
	}
	else
	{
		iLastKey = -1;
		iIndex   = 0;
	}
	
	sgMinLineNo  = 0xFF;
	bClearInData = (ucMode & ECHO_IN);
	while( 1 )
	{
		ShowWorkBuf(ucMode, szWorkBuff, iLen);
		ucKey = PubWaitKey(ucTimeOut);
		if( ucKey==KEYCANCEL ||ucKey==NOKEY )
		{
			return 0xFF;
		}
		else if( ucKey==KEYENTER )
		{
			bClearInData = FALSE;
			if( iLen<(int)ucMinlen )
			{
				continue;
			}
			sprintf((char *)pszOut, "%s", szWorkBuff);
			break;
		}
		else if( ucKey>=KEY0 && ucKey<=KEY9 )
		{
			if( bClearInData )
			{	// clear in buffer
				szWorkBuff[0] = 0;
				iLen          = 0;
				iLastKey      = -1;
				bClearInData  = FALSE;
			}
			// save key in data
			if( iLen<(int)ucMaxlen )
			{
				szWorkBuff[iLen++] = ucKey;
				szWorkBuff[iLen]   = 0;
				iLastKey = ucKey - KEY0;
				iIndex   = 0;
			}
		/*	if( !(ucMode & ALPHA_IN) || iLastKey<0 || iLen<1 )
			{
				continue;
			}
			iIndex = (iIndex+1)%strlen(pszAlphaTbl[iLastKey]);
			szWorkBuff[iLen-1] = pszAlphaTbl[iLastKey][iIndex];*/
		}
#ifdef _P80_
		else if( ucKey==KEYCLEAR )
		{
			szWorkBuff[0] = 0;
			iLen          = 0;
			iLastKey      = -1;
		}
		else if( ucKey==KEYF5 )	// backspace
#else
			else if( ucKey==KEYCLEAR )
#endif
		{
			bClearInData = FALSE;
			if( iLen<=0 )
			{
				continue;
			}
			szWorkBuff[--iLen] = 0;
			if( iLen>0 )
			{
				MapChar2Index(szWorkBuff[iLen-1], &iLastKey, &iIndex);
			}
			else
			{
				iLastKey = -1;
				iIndex   = 0;
			}
		}
	#ifndef _TERMINAL_DXX_
		else if(ucKey==KEYALPHA) 
		{
			bClearInData = FALSE;
			if( !(ucMode & ALPHA_IN) || iLastKey<0 || iLen<1 )
			{
				continue;
			}
			iIndex = (iIndex+1)%strlen(pszAlphaTbl[iLastKey]);
			szWorkBuff[iLen-1] = pszAlphaTbl[iLastKey][iIndex];
		}
	#endif
	}
	
	return 0;
}


uchar PubChkCardNo(uchar *pszPanNo)
{
	uchar	bFlag, ucTemp, ucResult;
	uchar	*pszTemp;
	
	if ((pszPanNo == NULL) || (strlen((char*)pszPanNo) == 0))
	{
		return 1;
	}

	// (2121算法)
	bFlag    = FALSE;
	pszTemp  = &pszPanNo[strlen((char *)pszPanNo)-1];
	ucResult = 0;
	while( pszTemp>=pszPanNo )
	{
		ucTemp = (*pszTemp--) & 0x0F;
		if( bFlag )    ucTemp *= 2;
		if( ucTemp>9 ) ucTemp -= 9;
		ucResult = (ucTemp + ucResult) % 10;
		bFlag = !bFlag;
	}
	
	if( ucResult!=0 )
	{
		return 1;
	}
	
	return 0;
}

////////////////////////////////////// Time calculation //////////////////////////////////////

uchar PubIsValidTime(uchar *psDateTime,uchar *psMaskYYYYMMDDhhmmss)
{
#define ERROR_MASK_DATATIME  9
	
	const uchar ucMonthDay[12]={31,29,31,30,31,30,31,31,30,31,30,31};
	uint  uiYear=0, uiMonth=0, uiDay=0, uiHour=0, uiMinute=0, uiSecond=0;
	uchar ucOffset=0;
	
	if (psMaskYYYYMMDDhhmmss==NULL)
	{
		return ERROR_MASK_DATATIME;							//Mask error
	}

	if (strlen(psMaskYYYYMMDDhhmmss)==0 || strlen(psMaskYYYYMMDDhhmmss)>14 || (strlen(psMaskYYYYMMDDhhmmss)%2)==1)
	{
		return ERROR_MASK_DATATIME;							//Mask error
	}
	
	//processing Year
	if (memcmp(psMaskYYYYMMDDhhmmss,"YYYY",4)==0)
	{
		uiYear = PubAsc2Long(psDateTime,4,NULL);
		ucOffset+=4; 
		if (uiYear<1900 || uiYear>2099)
		{
			return 1;										//Year error
		}
	}
	else if (memcmp(psMaskYYYYMMDDhhmmss,"YY",2)==0)
	{
		uiYear = PubAsc2Long(psDateTime,2,NULL);
		uiYear += 2000;
		ucOffset += 2; 
	}
	
	//processing Month
	if (psMaskYYYYMMDDhhmmss[ucOffset] == 0)
	{
		return 0;		//Finished and return OK
	}
	if (uiYear !=0)
	{
		if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"MM",2) !=0)
		{
			return ERROR_MASK_DATATIME;	//Mask error
		}
	}
	if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"MM",2) ==0)
	{
		uiMonth = PubAsc2Long(psDateTime+ucOffset,2,NULL);
		ucOffset += 2;
		if (uiMonth==0 || uiMonth>12)
		{
			return 2;										//Month error
		}
	}
	
	//processing Day
	if (psMaskYYYYMMDDhhmmss[ucOffset] == 0)
	{
		return 0;		//Finished and return OK
	}
	if (uiMonth !=0)
	{
		if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"DD",2) !=0)
		{
			return ERROR_MASK_DATATIME;	//Mask error
		}
	}
	if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"DD",2) ==0)
	{
		uiDay = PubAsc2Long(psDateTime+ucOffset,2,NULL);
		ucOffset += 2;
		if (uiDay==0 || uiDay>31)
		{
			return 3;										//Day error
		}
		if (uiMonth != 0)
		{
			if (uiDay > ucMonthDay[uiMonth-1])
			{
				return 3;										//Day error
			}
			if (uiYear!=0 && uiMonth==2 && uiDay==29)
			{
				if (IsLeapYear(uiYear)==0)
				{
					return 3;										//Day error
				}
			}
		}
	}
	
	//processing Hour
	if (psMaskYYYYMMDDhhmmss[ucOffset] == 0) return 0;		//Finished and return OK
	if (uiDay !=0)
	{
		if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"hh",2) !=0) return ERROR_MASK_DATATIME;	//Mask error
	}
	if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"hh",2) ==0)
	{
		uiHour = PubAsc2Long(psDateTime+ucOffset,2,NULL);
		ucOffset += 2;
		if (uiHour>23)
		{
			return 4;										//Hour error
		}
		uiHour++;		//for not equal 0
	}
	
	//processing Minite
	if (psMaskYYYYMMDDhhmmss[ucOffset] == 0)
	{
		return 0;		//Finished and return OK
	}
	if (uiHour !=0)
	{
		if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"mm",2) !=0)
		{
			return ERROR_MASK_DATATIME;	//Mask error
		}
	}
	if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"mm",2) ==0)
	{
		uiMinute = PubAsc2Long(psDateTime+ucOffset,2,NULL);
		ucOffset += 2;
		if (uiMinute>59)
		{
			return 5;										//Minute error
		}
		uiMinute++;		//for not equal 0
	}
	
	//processing Second
	if (psMaskYYYYMMDDhhmmss[ucOffset] == 0)
	{
		return 0;		//Finished and return OK
	}
	if (uiMinute !=0)
	{
		if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"ss",2) !=0)
		{
			return ERROR_MASK_DATATIME;	//Mask error
		}
	}
	if (memcmp(psMaskYYYYMMDDhhmmss+ucOffset,"ss",2) ==0)
	{
		uiSecond = PubAsc2Long(psDateTime+ucOffset,2,NULL);
		ucOffset += 2;
		if (uiSecond>59)
		{
			return 6;										//Minute error
		}
		uiSecond++;		//for not equal 0
	}
 
	if (ucOffset==0)
	{
		return ERROR_MASK_DATATIME;		//Mask error
	}
	return 0;
}


ulong PubDay2Long(uchar *psYYYYMMDD)
{
	uint uiYear,uiMonth,uiDate,uiNowYear;
	
	ulong ulRet;
	
	if (psYYYYMMDD==NULL)
	{
		return 0;
	}

	uiNowYear=PubAsc2Long(&psYYYYMMDD[0],4,NULL);
	
	uiYear=uiNowYear-STAR_YEAR;
	
	/* Leap year */
	//ulRet=uiYear*365l+(uiYear/4)+(uiYear/400)-(uiYear/100);
	ulRet=(uiNowYear/4)-(STAR_YEAR/4);
	ulRet=ulRet+(STAR_YEAR/100)-(uiNowYear/100);
	ulRet=ulRet+(uiNowYear/400)-(STAR_YEAR/400);
	
	
	ulRet=uiYear*365l+ulRet;
	uiMonth=PubAsc2Long(&psYYYYMMDD[4],2,NULL);
	
	uiDate=uiConstMonthday[uiMonth-1];
	ulRet=ulRet+uiDate;
	if(IsLeapYear(uiNowYear))
	{
		if(IsLeapYear(STAR_YEAR))
		{
			if(uiMonth>=3)
			{
				/* Leap year */
				ulRet=ulRet+1;
			}
		}
		else
		{
			if(uiMonth<3)
			{
				/* End year is leap year, But start year isn't */
				ulRet=ulRet-1;
			}
			
		}
	}
	else if(IsLeapYear(STAR_YEAR))
	{
		/* End year isn't leap year, But start year is */
		ulRet=ulRet+1;
	}
	
	uiDate=PubAsc2Long(&psYYYYMMDD[6],2,NULL);
	uiDate=uiDate-1;
	ulRet=ulRet+uiDate;
	return ulRet;
}

ulong PubTime2Long(uchar *psYYYYMMDDhhmmss)
{
	ulong ulRet;
	ulong ulTemp;
	
	if (psYYYYMMDDhhmmss==NULL)
	{
		return 0;
	}

	ulRet=PubDay2Long(psYYYYMMDDhhmmss);
	ulRet=ulRet*24;
	
	ulTemp=PubAsc2Long(&psYYYYMMDDhhmmss[8],2,NULL);
	ulRet=ulRet+ulTemp;
	/* convert to minute */
	ulRet=ulRet*60l;
	
	ulTemp=PubAsc2Long(&psYYYYMMDDhhmmss[10],2,NULL);
	ulRet=ulRet+ulTemp;
	/* convert to seconds */
	ulRet=ulRet*60l;
	
	ulTemp=PubAsc2Long(&psYYYYMMDDhhmmss[12],2,NULL);
	ulRet=ulRet+ulTemp;
	return ulRet;	
}

/* Check if leap year or not */
static uchar IsLeapYear(ulong ulYear)
{
	if((ulYear%400)==0)
	{
		return 1;
	}
	else if((ulYear%100)==0)
	{
		return 0;
	}
	else if((ulYear%4)==0)
	{
		return 1;
	}
	return 0;		
}	

void PubLong2Day(ulong ulDay,uchar *pszYYYYMMDD)  
{
	ulong ulYear,ulMonth;
	
	if(pszYYYYMMDD==NULL)
	{
		return;
	}

	ulYear=0;
	while(ulDay>365)
	{
		if(IsLeapYear(ulYear+STAR_YEAR))
		{
			ulDay--;
		}
		ulYear++;
		ulDay-=365;
	}
	if(ulDay==365)
	{
		if(IsLeapYear(ulYear+STAR_YEAR))
		{
			ulDay=31;
			ulMonth=12;
		}
		else
		{
			ulYear++;
			ulDay=1;
			ulMonth=1;
		}
		
	}
	else
	{
		for(ulMonth=1;ulMonth<=11;ulMonth++)
		{
			if(ulMonth==3)
			{
				if(IsLeapYear(ulYear+STAR_YEAR))
				{
					if(ulDay==uiConstMonthday[2])
					{
						ulMonth=2;
						break;
					}
					ulDay--;
				}
			}
			if(ulDay<uiConstMonthday[ulMonth])
			{
				break;
			}
		}
		ulDay=ulDay-uiConstMonthday[ulMonth-1];
		ulDay++;
	}
	sprintf(&pszYYYYMMDD[0],"%04lu",ulYear+STAR_YEAR);
	sprintf(&pszYYYYMMDD[4],"%02lu",ulMonth);
	sprintf(&pszYYYYMMDD[6],"%02lu",ulDay);
	pszYYYYMMDD[8]=0;
} 

void PubLong2Time(ulong ulSecond,uchar *pszYYYYMMDDhhmmss)   
{
	ulong ulYear,ulMonth,ulDay,ulDaySecond,ulWeek;
	
	if (pszYYYYMMDDhhmmss==NULL)
	{
		return;
	}

	ulDay=ulSecond/(24*3600l);
	ulWeek=(ulDay+STAR_WEEK)%7;
	ulWeek++;
	ulDaySecond=ulSecond%(24*3600);
	ulYear=0;
	while(ulDay>365)
	{
		if(IsLeapYear(ulYear+STAR_YEAR))
		{
			ulDay--;
		}
		ulYear++;
		ulDay-=365;
	}
	if(ulDay==365)
	{
		if(IsLeapYear(ulYear+STAR_YEAR))
		{
			ulDay=31;
			ulMonth=12;
		}
		else
		{
			ulYear++;
			ulDay=1;
			ulMonth=1;
		}
		
	}
	else
	{
		for(ulMonth=1;ulMonth<=11;ulMonth++)
		{
			if(ulMonth==3)
			{
				if(IsLeapYear(ulYear+STAR_YEAR))
				{
					if(ulDay==uiConstMonthday[2])
					{
						ulMonth=2;
						break;
					}
					ulDay--;
				}
			}
			if(ulDay<uiConstMonthday[ulMonth])
			{
				break;
			}
		}
		ulDay=ulDay-uiConstMonthday[ulMonth-1];
		ulDay++;
	}
	sprintf(&pszYYYYMMDDhhmmss[0],"%04lu",ulYear+STAR_YEAR);
	sprintf(&pszYYYYMMDDhhmmss[4],"%02lu",ulMonth);
	sprintf(&pszYYYYMMDDhhmmss[6],"%02lu",ulDay);
	sprintf(&pszYYYYMMDDhhmmss[8],"%02lu",ulDaySecond/3600);
	ulDaySecond=ulDaySecond%3600;
	sprintf(&pszYYYYMMDDhhmmss[10],"%02lu",ulDaySecond/60);
	ulDaySecond=ulDaySecond%60;
	sprintf(&pszYYYYMMDDhhmmss[12],"%02lu",ulDaySecond);
	//	sprintf(&pszYYYYMMDDhhmmss[14],"%02lu",ulWeek); //no need week yet
	pszYYYYMMDDhhmmss[14]=0;
}

void PubCalDateTime(uchar *psDateTimeIn, uchar *pszDateTimeOut, ulong ulInterval, uchar *psCalMode)
{//YYYYMMDDhhmmss format
	ulong ulYear,ulMonth,ulTemp;
	
	if (psDateTimeIn==NULL || pszDateTimeOut==NULL || psCalMode == NULL)
	{
		return;
	}

	if (psCalMode!=NULL)
	{
		switch(psCalMode[0])
		{
		case 'Y':
			ulYear = PubAsc2Long(psDateTimeIn,4,NULL);
			sprintf(pszDateTimeOut,"%04lu",ulYear+ulInterval);
			memcpy(pszDateTimeOut+4,psDateTimeIn+4,10);   
			pszDateTimeOut[14]=0;
			return;  //not break but return
		case 'M':
			ulYear = PubAsc2Long(psDateTimeIn,4,NULL);
			ulMonth = PubAsc2Long(psDateTimeIn+4,2,NULL);
			ulYear = ulYear + (ulMonth - 1 + ulInterval) / 12;
			ulMonth = (ulMonth -1 + ulInterval) % 12 + 1;
			sprintf(pszDateTimeOut,"%04lu",ulYear);
			sprintf(pszDateTimeOut+4,"%02lu",ulMonth);
			memcpy(pszDateTimeOut+6,psDateTimeIn+6,8);   
			pszDateTimeOut[14]=0;
			return;  //not break but return
		case 'D':
			ulInterval = ulInterval*3600*24;
			break;
		case 'h':
			ulInterval = ulInterval*3600;
			break;
		case 'm':
			ulInterval = ulInterval*60;
			break;
		case 's':
		default:
			break;
		}
	}
	
	ulTemp = PubTime2Long(psDateTimeIn)+ulInterval;
	PubLong2Time(ulTemp,pszDateTimeOut);
}

ulong PubCalInterval(uchar *psTimeBegin, uchar *psTimeEnd, uchar *psCalMode, uchar *pucSignal)
{
	ulong ulBegin,ulEnd,ulRet;
	uchar *psBig,*psSmall;

	if (psTimeBegin==NULL || psTimeEnd==NULL || psCalMode==NULL || pucSignal==NULL)
	{
		return 0;
	}

	*pucSignal = 0;
	ulBegin = PubTime2Long(psTimeBegin);
	ulEnd   = PubTime2Long(psTimeEnd);
	
	if (ulEnd >= ulBegin)
	{
		*pucSignal = 0;
		ulRet = ulEnd - ulBegin;
		psBig   = psTimeEnd;
		psSmall = psTimeBegin;
	}
	else
	{
		*pucSignal = 1;
		ulRet = ulBegin - ulEnd;
		psBig   = psTimeBegin;
		psSmall = psTimeEnd;
	}
	
 
	switch(psCalMode[0])
	{
	case 'Y':
		ulRet = PubAsc2Long(psBig,4,NULL) - PubAsc2Long(psSmall,4,NULL);
		if (memcmp(psBig+4,psSmall+4,10)<0)
		{
			ulRet--;
		}
		return ulRet; //not break but return
	case 'M':
		ulRet =  PubAsc2Long(psBig,4,NULL)*12 + PubAsc2Long(psBig+4,2,NULL) 
			- PubAsc2Long(psSmall,4,NULL)*12 - PubAsc2Long(psSmall+4,2,NULL);
		if (memcmp(psBig+6,psSmall+6,8)<0)
		{
			ulRet--;
		}
		return ulRet; //not break but return
	case 'D':
		ulRet = ulRet/3600/24;
		break;
	case 'h':
		ulRet = ulRet/3600;
		break;
	case 'm':
		ulRet = ulRet/60;
		break;
	case 's':
	default:
		break;
	}
	
	return ulRet;
}


////////////////////////////////////// Display //////////////////////////////////////


void PubShowTitle(uchar bClrScreen, uchar *pszTitle)
{
	int	 iLen;
	
	PubASSERT(pszTitle!=NULL);
	if (pszTitle==NULL)
	{
		return;
	}
	
	if (bClrScreen)
	{
		ScrCls();
	}
	
	iLen = strlen((char *)pszTitle);
	if (iLen > NUM_MAXZHCHARS)
	{
		iLen = NUM_MAXZHCHARS;
	}
	
	ScrPrint(0, 0, ASCII|REVER, "%*s", NUM_MAXZHCHARS, "");
#ifdef _TERMINAL_DXX_
	ScrPrint((uchar)((NUM_MAXCOLS-24*iLen)/2), 0, ASCII, "%.*s", iLen, (char *)pszTitle);
#else
	ScrPrint((uchar)((NUM_MAXCOLS-8*iLen)/2), 0, ASCII|REVER, "%.*s", iLen, (char *)pszTitle);
#endif
}


void PubShowMsg(uchar ucLine, uchar *pszMsg)
{
	int		iLen;
	uchar	ucMode;
	
	PubASSERT(pszMsg!=NULL);
	if (pszMsg==NULL)
	{
		return;
	}
	
	ucMode =  (ucLine & REVER) | CFONT;
	ucLine &= ~REVER;
	ScrClrLine(ucLine, (uchar)(ucLine+1));
	
	iLen = strlen((char *)pszMsg);
	if (iLen > NUM_MAXZHCHARS)
	{
		iLen = NUM_MAXZHCHARS;
	}
	
	ScrPrint((uchar)((NUM_MAXCOLS-8*iLen)/2), ucLine, ucMode,
		"%.*s", iLen, (char *)pszMsg);
}


void PubShowTwoMsg(uchar ucLine, uchar *pszMsg1, uchar *pszMsg2)
{
	if (ucLine>4)
	{
		ucLine = 4;
	}
	PubShowMsg(ucLine, pszMsg1);
	PubShowMsg((uchar)(ucLine+2), pszMsg2);
}

//api.c--apiDispString
//...
void PubDispString(void *pszStr, ushort nPosition)
{
	uchar mode, x, y, ucLen, buffer[60];
	uchar ucReverFlag;
	
	if (pszStr == NULL)
	{
		return;
	}
	
	mode  = nPosition & 0x00f0;
	y     = nPosition & 0x000f;

	ucLen = strlen(pszStr);
	if (ucLen > NUM_MAXZHCHARS)
	{
		ucLen = NUM_MAXZHCHARS;
	}
	
	switch(mode)
	{
	case DISP_LINE_CENTER:
		ScrClrLine(y, (uchar) (y + 1));
	#ifdef _TERMINAL_DXX_
		x = (160 - ucLen * 6);
	#else
		x = (64 - ucLen * 3);
	#endif
		break;
		
	case DISP_LINE_RIGHT:
	#ifdef  _TERMINAL_DXX_
		x = (320 - ucLen * 12);
	#else
		x = (128 - ucLen * 6);
	#endif
		break;
		
	case DISP_LINE_LEFT:
	default:
		x = 0;
		break;
	}

	if (nPosition&DISP_SCR_CENTER)
	{
		ScrCls();
	#ifdef _TERMINAL_DXX_
		x = (160 - ucLen * 6);
		y = 8;
	#else
		x = (64 - ucLen * 3);
		y = 3;
	#endif
	}
	ucReverFlag = 0;
	if (nPosition&DISP_LINE_REVER || nPosition&DISP_PART_REVER)
	{
		ucReverFlag = REVER;
	}

	memset(buffer, 0, sizeof(buffer));
	memcpy(buffer, pszStr, ucLen);
	ScrClrLine(y, (uchar)(y+1));
	if (nPosition&DISP_LINE_REVER)
	{
		ScrPrint(0, y, ASCII|REVER, "%*s", NUM_MAXZHCHARS, "");
	}
	ScrPrint(x, y, (uchar)(ASCII|ucReverFlag), "%s", (char *)buffer);
}


//...
void PubHalt(uchar *pszfile, uint uiLine)
{
	int iErr;
	
	ScrCls();
	iErr = GetLastError();
	
	ScrPrint(0, 0, ASCII, "File:%s,\nLine:%d,\nRet:%d", (char *)pszfile, uiLine, iErr);
	ScrPrint(0, 4, CFONT, "PLS RECORD");
	ScrPrint(0, 4, CFONT, "THEN RESTART POS");
	
	while (1)
	{
	};
}

////////////////////////////////////// Sound //////////////////////////////////////

void PubLongBeep(void)
{
	Beef(6, 800);
}


void PubBeepOk(void)
{
	uchar	i;
	
	for (i=0; i<3; i++)
	{
		Beef(6, 60);
		DelayMs(80);
	}
}


void PubBeepErr(void)
{
	Beef(1, 200);
	DelayMs(200);
}

////////////////////////////////////// File I/O //////////////////////////////////////


int PubFReadN(int fd, void *psData, int iNeedBytes)
{
	int		iReadBytes, iLeftBytes;
	uchar	*psTmp;
	
	PubASSERT( fd>=0 && psData!=NULL && iNeedBytes>=0 );
	psTmp      = (uchar *)psData;
	iLeftBytes = iNeedBytes;
	while(iLeftBytes>0)
	{
		iReadBytes = read(fd, psTmp, iLeftBytes);
		if (iReadBytes<=0)
		{
			PubTRACE1("errno:%d", errno);
			break;
		}
		iLeftBytes -= iReadBytes;
		psTmp	   += iReadBytes;
	}
	
	return (iNeedBytes-iLeftBytes);
}


uchar PubFileRead(uchar *pszFileName, long lOffset, void *psData, int iLen)
{
	int		iRet, fd, iReadBytes;
	
	PubASSERT( pszFileName!=NULL&& *pszFileName!=0 && iLen>0 );
	fd = open((char *)pszFileName, O_RDWR);
	if (fd<0)
	{
		PubTRACE1("open:%s", pszFileName);
		PubTRACE1("errno:%d", errno);
		return 1;
	}
	
	iRet = seek(fd, lOffset, SEEK_SET);
	if (iRet<0)
	{
		close(fd);
		PubTRACE2("seek:%s,%ld", pszFileName, lOffset);
		PubTRACE1("errno:%d", errno);
		return 2;
	}
	
	iReadBytes = PubFReadN(fd, psData, iLen);
	close(fd);
	if (iReadBytes!=iLen)
	{
		return 3;
	}
	
	return 0;
}


int PubFWriteN(int fd, void *psData, int iNeedBytes)
{
	int		iWriteBytes, iLeftBytes;
	uchar   *psTmp;
	
	PubASSERT( fd>=0 && psData!=NULL && iNeedBytes>=0 );
	psTmp      = (uchar *)psData;
	iLeftBytes = iNeedBytes;
	
	while(iLeftBytes>0)
	{
		iWriteBytes = write(fd, psTmp, iLeftBytes);
		if (iWriteBytes<=0)
		{
			PubTRACE2("FWN:%d,%d", iWriteBytes, iLeftBytes);
			PubTRACE1("errno:%d", errno);
			break;
		}
		iLeftBytes -= iWriteBytes;
		psTmp	   += iWriteBytes;
	}
	
	return (iNeedBytes-iLeftBytes);
}


uchar PubFileWrite(uchar *pszFileName, long lOffset, void *psData, int iLen)
{
	int	 iRet, fd, iWriteBytes;
	
	PubASSERT( pszFileName!=NULL && *pszFileName!=0 && iLen>=0 );
	fd = open((char *)pszFileName, O_RDWR|O_CREATE);
	if (fd<0)
	{
		PubTRACE1("open:%s", pszFileName);
		PubTRACE1("errno:%d", errno);
		return 1;
	}
	
	iRet = seek(fd, lOffset, SEEK_SET);
	if (iRet<0)
	{
		close(fd);
		PubTRACE2("seek:%s,%ld", pszFileName, lOffset);
		PubTRACE1("errno:%d", errno);
		return 2;
	}
	
	iWriteBytes = PubFWriteN(fd, psData, iLen);
	close(fd);
	if (iWriteBytes!=iLen)
	{
		PubTRACE2("FW:%d,%d", iWriteBytes, iLen);
		return 3;
	}
	
	return 0;
}

////////////////////////////////////// 计算 //////////////////////////////////////


void PubDes(uchar ucMode, uchar *psData, uchar *psKey, uchar *psResult)
{
	uchar   sTmp[8];
	
	PubASSERT(ucMode==ONE_ENCRYPT  || ucMode==ONE_DECRYPT ||
		ucMode==TRI_ENCRYPT  || ucMode==TRI_DECRYPT ||
		ucMode==TRI_ENCRYPT3 || ucMode==TRI_DECRYPT3);
	
	if ((psData == NULL) || (psKey == NULL) || (psResult == NULL))
	{
		return;
	}

	switch(ucMode)
	{
	case ONE_ENCRYPT:
		des(psData, psResult, psKey, ENCRYPT);
		break;
		
	case ONE_DECRYPT:
		des(psData, psResult, psKey, DECRYPT);
		break;
		
	case TRI_ENCRYPT:
		des(psData,   psResult, psKey,   ENCRYPT);
		des(psResult, sTmp,     psKey+8, DECRYPT);
		des(sTmp,     psResult, psKey,   ENCRYPT);
		break;
		
	case TRI_DECRYPT:
		des(psData,   psResult, psKey,   DECRYPT);
		des(psResult, sTmp,     psKey+8, ENCRYPT);
		des(sTmp,     psResult, psKey,   DECRYPT);
		break;
		
	case TRI_ENCRYPT3:
		des(psData,   psResult, psKey,    ENCRYPT);
		des(psResult, sTmp,     psKey+8,  DECRYPT);
		des(sTmp,     psResult, psKey+16, ENCRYPT);
		break;
		
	case TRI_DECRYPT3:
		des(psData,   psResult, psKey+16, DECRYPT);
		des(psResult, sTmp,     psKey+8,  ENCRYPT);
		des(sTmp,     psResult, psKey,    DECRYPT);
		break;
	}
}


void PubCalcMac(uchar ucMode, uchar *psKey, uchar *psMsg, uint uiLen, uchar *psMac)
{
	uchar   sOutMac[8];
	uint	uiOffset, i;
	
	PubASSERT(ucMode==MAC_FAST || ucMode==MAC_ANSIX99 || ucMode==MAC_ANSIX919);
	memset(sOutMac, 0, sizeof(sOutMac));
	uiOffset = 0;

	if ((psKey == NULL) || (psMsg == NULL) || (psMac == NULL))
	{
		return;
	}
	
	while(uiLen>uiOffset)
	{
		if (uiLen-uiOffset<=8)
		{
			for (i=0; i<uiLen-uiOffset; i++)
			{
				sOutMac[i] ^= psMsg[uiOffset+i];
			}
			break;
		}
		for (i=0; i<8; i++)
		{
			sOutMac[i] ^= psMsg[uiOffset+i];
		}
		if (ucMode==MAC_ANSIX99 || ucMode==MAC_ANSIX919)
		{
			PubDes(ONE_ENCRYPT, sOutMac, psKey, sOutMac);
		}
		uiOffset += 8;
	}
	
	PubDes(ONE_ENCRYPT, sOutMac, psKey, psMac);
	if (ucMode==MAC_ANSIX919)
	{
		PubDes(ONE_DECRYPT, psMac, psKey+8, sOutMac);
		PubDes(ONE_ENCRYPT, sOutMac, psKey, psMac);
	}
}

// 计算LRC
void PubCalcLRC(uchar *psData, ushort uiLength, uchar *pucInit)
{
	uchar ucInit;

	if ((psData == NULL) || (pucInit == NULL))
	{
		return;
	}

	ucInit = *psData++;
	uiLength--;
	
	while( uiLength>0 )
	{
		ucInit ^= *psData++;
		uiLength--;
	}

	*pucInit = ucInit;
}



/*********************************************************************************************/

//									Debug输出函数

/*********************************************************************************************/
#if 0
int UnPackElement(FIELD_ATTR *pAttr, uchar *pusIn, uchar *pusOut,
                   uint *puiInLen)
{
    uint    i, j, iInLen, iTmpLen;

    memset(pusOut, 0, pAttr->uiLength);
    *puiInLen = 0;

    switch( pAttr->eLengthAttr ){
    case Attr_fix:
        iInLen  = pAttr->uiLength;
        iTmpLen = iInLen;
        break;

    case Attr_var1:
        iTmpLen= (pusIn[0]>>4)*10 + (pusIn[0]&0x0F);
        pusIn++;
        iInLen= 1+iTmpLen;
        break;

    case Attr_var2:
        iTmpLen = (pusIn[0]&0x0F)*100 + (pusIn[1]>>4)*10 + (pusIn[1]&0x0F);
        pusIn += 2;
        iInLen = 2+iTmpLen;
        break;
    }   /*** switch(pAttr->eLengthAttr ***/

    if( iTmpLen>pAttr->uiLength ){
        return -1;
    }
    if( pAttr->eElementAttr==Attr_b ){
        pusOut[0] = (uchar)(iTmpLen>>8);
        pusOut[1] = (uchar)iTmpLen;
        pusOut   += 2;
    }

    switch( pAttr->eElementAttr ){
    case Attr_n:
        switch( pAttr->eLengthAttr ){
        case Attr_fix:
            iInLen= (pAttr->uiLength+1)/2;

            for(i=0,j=0; i<pAttr->uiLength; i+=2,j++){
                if( (pusIn[iInLen-j-1]&0x0F)<0x0A ){
                    pusOut[pAttr->uiLength-i-1] = (pusIn[iInLen-j-1]&0x0F)|0x30;
                }else{
                    pusOut[pAttr->uiLength-i-1] =
                        (pusIn[iInLen-j-1]&0x0F)-0x0A+'A';
                }

                if( i!=pAttr->uiLength-1 ){
                    if( (pusIn[iInLen-j-1]>>4)<0x0A ){
                        pusOut[pAttr->uiLength-i-2] =
                           (pusIn[iInLen-j-1]>>4)|0x30;
                    }else{
                        pusOut[pAttr->uiLength-i-2]=
                           (pusIn[iInLen-j-1]>>4)-0x0A+'A';
                    }
                }
            }
            break;

        case Attr_var1:
        case Attr_var2:
            iInLen = iInLen - iTmpLen + (iTmpLen+1)/2;

#ifndef VarAttrN_RightJustify
            for(i=0; i<(iTmpLen/2); i++){
                if( (pusIn[i]>>4)<0x0A ){
                    pusOut[2*i] = (pusIn[i]>>4) | 0x30;
                }else{
                     pusOut[2*i] = (pusIn[i]>>4) -0x0A + 'A';
                }

                if( (pusIn[i]&0x0F)<0x0A ){
                     pusOut[2*i+1] = (pusIn[i]&0x0F) | 0x30;
                }else{
                     pusOut[2*i+1] = (pusIn[i]&0x0F) - 0x0A + 'A';
                }
            }

            if( iTmpLen%2 ){
                if( (pusIn[i]>>4)<0x0A ){
                     pusOut[2*i] = (pusIn[i]>>4) | 0x30;
                }else{
                     pusOut[2*i] = (pusIn[i]>>4) - 0x0A + 'A';
                }
            }
#else
            for(i=0,j=0; i<iTmpLen; i+=2,j++){
                if( (pusIn[(iTmpLen+1)/2-j-1]&0x0F)<0x0A ){
                    pusOut[iTmpLen-i-1] = (pusIn[(iTmpLen+1)/2-j-1]&0x0F)|0x30;
                }else{
                     pusOut[iTmpLen-i-1] =
                        (pusIn[(iTmpLen+1)/2-j-1]&0x0F)-0x0A+'A';
                }

                if( i!=pAttr->uiLength-1 ){
                    if( (pusIn[(iTmpLen+1)/2-j-1]>>4)<0x0A ){
                        pusOut[iTmpLen-i-2] =
                           (pusIn[(iTmpLen+1)/2-j-1]>>4)|0x30;
                    }else{
                        pusOut[iTmpLen-i-2] =
                           (pusIn[(iTmpLen+1)/2-j-1]>>4)-0x0A+'A';
                    }
                }
            }
#endif
            break;
        }   /*** switch(pAttr->eLengthAttr) ***/
        break;

    case Attr_z:
        switch( pAttr->eLengthAttr ){
        case Attr_fix:
            iInLen = (pAttr->uiLength+1)/2;

            for(i=0,j=0; i<pAttr->uiLength; i+=2,j++){
                pusOut[pAttr->uiLength-i-1]= (pusIn[iInLen-j-1]&0x0F)|0x30;
                if( i!=pAttr->uiLength-1 ){
                     pusOut[pAttr->uiLength-i-2]= (pusIn[iInLen-j-1]>>4)|0x30;
                }
            }
            break;

        case Attr_var1:
        case Attr_var2:
            iInLen = iInLen - iTmpLen + (iTmpLen+1)/2;

            for(i=0; i<(iTmpLen/2); i++){
                pusOut[2*i]   = (pusIn[i]>>4) | 0x30;
                pusOut[2*i+1] = (pusIn[i]&0x0F) | 0x30;
            }
            if( iTmpLen%2 ){
                pusOut[2*i]= (pusIn[i]>>4) | 0x30;
            }
            break;
        }   /*** switch(pAttr->eLengthAttr) ***/
        break;

    case Attr_b:
        memcpy(pusOut, pusIn, iTmpLen);
        break;

    case Attr_a:
        memcpy(pusOut, pusIn, iTmpLen);
        break;
    }    /*** switch(pAttr->eElementAttr) ***/

    *puiInLen = iInLen;

    return 0;
}
#endif
#define END_FLAG	0x80

uchar ScrSend(uchar *psData, ushort nDataLen, ushort nStartLine)
{
	ushort nCurrLen;
	ushort nCurrLine;

	nCurrLine = nStartLine%6;
	nCurrLen = 0;

	DxxScrClrLine((uchar)(nCurrLine%6+2), 7);
	while(1)
	{
		if (nCurrLine%6 ==0 && nCurrLine != 0)
		{
			getkey();
			DxxScrClrLine(2, 7);
		}
		if (nDataLen - nCurrLen > 21)
		{
			ScrPrint(0, (uchar)(nCurrLine%6+2), ASCII, "%21.21s", psData+nCurrLen);
			nCurrLen += 21;
			nCurrLine++;
		}
		else
		{
			ScrPrint(0, (uchar)(nCurrLine%6+2), ASCII, "%.*s", nDataLen - nCurrLen, psData+nCurrLen);
			nCurrLen += nDataLen - nCurrLen;
			if (nCurrLine%6+2 == 7)
			{
				getkey();
				return 1;
			}
			nCurrLine++;
			break;
		}	
	}
	return 0;
}

uchar PortSendstr(uchar ucChannel, uchar *psStr, ushort nStrLen)
{
	uchar ucRet;

	while(nStrLen--)
	{
		ucRet = PortSend(ucChannel, *psStr++);
		if (ucRet != 0x00)
		{
			return ucRet;
		}
	}
	return 0;
}
	
uchar UnitSend(uchar *psData, ushort nDataLen, uchar ucDevice, uchar ucMode, ushort nStartLine)
{
	uchar ucPort;
	uchar ucRet;

	if (ucDevice&DEVICE_SCR)
	{
		ucRet = ScrSend(psData, nDataLen, nStartLine);
		if (ucDevice&END_FLAG && ucRet == 0)
		{
			getkey();
		}
	}

	if (ucDevice&DEVICE_PRN)
	{
		psData[nDataLen] = 0x00;
		PrnStr(psData);

		if (ucMode&ISO_MODE || ucMode&TLV_MODE)
		{
			PrnStr("\n");
		}
		if (ucDevice&END_FLAG)
		{
			PrnStr("\n\n\n\n\n\n\n");
			PrnStart();
			PrnInit();
			PrnFontSet(0,0);
		}
		else if (nDataLen >= 3360)
		{
			PrnStart();
			PrnInit();
			PrnFontSet(0,0);
		}
	}

	if (ucDevice&DEVICE_COM1 || ucDevice&DEVICE_COM2)
	{
		ucPort = ucDevice&DEVICE_COM1?0:1;
#ifdef _S_SERIES_
		ucPort = (ucPort==1 ? 3 : 0); // S80使用PINPAD口作为第二个COM口
#endif
		PortSendstr(ucPort, psData, nDataLen);
		DelayMs(50);//确保数据发送完毕
		if (ucMode&ISO_MODE || ucMode&TLV_MODE)
		{
			PortSendstr(ucPort, "\x0d\x0a", 2);
		}
		if (ucDevice&END_FLAG)
		{
			PortSendstr(ucPort, "\x0d\x0a\x0d\x0a", 4);
		}
	}

	return 0;
}

#if 0
void PubISODefine(void *pvDataDef)
{
	pstDataAttr = pvDataDef;
	return;
}
#endif
//

/***************************************************************************************
For Debug use
***************************************************************************************/


#ifdef _POS_DEBUG
void DispHexMsg(uchar *pszTitle, uchar *psMsg, uint uiMsgLen, short nTimeOut)
{
	uint	i, iLineNum, iPageNo, iPageNum, iDispLine;
	uchar	*pszBuff;
	uchar   ucKey, ucStartLine;
	
	// Format message
	uiMsgLen = MIN(uiMsgLen, LEN_DBGDATA);
	glszDbgInfo[0] = 0;
	for (pszBuff=glszDbgInfo,i=0; i<uiMsgLen; i+=MAX_CHARS)
	{
		if (uiMsgLen-i<MAX_CHARS)
		{
			pszBuff += DispHexLine(pszBuff, i, psMsg+i, uiMsgLen-i);
		}
		else
		{
			pszBuff += DispHexLine(pszBuff, i, psMsg+i, MAX_CHARS);
		}
	}   // end of for (pszBuff=
	
	// Display message
	if (pszTitle!=NULL && *pszTitle!=0)
	{
		PubShowTitle(TRUE, pszTitle);
		iDispLine   = NUM_MAXLINES-2;
		ucStartLine = 2;
	}
	else
	{
		iDispLine   = NUM_MAXLINES;
		ucStartLine = 0;
	}
	
	iLineNum = (strlen((char *)glszDbgInfo)+CHARS_PERLINE-1)/CHARS_PERLINE;
	iPageNum = (iLineNum+iDispLine-1)/iDispLine;
	iPageNo  = 0;
	
	while(1)
	{
		// Light/Close up & down icon
		ScrSetIcon(ICON_UP,   (uchar)(iPageNo>0		  ? OPENICON : CLOSEICON));
		ScrSetIcon(ICON_DOWN, (uchar)(iPageNo<iPageNum-1 ? OPENICON : CLOSEICON));
		
		ScrClrLine(ucStartLine, NUM_MAXLINENO);
		ScrPrint(0, ucStartLine, ASCII, "%.*s", CHARS_PERLINE*iDispLine,
			&glszDbgInfo[CHARS_PERLINE*iDispLine*iPageNo]);
		
		ucKey = PubWaitKey(nTimeOut);
		ScrSetIcon(ICON_UP,   CLOSEICON);
		ScrSetIcon(ICON_DOWN, CLOSEICON);
		
		if (ucKey==KEYUP)
		{
			iPageNo = (iPageNo==0) ? iPageNum-1 : iPageNo-1;
		}
		else if (ucKey==KEYDOWN || ucKey==KEYENTER)
		{
			iPageNo = (iPageNo>=iPageNum-1) ? 0 : iPageNo+1;
		}
		else if (ucKey==KEYCANCEL || ucKey==NOKEY)
		{
			return;
		}
	}   // end of while(1
}

// print a line as hexadecimal format
int DispHexLine(uchar *pszBuff, uint uiOffset, uchar *psMsg, uint uiMsgLen)
{
	uint	i;
	uchar	*p = pszBuff;
	
	// Print line information
	pszBuff += sprintf((char *)pszBuff, "%04Xh:", uiOffset);
	
	for (i=0; i<uiMsgLen; i++)
	{
		pszBuff += sprintf((char *)pszBuff, " %02X", psMsg[i]);
	}
	for (; i<MAX_CHARS; i++)
	{   // append blank spaces, if needed
		pszBuff += sprintf((char *)pszBuff, "   ");
	}
	
	return (pszBuff-p);
}

// Debug 使用, 功能：显示文件名与行数
void DispAssert(uchar *pszFileName, ulong ulLineNo)
{
	uchar	szFName[30];
	
	PubShowTitle(TRUE, (uchar *)"Assert Failure");
	GetBaseName(pszFileName, szFName);
	ScrPrint(0, 2, CFONT, "FILE:%.11s", szFName);
	ScrPrint(0, 4, CFONT, "LINE:%ld", ulLineNo);
	PubShowMsg(6, (uchar *)"PRESS ENTER EXIT");
	PubLongBeep();
	
	while(!PubYesNo(60));
}

// get basename of a full path name
void GetBaseName(uchar *pszFullPath, uchar *pszBaseName)
{
	uchar	*pszTmp;
	
	*pszBaseName = 0;
	if (!pszFullPath || !*pszFullPath)
	{
		return;
	}
	
	pszTmp = &pszFullPath[strlen((char *)pszFullPath)-1];
	while( pszTmp>=pszFullPath && *pszTmp!='\\' && *pszTmp!='/' )
	{
		pszTmp--;
	}
	sprintf((char *)pszBaseName, "%s", (char *)(pszTmp+1));
}

#else /* _POS_DEBUG */

#endif /* _POS_DEBUG */

//示例
// 功能描述: 从满足条件1、2的2种状态中选择，或者不选。函数返回相应的选择
uchar PubSelectOptions(uchar * pszTitle, uchar bCondition,  uchar* pszTruePrmpt,  uchar* pszFalsePrmpt, 
				 	                 uchar* pszOption1,     uchar* pszOption2,   ushort iTimeOut )
{
	uchar ucKey;
	
	ScrCls();
	PubShowTitle(TRUE, pszTitle);
	if(bCondition) 
	{
		PubDispString(pszTruePrmpt, DISP_LINE_LEFT|2);
	}
	else 
	{
		PubDispString(pszFalsePrmpt, DISP_LINE_LEFT|2);
	}
	
	ScrPrint(0,4,CFONT,"%s  %s", pszOption1, pszOption2);
	
	while(1)
	{
		ucKey = PubWaitKey(iTimeOut);
		switch(ucKey)
		{
		case KEY1:case KEY2:case KEYENTER:case KEYCANCEL:
			return ucKey;
		case NOKEY: return KEYCANCEL;
		default:
			continue;
		}
	}
}

//功    能： 提供2种方式选择  仅接收'1''2',确认或者取消按键
uchar PubSelectOneTwo(uchar *pszItem,uchar *pszChoice1,uchar *pszChoice2)
{
	uchar ucKey, szBuf[20];
	
	DxxScrClrLine(2,7);
	PubDispString(pszItem, DISP_LINE_CENTER|2);
	sprintf((char *)szBuf, "[1].%s", pszChoice1);
	PubDispString(szBuf, DISP_LINE_LEFT|4);
	sprintf((char *)szBuf, "[2].%s", pszChoice2);
	PubDispString(szBuf, DISP_LINE_LEFT|6);
	
	while(1)
	{
		ucKey = PubWaitKey(0);
		switch(ucKey)
		{
		case KEY1:case KEY2:case KEYENTER:case KEYCANCEL:
			return ucKey;
		default:
			continue;
		}
	}
}

 // 提供是否方式选择  仅接收'0''1',确认或者取消按键
uchar PubSelectZeroOne(uchar *pszTitle, uchar *pszItem, uchar *pszChoice1, uchar *pszChoice2)
{
	uchar ucKey, szBuf[20];
	
	PubShowTitle(TRUE, pszTitle);
	PubDispString(pszItem, DISP_LINE_CENTER|2);
	sprintf((char *)szBuf,"0.%s  1.%s",pszChoice1, pszChoice2);
	PubDispString(szBuf, DISP_LINE_LEFT|4);
	
	while(1)
	{
		ucKey = PubWaitKey(0);
		switch(ucKey)
		{
		case KEY1:case KEY2:case KEYENTER:case KEYCANCEL:
			return ucKey;
		default:
			continue;
		}
	}
}

//提供是否方式选择  仅接收确认或者取消按键
uchar PubSelectYesNo(uchar *pszItem,uchar *pszYesPrompt,uchar *pszNoPrompt)
{
	uchar ucKey, szBuf[20];
	
	DxxScrClrLine(2,7);
	PubDispString(pszItem, DISP_LINE_CENTER|2);
	sprintf((char *)szBuf,_T("[ENTER ].%s"),pszYesPrompt);
	PubDispString(szBuf, DISP_LINE_LEFT|4);
	sprintf((char *)szBuf,_T("[CANCEL].%s"),pszNoPrompt);
	PubDispString(szBuf, DISP_LINE_LEFT|6);
	
	while(1)
	{
		ucKey = PubWaitKey(0);
		switch(ucKey)
		{
		case KEY1:case KEY2:case KEYENTER:case KEYCANCEL:
			return ucKey;
		default:
			continue;
		}
	}
}
