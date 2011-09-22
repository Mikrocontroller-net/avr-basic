/*----------------------------------------------------------------------------
 *	Routinen fuer serielle Schnittstelle
 *  ====================================
 * 
 * Ideen von: Frank Meyer - frank(at)fli4l.de
 *            Radig Ulrich  mail(at)ulrichradig.de
 *------------------------------------------------------------------------------
 */

#include "usart.h"


#include <util/setbaud.h>


#define UART_TXBUFLEN                           64   // 64 Bytes ringbuffer for UART
#define UART_RXBUFLEN                           16   // 16 Bytes ringbuffer for UART

static volatile uint8_t uart_txbuf[UART_TXBUFLEN];   // tx ringbuffer
static volatile uint8_t uart_txsize = 0;             // tx size
static volatile uint8_t uart_rxbuf[UART_RXBUFLEN];   // rx ringbuffer
static volatile uint8_t uart_rxsize = 0;  
	


//----------------------------------------------------------------------------
// UART interrupt handler (RX)
ISR(UART0_RXC_vect)
{
	static uint8_t  uart_rxstop  = 0;    
	uint8_t         ch;

	ch = UART0_UDR;
	if (uart_rxsize < UART_RXBUFLEN) {    
		uart_rxbuf[uart_rxstop++] = ch;  
		if (uart_rxstop >= UART_RXBUFLEN) uart_rxstop = 0;  
		uart_rxsize++;     
	}
}

//----------------------------------------------------------------------------
// UART interrupt handler (TX)
ISR(UART0_UDRE_vect)
{
	static uint8_t  uart_txstart = 0;
	uint8_t         ch;

	if (uart_txsize > 0) {
		ch = uart_txbuf[uart_txstart++];
		if (uart_txstart == UART_TXBUFLEN) uart_txstart = 0;
		uart_txsize--; 
		UART0_UDR = ch;
	} else {
		UART0_UCSRB &= ~(1 << UART0_UDRIE);
	}
}


//----------------------------------------------------------------------------
//Init serielle Schnittstelle
void usart_init(void) 
{ 
	UART0_UBRRH = UBRRH_VALUE;
	UART0_UBRRL = UBRRL_VALUE;

#if USE_2X
	UART0_UCSRA |= (1<<UART0_U2X);
#else
	UART0_UCSRA &= ~(1<<UART0_U2X);
#endif

	UART0_UCSRC = UART0_UCSZ1_BIT_VALUE | UART0_UCSZ0_BIT_VALUE | UART0_URSEL_BIT_VALUE;
	UART0_UCSRB |= UART0_TXEN_BIT_VALUE | UART0_RXEN_BIT_VALUE | UART0_RXCIE_BIT_VALUE; 
}

//----------------------------------------------------------------------------
void usart_write_char(char ch)
{
	static uint8_t uart_txstop  = 0; 

	while (uart_txsize >= UART_TXBUFLEN) ;
	uart_txbuf[uart_txstop++] = ch;   
	if (uart_txstop >= UART_TXBUFLEN) uart_txstop = 0;        
	cli();
	uart_txsize++;                
	sei();
	UART0_UCSRB |= (1 << UART0_UDRIE);   
}

//------------------------------------------------------------------------------
void usart_write_P (const char *Buffer,...)
{
	va_list ap;
	va_start (ap, Buffer);	
	
	int format_flag;
	char str_buffer[10];
	char str_null_buffer[10];
	char move = 0;
	char Base = 0;
	int tmp = 0;
	char by;
	char *ptr;
		
	//Ausgabe der Zeichen
    for(;;)
	{
		by = pgm_read_byte(Buffer++);
		if(by==0) break; // end of format string
            
		if (by == '%')
		{
            by = pgm_read_byte(Buffer++);
			if (isdigit(by)>0)
				{
                                 
 				str_null_buffer[0] = by;
				str_null_buffer[1] = '\0';
				move = atoi(str_null_buffer);
                by = pgm_read_byte(Buffer++);
				}

			switch (by)
				{
                case 's':
                    ptr = va_arg(ap,char *);
                    while(*ptr) { usart_write_char(*ptr++); }
                    break;
				case 'b':
					Base = 2;
					goto ConversionLoop;
				case 'c':
					//Int to char
					format_flag = va_arg(ap,int);
					usart_write_char (format_flag++);
					break;
				case 'i':
					Base = 10;
					goto ConversionLoop;
				case 'o':
					Base = 8;
					goto ConversionLoop;
				case 'x':
					Base = 16;
					//****************************
					ConversionLoop:
					//****************************
					itoa(va_arg(ap,int),str_buffer,Base);
					int b=0;
					while (str_buffer[b++] != 0){};
					b--;
					if (b<move)
						{
						move -=b;
						for (tmp = 0;tmp<move;tmp++)
							{
							str_null_buffer[tmp] = '0';
							}
						//tmp ++;
						str_null_buffer[tmp] = '\0';
						strcat(str_null_buffer,str_buffer);
						strcpy(str_buffer,str_null_buffer);
						}
					usart_write_str (str_buffer);
					move =0;
					break;
				}
			
			}	
		else
		{
			usart_write_char ( by );	
		}
	}
	va_end(ap);
}

//----------------------------------------------------------------------------
//Ausgabe eines Strings
void usart_write_str(char *str)
{
	while (*str) usart_write_char(*str++);
}

//----------------------------------------------------------------------------
//Empfang eines Zeichens
unsigned char usart_receive_char( void )
{
	static uint8_t  uart_rxstart = 0; 
	uint8_t         ch;
	
	while (uart_rxsize == 0) {
		;
	}

	ch = uart_rxbuf[uart_rxstart++];
	if (uart_rxstart == UART_RXBUFLEN) uart_rxstart = 0;
	cli();
	uart_rxsize--;
	sei();
	return ch;
}

//----------------------------------------------------------------------------
// return 1, wenn Zeichen empfangen wurde
unsigned char usart_is_receive(void) 
{
	if (uart_rxsize) return 1; else return 0;
}

//----------------------------------------------------------------------------
// return 1, wenn noch nicht alle Zeichen gesendet
unsigned char usart_tx_not_empty(void) 
{
	if (uart_txsize) return 1; else return 0;
}

//----------------------------------------------------------------------------
uint8_t usart_read_line(char* buffer, uint8_t buffer_length)
{
    memset(buffer, 0, buffer_length);
    uint8_t read_length = 0;
    while(read_length < buffer_length - 1)
    {
        uint8_t c = usart_receive_char();
        if(c == 0x08 || c == 0x7f)
        {
            if(read_length < 1) continue;
            --read_length;
            buffer[read_length] = '\0';
            usart_write_char(0x08);
            usart_write_char(' ');
            usart_write_char(0x08);
            continue;
        }
        usart_write_char(c);
        if(c == 0x0d)
        {
            buffer[read_length] = '\0';
            break;
        }
        else
        {
            buffer[read_length] = c;
            ++read_length;
        }
    }
    return read_length;
}
