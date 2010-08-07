////////////////////////////////////////////////////////////////////////////
//
//  uParse.c
//
//  Implementierung des Parsers
//
//  Vorgehensweise :
//
//  Zentral gibt es eine Statemachine die in 2 Modi betrieben wird.
//  Einmal wird sie von einem ReferenzString "gefüttert" und vergleicht
//  dabei Zeichen für Zeichen (*1) mit der Eingabe, und zum anderen 
//  wird die SM durch die Eingabe "gefüttert" (bei den eingebauten Codecs). 
//  Dabei wird die Statemachine nicht direkt mit dem Zeichen gefüttert,
//  sondern es wird vorher das Zeichen kategorisiert und mit diesem
//  ermittelten Wert gearbeitet. Gleichzeitig gibt es zu der State-Machine
//  noch ein Operations-Array mit dem bestimmte Funktionen ausgeführt werden
//  (z.B. hänge Zeichen an Parameter string, Vergleiche Zeichen der Eingabe)
//  Diese Funktion wird nun auf eine Liste von ReferenzStrings (den Befehlen)
//  angewendet und mit der Eingabe verglichen. Bei Übereinstimmung wird
//  mit den ggf. ermittelten Parametern eine Funktion aufgerufen.
//
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

#ifdef WIN32
	#include <stdio.h>			// printf
#endif
	#include <string.h>			// memset
	#include "platform.h"
	#include "uParse.h"
	#include "uart.h"



	// Prototyp für Hilfefunktion
	void vParserHelpFunc (U8 ucCmd, T_CMD_DATAS stData);

	// ggf. Dummy Codec erzeugen
#ifdef PARSE_USES_CODECS
	U8 cDummyCodec (char **ppucText, T_CMD_DATA *pstData);
