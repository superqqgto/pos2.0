/*********************************************************************************
 Copyright (C) 2007 XieLihong
 版权所有      2007 谢利洪 

        程序多语言支持引擎  mult-language support engine
作者        谢利洪
Email       xiliho221@163.com xielh@pazsz.com
当前版本    1.1
历史记要
2008.01.12 V1.0.012 xlh
    今天我感冒了，思路可能有些不清晰，发现bug，或者对代码有所改进的话，希望可以给我email
    经过几次思路的修改，才有了这个版本，采用两级索引
	思路：
	1. 读取文件于fileBuf中
	2. 分析文件，同时将"\n"等字符串转化为对应的ascii码，保存字符串的索引在strIdx中
	3. 对strIdx进行快速排序 (qsort)
	4. 根据strIdx[i].src[0]建立ascii索引ascIdx (CreatAscIdx)
	   如strIdx[i].src[0]为 'c' , 'c' 的值为99,所有以'c'开头的都对应于ascIdx[99]中
	   ascIdx[99].begin 表示'c'开头的字符串在字符串索引strIdx中的开始位置
	   ascIdx[99].count 表示'c'开头的字符串在字符串索引strIdx中的个数
	5.  对于函数char *Translate(char *str);
	   当printf(_T("china")); 
	   1> 'c'开头的字符 在ascIdx中个数为0,直接返回字符串
	   2> 'c'开头的字符 在ascIdx中个数为1,直接返回begin所指的字符串 //(如果语言文件有误的话，这一项将会出错)
	   3> 'c'开头的字符 在ascIdx中个数大于2,根据begin和count,进行二分查找(bsearch),
	      由于count数值不大，所有查找很快，一般两三次比较就可以匹配的字符串

2008.01.13 V1.0.013 xlh
   1.改变语言文件的格式，原来的语言文件处理'='时有点问题，同时改变语言文件的读取算法
   2."去掉'c'开头的字符 在ascIdx中个数为1,直接返回begin所指的字符串"，因为如果语言文件有误的话，会导致错误的翻译
   3.改进了文件缓冲的fileBuf的安全使用算法，原来是全部清空，现在只是最后一个加上'\0'
   想过用hash来实现查找，但最终还是放弃了，好的hash函数不容易找，一般的还不如现在的"索引＋二分查找"

2008.01.14 V1.0.0114 xlh
   1.添加对 "\x64" "\x7" 转义字符的支持

2008.01.22 v1.0.0122 xlh
   1.添加const char *GetLngName(void)，用于检测外部语言的使用情况
   2.支持UNIX和DOS两种不同的文件格式

2008.07.09 v1.0.0709 xlh
   更改 SetLng的返回值，使之与新版本的兼容
   返回 0 表示成功
   <0 表示失败，将采用内部语言

2009.08.08 v1.1.0807 xlh
   上一版本在处理多字符集字符串时，如果最后一个字符的第二个字节为5c,如BIG5的"功"字。
   由于0x5c 的字符为 '\\' 这就导致字符串最后构成了 \" 这样的字符串，从而导致解析时出错。

   1. 这一版本的解决方法为：(新的方法理论上可以解决所有的字符集)
       " SALE " = "消  费"
       1> 从行头开始，解析第一个引号
       2> 解析 " = " 这个组合
         用权的方式来判断
         权 = 0  : 没有这个组合
	     权 = 1  : \" = " 组合，如 \" = " = "的第一个
	     权 = 2  : 0x80 0x5c " = " 组合，即在 "权 = 1" 的基础上，'\' 前面的字符大于0x7f, 为多字符集的情况
	     权 = 3  : " = " 组合，单纯的这个组合
	       备注: 实际情况中，可能不存在  "权 = 1" 和"权 = 2"，在一行中同时存在而影响解析，所以代码中将"权 = 1" 和"权 = 2统一为"权 = 1" 
      3> 如果找到 "权 = 3" 即找到所需的位置，找不到的话，取找到的最大的权。

      4> 从行尾开始，解析最后一个引号。

   2.  增加行的第一个字符为 '#' 时，这一行为注释
   3.  SetLng 增加两个返回值
      -5 MAX_LNG_FILE_SIZE 过小
	  -6 MAX_LNG_STR 过小
   4. 对私有的变量和函数加上static
   5. 加大默认 MAX_LNG_FILE_SIZE 和 MAX_LNG_STR 的值



**********************************************************************************/

#include "lng.h"
#include <string.h>
//#include <io.h>
//#include <fcntl.h>
#include <posapi.h>

