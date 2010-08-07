////////////////////////////////////////////////////////////////////////////
//
//  main.c
//
//  Demo des uBasic.
//
//  Hier wird die Eingabe via UART abgehändelt. Dazu gehören in erster Linie 
//  die Eingabe von load/list/run/...-Anweidungen die das uBasic ausführen 
//  bzw dessen Funktionalität zeigen.
//
//  load [number]   - Lädt Demo Programm [number]
//  list						- Listet aktuell geladenes Programm auf
//  run							- Führt geladenes Programm im Text-Modus aus.
//  compile					- Compiliert geladenes Prorgamm vor.
//  run compiled    - Führt vorcompiliertes Programm aus
//  mem             - Zeigt Speicherverbrauch an
//  test [number]   - Lädt, compiliert und führt Demo Programm [number] aus.
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


// AVR-Spezifische Header
#ifdef AVR
	#include <stdio.h>			// für printf
	#include <avr/io.h>			// IO (für UART)
	#include "platform.h"		// Platform-Defines um einheitliche Funktionen zu haben
#endif

// WIN32-Spezifische Header
#ifdef WIN32
	#include <windows.h>		// für allg. Funktionen
	#include <stdio.h>			// für printf
	#include <conio.h>			// für getch ()	
	#include <string.h>			// div. Funktionen
	#include "platform.h"		// Platform-Defines um einheitliche Funktionen zu haben

	// Diese Fkt. sind unter WIN32 nur Dummy
	int get_mem_unused(void);
	int get_mem_free(void);

#endif


// Gemeinsame Header
#include "uart.h"						// UART als IO (AVR und WIN32)
#include "ubasic.h"					// uBasic-Funktionen
#include "ubasic_tests.h"		// Test-Programme
#include "mem_check.h"			// Memcheck Funktionen (nur AVR)
#include "tokenizer.h"			// Tokenizer-Funktionen	
#include "uParse.h"					// Parser für Befehlseingabe


//Taktfrequenz
#ifndef F_CPU
	#define F_CPU 16000000UL
#endif

// Buffer für Programm (Text)
static char program[MAX_PROGRAM_LEN] = "";


#ifdef BASIC_USES_COMPILER

	// Buffer für Programm (Interpretiert)
	extern unsigned char pbuffer [MAX_COMPILER_BUFFER];
	extern unsigned int  sizeprg;

	// Fkt. zum "Droppen" eines vorinterpretiertes Tokens
	extern void drop_translated_token (unsigned char x);

#endif



extern void ubasic_set_text (void *program, CPOS leng);


//----------------------------------------------------------------------------
void basic_list(char *c) {
	while (*c) {
		if (*c == '\n') PRINTF_P (PSTR ("\r"));
		PRINTF_P (PSTR("%c"), *c++);
	}
}

//----------------------------------------------------------------------------
void load_from_flash(char* Buffer, int prog_idx) {
	const char *pstrflash;
	if (prog_idx >= (sizeof(progs)/sizeof(progs[0]))) {
		PRINTF_P (PSTR ("\n\rProgramm existiert nicht!\n\r"));
	} else {
		pstrflash = (const char*)(pgm_read_ptr(&(progs[prog_idx])));
		memcpy_P(Buffer, pstrflash, MAX_PROGRAM_LEN);
		PRINTF_P (PSTR ("\n\rProgramm %i geladen.\n\r"), prog_idx);
	}
}

//----------------------------------------------------------------------------
void basic_run_text (void)
{
	PRINTF_P (PSTR ("\n\r"));
	ubasic_reset ();
	ubasic_set_text (program, strlen (program));
	ubasic_init(RM_TEXT);
	do {
		ubasic_run();
		// Basic-Programm unterbrechen, wenn [ESC] empfangen
	  if (bUARTInputReceived)
		{
		  // Flag zurücksetzen
		  bUARTInputReceived  = FALSE;
			if (ucUARTReceivedInput == 0x1b)
			{
				PRINTF_P (PSTR ("\n\rProgramm unterbrochen...!\n\r"));
				break;
			}
		}
	} while(!ubasic_finished());
}

//----------------------------------------------------------------------------
#ifdef BASIC_USES_COMPILER
	void basic_run_compiled (void)
	{
		PRINTF_P (PSTR ("\n\r"));
		ubasic_reset ();
		ubasic_set_text ((char *) pbuffer, sizeprg);
		ubasic_init(RM_COMPILED);
		do {
			ubasic_run();
			// Basic-Programm unterbrechen, wenn [ESC] empfangen
		  if (bUARTInputReceived)
			{
			  // Flag zurücksetzen
			  bUARTInputReceived  = FALSE;
				if (ucUARTReceivedInput == 0x1b)
				{
					PRINTF_P (PSTR ("\n\rProgramm unterbrochen...!\n\r"));
					break;
				}
			}
		} while(!ubasic_finished());
	}

	//----------------------------------------------------------------------------
	void basic_compile (void)
	{
		PRINTF_P (PSTR ("\n\r"));
		sizeprg = 0;
		ubasic_set_text (program, strlen (program));
		tokenizer_translate (drop_translated_token);
		PRINTF_P (PSTR ("\n\r"));
		PRINTF_P (PSTR ("\n\rSize Compiled %d\n\rSize Source   %d\n\r"), sizeprg, strlen (program));
	}

	//----------------------------------------------------------------------------
	void basic_test_prg (int prgnum)
	{
		load_from_flash(program, prgnum);
		basic_compile ();
		basic_run_compiled ();
	}

#endif