#endif


	// ASCII Zeichen (0-127) in 16 Kategorien einteilen
	// 00 = \0 (EOT), 01 = '0', 02 = '1', 03 = 'x', 04 = 'b', 05 = ' ',
	// 06 = '_', 07 = '"' 08 = 2..9 09 = 'A'..'F','a'..'f',
	// 0a = 'G'..'Z','g..z', 0b = '$', 0c = '-', 0d = \x01,\x02,\x03
	// 0e = alles andere < 128 außer \x04-\x1f, 0f = alles >= 128 und \x04-\x1f
	U8 aucParserCharCat [] FLASHMEM = { 
	//00  01  02  03  04  05  06  07  08  09  0a  0b  0c  0d  0e  0f 
		 0, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,  // 00 
		13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,  // 10 
		 5, 14,  7, 14, 11, 14, 14, 14, 14, 14, 14, 14, 14, 12, 14, 14,  // 20 
		 1,  2,  8,  8,  8,  8,  8,  8,  8,  8, 14, 14, 14, 14, 14, 14,  // 30 
		14,  9,  9,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10,  // 40 
		10, 10, 10, 10, 10, 10, 10, 10,  3, 10, 10, 14, 14, 14, 14,  6,  // 50 
		14,  9,  4,  9,  9,  9,  9, 10, 10, 10, 10, 10, 10, 10, 10, 10,  // 60 
		10, 10, 10, 10, 10, 10, 10, 10,  3, 10, 10, 14, 14, 14, 14, 14,  // 70 
	};



	#define ERR 0xff    // -  1 = Fehler
	#define NUM 0x80		// -128 = Zahl
	#define NAM 0x81		// -127 = Name
	#define STR 0x82		// -126 = String
	#define PIF 0x83		// -125 = Parser Item Found

	// Parser Zustandsmaschine mit den Einängen Zeichenkategorie (16) und Ist-Zustand (10)

	U8 aucParserStates [] FLASHMEM = {
	// \0    0    1    x    b   sp    _    "  num  hex   al    $    -  cdc else  err
		ERR,   1,   4, ERR, ERR, ERR, ERR, ERR,   4, ERR, ERR,   2,   0, ERR, ERR, ERR,   // 00 - zahlen
		NUM,   4,   4,   2,   3, NUM, NUM, NUM,   4, NUM, NUM, NUM, NUM, ERR, NUM, ERR,   // 01
		NUM,   2,   2, NUM,   2, NUM, NUM, NUM,   2,   2, NUM, NUM, NUM, NUM, NUM, NUM,   // 02 - hexa
		NUM,   3,   3, NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM, NUM,   // 03 - bin
		NUM,   4,   4, NUM, NUM, NUM, NUM, NUM,   4, NUM, NUM, NUM, NUM, NUM, NUM, NUM,   // 04 - dezimal
		ERR, ERR, ERR,   6,   6,   5,   6, ERR, ERR,   6,   6, ERR, ERR, ERR, ERR, ERR,   // 05 - name
		NAM,   6,   6,   6,   6, NAM,   6, NAM,   6,   6,   6, NAM, NAM, NAM, NAM, NAM,   // 06
		ERR, ERR, ERR, ERR, ERR, ERR, ERR,   8, ERR, ERR, ERR, ERR, ERR, ERR, ERR, ERR,   // 07 - string
		ERR,   8,   8,   8,   8,   8,   8, STR,   8,   8,   8,   8,   8,   8,   8,   8,   // 08
		PIF,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9,   9, ERR,   // 09 - parser sm
	};

	#undef ERR
	#undef NUM 
	#undef NAM 
	#undef STR 
	#undef PIF 

	// Parser Opcodes :
	// 1 : zeichen von ref-string mit eingabe vergleichen. bei fehler -> state -1
	// 2 : ref-string und eingabe zeiger auf nächstes zeichen != 0x20 setzen
	// 3 : umschalten der statemachine von ref-string auf eingabe (für zahlen/namen/strings)
	// 4 : hexa-zahl an wert anhängen (länge prüfen, fehler -> state -1)
	// 5 : dez-zahl an wert anhängen (länge prüfen, fehler -> state -1)
	// 6 : bin-zahl an wert anhängen (länge prüfen, fehler -> state -1)
	// 7 : zeichen in zeichenkette anhängen (länge prüfen, fehler -> state -1)
	// 8 : zahlen-erkennung abschließen. speichern und wieder zur anderen statemachine wechseln
	// 9 : names-erkennung abschließen. speichern und wieder zur anderen statemachine wechseln
	// 10: string-erkennung abschließen. speichern und wieder zur anderen statemachine wechseln
	// 11: vorzeichen (bei zahl) prüfen. wenn schon gesetzt -> fehler

	U8 aucParserOps [] FLASHMEM = {
	// \0    0    1    x    b   sp    _    "  num  hex   al    $    -  cdc else  err
			0,   0,   5,   0,   0,   0,   0,   0,   5,   0,   0,   0,  11,   0,   0,   0,   // 00 - zahlen
			8,   0,   5,   0,   0,   8,   8,   8,   5,   8,   8,   8,   8,   8,   8,   0,   // 01
			8,   4,   4,   8,   4,   8,   8,   8,   4,   4,   8,   8,   8,   8,   8,   8,   // 02
			8,   6,   6,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   8,   // 03
			8,   5,   5,   8,   8,   8,   8,   8,   5,   8,   8,   8,   8,   8,   8,   8,   // 04
			0,   0,   0,   7,   7,   0,   7,   0,   0,   7,   7,   0,   0,   0,   0,   0,   // 05 - name
			9,   7,   7,   7,   7,   9,   7,   9,   7,   7,   7,   9,   9,   9,   9,   9,   // 06
			0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   // 07 - string
			0,   7,   7,   7,   7,   7,   7,  10,   7,   7,   7,   7,   7,   7,   7,   7,   // 08
			0,   1,   1,   1,   1,   2,   1,   1,   1,   1,   1,   1,   1,   3,   1,   0,   // 09 - parser sm
	};

	// Codec Zustände und Daten
	char acParserStarts [3] FLASHMEM = {  0,  5,  7, };  // Start Zustand
	char acParseVBase   [3] FLASHMEM = { 16, 10,  2, };	// Zahlenbasis
	char acParseVMCnt   [3] FLASHMEM = {  9, 10, 33, };  // max. Eingabelänge


	// Makros zum einfachen holen der Tabellendaten (Portabilität auf AVR)
	#define GET_CHAR_CAT(x)			(((U8) x > 128) ? 15 : (U8) pgm_read_byte (&(aucParserCharCat [(U8) (x)]))
	#define GET_NEXT_STATE(x)		pgm_read_byte (&(aucParserStates [x]))
	#define GET_STATE_OPT(x)		pgm_read_byte (&(aucParserOps    [x]))


	// Befehlsfunktionszeigerliste (inkl. Hilfszeiger für automatische Hilfe) zusammenbauen
	T_PARSER_CALL apfFunctions [] FLASHMEM = {
	  vParserHelpFunc, 
		#define UPARSE_FUNCTION(name,func) func,
		  #include UPARSE_CONFIG_FILE
		#undef  UPARSE_FUNCTION
	};

	// Funktionszeigerarray für die User-Codecs zusammenbauen
