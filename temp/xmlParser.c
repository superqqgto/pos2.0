
#include "appLib.h"
#include "util.h"
#include "posapi.h"
#include "xmlParser.h"
#include "xmlProc.h"
#include "lng.h"
#include "global.h"

char glSetFormFlag[20];
char glIDName[128] = "";

static unsigned char sCielo[]={
0x02,
0x00,0x23, 
   0x00,0xf0,0xf8,0x18,0x18,0x1c,0x08,0x00,0x38,0xf6,0xf6,0x00,0xe0,0xf0,0x98,0xdc,
   0xcc,0x5c,0x78,0x70,0x20,0x00,0xff,0xfe,0x00,0x00,0xe0,0xf0,0x18,0x08,0x08,0x18,
   0xf0,0xe0,0x00,
0x00,0x23, 
   0x00,0x01,0x03,0x06,0x06,0x06,0x06,0x02,0x00,0x07,0x07,0x00,0x01,0x01,0x07,0x06,
   0x06,0x06,0x06,0x02,0x00,0x00,0x03,0x07,0x06,0x00,0x01,0x03,0x06,0x06,0x06,0x06,
   0x03,0x01,0x00 
};


///////////////////////////////////////////////////////////////////////////////////////////////////////
int ParseForm(const char *xml_data, tag_form *pstForm)
{
	char *temp1 = NULL;
	char *temp2 = NULL;

	if (NULL==xml_data || NULL==pstForm)
	{
		return -1;
	}

	memset(pstForm, 0, sizeof(tag_form));

	pstForm->form_begin = strstr(xml_data, "<form");
	if (NULL == pstForm->form_begin)
	{
		pstForm->form_end = NULL;
		return TAG_NOT_FOUND;
	}

	pstForm->form_end = strstr(pstForm->form_begin, "</form>");
	if(pstForm->form_end == NULL)
	{
		return TAG_NOT_FOUND;
	}

	if (strstr(xml_data, "<form>") == NULL)
	{
		temp2 = strstr(pstForm->form_begin, "flag");
		if (NULL!=temp2 && temp2<pstForm->form_end)
		{
			temp1 = strchr(temp2, '\"') +1;
			temp2 = strchr(temp1, '\"');
			strncpy(pstForm->szFormFlag, temp1, temp2-temp1);
			pstForm->szFormFlag[temp2-temp1] = '\0';
		}
	}
	else
	{
		strcpy(pstForm->szFormFlag, "");
		temp2 = strstr(pstForm->form_begin, "<idtela>");
		if(NULL!=temp2 && temp2<pstForm->form_end)
		{
			temp2 += strlen("<idtela>");
			temp1 = strchr(temp2, '<');
			strncpy(pstForm->szID, temp2, temp1-temp2);
			pstForm->szID[temp1-temp2] = '\0';
		}
	}

	return 0;
}

int ParseComp(tag_comp *pstComp, char *tag_begin, char *tag_end)
{
	char *temp1 = NULL;
	char *temp2 = NULL;
	char szTemp[20];

	if (NULL==tag_begin || NULL==pstComp)
	{
		return -1;
	}
	
	memset(pstComp, 0, sizeof(tag_comp));
	
	pstComp->comp_begin = strstr(tag_begin, "<componentes");
	if (NULL==pstComp->comp_begin || pstComp->comp_begin>tag_end)
	{
		return TAG_NOT_FOUND;
	}
	pstComp->comp_end = strstr(pstComp->comp_begin, "</componentes>");

	temp2 = strstr(pstComp->comp_begin, "condicao");
	if(NULL!=temp2 && temp2<pstComp->comp_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(pstComp->szCondition, temp1, temp2-temp1);
	}
	
	temp2 = strstr(pstComp->comp_begin, "limpartela");
	pstComp->bClrScr = FALSE;
	if (NULL!=temp2 && temp2<pstComp->comp_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';

		if (strcmp(szTemp, "true") == 0)
		{
			pstComp->bClrScr = TRUE;
		}
	}

	return 0;
}

int ParseLabel(tag_label *pstLabel, char *tag_begin, char *tag_end)
{
	char *temp1 = NULL;
	char *temp2 = NULL;
	char szTemp[64];
	
	if (NULL==tag_begin || NULL==pstLabel)
	{
		return -1;
	}
	
	memset(pstLabel, 0, sizeof(tag_label));

	pstLabel->label_begin = strstr(tag_begin, "<label");
	if (NULL==pstLabel->label_begin || pstLabel->label_begin>tag_end)
	{
		return TAG_NOT_FOUND;
	}
	pstLabel->label_end =strstr(pstLabel->label_begin, "</label>");
	
	temp2 = strstr(pstLabel->label_begin, "ppcompnotify");
	pstLabel->bPPNotify = FALSE;
	if (temp2!=NULL && temp2<pstLabel->label_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';
		if (strcmp(szTemp, "true") == 0)
		{
			pstLabel->bPPNotify = TRUE;
		}
	}

	temp2 = strstr(pstLabel->label_begin, "id");
	if(temp2 != NULL && temp2<pstLabel->label_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(pstLabel->szID, temp1, temp2-temp1);
		pstLabel->szID[temp2-temp1] = '\0';
	}
		
	temp1 = strchr(pstLabel->label_begin, '>') + 1;
	temp2 = strchr(temp1, '<');
	strncpy(pstLabel->szText, temp1, temp2-temp1);
	pstLabel->szText[temp2-temp1] = '\0';
	return 0;
}

