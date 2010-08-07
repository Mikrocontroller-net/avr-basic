////////////////////////////////////////////////////////////////////////////
//
//  uParse_cfg.h
//
//  Parser-Konfiguration für uBasic Demo
//
//  Hier sind alle Befehle für das uBasic Demo definiert.
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

	#ifdef UPARSE_CONFIG

		// Anzahl Datenelemente bei Parameterübergabe
		#define PARSE_DATA_ELEMENTS					4
		// Maximale Stringlänge eines einzelnen Datenelementes
		#define PARSE_DATA_STR_LEN					16


		#define PARSE_USES_HELP

		#ifdef AVR
			#define PARSE_PRINT_HELP_CHAR(x)		vUARTSendChar (x)
		#endif
		#ifdef WIN32
		  #define PARSE_PRINT_HELP_CHAR(x)		printf ("%c", x);
		#endif

	#endif


	#ifdef UPARSE_FUNCTION
	//UPARSE_FUNCTION (name,                 	function        )
	  UPARSE_FUNCTION (DEMO_FUNC, 						vDemoCmd 				)
	#endif

	// in den parse-item-texten gibt es folgende steuerzeichen :
	// \x01 = zahl (z.b. -1, 0x34, -0b1100110, $1234, ...)
	// \x02 = name erstes zeichen alpha + '_', danach mit zahlen (z.b. _XY945, TEXT01, ...) 
	// \x03 = string (kompletten 8 bit) (also "hallo welt", "äöp")

	#ifdef UPARSE_ITEM
	//UPARSE_ITEM(name,					text,										func		)	
		UPARSE_ITEM(LOADPRG,			"load \x01", 						DEMO_FUNC)
		UPARSE_ITEM(LISTPRG,			"list",									DEMO_FUNC)
		UPARSE_ITEM(RUNCOMP,			"run compiled",					DEMO_FUNC)
		UPARSE_ITEM(RUNPRG,				"run", 									DEMO_FUNC)
		UPARSE_ITEM(COMPILE,			"compile",							DEMO_FUNC)
		UPARSE_ITEM(TESTPRG,			"test \x01", 						DEMO_FUNC)
		UPARSE_ITEM(MEMINFO,			"mem", 									DEMO_FUNC)

	#endif
