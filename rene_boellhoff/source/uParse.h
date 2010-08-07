////////////////////////////////////////////////////////////////////////////
//
//  uParse.h
//
//  Header File für Parser.
//  
//  Hier werden die benötigten Datentypen definiert, sowie die Enums für 
//  die Codecs und die Befehlstexte aus der Konfigurationsdatei erzeugt.
//  Weiters sind die Funktionsprototypen deklariert.
//
//	Author 	: Rene Böllhoff
//  Created : around Apr 10
//
//	(c) 2005-2010 by Rene Böllhoff - reneboellhoff (at) gmx (dot) de - 
//
//////////////////////////////////////////////////////////////////////////////
//
//  This source file may be used and distributed without
//  restriction provided that this copyright statement is not
//  removed from the file and that any derivative work contains
//  the original copyright notice and the associated disclaimer.
//
//  This program is free software; you can redistribute it and/or
//  modify it under the terms of the GNU General Public License
//  as published by the Free Software Foundation; either version 2
//  of the License, or (at your option) any later version.
//										
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the GNU
//  General Public License for more details.
//
//  You should have received a copy of the GNU General Public
//  License along with this source; if not, download it	from
//  http://www.gnu.org/licenses/gpl.html	
//
////////////////////////////////////////////////////////////////////////////	

			#ifndef __UPARSE_H__
				#define __UPARSE_H__

					// Namen des Konfigurationsfiles definieren
					#ifndef UPARSE_CONFIG_FILE
						#define UPARSE_CONFIG_FILE "uParse_cfg.h"
					#endif

					// Grundeinstellungen einlesen
					#define UPARSE_CONFIG
						#include UPARSE_CONFIG_FILE
					#undef UPARSE_CONFIG

					// Parameter-Element-Datentypen definieren
					// (mit ggf. Userdaten)
					typedef struct
					{
						U8 ucType;
						union {
							U32 ulLong;
							U8  aucText [PARSE_DATA_STR_LEN];
							void *pvData;
							// es wird hier einfach der Abschnitt UPARSE_USER_DATA_TYPE eingeblendet
							#define UPARSE_USER_DATA_TYPE
								#include UPARSE_CONFIG_FILE
							#undef  UPARSE_USER_DATA_TYPE
						};
					} T_CMD_DATA;

					// Parameter-Element-Datentyp als Array
					typedef T_CMD_DATA T_CMD_DATAS [PARSE_DATA_ELEMENTS];

					// Funktionszeiger für Aufruf vom Parser bei "Befehl erkannt"
					typedef void (* T_PARSER_CALL) (U8 ucCmd, T_CMD_DATAS stCmdDatas);
					// Makro für Parser-Aufruf-Funktionsrumpf
					#define PARSER_CALL(x)  void x (U8 ucCmd, T_CMD_DATAS stCmdDatas)

					// Funktionszeiger für Codec-Aufruf vom Parser 
					typedef char (* T_PARSER_CODEC) (U8 **pucText, T_CMD_DATA *pstCmdData);
					// Makro für Parser-Codec-Funktionsrumpf
					#define PARSER_COCDEC(x)  U8 x (char **pucText, T_CMD_DATA *pstCmdData)


					// enums für Aufruf-Funktionen erzeugen
					enum {
					  PARSE_INT_HELP_FUNC, 
						#define UPARSE_FUNCTION(name,func) name,
						  #include UPARSE_CONFIG_FILE
						#undef  UPARSE_FUNCTION
					};

					// External-Deklaration für Aufruf-Funktionsrumpf erzeugen
					#define UPARSE_FUNCTION(name,func) extern void func (U8 ucEntry, T_CMD_DATAS stCmdData);
					  #include UPARSE_CONFIG_FILE
					#undef  UPARSE_FUNCTION


					// Makros zum einfachen holen der Parameter (bei Verwendung
					// von stCmdDatas als Variablennamen  !!!!
					#define GET_PAR_TYPE(v,x,t,n)		t v = (t) stCmdDatas [x].n;

					// Makros für die gängigen signed/unsigned und string typen
					#define GET_PAR_U8(v,x)		GET_PAR_TYPE(v,x,U8 ,ulLong)
					#define GET_PAR_U16(v,x)	GET_PAR_TYPE(v,x,U16,ulLong)
					#define GET_PAR_U32(v,x)	GET_PAR_TYPE(v,x,U32,ulLong)
					#define GET_PAR_I8(v,x)		GET_PAR_TYPE(v,x,I8 ,ulLong)
					#define GET_PAR_I16(v,x)	GET_PAR_TYPE(v,x,I16,ulLong)
					#define GET_PAR_I32(v,x)	GET_PAR_TYPE(v,x,I32,ulLong)
					#define GET_PAR_STR(v,x)	GET_PAR_TYPE(v,x,char *,aucText)


					// enums für Codec-Funktionen erzeugen
					enum {
					  PARSE_DUMMY_CODEC,
						#define UPARSE_USER_CODEC(name,func,dispname) name,
							#include UPARSE_CONFIG_FILE
						#undef  UPARSE_USER_CODEC
						NUM_PARSER_CODECS
					};

					// External-Deklaration für Codec-Funktionsrumpf erzeugen
					#define UPARSE_USER_CODEC(name,func,dispname) extern U8 func (char **pucEntry, T_CMD_DATA *pstCmdData);
					  #include UPARSE_CONFIG_FILE
					#undef  UPARSE_USER_CODEC

					// enums für Befehle erzeugen
					enum {
						#define UPARSE_ITEM(name,text,func) name,	
						  #include UPARSE_CONFIG_FILE
						#undef  UPARSE_ITEM
						INT_PARSE_ITEM_HELP, 
						NUM_PARSE_ITEMS
					};

					// Funktionsprototypen
					I8   cParserSingle			(char **pucTextBuffer, char *pucRefText, T_CMD_DATAS *pstData);
					I8   cParserProcessWErr (char *pucText, char **ppucErr);
					I8   cParserProcess			(char *pucText);
					void vParserPrintHelp		(void);



				#endif
