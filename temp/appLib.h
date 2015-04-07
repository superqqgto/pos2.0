
/****************************************************************************
应用标准库

百富计算机技术(深圳)有限公司

修改历史：修改内容见<<百富应用标准库使用指南>>
1.0.0 2008年9月12日    标准库维护组
1.0.1 2008年10月14日   标准库维护组
1.0.2 2009年3月11日   修改者:孙云 测试Sabrina

****************************************************************************/

#ifndef _APPLIB_H
#define _APPLIB_H

#include "Util.h"
#ifndef uchar
	typedef unsigned char	uchar;
#endif

#ifndef ushort
	typedef unsigned short	ushort;
#endif

#ifndef uint
	typedef unsigned int	uint;
#endif

#ifndef ulong
	typedef unsigned long	ulong;
#endif

// #define _POS_DEBUG

#ifndef TRUE
	#define TRUE	1
	#define FALSE	0
#endif

#define TIMER_TEMPORARY			4       // 短时间的定时器(其他模块可以共用)

// macros for debug output
#define DEVICE_SCR   0x01
#define DEVICE_PRN   0x02
#define DEVICE_COM1  0x04
#define DEVICE_COM2  0x08

#define ASC_MODE	0x10
#define HEX_MODE	0x20
#define ISO_MODE	0x40
#define TLV_MODE	0x80

// macros for vDes()
#define ONE_DECRYPT     0           // 单DES解密
#define ONE_ENCRYPT     1           // 单DES加密
#define TRI_DECRYPT     2           // 三DES解密(16 bytes key)
#define TRI_ENCRYPT     3           // 三DES加密(16 bytes key)
#define TRI_DECRYPT3    4           // 三DES解密(24 bytes key)
#define TRI_ENCRYPT3    5           // 三DES加密(24 bytes key)

// macros for vCalcMac()
#define MAC_ANSIX99     0           // ANSI9.9 standard MAC algorithm
#define MAC_FAST        1           // HyperCom fast MAC algorithm
#define MAC_ANSIX919	2			// ANSI9.19

#ifndef REVER
#define REVER           0x80		// Reverse background when display
#endif

// macros for ScrSetIcon()
#ifndef ICON_PHONE
#define ICON_PHONE      1       // phone
#define ICON_SIGNAL     2       // wireless signal
#define ICON_PRINTER    3       // printer
#define ICON_ICCARD     4       // smart card IC
#define ICON_LOCK       5       // lock
#define ICON_SPEAKER    6       // speaker
#define ICON_UP         7       // up
#define ICON_DOWN       8       // down
#define CLOSEICON		0       // 关闭图标[针对所有图标]
#define OPENICON        1       // 显示图标[针对打印机/IC卡/锁/扬声器/上/下]
#endif

// 若需点亮摘机、挂机图标，则action的值设置如下
#define OFFHOOKICON     1       // hook off 摘机
#define ONHOOKICON      2       // hook on  挂机
// 若需点亮信号图标，则action的值设置为：INITSIGNAL+[0，5]
#define INITSIGNAL      1       // 初始信号强度[最强信号为INITSIGNAL+5]

// macros for PubGetString()
#define NUM_IN			0x01	// 数字输入
#define ALPHA_IN		0x02	// 字母数字混合输入
#define PASS_IN			0x04	// 秘密输入
#define CARRY_IN		0x08	// 有传入数据
#define ECHO_IN			0x10	// 回显传入数据,按键则清除原数据
#define AUTO_FONT		0x80	// RFU

// macros for PubGetStrParam()
#define GS_ECHOCONT         0x80        // 回车退出(回显初始值?)
#define GS_CFONT            0x40        // 大字体
#define GS_NUMBER           0x20        // 输数字
#define GS_ALPHA            0x10        // 输字符
#define GS_PASSWD           0x08        // 密码方式
#define GS_LALIGN           0x04        // 左对齐输入
#define GS_FLOAT            0x02        // 小数点
#define GS_NORMAL           0x01        // 正常显示

#ifdef _TERMINAL_DXX_
#define NUM_MAXZHCHARS	26
#define NUM_MAXCOLS		320
#else
#define NUM_MAXZHCHARS	21
#define NUM_MAXCOLS		128
#endif

#define NUM_MAXCOLNO        (NUM_MAXCOLS-1)     // 最大列号
#define NUM_MAXLINES        9//8           // 最多行数
#define NUM_MAXLINENO       (NUM_MAXLINES-1)    // 最大行号
#define NUM_MAXZHLINES      4           // 最大中文行数
#define NUM_MAXCHARS		21

#define DISP_LINE_LEFT		0x80
#define DISP_LINE_RIGHT		0x40
#define DISP_LINE_CENTER	0x20
#define DISP_SCR_CENTER		0x0100
#define DISP_LINE_REVER		0x0200
#define DISP_PART_REVER		0x0400

// menu display mode
#define MENU_NOSPEC		0x00	// 自动显示
#define MENU_USESNO		0x01	// 使用菜单自己的编号(优先于MENU_AUTOSNO)
#define MENU_AUTOSNO	0x02	// 自动编号
#define MENU_CENTER		0x10	// 中间对齐显示
#define MENU_PROMPT		0x20	// 强制提示信息在底部
#define MENU_ASCII		0x40	// 强制显示小字体
#define MENU_CFONT		0x80	// 强制显示大字体(优先于MENU_ASCII)

// types for PubGetMenu()
typedef void (*MenuFunc)(void);
typedef struct _tagMenuItem{
	uchar		bVisible;						// TRUE: show it, FALSE: hide
    uchar       szMenuName[NUM_MAXCHARS+1];		// menu name
    MenuFunc    pfMenuFunc;						// callback function(if set)
}MenuItem;

//智能菜单相关定义

