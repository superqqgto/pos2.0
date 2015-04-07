#ifndef _LNG_H_
#define _LNG_H_

// _T 调用翻译函数，由于本来就是Translate，所以不能用于全局变量的程序
#define _T(str) Translate(str)

// _T_NOOP 只是一个标识，为了pl工具能够解析，一般用于全局变量的标识及数组中字符串的标识。
#define _T_NOOP(str) str

int SetLng(char *name); //设置语言，name为语言文件的名字，返回1设置成功，返回0采用内置语言
const char *GetLngName(void); //返回NULL表示使用内部语言，其他字符串为当前使用语言的名称
char *Translate(char *str); //翻译函数，通过宏 _T 来使用


#endif