int ParseTextBox(tag_textbox *pstTxtbox, char *tag_begin, char *tag_end)
{
	char *temp1;
	char *temp2;
	char szTemp[100] = "";

	if (NULL==tag_begin || NULL==pstTxtbox)
	{
		return -1;
	}

	memset(pstTxtbox, 0, sizeof(tag_textbox));

	pstTxtbox->textbox_begin = strstr(tag_begin, "<textbox");
	if (NULL==pstTxtbox->textbox_begin || pstTxtbox->textbox_begin>tag_end)
	{
		return TAG_NOT_FOUND;
	}
	pstTxtbox->textbox_end = strstr(pstTxtbox->textbox_begin, "</textbox>");

	if(NULL==pstTxtbox->textbox_end || pstTxtbox->textbox_end>tag_end)
	{
		return TAG_NOT_FOUND;
	}

	temp2 = strstr(pstTxtbox->textbox_begin, "id");
	if(NULL!=temp2 && temp2<pstTxtbox->textbox_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';
		strcpy(pstTxtbox->szID, szTemp);
	}
	
	temp2 = strstr(pstTxtbox->textbox_begin, "formato");
	if (NULL!=temp2 && temp2<pstTxtbox->textbox_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';
		pstTxtbox->iFormat = atoi((void *)szTemp);
	}

	temp2 = strstr(pstTxtbox->textbox_begin, "formatodata");
	if (NULL!=temp2 && temp2<pstTxtbox->textbox_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';
		strcpy(pstTxtbox->szFormat, szTemp);
	}

	temp2 = strstr(pstTxtbox->textbox_begin, "opcional");
	if(NULL != temp2 && temp2<pstTxtbox->textbox_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';
		strcpy(pstTxtbox->szOpcional, szTemp);
	}
	
	temp2 = strstr(pstTxtbox->textbox_begin, "minlength");
	if (NULL!=temp2 && temp2<pstTxtbox->textbox_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';
		pstTxtbox->iMinLen = atoi((void *)szTemp);
	}

	temp2 = strstr(pstTxtbox->textbox_begin, "maxlength");
	if (NULL!=temp2 && temp2<pstTxtbox->textbox_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';
		pstTxtbox->iMaxLen = atoi((void *)szTemp);
	}

	temp2 = strstr(pstTxtbox->textbox_begin, "minvalue");
	if (NULL!=temp2 && temp2<pstTxtbox->textbox_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';
		pstTxtbox->ulMinValue = atol((void *)szTemp);
	}
		
	temp2 = strstr(pstTxtbox->textbox_begin, "maxvalue");
	if (NULL!=temp2 && temp2<pstTxtbox->textbox_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';
		pstTxtbox->ulMaxValue= atol((void *)szTemp);
	}

	temp2 = strstr(pstTxtbox->textbox_begin, "somenteleitura");
	if (NULL!=temp2 && temp2<pstTxtbox->textbox_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(szTemp, temp1, temp2-temp1);
		szTemp[temp2-temp1] = '\0';
		if ( 0==strcmp(szTemp, "true"))
			pstTxtbox->bSomenteleitura = 1;
		else
			pstTxtbox->bSomenteleitura = 0;
	}
	//linzhao
	temp2 = strstr(pstTxtbox->textbox_begin, ">");
	if (NULL!=temp2 && temp2<pstTxtbox->textbox_end)
	{
		temp1 = temp2 + 1;
		strncpy(szTemp, temp1, pstTxtbox->textbox_end-temp1);
		szTemp[pstTxtbox->textbox_end-temp1] = '\0';
		strcpy(pstTxtbox->szText, szTemp);
	}

	return 0;
}

int ParseRadButt(tag_radbut *pstButton, char *tag_begin, char *tag_end)
{
	char *temp1;
	char *temp2;

	if (NULL==tag_begin || NULL==pstButton)
	{
		return -1;
	}

	memset(pstButton, 0, sizeof(tag_radbut));

	pstButton->radbut_begin = strstr(tag_begin, "<radiobutton");
	if (NULL==pstButton->radbut_begin || pstButton->radbut_begin>tag_end)
	{
		return TAG_NOT_FOUND;
	}
	pstButton->radbut_end = strstr(pstButton->radbut_begin, "</radiobutton>");

	temp2 = strstr(pstButton->radbut_begin, "id");
	if(NULL!=temp2 && temp2<pstButton->radbut_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(pstButton->szID, temp1, temp2-temp1);
		pstButton->szID[temp2-temp1] = '\0';
	}

	temp2 = strstr(pstButton->radbut_begin, "teclaaceleradora");
	if(NULL !=temp2 && temp2<pstButton->radbut_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(pstButton->szSelectNo, temp1, temp2-temp1);
		pstButton->szSelectNo[temp2-temp1] = '\0';
	}
	
	temp2 = strstr(pstButton->radbut_begin, "setflag");
	if (NULL!=temp2 && temp2<pstButton->radbut_end)
	{
		temp1 = strchr(temp2, '\"') + 1;
		temp2 = strchr(temp1, '\"');
		strncpy(pstButton->szSetFlag, temp1, temp2-temp1);
		pstButton->szSetFlag[temp2-temp1] = '\0';
	}

	temp1 = strchr(pstButton->radbut_begin, '>') +1 ;
	temp2 = strchr(temp1, '<');
	strncpy(pstButton->szText, temp1, temp2-temp1);
	pstButton->szText[temp2-temp1] = '\0';

	return 0;
}