#ifdef PARSE_USES_CODECS
	T_PARSER_CODEC apfCodecs [] FLASHMEM =  {
		#define UPARSE_USER_CODEC(name,func,dspname) func,
			#include UPARSE_CONFIG_FILE
		#undef  UPARSE_USER_CODEC
		cDummyCodec,
	};
#endif

	// Bei Benutzung der Hilfe Texte für die Codecs im Klartext definieren
#ifdef PARSE_USES_HELP
	char acCodecName__Number	[] FLASHMEM = { "[number]"	};
	char acCodecName__Name		[] FLASHMEM = { "[name]"		};
	char acCodecName__String	[] FLASHMEM = { "[string]"	};

	#define UPARSE_USER_CODEC(name,func,dispname) char acCodecName__##name [] FLASHMEM = { "[" dispname "]" };
		#include UPARSE_CONFIG_FILE
	#undef  UPARSE_USER_CODEC

	char *apcCodecNames [] FLASHMEM =  {
		acCodecName__Number,
		acCodecName__Name,
		acCodecName__String,
		#define UPARSE_USER_CODEC(name,func,dispname) acCodecName__##name, 
			#include UPARSE_CONFIG_FILE
		#undef  UPARSE_USER_CODEC
	};

#endif






	// Befehlstexte generieren
	#define UPARSE_ITEM(name,text,func) char acParseItem__##name [] FLASHMEM = text;
	  #include UPARSE_CONFIG_FILE
	#undef  UPARSE_ITEM

	// Befehlstext für automatische Hilfe
	char acParseItem__Help [] FLASHMEM = "help";


	// Befehlstextliste zusammenbauen
	char *apucParseItems [] FLASHMEM = {
		#define UPARSE_ITEM(name,text,func)	acParseItem__##name,
		  #include UPARSE_CONFIG_FILE
		#undef  UPARSE_ITEM
		acParseItem__Help,
	};

	// Befehlsfunktionsliste zusammenbauen
	char aucParseFuncs [] FLASHMEM = {
		#define UPARSE_ITEM(name,text,func)	func,
		  #include UPARSE_CONFIG_FILE
		#undef  UPARSE_ITEM
	  0,
	};


	// Makros zum einfachen holen der Code-Generierten Daten (und Portabilität auf AVR)
	#define GET_PARSE_FPTR(x) 	pgm_read_ptr  (&(apfFunctions 	[(U8) x]))
	#define GET_PARSE_ITEM(x) 	pgm_read_ptr  (&(apucParseItems [(U8) x]))
	#define GET_PARSE_FUNC(x) 	pgm_read_byte (&(aucParseFuncs 	[(U8) x]))


	// Makros zum Konvertieren von Zahlen
	#define HEXVALUE(c)       	((c) >= 'a' ? (c) - 'a' + 10 : (c) >= 'A' ? (c) - 'A' + 10 : (c) - '0')
	#define ADDNUMBER(x,c,b)   	x *= b; x += c;


  // Makro um Case Sensitive/Case Insensitive zu realisieren
	// (Makro wird immer verwendet, nur im PARSE_IGNORE_CASE-Fall
	// gibt es eine Verarbeitung)
