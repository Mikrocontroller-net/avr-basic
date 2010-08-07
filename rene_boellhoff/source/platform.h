////////////////////////////////////////////////////////////////////////////
//
//  platform.h
//
//  Platformübergreifende Definitionen
//
//	- allg. gleiche Datentypen
//  - Funktionen zur Emulation der AVR-Funktionen für den Flash-Speicher
//    Trick : Man tut (programmiert) so als wenn man auf einem AVR 
//						programmiert und die AVR-Funktionen werden unter Windows
//            (transparent) nachgebildet (FLASH gibt es unter Windows nicht,
//            also wird RAM verwendet)
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

		#define U8   unsigned char		
		#define U16  unsigned short
		#define U32  unsigned long
		#define I8   signed		char
		#define I16  signed		short
		#define I32  signed		long
		#define R32  float
		#define B8   char

		#define TRUE		1
		#define FALSE		0


		
		#ifdef WIN32
			// alle anderen Platformen außer AVR (weiter unten)
			// hier wird eine Adaption der _P Befehle des AVR's gemacht, damit
			// der AVR Code (weitestgehend) 1:1 umgesetzt wird.
			// Man Programmiert quasi für den AVR, und auf einem anderen System
			// wird es (weitestgehend) korrekt umgesetzt. 

			// Makros für FLASH-Speichermodifizierer
			#define FLASHMEM
			#define FLASHMEMORY
			#define RAMMEMORY
			
			#define PSTR(x)		(x)
			#define printf_P	PRINTF
			#define PRINTF_P	PRINTF

			#define PRINTF		printf

			// Warnungen für VS6.0 disablen (hauptsächlich wegen deprecated und double als default)	
			#pragma warning (disable : 4305)
			#pragma warning (disable : 4244)
			#pragma warning (disable : 4146)
			#pragma warning (disable : 4761)
			#pragma warning (disable : 4996)

			// Makros für FLASH-Funktionen pgm_read_xxx
			#define pgm_read_byte(x)			(*((unsigned char  *) (x)))
			#define pgm_read_word(x)			(*((unsigned short *) (x)))
			#define pgm_read_dword(x)			(*((unsigned long  *) (x)))

			#define pgm_read_funcptr(x)							 *(x)
			#define pgm_read_ptr(x)									 *(x)

			// Makros für FLASH-Funktionen strxxx_P, memxxx_P
			#define strcpy_P(x,y)					strcpy (x,y)
			#define strlen_P(x)						strlen (x)
			#define strcmp_P(x,y)					strcmp (x,y)
			#define strcat_P(x,y)					strcat(x,y)
			#define stricmp_P(x,y)				stricmp (x,y)
			#define strncpy_P(x,y,z)			strncpy(x,y,z)
			#define strncmp_P(x,y,z)			strncmp(x,y,z)
			#define memcpy_P(x,y,z)				memcpy (x,y,z)
			#define strcasecmp_P(x,y)			stricmp (x,y)
			#define strcasecmp(x,y)			  stricmp (x,y)

		#endif

		#ifdef AVR
			
			#define FLASHMEM 		PROGMEM
			#define FLASHMEMORY PROGMEM
			#define RAMMEMORY


			#define PRINTF_P 		printf_P


			#define pgm_read_funcptr(x)								(pgm_read_word (x))
			#define pgm_read_ptr(x)				  ((void *) (pgm_read_word (x)))


			#include <avr/io.h>
			#include <avr/interrupt.h>
			#include <avr/pgmspace.h>
			#include <stdio.h>
			#include <string.h>


		#endif