int ParseList(tag_list *pstList, char *tag_begin, char *tag_end)
{
	if (NULL==tag_begin ||NULL==pstList)
	{
		return -1;
	}

	memset(pstList, 0, sizeof(tag_list));

	pstList->list_begin = strstr(tag_begin, "<listbox");
	if (NULL==pstList->list_begin || pstList->list_begin>tag_end)
	{
		return TAG_NOT_FOUND;
	}
	pstList->list_end = strstr(pstList->list_begin, "</listbox>");

	return 0;
}

int ParseMsgBox(tag_msgbox *pstMsgbox, char *tag_begin, char *tag_end)
{
	char *temp1 = NULL;
	char *temp2 = NULL;
	
	if(NULL == tag_begin || NULL == pstMsgbox)
	{
		return -1;
	}

	memset(pstMsgbox, 0, sizeof(tag_msgbox));

	pstMsgbox->msgbox_begin = strstr(tag_begin, "<messagebox");
	if(NULL == pstMsgbox->msgbox_begin || pstMsgbox->msgbox_begin>tag_end)
	{
		return TAG_NOT_FOUND;
	}
	
	pstMsgbox->msgbox_end = strstr(pstMsgbox->msgbox_begin, "</messagebox>");
	if(NULL == pstMsgbox->msgbox_end || pstMsgbox->msgbox_end>tag_end)
	{
		return TAG_NOT_FOUND;
	}

	temp1 = strchr(pstMsgbox->msgbox_begin, '>') + 1;
	temp2 = strchr(temp1, '<');
	strncpy(pstMsgbox->szText, temp1, temp2-temp1);
	pstMsgbox->szText[temp2-temp1] = '\0';

	return 0;
}

int ParsePicBox(tag_picbox *pstPicbox, char *tag_begin, char *tag_end)
{
	char *temp1 = NULL;
	char *temp2 = NULL;
	
	if(NULL == tag_begin || NULL == pstPicbox)
	{
		return -1;
	}

	memset(pstPicbox, 0, sizeof(tag_picbox));

	pstPicbox->picbox_begin = strstr(tag_begin, "<picturebox");
	if(NULL == pstPicbox->picbox_begin || pstPicbox->picbox_begin>tag_end)
	{
		return TAG_NOT_FOUND;
	}
	
	pstPicbox->picbox_end= strstr(pstPicbox->picbox_begin, "</picturebox>");
	if(NULL == pstPicbox->picbox_end || pstPicbox->picbox_end>tag_end)
	{
		return TAG_NOT_FOUND;
	}

	temp1 = strstr(pstPicbox->picbox_begin, "imagem=");
	if(temp1 == NULL || temp1 > pstPicbox->picbox_end)
	{
		return TAG_NOT_FOUND;
	}

	temp1 += strlen("imagem=");
	temp1 += 1;
	temp2 = strchr(temp1, '"');
	memcpy(pstPicbox->szFile, temp1, temp2-temp1);
	pstPicbox->szFile[temp2-temp1] = '\0';

	return 0;
}



int ParseOption(tag_option *pstOption, char *tag_begin, char *tag_end)
{
	char *temp1;
	char *temp2;

	if (NULL==tag_begin || NULL==pstOption)
	{
		return -1;
	}

	pstOption->option_begin = strstr(tag_begin, "<opcao");
	if (NULL==pstOption->option_begin || pstOption->option_begin>tag_end)
	{
		return TAG_NOT_FOUND;
	}
	pstOption->option_end = strstr(pstOption->option_begin, "</opcao>");

	temp2 = strstr(pstOption->option_begin, "valor");
	if(NULL != temp2 && temp2 < pstOption->option_end)
	{
		temp1 = strchr(temp2, '\"')+1;
		temp2 = strchr(temp1, '\"');
		strncpy(pstOption->szValue, temp1, temp2-temp1);
		pstOption->szValue[temp2-temp1] = '\0';
	}

	temp2 = strstr(pstOption->option_begin, "setflag");
	if (NULL!=temp2 && temp2<pstOption->option_end)
	{
		temp1 = strchr(temp2, '\"')+1;
		temp2 = strchr(temp1, '\"');
		strncpy(pstOption->szSetFlag, temp1, temp2-temp1);
		pstOption->szSetFlag[temp2-temp1] = '\0';
	}

	temp1 = strchr(pstOption->option_begin, '>') + 1;
	temp2 = strchr(temp1, '<');
	strncpy(pstOption->szText, temp1, temp2-temp1);
	pstOption->szText[temp2-temp1] = '\0';

	return 0;
}

