////////////////////////////////////////////////////////////////////////////
//
//  uart.h
//
//	Header der UART Implementierung
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


		#ifndef __UART_H__
			#define __UART_H__


				#define UART_INPUT_LENGTH 80


				extern volatile char  acUARTRecData [UART_INPUT_LENGTH];
				extern volatile U8 		ucUARTRPtr;
				extern volatile U8 		ucUARTInputLength;
				extern volatile B8 		bUARTProcessInput;
				extern volatile U8		ucUARTReceivedInput;
				extern volatile B8 		bUARTInputReceived;


				void vUARTInit 					(void);
				void vUARTSendChar 			(char cValue);
				void vUARTSendString 		(char *pcString);
				void vUARTSendString_P 	(char *pcString);
				void vUARTSendDecimal 	(U32 ulLong);
				void vUARTSendCRLF 			(void);
				void vUARTSendHexU8 		(U8 ucX);
				void vUARTSendHexU16 		(U16 uiX);
				void vUARTSendHexU32 		(U32 ulX);
				void vUARTSendDecU16 		(U16 uiX);

#ifdef WIN32
				void vUARTSim 					(void);
#endif


			#endif