//每行容纳的字符数（以英文字符计算，汉字占2个位置）
#define SM_LINE_WIDTH  24 //16 modified by wuc 2014.4.1
//最大菜单行数
#define SM_MAX_LINE    32
/***********++add by wuc 2013.12.17************/
//单页小字体最大菜单行数
#define SM_ASCII_MAX_LINE    9
//单页大字体最大菜单行数
#define SM_CFONT_MAX_LINE    4
/***********--add by wuc 2013.12.17************/

/*
 * 单个菜单项
 */
struct SMART_MENU;

typedef struct _tagMENU_ITEM{
    char                *pszText;  /*菜单文字提示*/
    int                 iValue;      /*对应的交易号*/    
    struct SMART_MENU * pstSubMenu;   /*子菜单，如无，则为NULL*/
}MENU_ITEM;

typedef struct SMART_MENU{
    MENU_ITEM       *pstContent;
    int             iNum;        /*菜单项数 */
    uchar			sMask[SM_MAX_LINE];
}SMART_MENU;


#define  MENU_ICON_UP       7   /*up 向上*/
#define  MENU_ICON_DOWN     8   /*down 向下*/
    /*action - 指定动作,    1:点亮，0:熄灭。*/
#define  MENU_CLOSEICON     0   /*关闭图标[针对所有图标]*/
#define  MENU_OPENICON      1   /* 显示图标[针对打印机、IC卡、锁、扬声器、向上、向下]*/

//每行显示一个菜单项（缺省显示两个）
#define SM_1ITEM_PER_LINE 0x00000001

//使用上下箭头和确认键选择（缺省以数字键选择）
#define SM_ARROW_SEL      (0x00000001<<1)

//从0开始计算菜单项，SM_ARROW_SEL置位时无效（缺省从1开始）
#define SM_ZERO_START     (0x00000001<<2)

//居中显示标题
#define SM_MIDDLE_TITLE   (0x00000001<<3)

//居中显示菜单项
#define SM_MIDDLE_ITEM    (0x00000001<<4)

//按取消键直接返回
#define SM_DIRECT_RETURN  (0x00000001<<5)

#define SM_ON   1
#define SM_OFF  0

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/****************************************************************************
 Function:      Calculate the offset of the struct member
 Param In:
    type        struct type
    member      struct member name
 Param Out:
    none
 Return Code:   the offset of the struct member
****************************************************************************/
/****************************************************************************
 功    能:      计算结构成员的偏移
 输入参数:
    type        结构体类型
    member      结构体成员名称
 输出参数:
    none
 返 回 值:      该结构体成员的偏移量
****************************************************************************/
#define OFFSET(type, member)    ( (ulong)(&((type *)0)->member) )

/****************************************************************************
 Function:      Calculate the maximum(minimum) of two int number/expression
 Param In:
    a,b         number/expression need to be compare
 Param Out:
    none
 Return Code:   maximum(minimum)
****************************************************************************/
/****************************************************************************
 功    能:      计算两个整数数字/表达式最大(小)值
 输入参数:
    a,b         待比较的整数/表达式
 输出参数:
    none
 返 回 值:      最大(小)值
****************************************************************************/
#define MAX(a, b)       ( (a)>=(b) ? (a) : (b) )
#define MIN(a, b)       ( (a)<=(b) ? (a) : (b) )

/****************************************************************************
 Function:      Calculate the absolute value of int number/expression
 Param In:
    a           int number/expression
 Param Out:
    none
 Return Code:   absolute value
****************************************************************************/
/****************************************************************************
 功    能:      计算整数数字/表达式绝对值
 输入参数:
    a           整数数字/表达式
 输出参数:
    none
 返 回 值:      绝对值
****************************************************************************/
#define ABS(a)      ( (a)>=0 ? (a) : (-1)*(a) )

/****************************************************************************


                                      1.系统基本函数
 
   
	 
*****************************************************************************/


  
/****************************************************************************
功能： 	        将源串与目标串按一定长度异或后存入目标串,函数将把2个字符串前
                uiLength字节异或后放入psOut。 
输入参数	psVect1     被异或的串
                psVect2     被异或的串
                uiLength    要异或的长度
输出参数	psOut       异或后的目标串
返回：  	无
说明：	        psVect1与psOut可为同一个地址
如果3个指针参数为NULL，则函数将直接返回；
***************************************************************************/
void PubXor(uchar *psVect1, uchar *psVect2, uint uiLength, uchar *psOut);

/****************************************************************************
功能： 	        将2进制源串分解成双倍长度可读的16进制串，BCD 码转换成可见ASCII码。
输入参数	psIn      源二进制串
                uiLength  需要转换的源串长度字节数
输出参数	psOut     转换后的ASC串
返回：  	无
****************************************************************************/
void PubBcd2Asc(uchar *psIn, uint uiLength, uchar *psOut);

/****************************************************************************
功能：  	将2进制串分解成双倍长度可读的16进制串,转换后的串以‘\0’结束。 
                BCD码转换成ASCII码字符串，并带结束符
输入参数	psIn      源串
                uiLength  需要转换的源串长度字节数
输出参数	psOut    目标串
返回：  	无
****************************************************************************/
void PubBcd2Asc0(uchar *psIn, uint uiLength, uchar *pszOut);

/****************************************************************************
功能： 	        将可读的16进制串合并成其一半长度的二进制串,"12AB"-->0x12AB 
输入参数	psIn      源串
                uiLength  源Ascii串长度
输出参数	psOut     目标串
返回：  	无

****************************************************************************/
void PubAsc2Bcd(uchar *psIn, uint uiLength, uchar *psOut);

/****************************************************************************
//将数字串转变成长整数,相当于atol(),但是该函数不要求零字符结束
//Convert number string to long integer, similar to atol(). This function don't request end with '\0'.
****************************************************************************/
ulong PubAsc2Long(uchar *psString, uint uiLength, ulong *pulOut);

