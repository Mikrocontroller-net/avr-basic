/*----------------------------------------------------------------------------
 *	Routinen fuer serielle Schnittstelle
 *  ====================================
 * 
 * Ideen von: Frank Meyer - frank(at)fli4l.de
 *            Radig Ulrich  mail(at)ulrichradig.de
 *------------------------------------------------------------------------------
 */


#ifndef _UART_H
	#define _UART_H

	//----------------------------------------------------------------------------
	
	#include <avr/interrupt.h>
	#include <avr/pgmspace.h>
	#include <stdlib.h>
	#include <stdarg.h>
	#include <ctype.h>
	#include <string.h>
	#include <avr/io.h>

#ifndef BAUD
#define BAUD	57600L
#endif	
	
/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * Newer ATmegas, e.g. ATmega88, ATmega168
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifdef UBRR0H                                                           

#define UART0_UBRRH                             UBRR0H
#define UART0_UBRRL                             UBRR0L
#define UART0_UCSRA                             UCSR0A
#define UART0_UCSRB                             UCSR0B
#define UART0_UCSRC                             UCSR0C
#define UART0_UDRE_BIT_VALUE                    (1<<UDRE0)
#define UART0_UCSZ1_BIT_VALUE                   (1<<UCSZ01)
#define UART0_UCSZ0_BIT_VALUE                   (1<<UCSZ00)
#ifdef URSEL0
#define UART0_URSEL_BIT_VALUE                   (1<<URSEL0)
#else
#define UART0_URSEL_BIT_VALUE                   (0)
#endif
#define UART0_TXEN_BIT_VALUE                    (1<<TXEN0)
#define UART0_RXEN_BIT_VALUE                    (1<<RXEN0)
#define UART0_RXCIE_BIT_VALUE                   (1<<RXCIE0)
#define UART0_UDR                               UDR0
#define UART0_U2X                               U2X0
#define UART0_RXC                               RXC0

#ifdef USART0_TXC_vect                                                  // e.g. ATmega162 with 2 UARTs
#define UART0_TXC_vect                          USART0_TXC_vect
#define UART0_RXC_vect                          USART0_RXC_vect
#define UART0_UDRE_vect                         USART0_UDRE_vect
#elif defined(USART0_TX_vect)                                           // e.g. ATmega644 with 2 UARTs
#define UART0_TXC_vect                          USART0_TX_vect
#define UART0_RXC_vect                          USART0_RX_vect
#define UART0_UDRE_vect                         USART0_UDRE_vect
#else                                                                   // e.g. ATmega168 with 1 UART
#define UART0_TXC_vect                          USART_TX_vect
#define UART0_RXC_vect                          USART_RX_vect
#define UART0_UDRE_vect                         USART_UDRE_vect
#endif

#define UART0_UDRIE                             UDRIE0

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * ATmegas with 2nd UART, e.g. ATmega162
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#ifdef UBRR1H
#define UART1_UBRRH                             UBRR1H
#define UART1_UBRRL                             UBRR1L
#define UART1_UCSRA                             UCSR1A
#define UART1_UCSRB                             UCSR1B
#define UART1_UCSRC                             UCSR1C
#define UART1_UDRE_BIT_VALUE                    (1<<UDRE1)
#define UART1_UCSZ1_BIT_VALUE                   (1<<UCSZ11)
#define UART1_UCSZ0_BIT_VALUE                   (1<<UCSZ10)
#ifdef URSEL1
#define UART1_URSEL_BIT_VALUE                   (1<<URSEL1)
#else
#define UART1_URSEL_BIT_VALUE                   (0)
#endif
#define UART1_TXEN_BIT_VALUE                    (1<<TXEN1)
#define UART1_RXEN_BIT_VALUE                    (1<<RXEN1)
#define UART1_RXCIE_BIT_VALUE                   (1<<RXCIE1)
#define UART1_UDR                               UDR1
#define UART1_U2X                               U2X1
#define UART1_RXC                               RXC1

#ifdef USART1_TXC_vect                                                  // e.g. ATmega162 with 2 UARTs
#define UART1_TXC_vect                          USART1_TXC_vect
#define UART1_RXC_vect                          USART1_RXC_vect
#define UART1_UDRE_vect                         USART1_UDRE_vect
#else                                                                   // e.g. ATmega644 with 2 UARTs
#define UART1_TXC_vect                          USART1_TX_vect
#define UART1_RXC_vect                          USART1_RX_vect
#define UART1_UDRE_vect                         USART1_UDRE_vect
#endif

#define UART1_UDRIE                             UDRIE1
#endif // UBRR1H

#else

/*---------------------------------------------------------------------------------------------------------------------------------------------------
 * older ATmegas, e.g. ATmega8, ATmega16
 *---------------------------------------------------------------------------------------------------------------------------------------------------
 */
#define UART0_UBRRH                             UBRRH
#define UART0_UBRRL                             UBRRL
#define UART0_UCSRA                             UCSRA
#define UART0_UCSRB                             UCSRB
#define UART0_UCSRC                             UCSRC
#define UART0_UDRE_BIT_VALUE                    (1<<UDRE)
#define UART0_UCSZ1_BIT_VALUE                   (1<<UCSZ1)
#define UART0_UCSZ0_BIT_VALUE                   (1<<UCSZ0)
#ifdef URSEL
#define UART0_URSEL_BIT_VALUE                   (1<<URSEL)
#else
#define UART0_URSEL_BIT_VALUE                   (0)
#endif
#define UART0_TXEN_BIT_VALUE                    (1<<TXEN)
#define UART0_RXEN_BIT_VALUE                    (1<<RXEN)
#define UART0_RXCIE_BIT_VALUE                   (1<<RXCIE)
#define UART0_UDR                               UDR
#define UART0_U2X                               U2X
#define UART0_RXC                               RXC
#define UART0_UDRE_vect                         USART_UDRE_vect
#define UART0_TXC_vect                          USART_TXC_vect
#define UART0_RXC_vect                          USART_RXC_vect
#define UART0_UDRIE                             UDRIE

#endif

//----------------------------------------------------------------------------
	
void usart_init(); 
void usart_write_char(char ch);
void usart_write_str(char *str);

void usart_write_P (const char *Buffer,...);
#define usart_write(format, args...)   usart_write_P(PSTR(format) , ## args)
unsigned char usart_receive_char(void);
unsigned char usart_is_receive(void);
unsigned char usart_tx_not_empty(void);
uint8_t usart_read_line(char* buffer, uint8_t buffer_length);
	
//----------------------------------------------------------------------------

#endif //_UART_H
