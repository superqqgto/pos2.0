/*********************** SETIS - Automação e Sistemas ************************\

 Arquivo         : PPCOMP.H
 Projeto         : Pinpad Compartilhado
 Plataforma      : Genérica
 Data de Criacao : ---
 Autor           : Wilson F. Martins

 Descrição: Definições e funções da "API Compartilhada para Pinpad".

 ================================== HISTORICO ==================================
 data      responsável  modificação
 --------- ----------- -------------------------------------------------------
 19/09/11  Talita      - Retirado parametro de notificação do pChipDirect.
\*****************************************************************************/

#ifndef _PPCOMP_INCLUDED_
#define _PPCOMP_INCLUDED_

/* Códigos de erro do PIN-Pad Compartilhado */

#define PPCOMP_OK             0
#define PPCOMP_PROCESSING     1
#define PPCOMP_NOTIFY         2

#define PPCOMP_F1             4
#define PPCOMP_F2             5
#define PPCOMP_F3             6
#define PPCOMP_F4             7
#define PPCOMP_BACKSP         8

#define PPCOMP_INVCALL       10
#define PPCOMP_INVPARM       11
#define PPCOMP_TIMEOUT       12
#define PPCOMP_CANCEL        13
#define PPCOMP_ALREADYOPEN   14
#define PPCOMP_NOTOPEN       15
#define PPCOMP_EXECERR       16
#define PPCOMP_INVMODEL      17
#define PPCOMP_NOFUNC        18
#define PPCOMP_ERRMANDAT     19

#define PPCOMP_TABEXP        20
#define PPCOMP_TABERR        21
#define PPCOMP_NOAPPLIC      22

#define PPCOMP_PORTERR       30
#define PPCOMP_COMMERR       31
#define PPCOMP_UNKNOWNSTAT   32
#define PPCOMP_RSPERR        33
#define PPCOMP_COMMTOUT      34

#define PPCOMP_INTERR        40
#define PPCOMP_MCDATAERR     41
#define PPCOMP_ERRPIN        42
#define PPCOMP_NOCARD        43
#define PPCOMP_PINBUSY       44

#define PPCOMP_SAMERR        50
#define PPCOMP_NOSAM         51
#define PPCOMP_SAMINV        52

#define PPCOMP_DUMBCARD      60
#define PPCOMP_ERRCARD       61
#define PPCOMP_CARDINV       62
#define PPCOMP_CARDBLOCKED   63
#define PPCOMP_CARDNAUTH     64
#define PPCOMP_CARDEXPIRED   65
#define PPCOMP_CARDERRSTRUCT 66
#define PPCOMP_CARDINVALIDAT 67
#define PPCOMP_CARDPROBLEMS  68
#define PPCOMP_CARDINVDATA   69
#define PPCOMP_CARDAPPNAV    70
#define PPCOMP_CARDAPPNAUT   71
#define PPCOMP_NOBALANCE     72
#define PPCOMP_LIMITEXC      73
#define PPCOMP_CARDNOTEFFECT 74
#define PPCOMP_VCINVCURR     75
#define PPCOMP_ERRFALLBACK   76

/* Tipos - varia para cada plataforma */

#include "ppc_type.h"

/* Funções para carga e descarga da DLL (quando usado _ppcomp.c) */

int iPPCompLoadDLL (void);
void PPCompFreeDLL (void);

/* Funções de controle */

int PPEXP PP_Open (INPUT psCom);
int PPEXP PP_Close (INPUT psIdleMsg);
int PPEXP PP_Abort (void);

/* Funções básicas de pinpad */

int PPEXP PP_GetInfo (INPUT psInput, OUTPUT psOutput);
int PPEXP PP_DefineWKPAN (INPUT psInput, OUTPUT psOutput);
int PPEXP PP_Display (INPUT psMsg);
int PPEXP PP_DisplayEx (INPUT psMsg);
int PPEXP PP_StartGetKey (void);
int PPEXP PP_GetKey (void);
int PPEXP PP_StartGetPIN (INPUT psInput);
int PPEXP PP_GetPIN (OUTPUT psOutput, OUTPUT psMsgNotify);
int PPEXP PP_StartCheckEvent (INPUT psInput);
int PPEXP PP_StartCheckEventV108 (INPUT psInput);
int PPEXP PP_CheckEvent (OUTPUT psOutput);
int PPEXP PP_EncryptBuffer (INPUT psInput, OUTPUT psOutput);
int PPEXP PP_GetDUKPT (INPUT psInput, OUTPUT psOutput);
int PPEXP PP_StartChipDirect (INPUT psInput);
int PPEXP PP_ChipDirect (OUTPUT psOutput);
int PPEXP PP_StartRemoveCard (INPUT psMsg);
#ifndef _IP_POSWEB_
int PPEXP PP_RemoveCard (OUTPUT psMsgNotify);
#else /*_IP_POSWEB_*/
int PPEXP PP_RemoveCard (INPUT psMsgNotify);
#endif /*_IP_POSWEB_*/
int PPEXP PP_StartGenericCmd (INPUT psInput);
int PPEXP PP_GenericCmd (OUTPUT psOutput, OUTPUT psMsgNotify);

/* Funções de processamento de cartão */

int PPEXP PP_StartGetCard (INPUT psInput);
int PPEXP PP_GetCard (OUTPUT psOutput, OUTPUT psMsgNotify);
int PPEXP PP_ResumeGetCard (void);
int PPEXP PP_ChangeParameter (INPUT psInput);
#ifndef _IP_POSWEB_
int PPEXP PP_StartGoOnChip (INPUT psInput, INPUT psTags, INPUT psTagsOpt);
int PPEXP PP_GoOnChip (OUTPUT psOutput, OUTPUT psMsgNotify);
#else /*_IP_POSWEB_*/
int PPEXP PP_StartGoOnChip (INPUT psInput, INPUT psTags);
int PPEXP PP_GoOnChip (OUTPUT psOutput);
#endif /*_IP_POSWEB_*/
int PPEXP PP_FinishChip (INPUT psInput, INPUT psTags, OUTPUT psOutput);

/* Funções para manutenção das tabelas */

int PPEXP PP_TableLoadInit (INPUT psInput);
int PPEXP PP_TableLoadRec (INPUT psInput);
int PPEXP PP_TableLoadEnd (void);
int PPEXP PP_GetTimeStamp (INPUT psInput, OUTPUT psOutput);

/* Funções proprietárias */

void PPEXP PP_InitLib (void);
void PPEXP PP_SetDebugFunc (void (CALLBACK * pfDebugFunc) (char cData));
int PPEXP PP_SendPropCmd (INPUT psCmd, INPUT pszInput, OUTPUT pszOutput);
int PPEXP PP_OpenHandle (void *pComHandle);
int PPEXP PP_OpenFast (INPUT psCom);
int PPEXP PP_KeyLoad (INPUT psInput);

/* Funções callback usadas pela lib - implementadas pela aplicação */
int APP_iTestCancel (void);


#endif /*_PPCOMP_INCLUDED_*/