/****************************************************************************
功能： 	        整型数转换成高位在前的字符串表示(按照网络字节格式存放)
输入参数	ulSource   待转换的长整数
                uiTCnt    目标缓冲区的大小(字节数)
输出参数	psTarget   转换后的串
返回：  	无
****************************************************************************/
void PubLong2Char(ulong ulSource, uint uiTCnt, uchar *psTarget);

/****************************************************************************
功能： 	        高位在前的字符串表示转换为整型数
输入参数	psSource   待转换的串，最多4个字符
                uiSCnt     源串的长度，最大为4，超过时，取后psSource前的4个字
                           符进行运算
输出参数	*pulTarget 转换后整数
返回：  	转换后整数
****************************************************************************/
ulong PubChar2Long(uchar *psSource, uint uiSCnt, ulong *pulTarget);

/****************************************************************************
功能： 	        将整数转换为BCD格式的串存放
输入参数	ulSource    待转换的整数
uiTCnt          目标缓冲区的大小(字节数)
输出参数	psTarget    目标缓冲区
返回：  	无
****************************************************************************/
void PubLong2Bcd(ulong ulSource, uint uiTCnt, uchar *psTarget);

/****************************************************************************
功能： 	        转换BCD串为整数
输入参数	psSource    待转换的BCD数据格式的串
                uiSCnt      源串的待转换长度
输出参数	*pulTarget  转换后整数；
                pulTarget地址可以为NULL，如果为NULL，则不输出
                atol的返回是long，所以最大取值为2147483647
返回：  	转换后的整数，值同pulTarget
****************************************************************************/
ulong PubBcd2Long(uchar *psSource, uint uiSCnt, ulong *pulTarget);

/****************************************************************************
功能： 	        将字符串转换为大写
输入参数	pszString   待转换的字符串,以'\0'结束
输出参数	pszString   转换后的字符串,以'\0'结束
返回：  	无
****************************************************************************/
void PubStrUpper(uchar *pszString);

/****************************************************************************
功能： 	        将字符串转换为小写
输入参数	pszString   待转换的字符串,以'\0'结束 
输出参数	pszString   转换后的字符串,以'\0'结束 
返回：  	无
****************************************************************************/
void PubStrLower(uchar *pszString);

/****************************************************************************
功能： 	        删除字符串两端的空白字符(包括,空格/回车/换行/TAB字符)
                两端的或左侧右侧的概念:第一个和最后一个非此4个字符为标记,中间
                的不删除.
输入参数	pszString   待转换的字符串,以'\0'结束
输出参数	pszString   转换后的字符串,以'\0'结束
返回：  	无
****************************************************************************/
void PubTrimStr(uchar *pszString);

/****************************************************************************
功能： 	        删除字符串两端的的特定字符,即删除左侧或右侧连续的特定字符
                两端的概念:同PubTrimStr.
输入参数	pszString   待转换的字符串,以'\0'结束
                ucSpcChar   特定字符
输出参数	pszString   转换后的字符串,以'\0'结束
返回：  	无
****************************************************************************/
void PubTrimSpcStr(uchar *pszString, uchar ucSpcChar);

/****************************************************************************
功能：          删除字符串左边连续的特定字符
输入参数	pszStringInOut     待转换的字符串,以'\0'结束
                ucRemoveChar  待删除字符
输出参数	pszString   转换后的字符串,以'\0'结束
返回：  	无

****************************************************************************/
void PubTrimHeadChars(uchar *pszString, uchar ucRemoveChar);

/****************************************************************************
功能： 	        删除字符串右边连续的特定字符
输入参数	pszStringInOut     待转换的字符串,以'\0'结束
                ucRemoveChar  待删除字符
输出参数	pszStringInOut    转换后的字符串,以'\0'结束
返回：  	无

****************************************************************************/
void PubTrimTailChars(uchar *pszString, uchar ucRemoveChar);

/****************************************************************************
功能： 	        不区分大小写的字符串比较函数
输入参数	pszStr1    待比较的字符串1
                pszStr2    待比较的字符串2 
输出参数	无/NONE
返回：  	0          两串相等
                <0         pszStr1 < pszStr2
                >0         pszStr1 > pszStr2
                参数错误，不考虑输出

****************************************************************************/
int PubStrNoCaseCmp(uchar *pszStr1, uchar *pszStr2);

/****************************************************************************
功能： 	填充前导字符,以便达到uiTargetLen长度
输入参数	pszStringInOut    待填充前导字符的串
uiTargetLen       目标串长度 
ucAddChar         填充的字符
输出参数	pszStringInOut    填充后的串
返回：  	无
****************************************************************************/
void PubAddHeadChars(uchar *pszString, uint uiTargetLen, uchar ucAddChar);

/****************************************************************************
功能： 	        反转字符串
输入参数	pszStringInOut   待反转的字符串,以'\0'结束
输出参数	pszStringInOut   反转后的字符串,以'\0'结束
返回：  	无
****************************************************************************/
void PubStrReverse(uchar *pszStringInOut);

/****************************************************************************
功能： 	        取字节的高四位
输入参数	ucInChar    待取高四位的字节
输出参数	无
返回：  	高四位的字节
****************************************************************************/
uchar PubHigh4Bit(uchar ucInChar);

/****************************************************************************
功能： 	        取字节的低四位
输入参数	ucInChar    待取低四位的字节
输出参数	无
返回：  	低四位的字节
****************************************************************************/
uchar PubLow4Bit(uchar ucInChar);

/****************************************************************************
功能：  	两个ASC字符串相乘
输入参数	pszFaciend  被乘数ASC字符串，以\0为结束符
                pszMultiplier乘数ASC字符串，以\0为结束符
输出参数	pszProduct相乘的结果，以\0为结束符
返回：  	无
****************************************************************************/
void PubAscMul(uchar *pszFaciend, uchar *pszMultiplier, uchar *pszProduct);

