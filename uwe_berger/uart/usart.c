/*----------------------------------------------------------------------------
 Copyright:      Radig Ulrich  mailto: mail@ulrichradig.de
 Author:         Radig Ulrich
 Remarks:        
 known Problems: none
 Version:        24.10.2007
 Description:    RS232 Routinen

 Dieses Programm ist freie Software. Sie können es unter den Bedingungen der 
 GNU General Public License, wie von der Free Software Foundation veröffentlicht, 
 weitergeben und/oder modifizieren, entweder gemäß Version 2 der Lizenz oder 
 (nach Ihrer Option) jeder späteren Version. 

 Die Veröffentlichung dieses Programms erfolgt in der Hoffnung, 
 daß es Ihnen von Nutzen sein wird, aber OHNE IRGENDEINE GARANTIE, 
 sogar ohne die implizite Garantie der MARKTREIFE oder der VERWENDBARKEIT 
 FÜR EINEN BESTIMMTEN ZWECK. Details finden Sie in der GNU General Public License. 

 Sie sollten eine Kopie der GNU General Public License zusammen mit diesem 
 Programm erhalten haben. 
 Falls nicht, schreiben Sie an die Free Software Foundation, 
 Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA. 
------------------------------------------------------------------------------*/
#include "usart.h"

	
//----------------------------------------------------------------------------
//Init serielle Schnittstelle
void usart_init(unsigned long baudrate) 
{ 
	//Serielle Schnittstelle 1
  	//Enable TXEN im Register UCR TX-Data Enable
	UCR =(1 << TXEN0 | 1 << RXEN0);
	//Teiler wird gesetzt 
	UBRR=(F_CPU / (baudrate * 16L) - 1);
}

//----------------------------------------------------------------------------
//Routine für die Serielle Ausgabe eines Zeichens (Schnittstelle0)
void usart_write_char(char c)
{
	//Warten solange bis Zeichen gesendet wurde
	while(!(USR & (1<<UDRE)));
	//Ausgabe des Zeichens
	UDR = c;
	return;
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
	while (*str)
	{
		usart_write_char(*str++);
	}
}

//----------------------------------------------------------------------------
//Empfang eines Zeichens
unsigned char usart_receive_char( void )
{
  // Warten bis Zeichen vollstaendig
  while ( !(UCSR0A & (1<<RXC0)) );
  // Zeichen zurueckgeben
  return UDR0;
}

//----------------------------------------------------------------------------
// return 1, wenn Zeichen empfangen wurde
unsigned char usart_is_receive(void) 
{
	return (UCSR0A & (1<<RXC0));
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