//语言文件缓冲的最大长度
#define MAX_LNG_FILE_SIZE  64*1024
//字符串的最大行数
#define MAX_LNG_STR        2048

typedef struct 
{
	char *src;
	char *dst;
}tStrIdx;

typedef struct 
{
	int begin;
	int count;
}tASCIdx;

static char      fileBuf[MAX_LNG_FILE_SIZE];    //文件缓冲
static tStrIdx   strIdx[MAX_LNG_STR];           //字符串索引
static tASCIdx   ascIdx[256];    // ascii 索引
static int       idxCnt = 0;                  //索引个数，即字符串的条数
static int       useExtLng = 0;   // bool 使用外部语言
static char		 lngName[20];

static int search_compare (const void *x, const void *y);
static int qsort_compare (const void *x, const void *y);

static void *bsearch(const void *key, const void *base, size_t /* nmemb */ high,
			  size_t size, int (*compar)(const void *, const void *));

static void qsort (void  *base,
            size_t nel,
            size_t width,
            int (*comp)(const void *, const void *));
static char *FormatStr(char *str);
static void CreatAscIdx(void);

//建立strIdx[i].src[0] 的索引
static void CreatAscIdx(void)
{
	int i;
	char base;
	int cnt;

	memset(ascIdx,0,sizeof(ascIdx));

	i = 0;
	do {
		cnt = 0;
		base = strIdx[i].src[0];
		ascIdx[(unsigned char)base].begin = i;
		while ((i < idxCnt) && (base == strIdx[i].src[0])) //这两个条件不可改变顺序
		{
			i++;
			cnt++;
		}
		ascIdx[(unsigned char)base].count = cnt;
	} while(i < idxCnt);
}


//格式化字符中，将 "\n"  "\r"  "\t"  "\\"  "\""  "\x32" 转化为 '\n'  '\r'  '\t' '\\' '\"' '\x32'
static char *FormatStr(char *str)
{
				           	//'0','1','2','3','4','5','6','7','8','9',                                              'A','B','C','D','E','F'
const unsigned char tab2[] = {0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x3A,0x3B,0x3C,0x3D,0x3E,0x3F,0x40,0x0A,0x0B,0x0C,0x0D,0x0E,0x0F};
	int i,j;
	char ch;
	int shift;
	ch = '\0';
	for (i=0; str[i] != '\0'; i++)
	{
		if (str[i] == '\\')
		{
			switch(str[i+1])
			{
				case 'n'  : ch = '\n'; shift = 1; break;
				case 'r'  : ch = '\r'; shift = 1; break;
				case 't'  : ch = '\t'; shift = 1; break;
				case '\\' : ch = '\\'; shift = 1; break;
				case '\"' : ch = '\"'; shift = 1; break;
				case 'x'  : 
					{
						unsigned char c;
						shift = 1;
						c = str[i+2];
						if(c >='a' && c <='z')
						{
							c = c -'a'+'A';
						}
						if(((c>='0' && c<='9') || (c>='A' && c<='F')))
						{
							ch = tab2[c-'0'];
							shift++;
						}
						c = str[i+3];
						if(c >='a' && c <='z')
						{
							c = c -'a'+'A';
						}
						if(((c>='0' && c<='9') || (c>='A' && c<='F')))
						{
							ch <<=4;
							ch |= tab2[c-'0'];
							shift++;
						}	
						
						break;
					}
				default   : ch = '\0'; break;
			}
			
			if (ch != '\0')
			{
				str[i] = ch;
				for (j=i+1; str[j] != '\0'; j++)
				{
					str[j] = str[j+shift];
				}
				str[j] = '\0';
			}
		}
	}
	return str;
}