/****************************************************************************
功能： 	        长度相等的两个ASC数字串相加。
输入参数	psAddend1   被加数ASC字符串，待运算串不足uiLen要先手动做前补
                            0操作，下同。
                psAddend2   加数ASC字符串
                uiLen       字符串的长度
输出参数	pszSum      相加的结果
返回：  	无
****************************************************************************/
void PubAscAdd(uchar *psAddend1, uchar *psAddend2, uint uiLen, uchar *pszSum);
#define SafeAscAdd(a,b,c)	PubAscAdd(a,b,c,a)

/****************************************************************************
功能： 	        长度相等的两个ASC字符串相减，前补0
输入参数	psMinuend    被减数ASC数字字符串
                psSubtrahend 减数ASC数字字符串
uiLen           字符串的长度
输出参数	pszResult    结果
返回：  	无
****************************************************************************/
void PubAscSub(uchar *psMinuend, uchar *psSubtrahend, uint uiLen, uchar *pszResult);
#define SafeAscSub(a,b,c)	PubAscSub(a,b,c,a)

/****************************************************************************
功能： 	        ASC字符串减1,结果放在原串里 
输入参数	psAscStr    需减1的ASC字符串
                uiStrLen     psAscStr的长度
输出参数	psAscStr    减1后的ASC字符串
返回：  	0   成功    successful
                1   失败    failed
****************************************************************************/
uchar PubAscDec(uchar *psAscStr, uint uiStrLen);

/****************************************************************************
功能： 	        ASC字符串加1,结果放在原串里 
输入参数	psAscStr    需加1的ASC字符串
                uiStrLen     psAscStr的长度
输出参数	psAscStr    加1后的ASC字符串
返回：  	0   成功
                1   失败
****************************************************************************/
uchar PubAscInc(uchar *psAscStr, uint uiStrLen);

/****************************************************************************
功能： 	        长度相等的两个BCD字符串相加
输入参数	psAddend1   被加数BCD字符串
                psAddend2   加数BCD字符串
                uiLen       字符串的长度
输出参数	psResult    相加的结果
****************************************************************************/
void PubBcdAdd(uchar *psAddend1, uchar *psAddend2, uint uiLen, uchar *psResult);
#define SafeBcdAdd(a,b,c)		PubBcdAdd(a, b, c, a)

/****************************************************************************
功能： 	        长度相等的两个BCD字符串相减，前补0
输入参数	psMinuend      被减数BCD字符串 
                psSubtrahend   减数BCD字符串
                uiLen          字符串的长度
输出参数	psResult        相减的结果
****************************************************************************/
void PubBcdSub(uchar *psAddend1, uchar *psAddend2, uint uiLen, uchar *psResult);
#define SafeBcdSub(a,b,c)		PubBcdSub(a, b, c, a)

/****************************************************************************
功能： 	        BCD字符串减1,结果放在原串里
输入参数	psBcdStr    需减1的BCD字符串
                uiStrLen     psBcDStr的长度
输出参数	psBcdStr    减1后的BCD字符串
返回：  	0   成功    successful
                1   失败    failed
****************************************************************************/
uchar PubBcdDec(uchar *psBcdStr, uint uiStrLen);

/****************************************************************************
功能： 	        BCD字符串加1,结果放在原串里 
输入参数	psBcdStr    需加1的BCD字符串
                iStrLen     psBcdStr的长度
输出参数	psBcdStr    加1后的BCD字符串
返回：  	0   成功    successful
                1   失败    failed
****************************************************************************/
uchar PubBcdInc(uchar *psBcdStr, uint uiStrLen);

/****************************************************************************
功能： 	        转换数字串到金额格式，如HKD$****.**, RMB****.**
输入参数	pszPrefix  前额的前缀，如“HKD 200.00”中的“HKD”或是
                          “HKD$200.00”中的“HKD.”，最大允许4个字符.如果为空，
                           则不加前缀。
                pszIn      需要转换的纯数字串,过滤到前面的‘0’，如”00100”
                           会过滤为”100”,最大长度为40
                ucDeciPos   小数点后的金额位数，必需小于4
                           例如 当ucDeciPos==2, "14277" 转换为 "142.77"
                           if ucDeciPos==0, "14277" 转换为 "14,277"或不变。
                ucMisc      其它参数 
                           GA_SEPARATOR: 使用逗号分隔大金额，如"12,345.67"
                           GA_NEGATIVE: 金额前加负号： "SGD-123,45.67"
输出参数	pszOut      格式化后的金额串
返回：  	无
****************************************************************************/
void PubConvAmount(uchar *pszPrefix, uchar *pszIn, uchar ucDeciPos, uchar ucIgnoreDigit, uchar *pszOut, uchar ucMisc);

/****************************************************************************
应用执行异常中断函数
原型：	void PubHalt(uchar *pszfile, uint uiLine)
功能： 	提示一些信息,进入死循环.
提示信息:
	File:文件名, Line:行数
	Ret:系统Error值
	PLS RECORD THEN RESTART POS
输入参数	pszfile     文件名.
			uiLine      错误发生时所处的代码行数
输出参数	None
返回：  	None
示例说明：	
****************************************************************************/
void PubHalt(uchar *pszfile, uint uiLine);

/****************************************************************************
 Function:      Beep 800ms at frequency 6.
 Param In:      None.
 Param Out:     None
 Return Code:   None
****************************************************************************/
void PubLongBeep(void);	

/****************************************************************************
 Function:      Beep 60ms at frequency 6 for 3 times, with 80ms interval.
 Param In:      None.
 Param Out:     None
 Return Code:   None
****************************************************************************/
void PubBeepOk(void);		

/****************************************************************************
 Function:      Beep 200ms at frequency 6,then delay 200ms.
 Param In:      None.
 Param Out:     None
 Return Code:   None
****************************************************************************/
void PubBeepErr(void);	