#ifdef PARSE_IGNORE_CASE
  
#ifdef AVR
inline 
#endif	
	char cUCase (char ucX)
	{
	  return ( (((ucX) >= 'a') && ((ucX) <='z')) ? (ucX) ^ 0x20 : (ucX) );
	}
	
	#define UCASE(x)            cUCase (x)
#else
	#define UCASE(x)            (x)
#endif


// Folgezustand aus Istzustand und aktuellem Zeichen (über Kategorie) bestimmen
#ifdef AVR
inline 
#endif
I8 cParserNextState (I8 iChar, I8 iState, I8 *iOp)
{
	short sSA		= (short) iState * 16 + GET_CHAR_CAT (iChar));
	*iOp				= GET_STATE_OPT  (sSA);
	return				GET_NEXT_STATE (sSA);
}

// Eingabe (pucTextBuffer) mit Referenzstring (pucRefText) vergleichen,
// und bei Übereinstimmung ggf. ermittelte Werte zurückgeben
I8 cParserSingle (char **pucTextBuffer, char *pucRefText, T_CMD_DATAS *pstData)
{																																									  
	I8   	iS = 9,      	// Zustand (Start in Zustand 9)
				iC,          	// akt. Zeichen
				bS = FALSE, 	// Sign wurde gesetzt (erneutes setzen -> Fehler)
				iV = 0, 			// Wert des akt. Zeichens (für Zahlen Codec)
				iO;           // Options-Wert für interne Befehle
	U8 		cCnt = 0, 		// Zeichenzähler (für Codecs)
				cDI = 0;			// Data Index (zeigt auf aktuellen Parameter in stCmdData)
	char *pucText = *pucTextBuffer; // Zeichen-Zeiger auf akt. Zeichen
	I32 	lS = 1;				// Sign als Wert (1 oder -1)

	T_CMD_DATAS stData;	// Parameter-Elemente für Rückgabe

#ifdef PARSE_USES_CODECS
	T_PARSER_CODEC pfCodec;	// Funktionszeiger für Codec
#endif

	// erstes Zeichen != ' ' ermitteln
	while (*pucText == 0x20) { pucText++; };
	// wenn Ende dann Error-Code -2 -> EOT
	if (*pucText == 0) { return -2; }
	
	// Variablen und Parameter-Elemente initialisieren
	iS = 9; cDI = 0; cCnt = 0; bS = FALSE;
	memset (&stData, 0, sizeof (stData));
	do
	{
	  // Trick : Nur wenn wir im Status 9 sind bekommen wir Zeichen vom
		//         Referenzstring und vergleichen diesen (mit iO = 1) mit
		//         der Eingabe. In allen anderen Zuständen wird mit der
		//         Eingabe gearbeitet (für die Codecs)
		if (iS == 9) { iC  = pgm_read_byte (pucRefText); }
						else { iC  = *pucText++; iC = (iC == 9) ? 0x20 : iC; iV = HEXVALUE (iC); }
		// Folgezustand aus Ist-Zustand und akt. Zeichen ermitteln
		iS = (I8) cParserNextState (UCASE (iC), iS, &iO);
		// Internen Befehl ausführen
		switch (iO)
		{
		  // zeichen von ref-string mit eingabe vergleichen. bei fehler -> state -1
			case 1:	pucRefText++; if (UCASE (iC) != UCASE (*pucText)) { iS = (I8) -1; } pucText++; break;
			//ref-string und eingabe zeiger auf nächstes zeichen != 0x20 setzen
			case 2:	while (pgm_read_byte (pucRefText) == 0x20) { pucRefText++; }; 
				      while (*pucText == 0x20) { pucText++; }; break;
			// umschalten der statemachine von ref-string auf eingabe (für zahlen/namen/strings)
			// und ggf. ausführen des user-codecs
			case 3:	if (iC < 4) 
							{
								iS = (I8) pgm_read_byte (&(acParserStarts [(U8) (iC - 1)])); 
								stData [cDI].ucType = (U8) iC; 
								pucRefText++; cCnt = 0; bS = FALSE; 
							}
#ifdef PARSE_USES_CODECS
							else if (iC < (NUM_PARSER_CODECS + 4))
							{
								pucRefText++;
								pfCodec = pgm_read_ptr (&(apfCodecs [iC - 4]));
								stData [cDI].ucType = (U8) iC;
								if (pfCodec (&pucText, &(stData [cDI])) == 0) 
											{ iS = 9; if (cDI++ >= 4) { iS = (I8) -1; } }
								else 	{ iS = (I8) -1;  }
							}
#endif
							else
							{
								iS = (I8) -1;
							}

							break;
			// zahl an wert anhängen (je nach modus hexa,dez,bin) länge prüfen, fehler -> state -1
			case 4:
			case 5:
			case 6: ADDNUMBER (stData [(U8) cDI].ulLong, iV,  pgm_read_byte (&(acParseVBase [(U8) (iO - 4)]))); cCnt++; 
				      if (cCnt == pgm_read_byte (&(acParseVMCnt [(U8) (iO - 4)]))) { iS = (I8) -1; } break;
			// zeichen in zeichenkette anhängen (länge prüfen, fehler -> state -1)
			case 7: if (cCnt == PARSE_DATA_STR_LEN) { iS = (I8) -1; } 
							else { stData [cDI].aucText [cCnt] = iC; cCnt++; } break;
			// interner codec erfolgreich ausgeführt
			// speichern, zeiger weitersetzen und wieder zur anderen statemachine wechseln
			case 8:
			case 9:
			case 10:iS = 9; 
							if ((iO == 8) || (iO ==  9)) { pucText--;  }
							if ((iO == 9) || (iO == 10)) { stData [cDI].aucText [cCnt] = 0;  }
																			else { stData [cDI].ulLong = stData [cDI].ulLong * lS;  }
							if (cDI++ >= 4) { iS = (I8) -1; } 
				break;
			// vorzeichen (bei zahl) prüfen. wenn schon gesetzt -> fehler
			case 11: if (bS) { iS = (I8) -1; } else { bS = TRUE; lS = -1; }; break;
		}
	} while (((I8) iS) >= (I8) 0);
	// -125 ist der exit-code für refstring erfolgreich verglichen -> ok
	if (((I8) iS) == ((I8) -125)) { iS = 0; }
	// wenn alles ok, dann Parameter-Elemente kopieren, und Zeichenzeiger weitersetzen
	if (iS == 0) { memcpy (pstData, &stData, sizeof (stData)); *pucTextBuffer = pucText; }
	return iS == 0 ? 0 : (I8) -1;
}