int SetLng(char *name)
{
	char *pLineBegin=NULL;     // 指向行的开头
	char *pLineEnd=NULL;     // 指向行的结尾 '\n'
	char *pFile=NULL;           // 对文件缓冲操作的指针
	int fd;
	int fileSize;

	useExtLng = 0;
	if (name == NULL)
	{
		return -1;
	}

	fd = open(name, O_RDWR);
	if (fd < 0) //打开文件失败
	{
		return -2;
	}

	if (MAX_LNG_FILE_SIZE <= filesize(name))
	{
		// 缓冲区过小了，要加大MAX_LNG_FILE_SIZE的值了
		return -5;
	}

	fileSize = read(fd,fileBuf,sizeof(fileBuf));
	close(fd);
	if (fileSize <= 0)
	{
		return -3;
	}
	fileBuf[fileSize] = '\0';
	
	// 对文件进行分析，建立字符串索引数组
	idxCnt = 0;
	pFile = fileBuf;
	pLineEnd = NULL;
	pLineBegin = pFile;
	while (pFile <= (fileBuf+fileSize)) // <=  是因为要包含文件的结束符 '\0'
	{
	    //认为 '\n' 是行末 ,'\0' 是文件结束
		if (*pFile == '\n' || *pFile == '\0')
		{
			pLineEnd = pFile;
			*pLineEnd = '\0'; // 将行末 '\n' 转化为 '\0'
		}
		pFile++;
		if (pLineEnd != NULL )   //已经找到一整行
		{
			if (*pLineBegin != '#') // 不是注释行
			{
				char *pMark1=NULL; // 指向 第一个 "
				char *pMark2=NULL; // 指向 第二个 "
				char *pMark3=NULL; // 指向 第三个 "
				char *pMark4=NULL; // 指向 第四个 "
				char *p;  
				char *pMark2Tmp;
				char *pMark3Tmp;
				int quan=0;
				int quanTmp;

				do{		
					// 取第一个 "
					pMark1 = pLineBegin;
					while(1)
					{
						if (*pMark1 == '\"') 
						{
							// 找到了
							break;
						}
						else if (pMark1 >= pLineEnd)
						{
							// 字符串完了
							pMark1 = NULL;
							break;
						}
						pMark1++;
					}
					if (pMark1 == NULL)
					{
						// 第一个取不到，立即退出
						break;
					}

					// 取 " = ",即取第二，三个
					// 会取多次，最后取权最大那个
					p = pMark1;
					while(p < pLineEnd)
					{
GET_MARK2:
						pMark2Tmp = NULL;
						pMark3Tmp = NULL;
						quanTmp   = 0;
						// 找第二个 "
						while(p < pLineEnd)
						{
							p++;
							if (*p == '\"') 
							{
								pMark2Tmp = p;
								break;
							}
						}
						// 过滤空字符,并找 =
						while(p < pLineEnd)
						{
							p++;
							if (*p == ' ' || *p == '\t')
							{
								/* do nothing */
							}
							else if (*p == '=')
							{
								// 找到 =
								break;
							}
							else 
							{
								goto GET_MARK2;
							}
						}
						if (p >= pLineEnd)
						{
							// 字符串完了
							break;
						}
						// 这里已经找到 = 了
						//  过滤空字符,并找 第三个 "
						while(p < pLineEnd)
						{
							p++;
							if (*p == ' ' || *p == '\t')
							{
								/* do nothing */
							}
							else if (*p == '\"')
							{
								// 找到第三个 "
								pMark3Tmp = p;
								break;
							}
							else 
							{
								goto GET_MARK2;
							}
						}

						if (p >= pLineEnd)
						{
							// 字符串完了
							break;
						}
						p = pMark3Tmp -1; // 让下一次, 第三个 " 还能被解析到

						// 这里 某一组的 第二，第三个都解析到了
						
						if (*(pMark2Tmp-1) == '\\')
						{
							quanTmp = 1;
						}
						else 
						{
							quanTmp = 3;
						}

						if (quanTmp > quan)
						{
							quan = quanTmp;
							pMark2 = pMark2Tmp;
							pMark3 = pMark3Tmp;
						}
						if (quan == 3)
						{
							break;
						}
					
					}
					if (quan == 0)
					{
						// 取不到第二第三个引号，退出
						break;
					}

					// 这里 取到了 最后的 第二第三个引号


					// 取最后一个
					pMark4 = pLineEnd;
					while(1)
					{
						pMark4--;
						// 注意 if (pMark4 <= pMark3) 一定要先于 if (*pMark4 == '\"')  
						if (pMark4 <= pMark3)
						{
							// 到了第三个了还找不到
							pMark4 = NULL;
							break;					
						}
						else if (*pMark4 == '\"') 
						{
							// 找到了
							break;
						}

					}
				}while(0);

				if (pMark4 != NULL) // 所有操作均正确了
				{
					*pMark2 = '\0';   //源字符串的结束符
					*pMark4 = '\0';   //目标字符串的结束符
					strIdx[idxCnt].src = FormatStr(pMark1 + 1);
					strIdx[idxCnt].dst = FormatStr(pMark3 + 1);
					//printf("\"%s\" -> \"%s\"\n",strIdx[idxCnt].src,strIdx[idxCnt].dst);
					idxCnt++;		
					if (MAX_LNG_STR < idxCnt)
					{
						// MAX_LNG_STR 过小了，要加大
						return -6;
					}
				}

			} /* end of if (*pLineBegin != '#') */
			pLineBegin = pLineEnd + 1;
			pLineEnd = NULL;
		}
	}
	if (idxCnt == 0)
	{
		return -4;
	}
	else
	{
		qsort(strIdx,idxCnt,sizeof(strIdx[0]),qsort_compare);
	
		CreatAscIdx();
		strncpy(lngName,name,sizeof(lngName));
		useExtLng = 1;
		return 0;
	}

}