/****************************************************************************
 Function:      多功能DES加密/解密运算
 Param In:
    ucMode      DES加密/解密模式:
                ONE_ENCRYPT --> 单DES加密
                ONE_DECRYPT --> 单DES解密
                TRI_ENCRYPT --> 三DES加密
                TRI_DECRYPT --> 三DES解密
    psData:     待加密/解密的数据(8字节)
    psKey:      DES运算的密钥(单倍长度或者两倍长度,根据ucMode确定)
 Param Out:
    psResult    DES运算的结果(8字节)
 Return Code:   none
****************************************************************************/
void PubDes(uchar ucMode, uchar *psData, uchar *psKey, uchar *psResult);	//

/****************************************************************************
 Function:      报文MAC运算函数
 Param In:
    ucMode      报文MAC运算模式:
                MAC_FAST    --> 快速MAC运算(HyperCom快速方式)
                MAC_ANSIX99 --> ANSI x9.9 方式
				MAC_ANSIX919--> ANSI x9.19 方式
    psKey:      MAC运算的密钥(8字节)
    psMsg:      计算MAC的数据
    uiLength:   计算MAC的数据的长度
 Param Out:
    psMAC       报文的MAC(8字节)
 Return Code:   none
****************************************************************************/
void PubCalcMac(uchar ucMode, uchar *psKey, uchar *psMsg, uint uiLen, uchar *psMAC);
	
/****************************************************************************
功能： 	        计算LRC
输入参数	psData    需要计算的数据
                uiLength  数据的长度
输出参数	*pucInit  LRC值
返回：  	无
****************************************************************************/
void PubCalcLRC(uchar *psData, ushort uiLength, uchar *pucInit);

/****************************************************************************
功能： 	        校验卡号是否满足模10校验
输入参数	pszPan   卡号
输出参数	无
返回：  	0    满足
                1    不满足
****************************************************************************/
uchar PubChkCardNo(uchar *pszPanNo);



/****************************************************************************


                                      2	显示函数
 
   
	 
*****************************************************************************/

/****************************************************************************
功能： 	在屏幕第一行以大字体显示TITLE，反显。并根据参数bClrScreen决定是否清除
        屏幕下方(2-7行)的内容。
输入参数	bClrScreen   TRUE: 清除屏幕下方(2-7行)的内容
                             FALSE: 保留原屏幕(2-7行)的内容
                pszTitle     以’\0’结束的需要显示的字符串。一般一行只能显示
                             16个的字符，8个汉字。
输出参数	无
返回：  	无

****************************************************************************/
void PubShowTitle(uchar bClrScreen, uchar *pszTitle);	//

/****************************************************************************
功能： 	在ucLine行处大字体居中显示消息pszMsg，若 pszMsg长度超过一行的将会被截掉
输入参数	ucLine      显示消息的行数。若在参数为ucLine & REVER，则该消息
                            在ucLine行显示的同时，为反显状态显示
                pszMsg      需要显示的消息，以’\0’为结束符
输出参数	无
返回：  	无
 Return Code:   none
****************************************************************************/
void PubShowMsg(uchar ucLine, uchar *pszMsg);			//

/****************************************************************************
功能： 	按大字符的模式居中显示2行信息，分别显示在ucLine行及ucLine+2行，每行超
        出一行显示信息的内容将会被截掉。
输入参数	ucLine    第一行显示的内容。若在参数为ucLine & REVER，则该消
                          息在ucLine行显示的同时，为反显状态显示。由于要显示
                          2行大字符，因此ucLine的值应为0-4.
                pszMsg1   首行显示的信息，以’\0’为结束符
                pszMsg2   第二行显示的信息，以’\0’为结束符
输出参数	无
返回：  	无
****************************************************************************/
void PubShowTwoMsg(uchar ucLine, uchar *pszMsg1, uchar *pszMsg2);		//


/****************************************************************************
功能： 	按大字符在指定行显示指定模式的字符串
输入参数	pszStr      需要显示的字符串，以’\0’为结束符
                ucPosition   需要被显示的行数|显示的方式
                显示的方式： 
                DISP_SCR_CENTER,   居中屏幕
                DISP_LINE_CENTER,  行居中
                DISP_LINE_ RIGHT    行居右
                DISP_LINE_LEFT      行居左
输出参数	无
返回：  	无
****************************************************************************/
void PubDispString(void *pszStr, ushort nPosition);

/****************************************************************************


                                      3	时间函数
 
   
	 
*****************************************************************************/
#define STAR_YEAR 1980
#define STAR_WEEK 6

/****************************************************************************
功能： 	将一个ulong型整数转换为日期+时间字符串，以1980年1月1日0时开始算， 最大
        时间可以到2116年1月1日
输入参数	ulSecond            待转换的ulong整数,其单位的含义是秒
输出参数	pszYYYYMMDDhhmmss   得到的日期+时间字符串，14个字节以00h结尾
返回：  	None
****************************************************************************/
void PubLong2Time(ulong ulSecond,uchar *pszYYYYMMDDhhmmss);

/****************************************************************************
功能： 	将一个ulong型整数转换为日期字符串，以1980年1月1日开始算
输入参数	ulDay           待转换的ulong整数,其单位的含义是日
输出参数	pszYYYYMMDD     得到的日期字符串，8个字节以00h结尾
返回：  	None
****************************************************************************/ 
void PubLong2Day(ulong ulDay,uchar *pszYYYYMMDD);  

/****************************************************************************
功能： 	将一个日期+时间字符串转换为ulong型整数，以1980年1月1日0时开始算，最大
        时间可以到2116年1月1日
输入参数	psYYYYMMDDhhmmss       需要被转换的日期+时间字符串
输出参数	None
返回：  	转换后ulong整数，其单位的含义是秒
****************************************************************************/ 
ulong PubTime2Long(uchar *psYYYYMMDDhhmmss);