// Eine mit \0 abgeschlossene Eingabe mit der Befehlsliste im Abchnitt PARSE_ITEM
// vergleichen und bei Übereinstimmung entsprechende Funktion mit den ggf übergebenen
// Werten aufrufen. Optional im Fehlerfalle Position zurückgeben.
I8 cParserProcessWErr (char *pucText, char **ppucErr)
{
	I8  	cRSP = 0,    		// aktuell zu vergleichender Referenz-String
				cRC;						// Return-Code der Vergleichsfunktion
	T_CMD_DATAS stData;		// Parameter-Elemente für Aufruf
	T_PARSER_CALL pfFunc; // aufzurufende Funktion
	char *pucTextBak = pucText;			// aktuelle Position speichern
	char **ppucText = &pucTextBak;	// Zeiger auf Zeichenzeiger

	// solange durchgehen bis EOT (\0) oder Fehler
	do
	{
		// über alle Elemente der Liste
		cRSP = 0;
		do
		{
		  // einzeln vergleichen
			cRC = cParserSingle (ppucText, GET_PARSE_ITEM (cRSP), &stData);
			if (cRC == (I8) -2) { return 0; } // EOT -> alles Ok. Eingabe korrekt
			if (cRC == (I8) -1) { cRSP++; }   // Fehler -> nächstes Element
		} while ((cRC != 0) && (cRSP < NUM_PARSE_ITEMS)); 
		// Funktionszeiger über Funktionszeigerindex holen
		pfFunc = GET_PARSE_FPTR (GET_PARSE_FUNC ((U8) cRSP));
		// wenn eintrag gefunden -> Funktion mit Parametern aufrufen
		if (cRC == 0) { pfFunc (cRSP, stData); }
	} while (!((cRC == (I8) -1) || (cRC == (I8) -2)));
	// wenn rückgabe der fehlerposition gewünscht ...
	if (ppucErr != NULL)
	{
	  // dann fehlerposition setzen 
		*ppucErr = *ppucText;
	}
	return cRC == ((I8) -2) ? 0 : (I8) -1;  // 0 für alles ok; -1 für fehler
}