//GetLngName
const char *GetLngName(void)
{
	if (useExtLng)
	{
		return lngName;
	}
	else
	{
		return NULL;
	}

}

//translate
char *Translate(char *str)
{
	tStrIdx *line = NULL;
	tASCIdx *idx;

	if (str == NULL)
	{
		return NULL;
	}

	if (!useExtLng) //不使用外部语言
	{
		return str;
	}

	//ascIdx[(unsigned char)str[0]] 记录了以 str[0] 开头的字符串
	//在strIdx索引中从第几个开始(begin)，总共有几个(count)
	idx = &ascIdx[(unsigned char)str[0]];

	//没有的话，直接返回，更快
	if (idx->count == 0)
	{
		return str;
	}

	//只有一个时，直接返回，更快(在语言文件有误时可能会翻译出错)
	//if (idx->count == 1)
	//{
	//	return strIdx[idx->begin].dst;
	//}

	//二分查找
	line = bsearch(str, &strIdx[idx->begin], 
		idx->count, sizeof(strIdx[0]),search_compare);
	if ((line == NULL) || (line->dst[0] == '\0'))
	{
		return str;
	}
	else
	{
		return line->dst;
	}
	
}

static int search_compare (const void *x, const void *y)
{
  const char *name = x;
  const tStrIdx *line = y;

  return strcmp (name, line->src);
}


//二分查找算法 来自 uClibc
static void *bsearch(const void *key, const void *base, size_t /* nmemb */ high,
			  size_t size, int (*compar)(const void *, const void *))
{
	register char *p;
	size_t low;
	size_t mid;
	int r;

	if (size > 0)
	{				/* TODO: change this to an assert?? */
		low = 0;
		while (low < high)
		{
			mid = low + ((high - low) >> 1); /* Avoid possible overflow here. */
			p = ((char *)base) + mid * size; /* Could overflow here... */
			r = (*compar)(key, p); /* but that's an application problem! */
			if (r > 0)
			{
				low = mid + 1;
			}
			else if (r < 0)
			{
				high = mid;
			}
			else
			{
				return p;
			}
		}
	}
	return NULL;
}


static int qsort_compare (const void *x, const void *y)
{
  const tStrIdx *line1 = x;
  const tStrIdx *line2 = y;

  return strcmp (line1->src, line2->src);
}


//快速排序算法来自 uClibc
/* This code is derived from a public domain shell sort routine by
 * Ray Gardner and found in Bob Stout's snippets collection.  The
 * original code is included below in an #if 0/#endif block.
 *
 * I modified it to avoid the possibility of overflow in the wgap
 * calculation, as well as to reduce the generated code size with
 * bcc and gcc. */

static void qsort (void  *base,
            size_t nel,
            size_t width,
            int (*comp)(const void *, const void *))
{
	size_t wgap, i, j, k;
	char tmp;

	if ((nel > 1) && (width > 0))
	{
//		assert( nel <= ((size_t)(-1)) / width ); /* check for overflow */
		wgap = 0;
		do {
			wgap = 3 * wgap + 1;
		} while (wgap < (nel-1)/3);
		/* From the above, we know that either wgap == 1 < nel or */
		/* ((wgap-1)/3 < (int) ((nel-1)/3) <= (nel-1)/3 ==> wgap <  nel. */
		wgap *= width;			/* So this can not overflow if wnel doesn't. */
		nel *= width;			/* Convert nel to 'wnel' */
		do {
			i = wgap;
			do {
				j = i;
				do {
					register char *a;
					register char *b;

					j -= wgap;
					a = j + ((char *)base);
					b = a + wgap;
					if ( (*comp)(a, b) <= 0 )
					{
						break;
					}
					k = width;
					do {
						tmp = *a;
						*a++ = *b;
						*b++ = tmp;
					} while ( --k );
				} while (j >= wgap);
				i += width;
			} while (i < nel);
			wgap = (wgap - width)/3;
		} while (wgap);
	}
}