/****************************************************************************
功能： 	将一个日期字符串转换为ulong型整数，以1980年1月1日开始算
输入参数	psYYYYMMDD       需要被转换的日期字符串
输出参数	None
返回：  	转换后ulong整数，其单位的含义是日
****************************************************************************/ 
ulong PubDay2Long(uchar *psYYYYMMDD);

/****************************************************************************
功能： 	 按照MASK定义的格式，检查日期时间格式是否有效
输入参数	 psDateTime       被检查的日期时间字符串
                 psMaskYYYYMMDDhhmmss
                 格式控制，该字符串可以为"YYYYMMDDhhmmss"或者其子串（必须是偶
                 数），详见用法中的例子。
                 如果年是YYYY格式，则必须为19YY或者20YY，否则认为非法
                 如果年是YY格式，则自动认为是20YY年
                 如果格式类似YMMD等非正常使用，则不保证判断结果正确。
输出参数	 
返回：  	 0，	有效
                 1，2，3，4，5，6，分别代表年、月、日、时、分、秒、非法
                 9，代表MASK格式错误
****************************************************************************/ 
uchar PubIsValidTime(uchar *psDateTime,uchar *psMaskYYYYMMDDhhmmss);

/****************************************************************************
功能： 	计算从输入时间开始,经过间隔时间后的时间点,有闰年处理
输入参数	 psDateTime:      需要累加的基础时间,格式为YYYYMMDDhhmmss
                 pszDateTimeOut:  累加后的时间,格式为YYYYMMDDhhmmss
                 ulInterval:      需要累加的时间,可为秒,分,时,天,月,年
                 psCalMode:       标记累加的单位,分别代表秒,分,时,天,月,年;
                                  可为“YY” “MM” “DD”“hh”“mm”“ss”，
                                  如果模式错误或者为NULL，则默认为“ss
输出参数	 None
返回：  	 None 
****************************************************************************/ 
void PubCalDateTime(uchar *psDateTimeIn, uchar *pszDateTimeOut, ulong ulInterval, uchar *psCalMode);

/****************************************************************************
功能： 	计算两个时间点之间的间隔
输入参数	psTimeBegin:    开始时间,格式为YYYYMMDDhhmmss
                psTimeEnd:      结束时间,格式为YYYYMMDDhhmmss
                psCalMode:      间隔的单位,分别代表秒,分,时,天,月,年；
                                可为“YY”“MM”“DD”“hh”“mm”“ss”；
                                如果模式错误或者为NULL，则默认为“ss”
输出参数	pucSignal:      相减的正负, 1:负,0:正
返回：  	间隔时间
****************************************************************************/ 
ulong PubCalInterval(uchar *psTimeBegin, uchar *psTimeEnd, uchar *psCalMode, uchar *pucSignal);

/****************************************************************************


                                      4	输入函数
 
   
	 
*****************************************************************************/
/****************************************************************************
功能： 	在超时时间内等待确认及取消键 
输入参数	nWaitTime    >0: 等待输入的时间，单位为秒
                             =0: 立即返回.
输出参数	无
返回：  	0           按下ENTER键
                0xFF        按下CANCEL键或超时.
****************************************************************************/
uchar PubYesNo(ushort nWaitTime);

/****************************************************************************
功能： 	读取布尔参数的值
输入参数	ucLine       显示提示信息的行，大字体显示
                pszPrompt    需要显示的提示信息。
输出参数	pucResult    布尔参数的值.
返回：  	0            输入OK
                0xFF         用户取消或者超时（超时时间为60秒）
****************************************************************************/
uchar PubGetBoolParam(uchar *pszPrompt, uchar ucLine, uchar *pucResult);

/****************************************************************************
功能： 	格式化输入金额，将会用到屏幕的第6-7行进行格式化显示
输入参数	pszPrefix    货币代码的指针，例如"HKD", "SGD$"，范围是[0,4]字
                             节，可以为NULL 
                ucDeciPos    小数点的位置，范围是[0,5]，例如输入”RMB”的时候
                             是2
                ucMinLen     金额的最小长度
                ucMaxLen     金额的最大长度
                ucTimeout    输入超时时间，单位秒，0表示立即返回.
                ucMisc       控制参数。GA_SEPARATOR:表示使用',符号进行金额格式
                             化显示，如: "HKD$12,345.67"；GA_NEGATIVE:显示负号，
                             如: "SGD-123,45.67"。
输出参数	pszData      输出金额字符串，以00H结尾。如显示”HKD$12,345.67”,
                             将得到”1234567”的结果。
返回：  	0            OK.
                0xFE         输入参数非法
                0xFF         按了CANCEL键，或者超时退出
****************************************************************************/
#define GA_SEPARATOR	0x80
#define GA_NEGATIVE		0x40
uchar PubGetAmount(uchar *pszPrefix, uchar ucDeciPos,
				  uchar ucMinLen, uchar ucMaxLen, uchar *pszData,
				  uchar ucTimeout, uchar ucMisc);

/****************************************************************************
功能： 	输入数字字符串，字母字符串，密码
输入参数	ucMode     输入的模式
                  NUM_IN		0x01  // 数字输入
                  ALPHA_IN		0x02  // 字母数字混合输入
                  PASS_IN		0x04  // 秘密输入
                  CARRY_IN		0x08  // 有传入数据
                  ECHO_IN		0x10  // 回显传入数据,按键则清除原数据
                ucMinlen    输入的最小长度
                ucMaxlen    输入的最大长度
                pszOut      在CARRY_IN及ECHO_IN两种模式时传入的数据（字符串），
                            以’\0’结尾
                ucTimeOut   输入超时时间，单位秒，0表示立即返回.
输出参数	pszOut      输入的字符串，以’\0’结尾
返回：  	0           按下ENTER键
                0xFF        按下CANCEL键或超时.
****************************************************************************/
uchar PubGetString(uchar ucMode, uchar ucMinlen, uchar ucMaxlen, uchar *pszOut, uchar ucTimeOut);