// Verkürzte Variante ohne Fehlerpositionsrückgabe.
I8 cParserProcess (char *pucText)
{
	return cParserProcessWErr (pucText, NULL);
}


// Abschnitt "automatisch generierte Hilfe"
#ifdef PARSE_USES_HELP

	// Zeichenkette aus dem Flash ausgeben
	void vParserPrint_P (char *pucText)
	{
		while (pgm_read_byte (pucText)) { PARSE_PRINT_HELP_CHAR (pgm_read_byte (pucText++)); }
	}

  // Alle Befehle ausgeben 
	void vParserHelpFunc (U8 ucCmd, T_CMD_DATAS stData)
	{
		U8 ucCnt;
		I8 *pucText, cCh;

		// über alle einträge
		for (ucCnt = 0; ucCnt < NUM_PARSE_ITEMS; ucCnt++)
		{
		  pucText = GET_PARSE_ITEM (ucCnt);
			do
			{
				// zeichen für zeichen ausgeben
				cCh = pgm_read_byte (pucText);
				// nur ascii-zeichen (0-127)
				if (cCh > 0)
				{
								// die ersten 3-x steuerzeichen im klartext anzeigen
								if (cCh < NUM_PARSER_CODECS + 3) 
								{ 
									vParserPrint_P (pgm_read_ptr(&(apcCodecNames [cCh - 1]))); 
								}
								// ansonsten alles ausgeben
					else  if (cCh > 0x1f) 
								{ 
									PARSE_PRINT_HELP_CHAR (cCh); 
								}
				}
				// nächstes zeichen
				pucText++;
			} while (cCh);
			vParserPrint_P (PSTR("\n\r"));
		}  
	}

	// funktion um hilfe aus dem programm heraus aufzurufen
	void vParserPrintHelp		(void)
	{
		T_CMD_DATAS stDummy;
		vParserHelpFunc (0, stDummy);
	}
#else
  // wenn keine automatische Hilfe benötigt wird,
	// wird eine dummy-funktion benötigt 
	void vParserHelpFunc (U8 ucCmd, T_CMD_DATAS stData)
	{
	}

#endif


	// dummy codec
#ifdef PARSE_USES_CODECS
	U8 cDummyCodec (char **ppucText, T_CMD_DATA *pstData)
	{
		return -1;
	}
#endif

