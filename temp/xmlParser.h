
#ifndef _XML_PARSER_H
#define _XML_PARSER_H

#include "appLib.h"
#include <string.h>
#include <stdio.h>
#include "ppcomp.h"
#include <posapi.h>
#include <posapi_s80.h>

#define TAG_NOT_FOUND	-10

#define MAX_TAG_NUM		12	

/************************************************************************
BEGIN: definition of tag struct
************************************************************************/
typedef struct XML_TAG_FORM{
	char *form_begin;
	char *form_end;
	char szFormFlag[20];
	char szID[128];
}tag_form;

typedef struct XML_TAG_COMPONENTS{
	char *comp_begin;
	char *comp_end;
	char szCondition[64];
	int bClrScr;
}tag_comp;

typedef struct XML_TAG_LABEL{
	char *label_begin;
	char *label_end;
	char szText[100];
	char szID[64];
	char bPPNotify;
}tag_label;

typedef struct XML_TAG_TEXTBOX{
	char *textbox_begin;
	char *textbox_end;
	char szID[64];
	int iFormat;
	int iMaxLen;
	int iMinLen;
	ulong ulMaxValue;
	ulong ulMinValue;
	char szFormat[64];
	char szOpcional[16];
	char szText[100];//补充文本结点，linzhao
	char bSomenteleitura;//linzhao
}tag_textbox;

typedef struct XML_TAG_RADBUT{
	char *radbut_begin;
	char *radbut_end;
	char szID[20];
	char szSelectNo[2];
	char szSetFlag[20];
	char szText[20];
	char bIsSelect;
}tag_radbut;

typedef struct XML_TAG_LIST
{
	char *list_begin;
	char *list_end;
}tag_list;

typedef struct XML_TAG_MSGBOX
{
	char *msgbox_begin;
	char *msgbox_end;
	char szText[100];
}tag_msgbox;

typedef struct XML_TAG_PICBOX
{
	char *picbox_begin;
	char *picbox_end;
	char szFile[100];
}tag_picbox;

typedef struct XML_TAG_LIST_OPTION{
	char *option_begin;
	char *option_end;
	char szValue[2+1];
	char szSetFlag[20+1];
	char szText[20+1];
}tag_option;

typedef struct XML_TAG_PPINTPOS{
	char *ppintpos_begin;
	char *ppintpos_end;
	char szID[32+1];
	char szData[4096];
}tag_intpos;

typedef struct XML_TAG_PPCOMP{
	char *ppcomp_begin;
	char *ppcomp_end;
}tag_ppcomp;

typedef struct XML_ELEMENT_ATTR{
	uchar szID[64];
	uchar szInput[3][512];
	uchar szFlag[64];
}ELEMENT_ATTR_XML;

/************************************************************************
END: definition of tag struct                                                                  
************************************************************************/

int ParseForm(const char *xml_data, tag_form *pstForm);
int ParseComp(tag_comp *pstComp, char *tag_begin, char *tag_end);
int ParseLabel(tag_label *pstLabel, char *tag_begin, char *tag_end);
int ParseTextBox(tag_textbox *pstTxtbox, char *tag_begin, char *tag_end);
int ParseRadButt(tag_radbut *pstButton, char *tag_begin, char *tag_end);
int ParseList(tag_list *pstList, char *tag_begin, char *tag_end);
int ParseOption(tag_option *pstOption, char *tag_begin, char *tag_end);
int ParseComp(tag_comp *pstComp, char *tag_begin, char *tag_end);
int ParseMsgBox(tag_msgbox *pstMsgbox, char *tag_begin, char *tag_end);
int ParsePicBox(tag_picbox *pstPicbox, char *tag_begin, char *tag_end);
int ParseIntPos(tag_intpos *pstIntpos, char *tag_begin, char *tag_end);
int ParsePPcomp(tag_ppcomp *pstPPcomp, char *tag_begin, char *tag_end);


int DispLabel(const char *pszText, char ucLine, ushort nPosition);
int DispTextBox(tag_textbox *pstTxtbox, char *pszOut);
void DispRadioButton(const char *pszText, uchar ucLine);
void DispOption(const char *pszText, int iOptNum, uchar ucLine);
void DispMsgBox(const char *pszText);
void DispPictureBox(char *pszFile, int type, int x, int y);
	
int XML_AddElement(uchar *xml_buff, uchar *xml_attr, uchar *ele_name, uchar *ele_value);
int AppendPostElement(uchar *xml_buff, uchar *ele_name, uchar *id, int rc, uchar value[][512] );
int XML_GetElement(uchar *xml_buff, uchar *ele_name, uchar *xml_attr, uchar *ele_value, int *iOutLen);
int ParseElementAttr(uchar *szAttr, ELEMENT_ATTR_XML *pAttr);
int OpenCombKey(void); //add by wuc 2014.1.2
extern char glSetFormFlag[20];

#endif

// end of file