/****************************************************************************
功能： 	判断是否有按键
输入参数	无
输出参数	无
返回：  	TRUE       有按键
                FALSE      无按键
****************************************************************************/
uchar PubKeyPressed(void);

/****************************************************************************
功能： 	等待按键.
输入参数	iWaitTime    >=0: 超时时间
                             <0:  无超时时间.
输出参数	无.
返回：  	按键值或是无按键超时退出(NOKEY).
****************************************************************************/
uchar PubWaitKey(short iWaitTime);


/********************************************************************************
 * 函数名: PubSmInit
 * 功  能: 绑定只能菜单名称和内容
 * 入  参: pstMenuItem	- 初始菜单内容
 * 出  参: pstSmartMenu	- 智能菜单指针
 * 返回值: 无
 ********************************************************************************/
void PubSmInit(SMART_MENU *pstSmartMenu, MENU_ITEM *pstMenuItem);


/************************************************************************
功能: 按菜单名称开关菜单项目
输入参数: pszName		- 菜单项名称
          iFlag			- 操作 SM_ON / SM_OFF
输出参数: pstSmartMenu	- 菜单指针
返回:无
************************************************************************/
void PubSmMask(SMART_MENU *pstSmartMenu, const char *pszName, int iFlag);



/********************************************************************************
功能: 智能菜单
输入参数: pstSmartMenu   - 智能菜单指针
          uiMode - 菜单显示模式 可以为以下参数的组合，设为0则取采用缺省方式
                 SM_1ITEM_PER_LINE 每行显示一个菜单项
                （缺省显示两个,但SM_ARROW_SEL模式下强制每行一个）
                 SM_ARROW_SEL  使用上下箭头和确认键选择（缺省以数字键选择）
                 SM_ZERO_START  从0开始计算菜单项(SM_ARROW_SEL模式下无效)
                （缺省从1开始）
                 SM_MIDDLE_TITLE   居中显示标题 (缺省居左)
                 SM_MIDDLE_ITEM  居中显示菜单项(缺省居左)
        iTimeout - 超时时间（以秒为单位），设为零表示无超时 
输出参数: 无
返回:    >0   选定的交易号
         -1   取消（包括超时）
 ********************************************************************************/
int PubSmartMenuEx(SMART_MENU *pstSmartMenu, uint uiMode, int iTimeout);

  
/****************************************************************************


                                      6	文件函数
 
   
	 
*****************************************************************************/
/****************************************************************************
 Function:      Read data from file. Should use Seek() to set write pointer first.
 Param In:
    fd          file handle to be read from.
	psData      data pointer to place data read
    iNeedByte   number of bytes to be read out.
 Param Out:
    none.
 Return Code:
    total read out bytes.
****************************************************************************/
int PubFReadN(int fd, void *psData, int iNeedBytes);

/****************************************************************************
功能： 	读文件写的数据
输入参数	pszFileName   进行读操作的文件名
                lOffset        读操作的偏移量
                iLen          读出的数据长度
输出参数	psData        读出的数据首地址
返回：  	0	成功
                1	打开文件出错
                2	定位偏移地址出错
                3	读数据出错
****************************************************************************/
uchar PubFileRead(uchar *pszFileName, long lOffset, void *psData, int iLen);		//

/****************************************************************************
 Function:      Write data into file. Should use Seek() to set write pointer first.
 Param In:
    fd          file handle to be written into
	psData      pointer of data source to write into file
    iNeedBytes  number of bytes to write
 Param Out:
    none
 Return Code:
    total write bytes.
****************************************************************************/
int PubFWriteN(int fd, void *psData, int iNeedBytes);

/****************************************************************************
功能： 	将数据写入文件
输入参数	pszFileName    进行写操作的文件名
                lOffset        写操作的偏移量
                psData         写入的数据首地址
                iLen           写入的数据长度
输出参数	无
返回：  	0	成功
                1	打开文件出错
                2	定位偏移地址出错
                3	写数据出错
****************************************************************************/
uchar PubFileWrite(uchar *pszFileName, long lOffset, void *psData, int iLen);	//




/****************************************************************************


                                      7	Debug输出函数
 
   
	 
*****************************************************************************/


/****************************************************************************
功能： 	多功能Debug数据输出函数
输入参数	pszTitle     Debug数据的标题字符串，可以为NULL，不为NULL则要以
                             0x00结尾，超过32字节的会被剪掉
                psData       Debug数据指针
                usDataLen    Debug数据长度
                ucDevice     输出设备，分为：
                             屏幕输入，DEVICE_SCR(0x01)
                             打印输出，DEVICE_PRN(0x02)
                             串口1输出, DEVICE_COM1(0x04)
                             串口2输出, DEVICE_COM2(0x08)
                ucMode       数据的解析模式，分为：
                             ASC码模式，ASC_MODE(0x10) 
                             十六进制模式，HEX_MODE(0x20) 
                             ISO8583模式,ISO_MODE(0x40) 
                             TLV模式,TLV_MODE(0x80) 
输出参数	无
返回：  	0x00         成功
                0xff         失败
****************************************************************************/
uchar PubDebugOutput(uchar *pszTitle, uchar *psData, ushort nDataLen, uchar ucDevice, uchar ucMode);


/****************************************************************************
功能： 	指定PubDebugOutput函数中解包时的ISO8583包的定义结构
参数：	pvDataDef： 指向ISO8583包定义结构的指针
返回：  无
****************************************************************************/
void PubISODefine(void *pvDataDef);

#ifdef _POS_DEBUG

/****************************************************************************
 Function:      Show HEX message on screen.
 Param In:
    pszTitle    Title of the message.
    psMsg       Pointer of message to be displayed.
    uiMsgLen    Bytes of the message.
    nTimeOut    Seconds between user interaction.
 Param Out:
    none
 Return Code:   none
****************************************************************************/
void DispHexMsg(uchar *pszTitle, uchar *psMsg, uint uiMsgLen, short nTimeOut);

