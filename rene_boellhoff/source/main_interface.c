////////////////////////////////////////////////////////////////////////////
//
//  main_interface.c
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


	#ifdef AVR
		#include <avr/io.h>
	#endif

	#ifdef WIN32
		#define PROGMEM 
		#include <windows.h>
		#include <conio.h>
		#include <string.h>
	#endif

	#include <stdio.h>
	#include "platform.h"
	#include "tokenizer.h"
	#include "uart.h"


	
#ifdef BASIC_USES_COMPILER

	unsigned int  cnt, sizeprg = 0;
	unsigned char pbuffer [MAX_COMPILER_BUFFER];

	void drop_translated_token (unsigned char x);

#endif


typedef void (* T_RESET) (void);

void fatal_abort (char error)
{
	T_RESET reset = NULL;
	PRINTF_P (PSTR ("Fatal Abort with Error \"%s\"\n\r"), tokenizer_geterror_text (error));
#ifdef AVR
	reset ();
#endif
#ifdef WIN32
	while (getch () != 0x1b);
	exit (1);
#endif
}




#ifdef BASIC_USES_EXTENSIONS


	I16 systick = 12287;



	I16 adc [8] = { 1, 2, 3, 4, 5, 6, 7, 8, };



	I16 lcdoutfunc (T_USER_FUNC_DATAS stDatas)
	{
		PRINTF_P (PSTR ("lcd write %d %d %s\n\r"), stDatas [0].value, stDatas [1].value, stDatas [2].text);
		return 0;
	}

	I16 halffunc (T_USER_FUNC_DATAS stDatas)
	{
		if (stDatas [0].type != 1) { FATAL_ABORT (255); }
		return stDatas [0].value / 2;
	}

	I16 dblfunc (T_USER_FUNC_DATAS stDatas)
	{
		if (stDatas [0].type != 1) { FATAL_ABORT (255); }
		return stDatas [0].value * 2;
	}

	I16 resetfunc (T_USER_FUNC_DATAS stDatas)
	{
		return 0;
	}

#endif



#ifdef BASIC_USES_COMPILER

	void drop_translated_token (unsigned char x)
	{
		PRINTF_P (PSTR ("%02x "), x); 
		pbuffer [sizeprg] = x;
		cnt++; sizeprg++;
		if (cnt == 16) { cnt = 0; vUARTSendCRLF (); }
	}

#endif


#ifdef BASIC_USES_CHARDEVICE

#ifdef CHARDEVICE_RAM

	// Zeiger für Zeichengerät (RAM)
	char *ptr, *ptrbak;

//----------------------------------------------------------------------------
//  Zeichengerät RAM Routinen

	// Setze Text im Zeichengerät
	//
	// Es wird lediglich der Zeiger auf den Text (der ja im RAM liegt) übernommen
	// und eine Kopie des Zeigers (für Positionierung) gespeichert
	void ram_tokenizer_settext (void *data, CPOS len)
	{
		ptr = ptrbak = data;
	}

	// Hole Zeichen an aktueller Position
	//
	// Zeichen an aktueller Zeigerposition holen
	char ram_tokenizer_getchar	(void)
	{
		return *ptr;
	}


	// Zeiger auf nächstes Zeichen setzen
	//
	// Zeiger imkrementieren
	void ram_tokenizer_nextchar (void)
	{
		ptr++;
	}

	void ram_tokenizer_setrelpos (CPOS stPos)
	{
		ptr = &(ptrbak [(unsigned int) stPos]);
	}

	CPOS ram_tokenizer_getrelpos (void)
	{
		return (CPOS) (ptr - ptrbak);
	}

#endif



//----------------------------------------------------------------------------
//  Zeichengerät Block-Gerät (RAM-simuliert) Routinen


#ifdef CHARDEVICE_BLOCK

	#define BLOCKLEN 		16
	#define BLOCKLEN_SH	4

	void blk_load_page (int p);

	char *src;
	char tempbuffer [BLOCKLEN];
	char bufferptr;
	int  size;
	int  page;
	int  pos;

	void blk_tokenizer_settext (void *data, CPOS len)
	{
		src = (char *) data;
		size = len;
		pos = bufferptr = page = 0;
		blk_load_page (0);
	}

	void blk_load_page (int p)
	{
		I8 i;

		page = p;
		if (p > (size >> BLOCKLEN_SH)) { memset (tempbuffer, 0, BLOCKLEN); return; }

		i = 16;
		if (p == ((size >> BLOCKLEN_SH))) { i = size & (BLOCKLEN - 1); }

		memset (tempbuffer, 0, BLOCKLEN);
		memcpy (tempbuffer, &(src [p << BLOCKLEN_SH]), i);

	}

	char blk_tokenizer_getchar	(void)
	{
		return tempbuffer [(U8) bufferptr];
	}

	void blk_tokenizer_nextchar (void)
	{
		bufferptr++;
		pos++;
		if (bufferptr == BLOCKLEN)
		{
			bufferptr = 0;
			page++;
			blk_load_page (page);
		}
	}

	CPOS blk_tokenizer_getrelpos  	(void)
	{
		return (CPOS) pos;
	}

	void blk_tokenizer_setrelpos		(CPOS ps)
	{
		if (page != (pos >> BLOCKLEN_SH))
		{
			blk_load_page (ps >> BLOCKLEN_SH);
		}
		bufferptr = ps & (BLOCKLEN - 1);
		pos = ps;
	}


#endif

#endif



//----------------------------------------------------------------------------
void ubasic_set_text (void *program, CPOS leng)
{
	/*#ifdef BASIC_USES_CHARDEVICE
		#ifdef CHARDEVICE_BLOCK
			blk_set_text 			(program);
		#endif
		#ifdef CHARDEVICE_RAM
			ram_set_text			(program);
		#endif
	#else*/
		tokenizer_set_text (program, leng);
	//#endif
}