int ParseIntPos(tag_intpos *pstIntpos, char *tag_begin, char *tag_end)
{
	char *temp1, *temp2;
	
	if(NULL==tag_begin || NULL==pstIntpos)
	{
		return -1;
	}

	memset(pstIntpos, 0, sizeof(tag_intpos));
	pstIntpos->ppintpos_begin = strstr(tag_begin, "<intpos");
	if(NULL == pstIntpos->ppintpos_begin || pstIntpos->ppintpos_begin>tag_end)
	{
		return TAG_NOT_FOUND;
	}
	
	pstIntpos->ppintpos_end= strstr(pstIntpos->ppintpos_begin, "/>");
	if(NULL == pstIntpos->ppintpos_end || pstIntpos->ppintpos_end>tag_end)
	{
		return TAG_NOT_FOUND;
	}

	temp1 = strstr(pstIntpos->ppintpos_begin, "id=");
	if(temp1 == NULL || temp1 > pstIntpos->ppintpos_end)
	{
		return TAG_NOT_FOUND;
	}

	temp1 += strlen("id=");
	temp1 += 1;
	temp2 = strchr(temp1, '"');
	memcpy(pstIntpos->szID, temp1, temp2-temp1);
	pstIntpos->szID[temp2-temp1] = '\0';

	temp1 = strstr(pstIntpos->ppintpos_begin, "input00=");
	if(temp1 == NULL || temp1 > pstIntpos->ppintpos_end)
	{
		return TAG_NOT_FOUND;
	}

	temp1 += strlen("input00=");
	temp1 += 1;
	temp2 = strchr(temp1, '"');
	memcpy(pstIntpos->szData, temp1, temp2-temp1);
	pstIntpos->szData[temp2-temp1] = '\0';

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////
//pp_parse

int ParsePPcomp(tag_ppcomp *pstPPcomp, char *tag_begin, char *tag_end)
{
	if (NULL==tag_begin || NULL==pstPPcomp)
	{
		return -1;
	}
	
	memset(pstPPcomp, 0, sizeof(tag_ppcomp));
	
	pstPPcomp->ppcomp_begin = strstr(tag_begin, "<ppcomp>");
	if (NULL==pstPPcomp->ppcomp_begin || pstPPcomp->ppcomp_begin>tag_end)
	{
		return TAG_NOT_FOUND;
	}
	pstPPcomp->ppcomp_end = strstr(pstPPcomp->ppcomp_begin, "</ppcomp>");
	
	return 0;
}



////////////////////////////////////////////////////////////////////////////////////////////////////////
int DispStringMutiLine(void *pszStr, ushort ucLine, ushort nPosition)
{
	uchar mode, x, y, buffer[60];
	uchar ucReverFlag, ucDispLen, i;
	uchar *p, *p1;
	
	if (pszStr == NULL)
	{
		return -1;
	}
	
	mode  = (uchar)nPosition;
	y     = (uchar)ucLine;

	i = 0;
	p = pszStr;
	while(1)
	{
		memset(buffer, 0, sizeof(buffer));
		for(i=0;;i++, p++)
		{
			buffer[i] = *p;
			if(i == (NUM_MAXZHCHARS-1))
			{
				if(*p == 0)
				{
					ucDispLen = i;
					break;
				}
				else if(*(p+1) == 0x20)
				{
					ucDispLen = NUM_MAXZHCHARS;
					break;
				}
				else
				{
					p1 = strrchr(buffer, 0x20);
					if(p1 != NULL)
					{
						ucDispLen = p1-buffer+1;
						p -= (NUM_MAXZHCHARS-ucDispLen-1);
						buffer[ucDispLen-1] = 0;
						break;
					}
				}
			}
			else
			{
				if(*p == 0)
				{
					ucDispLen = i;
					break;
				}
			}
		}
		
		switch(mode)
		{
		case DISP_LINE_CENTER:
			ScrClrLine(y, (uchar) (y + 1));
		#ifdef _TERMINAL_DXX_
			x = (160 - ucDispLen * 6);
		#else
			x = (64 - ucDispLen * 3);
		#endif
			break;
			
		case DISP_LINE_RIGHT:
		#ifdef _TERMINAL_DXX_
			x = (320 - ucDispLen * 12);
		#else
			x = (128 - ucDispLen * 6);
		#endif
			break;
			
		case DISP_LINE_LEFT:
		default:
			x = 0;
			break;
		}

		ucReverFlag = 0;
		if (nPosition&DISP_LINE_REVER || nPosition&DISP_PART_REVER)
		{
			ucReverFlag = REVER;
		}
		
		if (nPosition&DISP_LINE_REVER)
		{
			ScrPrint(0, y, ASCII|REVER, "%*s", NUM_MAXZHCHARS, "");
		}
		ScrPrint(x, y, (uchar)(ASCII|ucReverFlag), "%s", (char *)buffer);
		y += 1;
		if(*p == 0)
		{
			break;
		}
	}

	return y;
}

//display
int DispLabel(const char *pszText, char ucLine, ushort nPosition)
{
	uchar  *p1, *p2, *p3, *p4, *p5;
	uchar szDisp[64];
	uint i;
	
	ScrClrBelow(ucLine);
	i = ucLine;
	p1 = (uchar *)pszText;
	p2 = p3 = p4 = p5 = NULL;
	while(1)
	{
		memset(szDisp, 0, sizeof(szDisp));
		p2 = strstr(p1, "\\n");
		p3 = strchr(p1, 0x0a);
		p4 = strstr(p1, ". ");
		p5 = strstr(p1, "&#xA;");
		if(p2 != NULL)
		{
			strncpy(szDisp, p1, p2-p1);
			i = DispStringMutiLine(szDisp, (ushort)i, nPosition);
			p1 = p2+strlen("\\n");
			p2 = NULL;
		}
		else if(p3 != NULL)
		{
			*p3 = 0;
			i = DispStringMutiLine(p1, (ushort)i, nPosition);
			p1 = p3+1;
			p3 = NULL;
		}
		else if(p4 != NULL)
		{
			strncpy(szDisp, p1, p4-p1);
			i = DispStringMutiLine(szDisp, (ushort)i, nPosition);
			p1 = p4+2;
			p4 = NULL;
		}
		else if(p5 != NULL)
		{
			strncpy(szDisp, p1, p5-p1);
			i = DispStringMutiLine(szDisp, (ushort)i, nPosition);
			p1 = p5+strlen("&#xA;");
			p5 = NULL;
		}
		else
		{
			i = DispStringMutiLine(p1, (ushort)i, nPosition);
			break;
		}
	}
	
	return i;
}


int DispTextBox(tag_textbox *pstTxtbox, char *pszOut)
{
	int iRet;
	uchar ucCnt, bSmallAlpha;
	ulong ulValue;
	char szData[64];

	while(1)
	{
		memset(szData, 0, sizeof(szData));
		ScrClrBelow(4);
		switch(pstTxtbox->iFormat)
		{
			case FORMAT_NUM:
				iRet = PubGetString(NUM_IN, (uchar)pstTxtbox->iMinLen, (uchar)pstTxtbox->iMaxLen, szData, 60);
				break;
				
			case FORMAT_AMT:
				DebugOutput("%s--%d, szTotalValue:%s\n", __FILE__, __LINE__, gstIntpos.szTotalValue);//linzhao
				//add by ivy 2013.12.31
				if(strlen(gstIntpos.szTotalValue) != 0) //modify by wuc 2014.4.1
				{
					PubConvAmount(NULL, gstIntpos.szTotalValue, 2, 0, szData, GA_SEPARATOR);
					DebugOutput("%s-%d\n szTotalVale:%s, szData:%s\n", __FILE__, __LINE__, 
						gstIntpos.szTotalValue, szData);//linzhao
					SetScreenLine(ROW_8_LINE);
					PubDispString(szData, 3|DISP_LINE_CENTER);
					//PubDispString(_T("CORRECT ? Y/N "), 5|DISP_LINE_CENTER);
					PubDispString(_T("1 - YES"), 5|DISP_LINE_CENTER);
					ScrDrawLine(5, GRAY_COLOR);
					PubDispString(_T("2 - NO"), 6|DISP_LINE_CENTER);
					ScrDrawLine(6, GRAY_COLOR);
					ScrDrawLine(7, GRAY_COLOR);
					SetScreenLine(ROW_9_LINE);
					while(1)
					{
						iRet = PubWaitKey(USER_OPER_TIMEOUT);
						if(iRet == KEY1)
						{
							memset(szData, 0, sizeof(szData));
							memcpy(szData, gstIntpos.szTotalValue, strlen(gstIntpos.szTotalValue));
							strcpy(pszOut, szData);
							memset(gstIntpos.szTotalValue, 0, sizeof(gstIntpos.szTotalValue)); //add by ivy 2014.1.15
							return 0;
						}
						else if(iRet == KEY2)
						{
							ScrClrBelow(3);
							memset(szData, 0, sizeof(szData));
							memset(gstIntpos.szTotalValue, 0, sizeof(gstIntpos.szTotalValue)); //add by ivy 2014.1.15
							break;
						}
						else if(iRet == KEYCANCEL || iRet == NOKEY)
						{
							return -1;
						}
					}
				}
				//add end
				iRet = PubGetAmount(NULL, 2, (uchar)pstTxtbox->iMinLen, (uchar)pstTxtbox->iMaxLen, szData, 60, GA_SEPARATOR);
				break;
				
				//只能输入数字，与文档要求不符合。 linzhao
			/*case FORMAT_STR:
				iRet = PubGetString(ALPHA_IN|ECHO_IN, (uchar)pstTxtbox->iMinLen, (uchar)pstTxtbox->iMaxLen, szData, 60);
				break;
				*/

			case FORMAT_DATE:	//DD/MM/AA
				if(strcmp(pstTxtbox->szFormat, "DD/MM/AA") == 0)
				{
					pstTxtbox->iMinLen = pstTxtbox->iMaxLen = 6;
				}
				else if(strcmp(pstTxtbox->szFormat, "DD/MM/AAAA") == 0)
				{
					pstTxtbox->iMinLen = pstTxtbox->iMaxLen = 8;
				}
				else if(strcmp(pstTxtbox->szFormat, "MM/AA") == 0)
				{
					pstTxtbox->iMinLen = pstTxtbox->iMaxLen = 4;
				}
				iRet = PubGetString(NUM_IN, (uchar)pstTxtbox->iMinLen, (uchar)pstTxtbox->iMaxLen, szData, 60);
				break;
				
				//linzhao
			case FORMAT_STR_SPECIAL:
			case FORMAT_STR:
				strcpy(szData, pstTxtbox->szText);
				ScrGotoxy(0, 96);
				if (0==pstTxtbox->bSomenteleitura)
				{
					iRet = GetString(szData, 0xF7, (uchar)pstTxtbox->iMinLen, (uchar)pstTxtbox->iMaxLen);//linzhao
				}
				else if (1==pstTxtbox->bSomenteleitura)
				{
					PubDispString(szData, DISP_LINE_LEFT);
					PubWaitKey(3);
				}
				break;

			case FORMAT_STR_UPPER_LEFT:
				iRet = GetString(szData, 0xF7, (uchar)pstTxtbox->iMinLen, (uchar)pstTxtbox->iMaxLen);//linzhao
				for(ucCnt=1; ucCnt<=szData[0]; ucCnt++)
				{
					if (szData[ucCnt]>='a' && szData[ucCnt]<='z')
					{	   
						szData[ucCnt] += ('A' - 'a');
					}
				}
				break;

			case FORMAT_PW:	//password
				iRet = PubGetString(PASS_IN, (uchar)pstTxtbox->iMinLen, (uchar)pstTxtbox->iMaxLen, szData, 60);
				break;
				
			default:
				return 0;
				break;
		}

		if(iRet != 0)
		{
			return iRet;
		}

		if(pstTxtbox->ulMinValue != 0)
		{
			ulValue = atol(szData);
			if((pstTxtbox->iFormat == 7) && (ulValue != pstTxtbox->ulMinValue))
			{
				ScrClrBelow(4);
				PubDispString(_T("PASSWORD INCORRECT"), DISP_LINE_CENTER|4);
				PubDispString(_T("RE-ENTER"), DISP_LINE_CENTER|5);
				PubWaitKey(5);
				continue;
			}
			
			if(ulValue < pstTxtbox->ulMinValue)
			{
				ScrClrBelow(4);
				PubDispString(_T("VALUE TOO SMALL"), DISP_LINE_CENTER|4);
				PubDispString(_T("RE-ENTER"), DISP_LINE_CENTER|5);
				PubWaitKey(5);
				continue;
			}
		}

		if(pstTxtbox->ulMaxValue != 0)
		{
			ulValue = atol(szData);
			if((pstTxtbox->iFormat == 7) && (ulValue != pstTxtbox->ulMinValue))
			{
				ScrClrBelow(4);
				PubDispString(_T("PASSWORD INCORRECT"), DISP_LINE_CENTER|4);
				PubDispString(_T("RE-ENTER"), DISP_LINE_CENTER|5);
				PubWaitKey(5);
				continue;
			}
			
			if(ulValue > pstTxtbox->ulMaxValue)
			{
				ScrClrBelow(4);
				PubDispString(_T("VALUE TOO BIG"), DISP_LINE_CENTER|4);
				PubDispString(_T("RE-ENTER"), DISP_LINE_CENTER|5);
				PubWaitKey(5);
				continue;
			}
		}

		break;
	}

	strcpy(pszOut, szData);
	return 0;
}

void DispRadioButton(const char *pszText, uchar ucLine)
{
	if (NULL == pszText)
	{
		return;
	}

	PubShowMsg(ucLine, (uchar *)pszText);
	
	return;
}

void DispOption(const char *pszText, int iOptNum, uchar ucLine)
{
	char szTemp[30];
	
	if (NULL == pszText)
	{
		return;
	}

	memset(szTemp, 0, sizeof(szTemp));
	sprintf(szTemp, "%d. %s", iOptNum, pszText);
	
	PubShowMsg(ucLine, szTemp);
	
	return;
}

void DispMsgBox(const char *pszText)
{
	if(pszText == NULL)
	{
		return;
	}

	ScrClrBelow(2);
	DispLabel(pszText, 2, DISP_LINE_CENTER);
	PubWaitKey(USER_OPER_TIMEOUT);
}

void DispPictureBox(char *pszFile, int type, int x, int y)
{
#ifdef _TERMINAL_DXX_
	if(pszFile == NULL)
	{
		return;
	}

	ScrProcessImage(pszFile, type, 0, x, y);
	ScrProcessImage(pszFile, type, 1, x, y);
	ScrProcessImage(pszFile, type, 3, x, y);
#else
	ScrDrLogoxy(93, 0, sCielo);
#endif
}
//////////////////////////////////////////////////////////////////////////////////

int XML_GetElement(uchar *xml_buff, uchar *ele_name, uchar *xml_attr, uchar *ele_value, int *iOutLen)
{
	uchar *ptr_begin, *ptr_attr, *ptr_end, ucAttiFlag;
	int ele_value_len = 0;
	uchar szTag[64] = "";
	
	if(xml_buff == NULL || strlen(xml_buff) == 0)
	{
		return -1;
	}

	ptr_begin = NULL;
	ptr_end = NULL;
	ucAttiFlag = 0;

	ptr_begin = strchr(xml_buff, '<');
	ptr_end = strchr(xml_buff, '>');
	ptr_attr = NULL;

	if(ptr_begin != NULL)
	{
		ptr_attr = strchr(ptr_begin, 0x20);
		if((ptr_attr != NULL) && (ptr_attr < ptr_end))
		{
			ucAttiFlag = 1;
			memcpy(ele_name, ptr_begin+1, ptr_attr-ptr_begin-1);
			ele_name[ptr_attr-ptr_begin-1] = 0;
		}
		else
		{
			ptr_attr = NULL;
			memcpy(ele_name, ptr_begin+1, ptr_end-ptr_begin-1);
			ele_name[ptr_end-ptr_begin-1] = 0;
		}
	}
	else
		return -1;


	ptr_begin += strlen(ele_name)+2;

	sprintf(szTag, "</%s>", ele_name);
	ptr_end = strstr(ptr_begin, szTag);
	if(ptr_end == NULL)
	{
		ptr_end = strstr(ptr_begin, "/>");
		if(ptr_end == NULL)
		{
			return -1;
		}
		else
		{
			return (ptr_end+2-xml_buff);
		}
	}

	if(ucAttiFlag)
	{
		ptr_attr = strchr(ptr_begin, '>');
		if(ptr_attr == NULL)
		{
			return -1;
		}

		memcpy(xml_attr, ptr_begin, ptr_attr-ptr_begin);
		xml_attr[ptr_attr-ptr_begin] = 0;
		ptr_begin = ptr_attr+1;
	}

//	memset(ele_value, 0x00, iExpLen);
	ele_value_len = ptr_end-ptr_begin;
	memcpy(ele_value, ptr_begin, ele_value_len);
	ele_value[ele_value_len] = 0;
	*iOutLen = ele_value_len;

	return (ptr_end+3+strlen(ele_name)-xml_buff);
}

int ParseElementAttr(uchar *szAttr, ELEMENT_ATTR_XML *pAttr)
{
	uchar *ptr1 = NULL, *ptr2 = NULL, idex[3];
	int i;

	if(szAttr == NULL)
	{
		return -1;
	}
	if(pAttr == NULL)
	{
		return -1;
	}

	memset(pAttr, 0x00, sizeof(ELEMENT_ATTR_XML));
	memset(idex, 0, 3);
	
	ptr1 = strstr(szAttr, "id");
	if(ptr1 != NULL)
	{
		ptr1 += strlen("id=");
		ptr1 += 1;
		ptr2 = strchr(ptr1, '"');
		memcpy(pAttr->szID, ptr1, ptr2-ptr1);
	}

	ptr1 = strstr(szAttr, "flag");
	if(ptr1 != NULL)
	{
		ptr1 += strlen("flag=");
		ptr1 += 1;
		ptr2 = strchr(ptr1, '"');
		memcpy(pAttr->szFlag, ptr1, ptr2-ptr1);
	}

	ptr2 = szAttr;
	while(1)
	{
		ptr1 = strstr(ptr2, "input");
		if(ptr1 != NULL)
		{
			ptr1 += strlen("input");
			idex[0] = *(ptr1++);
			idex[1] = *(ptr1++);
			i = atoi(idex);
			ptr1 += 2;
			ptr2 = strchr(ptr1, '"');
			memcpy(pAttr->szInput[i], ptr1, ptr2-ptr1);
		}
		else
			break;
	}

	return 0;
	
}

int XML_AddElement(uchar *xml_buff, uchar *xml_attr, uchar *ele_name, uchar *ele_value)
{
	int ele_name_len=0, ele_value_len=0, ele_out_len=0, attr_len=0;
	uchar *p;
	
	if(xml_buff == NULL ||  ele_name == NULL)
	{
		return -1;
	}

	ele_name_len = strlen(ele_name);
	if(ele_value != NULL)
	{
		ele_value_len = strlen(ele_value);
	}
	
	p = xml_buff;
	*(p++) = '<';
	memcpy(p, ele_name, ele_name_len);
	p += ele_name_len;
	if(xml_attr != NULL)
	{
		attr_len = strlen(xml_attr);
		*(p++) = 0x20;
		memcpy(p, xml_attr, attr_len);
		p += attr_len;
		
	}
	*(p++) = '>';

	memcpy(p, ele_value, ele_value_len);
	p += ele_value_len;
	*(p++) = '<';
	*(p++) = '/';
	memcpy(p, ele_name, ele_name_len);
	p += ele_name_len;
	*(p++) = '>';

	ele_out_len = p-xml_buff;
	return ele_out_len;
}


int AppendPostElement(uchar *xml_buff, uchar *ele_name, uchar *id, int rc, uchar value[][512] )
{
	int xml_len = 0, ele_name_len=0, id_len=0, value_len=0, ele_out_len=0;
	uchar *p;
	int i;
	
	if(xml_buff == NULL ||  ele_name == NULL)
	{
		return -1;
	}
	
	ele_name_len = strlen(ele_name);
	id_len = strlen(id);
	xml_len = strlen(xml_buff);
	
	p = xml_buff+xml_len;
	*(p++) = '<';
	memcpy(p, ele_name, ele_name_len);
	p += ele_name_len;
	*(p++) = 0x20;
	
	sprintf(p, "id=\"%s\"", id);
	p += (id_len+5);
	*(p++) = 0x20;

	if(rc != 0xff)
	{
		sprintf(p, "rc=\"%02d\"", rc);
		p += 7;
		*(p++) = 0x20;
	}

	for(i=0; i<2; i++)
	{
		if(strlen(value[i]) != 0)
		{
			sprintf(p, "value%02d=\"%s\"", i, value[i]);
			value_len = strlen(value[i]);
			p += (value_len+10);
			*(p++) = 0x20;
		}
	}

	*(p++) = '/';
	*(p++) = '>';

	ele_out_len = p - xml_buff;
	return ele_out_len;
}

/*************************++add by wuc 2014.1.2*****************************/
void xml_strlwr(char * turned)
{
	int i;
	i=0;
	while (turned[i]!=0)
	  {
	  	if  ((turned[i]>='A')&&(turned[i]<='Z'))
	  	      turned[i]+=32;
	  	i+=1;
	   }
}

/**************************************************************************
  int xml_find(char * doc,char *find_flag,int find_flag_len,int *find_location)
doc		:	被查找的字符串
doc_len		:	被查找的字符串的长度
find_flag	:	要找的字符串（find_flag中可以有任何的字符，包括'\0'
find_flag_len	:	find_flag的长度
find_location 	:	find_flag的第一个字符 在doc中的位置
**************************************************************************/
int xml_find(uchar *doc, int doc_len, char *find_flag, int find_flag_len, int *find_location)
{
	int flag,i;
	char temp[200];

	flag=0;

	for(i=0;(i<doc_len-find_flag_len+1)&&(flag==0);i++)
	{
		memcpy(temp, doc+i, find_flag_len);
		temp[find_flag_len] = 0;
		xml_strlwr(temp);     
		if (memcmp(temp,find_flag,find_flag_len)==0)
	          	flag=1;
	}

	if (flag)
	{
		*find_location=i-1;
		return(0);
	}
	else
	{
		return(1);
	}
}

/*******************************************************************************************
  int XmlAddElement(char * xml_doc,char *ele_name,char *ele_value,int value_len,int *xml_real_len)

  参数说明
  xml_doc         : xml document 名
  xml_doc_max_len : xml_doc buffer最大长度
  ele_name        : 如果是root则新建一个xml document，xml_doc 指向该xml document。value_len,ele_value任意，
                    ×xml_real_len ：为新建的xml document 的实际长度。
                    如果不是root则在xml_doc最后（</root>前)加入一个标签和标签值为ele_value 的前value_len个字符。
  ele_value       : 要新建的标签值的指针。
  value_len       : 新建标签值的总长度，程序拷贝ele_value中的value_len到xml_doc中
  xml_real_len    : xml_doc 的总长度 调用时使用是xml_doc的长度，调用成功后返回的是xml_doc的新长度。

*******************************************************************************************/
int XmlAddElement(uchar *xml_doc, int xml_doc_max_len, char *ele_name, uchar *ele_value, int value_len, int *xml_real_len)
{
	char *insert_location;
	int name_len,xml_doc_len,find_location[1];
	char tmp[ELE_NAME_MAX+1];

	
	if (ele_name[0]==0) return (101);

	name_len=strlen(ele_name);
	xml_strlwr(ele_name);
	if(name_len>ELE_NAME_MAX)
	{
		DebugOutput("ERROR!The Element's Name is TOO LONG or It is not end by normal!\n");
		return(102);
	}
	if  (value_len>XML_DOC_MAX)
	{
		DebugOutput("The length of element value is TOO LONG\n");
		return(103);
	}

	if ((strcmp(ele_name,"root")==0)||(strcmp(ele_name,"ROOT")==0))
	{
		strcpy(xml_doc,"<?xml version=\"1.0\"?><root></root>");
		*xml_real_len=strlen("<?xml version=\"1.0\"?><root></root>");
		return(0);
	}
	else
	{
		xml_doc_len=*xml_real_len;/*add by prs 20040116 */
		if ((xml_doc_len>XML_DOC_MAX)||(xml_doc_len<0))
		{	
			DebugOutput("ERROR!The xml_document length=[%d] is 0 too long !\n",xml_doc_len);
			return(100);
		}	
		if (xml_doc[0]==0)
		{
			DebugOutput("ERROR!The xml_document is NULL!\n");
			return(104);
		}


		if (xml_find(xml_doc,xml_doc_len,"</root>",7,find_location))
			return(105);

		xml_doc_len=find_location[0]+7;

		if (name_len+name_len+5+value_len+xml_doc_len>XML_DOC_MAX)
		{
			DebugOutput("ERROR!The Length of xml_document will TOO LONG!\n");
			return(106);
		}
		memcpy(tmp,"<",1);
		memcpy(tmp+1,ele_name,name_len);
		memcpy(tmp+1+name_len,">",1);
		tmp[name_len+2]=0;
		if (!xml_find(xml_doc,xml_doc_len,tmp,2+name_len,find_location))
			{
				DebugOutput("ERROR!The Length of xml_element_name repeat !\n");
				return(107);
			}
		/* 判断是否标签名是否已经存在 */
	
			if (name_len+name_len+5+value_len+xml_doc_len>xml_doc_max_len)
			{
				DebugOutput("ERROR!xml_doc over flow!\n");
				return(108);
			}	

		*xml_real_len=name_len+name_len+5+value_len+xml_doc_len;
		
		insert_location=xml_doc+find_location[0];

		/*定位指针到xml文档的最后*/

		memcpy(insert_location,"<",1);
		insert_location+=1;
		memcpy(insert_location,ele_name,name_len);
		insert_location+=name_len;
		memcpy(insert_location,">",1);
		insert_location+=1;
		/*添加新的标签名 add new element name start*/

		memcpy(insert_location,ele_value,value_len);
		insert_location+=value_len;
		/*添加新的值,add new element value*/
		memcpy(insert_location,"</",2);
		insert_location+=2;
		memcpy(insert_location,ele_name,name_len);
		insert_location+=name_len;
		memcpy(insert_location,">",1);
		insert_location+=1;
		/*添加新标签名的结束符 add new element name end*/
		memcpy(insert_location,"</root>",7);
		insert_location+=7;
		insert_location[0]=0;

		return(0);
	}
}

int OpenCombKey(void) 
{ 
	uchar ele_name[ELE_NAME_MAX], InfoOut[XML_INFO_MAX], ele_doc[XML_DOC_MAX]; 
	int iRet, len = 0; 
	
	memset(ele_name, 0, sizeof(ele_name)); 
	memset(InfoOut, 0, sizeof(InfoOut)); 
	memset(ele_doc, 0, sizeof(ele_doc)); 

	strcpy(ele_name, "root");  
	iRet = XmlAddElement(InfoOut, XML_INFO_MAX, ele_name, ele_doc, strlen(ele_doc), &len); 
	if (iRet != 0)
	{
		return -1;
	} 
	
	strcpy(ele_name, "KeyCombination"); 
	sprintf(ele_doc, "1"); 
	iRet = XmlAddElement(InfoOut, XML_INFO_MAX, ele_name, ele_doc, strlen(ele_doc), &len); 
	if (iRet != 0)
	{
		return -1;
	} 
	
	iRet = SysConfig(InfoOut, XML_INFO_MAX); 
	if(glSysCtrl.bDebugOpt)
	{
		DebugOutput("ConfigCombKey:%d", iRet);
	}
	return iRet;
}
/*************************--add by wuc 2014.1.2*****************************/

// end of file