// print a line as hexadecimal format
int DispHexLine(uchar *pszBuff, uint uiOffset, uchar *psMsg, uint uiMsgLen);

// For Debug use, disp file name and line

/****************************************************************************
功能： 	显示文件名和行数
输入参数：	pszFileName:       显示的文件名
                ulLineNo：         显示的行数
输出参数：	无
返回：  	无
****************************************************************************/
void DispAssert(uchar *pszFileName, ulong ulLineNo);

    // debug macro for boolean expression
	#define _POS_DEBUG_WAIT		15
    #define PubASSERT(expr)                                           \
                if( !(expr) ){                                      \
                    DispAssert((uchar *)__FILE__, (ulong)(__LINE__));       \
                }

    // print string debug information
    #define PubTRACE0(sz)                                             \
            {                                                       \
                ScrCls();                                           \
                ScrPrint(0, 0, CFONT, "%s", sz);                    \
                PubWaitKey(_POS_DEBUG_WAIT);							\
            }

    #define PubTRACE1(sz, p1)                                         \
            {                                                       \
                ScrCls();                                           \
                ScrPrint(0, 0, CFONT, sz, p1);                      \
                PubWaitKey(_POS_DEBUG_WAIT);							\
            }

    #define PubTRACE2(sz, p1, p2)                                     \
            {                                                       \
                ScrCls();                                           \
                ScrPrint(0, 0, CFONT, sz, p1, p2);                  \
                PubWaitKey(_POS_DEBUG_WAIT);							\
            }

    #define PubTRACE3(sz, p1, p2, p3)                                 \
            {                                                       \
                ScrCls();                                           \
                ScrPrint(0, 0, CFONT, sz, p1, p2, p3);              \
                PubWaitKey(_POS_DEBUG_WAIT);							\
            }

    #define PubTRACEHEX(t, s, l)    DispHexMsg((t), (s), (l), _POS_DEBUG_WAIT)

#else /* _POS_DEBUG */

    #define PubASSERT(expr)
    #define PubTRACE0(sz)
    #define PubTRACE1(sz, p1)
    #define PubTRACE2(sz, p1, p2)
    #define PubTRACE3(sz, p1, p2, p3)
    #define PubTRACEHEX(t, s, l)

#endif /* _POS_DEBUG */


/****************************************************************************
                           条件选择函数
 功能描述: 从满足条件1、2的2种状态中选择，或者不选。函数返回相应的选择
 输入参数:                 pszTitle:     第一行显示标题
                           bCondition:  条件值(TRUE FALSE)
                           pszTruePrmpt: 条件成立提示
                           pszFalsePrmpt:条件不成立提示
                           pszOption1:     选项一提示
                           pszOption2:     选项二提示
                           iTimeOut:   超时时间，秒, 0表示无超时
 输出参数: 
 返 回 值:                 KEY1－选择第一个选项
                           KEY2－选择第二个选项
                           KEYCANCEL－取消选择
                           KEYENTER －不作选择（可继续下一个项目的处理）
使用示例:   key = OptionsSelect("消费撤销流程设置", glb_file.pre_cm_rev_swipe == '1',
                            "状态:刷卡", "状态:不刷卡", "1.刷卡  ", "2.不刷卡", 0);

****************************************************************************/
uchar PubSelectOptions(uchar * pszTitle, uchar bCondition,  uchar* pszTruePrmpt,  uchar* pszFalsePrmpt, 
				 	                 uchar* pszOption1,     uchar* pszOption2,   ushort iTimeOut );

/****************************************************************************
函    数:  Api_OneTwoSelect()
功    能： 提供2种方式选择  仅接收'1''2',确认或者取消按键
传入参数： pszItem           显示在第二行的是否询问内容
           pszChoice1        选择"1"的提示内容
           pszChoice2        选择"2"的提示内容
传出参数： NONE
返回值：   KEY1 KEY2, KEYENTER(无选择) 或者 KEYCANCEL(无选择)
例子：
           终端结帐        //第一行内容外部函数显示
           是否打印明细单？
           [1].是
           [2].否
****************************************************************************/
uchar PubSelectOneTwo(uchar *pszItem,uchar *pszChoice1,uchar *pszChoice2);

/****************************************************************************
函    数:  Api_ZeroOneSelect()
功    能： 提供是否方式选择  仅接收'0''1',确认或者取消按键
传入参数： 
         pszTitle  显示在第一行的标题内容
         pszItem   显示在第二行的是否询问内容
         pszChoice1        选择"0"的提示内容
         pszChoice2        选择"1"的提示内容
传出参数： NONE
返回值：  KEY0 KEY1, KEYENTER(无选择) 或者 KEYCANCEL(无选择)
例子：
         key = Api_ZeroOneSelect("批  结","打印结算明细单？","打印","不打印");
         if(key == KEY0)
                           ......
           批  结        
         打印结算明细单？
         0.打印 1.不打印
****************************************************************************/
uchar PubSelectZeroOne(uchar *pszTitle, uchar *pszItem, uchar *pszChoice1, uchar *pszChoice2);

/****************************************************************************
函    数:   Api_YesNoSelect()
功    能：  提供是否方式选择  仅接收确认或者取消按键
传入参数：  pszItem                显示在第二行的是否询问内容
            pszYesPrompt           "是"的提示内容
            pszNoPrompt            "否"的提示内容
传出参数：  NONE
返回值：    KEYENTER "确认" 或者 KEYCANCEL "取消"
例子：      终端结帐        //第一行内容外部函数显示
            是否打印明细单？
             [确认].是
             [取消].否
****************************************************************************/
uchar PubSelectYesNo(uchar *pszItem,uchar *pszYesPrompt,uchar *pszNoPrompt);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif	/* _UTIL_H */
