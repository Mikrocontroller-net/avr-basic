////////////////////////////////////////////////////////////////////////////
//
//  uart.c
//
//	Einfache UART Implementierung (nur beispielhaft)
//  Enthält einen Buffer in dem Eingaben interruptgesteuert
//  entgegengenommen werden. 
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
	#include <stdio.h>
	#include <conio.h>
#endif
	#include "platform.h"
	#include "uart.h"


  // Eingangsbuffer 
	volatile char 	acUARTRecData [UART_INPUT_LENGTH];
  // Eingangsbuffer-Index 
	volatile U8 		ucUARTRPtr;
  // Länge der Eingabe (entspricht ucUARTRPtr)
	volatile U8 		ucUARTInputLength;
	// Flag "Eingabe (mit Return abgeschlossen) liegt vor"
	volatile B8   	bUARTProcessInput  	= FALSE;
	// Zuletzt empfangenes Zeichen
	volatile U8  		ucUARTReceivedInput;
	// Flag für "Zeichen empfangen
	volatile B8   	bUARTInputReceived 	= FALSE;

#ifdef AVR
	// Ausgabe-Funktion und -File für printf
	extern int uart_putchar (char x, FILE *stream);
	// "Datei" für printf erstellen
	FILE uartfile 		= FDEV_SETUP_STREAM (uart_putchar, NULL, _FDEV_SETUP_WRITE);
#endif

#ifdef WIN32
	// Prototyp für UART Verarbeitung
	void vUARTInputProcess (U8 ucData);
#endif

#ifdef AVR
// Initialisierung des UART auf 38400bd bei 16MHz
void vUARTInit (void)
{
	UBRRL = 25;  // 16MHz / (16*38.4kbd) = 26 -> 26-1 = 25 eff. Baudrate 38462bd
	UBRRH = 0;

	UCSRB |= (1 << RXEN)  | (1 << TXEN);			// sender & empfänger einschalten
	UCSRC |= (1 << URSEL) | (3 << UCSZ0); 		// uart mode
  UCSRB |= (1 << RXCIE);										// uart interrupt einschalten

	DDRD  |= 1 << 1;													// tx-pin auf ausgang

	// eingangsbuffer intialisieren
	memset ((void *) acUARTRecData, 0, sizeof (acUARTRecData));

	// variablen initialisieren
	ucUARTRPtr 			 		= 0;
	ucUARTInputLength 	= 0;
	bUARTProcessInput 	= FALSE;
	ucUARTReceivedInput = 0;
	bUARTInputReceived  = FALSE;

	// Ausgabe-"Datei" für stdout von printf verwenden
	stdout = &uartfile;
}

// einzelnes Zeichen senden
void vUARTSendChar (char cValue)
{
	while (!(UCSRA & (1 << UDRE)));
	UDR = cValue;
}

// Ausgabe-Funktion für printf
int uart_putchar (char x, FILE *stream)
{
	vUARTSendChar (x);
	return 0;
}

#endif

#ifdef WIN32
	void vUARTSendChar (char cValue)	
	{
		printf ("%c", cValue);
	}
#endif

// String aus RAM senden
void vUARTSendString (char *pcString)
{
  while (*pcString) { vUARTSendChar (*pcString++); };
}

// String aus Flash senden
void vUARTSendString_P (char *pcString)
{
  while (pgm_read_byte (pcString)) { vUARTSendChar (pgm_read_byte (pcString++)); };
}

// CRLF senden
void vUARTSendCRLF (void)
{
  vUARTSendChar ('\n');
	vUARTSendChar ('\r');
}

void vUARTSendHexU8 (U8 ucX)
{
	U8 ucT = ucX;
	ucX = ucX >> 4;
	ucT = ucT & 0x0f;
	vUARTSendChar ( ucX > 9 ? ucX - 10 + 'A' : ucX + '0');
	vUARTSendChar ( ucT > 9 ? ucT - 10 + 'A' : ucT + '0');
}

void vUARTSendHexU16 (U16 uiX)
{
	vUARTSendHexU8  ((uiX >>  8) & 0xff);
	vUARTSendHexU8  ((uiX >>  0) & 0xff);
}


void vUARTSendHexU32 (U32 ulX)
{
	vUARTSendHexU16 ((ulX >> 16) & 0xffff);
	vUARTSendHexU16 ((ulX >>  0) & 0xffff);
}

void vUARTSendDecU16 (U16 uiX)
{
  U16 uiDecs [] = { 10000, 1000, 100, 10, };
	U8  ucDP = 0, ucD;
	do
	{
	  ucD = 0;
		while (uiX > uiDecs [ucDP])
		{
		  uiX -= uiDecs [ucDP];
			ucD++;
		}
		vUARTSendChar ('0' + ucD);
		ucDP++;
	} while (ucDP != 4);
	vUARTSendChar ('0' + uiX);
}

void vUARTInputProcess (U8 ucData)
{
	// Emfpangsflag setzen und daten speichern
	bUARTInputReceived 	= TRUE;
	ucUARTReceivedInput = ucData; 

	// Max. Eingabelänge begrenzen
	if (ucUARTRPtr > UART_INPUT_LENGTH) { ucUARTRPtr = UART_INPUT_LENGTH; }
	// Return ?
  if (ucData == 0x0d)
	{
		// Eingabe-String abschließen und Flag sowie weitere Informationen setzen
	  acUARTRecData [ucUARTRPtr] = 0;
		bUARTProcessInput = TRUE; ucUARTInputLength = ucUARTRPtr; ucUARTRPtr = 0;
	}
	// Delete ? (\x7f daher weil beim vt100 del std-mässig 0x7f ist)
	else if ((ucData == 0x08) || (ucData == 0x7f))
	{ 
		// wenn noch zeichen da, dann einen zurücksetzen
		if (ucUARTRPtr > 0)
		{
			ucUARTRPtr--;
			acUARTRecData [ucUARTRPtr] = 0; 
		}
	}
	// jedes andere zeichen einfach wegspeichern
	else
	{ 
		acUARTRecData [ucUARTRPtr] = ucData; 
		ucUARTRPtr++;
	}
}


#ifdef AVR

// Empfangsinterrupt
ISR (USART_RXC_vect)
{
	U8 ucData = UDR; 	// Daten holen

	vUARTInputProcess (ucData);
}


#endif


#ifdef WIN32

	void vUARTSim (void)
	{
	  if (kbhit ())
		{
			U8 ucT;

		  ucT = getch ();

			if (!((ucT == 0) || (ucT == 0xe0)))
			{
			  vUARTInputProcess (ucT);
			}
			else
			{
			  getch ();
			}
		}
	}

#endif
