
#ifndef _XML_PROC_H
#define _XML_PROC_H

#include "util.h"
#define FILE_NAME_LEN	15
#define TELABLOCANTE_TRUE 0 
#define TELABLOCANTE_FALSE -1

int FlagProc(char *pszFlag);
char *FormProc(char *xml_data, char *szID);
void put_long(unsigned char *string, unsigned long x, unsigned char cMode) ;
int GetXmlEleVal(uchar *pszData, uchar *pszeleName, uchar *pszValue);
int StatusProc(uchar *pszData);
int XML_GetAttr(uchar *pszPos, uchar *pszAttr_key, uchar *pszAttr_value);
int PPCompProc(uchar *pszData);
int TelablocanteProc(uchar *file);
int ProcXMLPage(uchar *szFile);

#endif

// end of file