//----------------------------------------------------------------------------
void basic_mem_info (void)
{
	PRINTF_P (PSTR ("\n\runused mem: %i\n\r"), get_mem_unused());
	PRINTF_P (PSTR ("free mem..: %i\n\r"), get_mem_free());
}



//----------------------------------------------------------------------------
void vDemoCmd (unsigned char ucCmd, T_CMD_DATAS stDatas)
{
	switch (ucCmd)
	{
		case LOADPRG : load_from_flash(program, stDatas [0].ulLong);	break;
		case LISTPRG : basic_list (program);													break;
		case RUNPRG  : basic_run_text (); 														break;
		case MEMINFO : basic_mem_info (); 														break;
#ifdef BASIC_USES_COMPILER
		case COMPILE : basic_compile ();															break;
		case RUNCOMP : basic_run_compiled (); 												break;
		case TESTPRG : basic_test_prg (stDatas [0].ulLong);						break;

#endif

	}
}



U8 ucInputPos = 0; // Wird nur gebraucht um ein Delete am Zeilenanfang zu verhindern

//----------------------------------------------------------------------------
void vUARTHandleInput (void)
{
	U8 ucData;
	char *pcErrPos;
	// wenn Zeichen empfangen
  if (bUARTInputReceived)
	{
	  // Flag zurücksetzen
	  ucData = ucUARTReceivedInput;
	  bUARTInputReceived  = FALSE;

		// bei Del (\x08 oder \x7f für vt100 via putty)
		// letztes Zeichen (sofern wir nicht am Zeilenanfang sind) löschen
		if ((ucData == 0x08) || (ucData == 0x7f))
		{
		  if (ucInputPos > 0)
			{
			  ucInputPos--;
		  	vUARTSendString_P (PSTR ("\x08 \x08"));
			}
		}
		// ansonsten ausgeben
		else
		{
		  ucInputPos++;
			vUARTSendChar (ucData);
		}
	}

	// Eingabe mit \x0d (Enter) abgeschlossen ?
	if (bUARTProcessInput)
	{
		// CRLF
		PRINTF_P (PSTR ("\n\r"));
		// und Parser aufrufen. Im Fehlerflle bekommen wir mit pucErrPos
		// die Stelle des Fehlers (wenn mehrere Befehle hintereinander stehen)
		if (cParserProcessWErr ((char *)acUARTRecData, &pcErrPos) == 0)
		  PRINTF_P (PSTR ("Ok\n\r>"));
	  else
		  PRINTF_P (PSTR ("Err : '%s'\n\r>"), pcErrPos);


	  bUARTProcessInput = FALSE;
		ucInputPos = 0;
	}
}



//----------------------------------------------------------------------------
//Hier startet das Hauptprogramm
int main(void)
{  

#ifdef BASIC_USES_CHARDEVICE

	#ifdef CHARDEVICE_BLOCK
//		extern T_CHAR_DEVICE blk_chardev;
//		tokenizer_chardevice (blk_chardev);
		tokenizer_chardevice (CHARDEV_BLOCKDEV);
	#endif
	
	#ifdef CHARDEVICE_RAM
//		extern T_CHAR_DEVICE ram_chardev;
//		tokenizer_chardevice (ram_chardev);
		tokenizer_chardevice (CHARDEV_SIMPLERAM);
	#endif

#endif

#ifdef AVR
	
  vUARTInit ();

	sei ();

#endif

	
  vUARTSendString_P (PSTR ("AVR-BASIC-Entwicklungssystem\n\r"));
  vUARTSendString_P (PSTR ("Rene Boellhoff, Jul 2010\n\r\n\r"));
	vUARTSendString_P (PSTR ("Compiliert am "__DATE__" um "__TIME__"\n\r"));
#ifdef AVR
	vUARTSendString_P (PSTR ("Compiliert mit GCC Version "__VERSION__"\n\r"));
#endif
	// Prompt
  vUARTSendString_P (PSTR ("\n\r'help' fuer Hilfe\n\r\n\r"));
  vUARTSendString_P (PSTR ("Basic-Features :\n\r"));
  vUARTSendString_P (PSTR (" - Std-Funktionalitaet (uBasic Adam Dunkels)\n\r"));

#ifdef BASIC_USES_EXTENSIONS
	vUARTSendString_P (PSTR (" - C-Erweiterungen (Aufrufe, int-Variablen, int-Arrays)\n\r"));
#endif
#ifdef BASIC_USES_COMPILER
	vUARTSendString_P (PSTR (" - Compiler-Funktionalitaet (Uebersetzen und Ausfuehren)\n\r"));
#endif
#ifdef BASIC_USES_CHARDEVICE
	vUARTSendString_P (PSTR (" - Zeichengeraete (Lesen von versch. Medien)\n\r"));
#endif
#ifdef CHARDEVICE_BLOCK
	vUARTSendString_P (PSTR ("   - Zeichengeraet BLOCK\n\r"));
#endif
#ifdef CHARDEVICE_RAM
	vUARTSendString_P (PSTR ("   - Zeichengeraet RAM\n\r"));
#endif
#ifdef BASIC_USES_FUNC_AS_OPS
	vUARTSendString_P (PSTR (" - Funktionsaufrufe fuer Operationen (Bereichspruefung usw)\n\r"));
#endif
#ifdef BASIC_USES_ERROR_TEXTS
	vUARTSendString_P (PSTR (" - Fehlertexte\n\r"));
#endif



  vUARTSendString_P (PSTR ("\n\r\n\r>"));

	while(1){

#ifdef WIN32

		vUARTSim ();

#endif

		vUARTHandleInput ();
	}


	return(0);
}

