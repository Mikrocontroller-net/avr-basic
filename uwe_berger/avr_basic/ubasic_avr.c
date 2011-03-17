/*--------------------------------------------------------
*    Implementierung Basic-Befehl fuer AVR
*    =====================================
*     Uwe Berger (bergeruw@gmx.net); 2010
* 
*
* 
* Have fun!
* ---------
*
----------------------------------------------------------*/
#include "ubasic_config.h"
#include "tokenizer_access.h"
#include "ubasic.h"
#include "tokenizer.h"
#include "ubasic_avr.h"

#include <avr/eeprom.h>
#include <util/delay.h>




#if USE_AVR
	#include "../uart/usart.h"
#else
	#include <string.h>
	#include <stdio.h>
#endif

#if USE_AVR

/*---------------------------------------------------------------------------*/
int epeek_expression(void) {
	int adr, r;
	
    accept(TOKENIZER_EPEEK);
    accept(TOKENIZER_LEFTPAREN);
    adr = expr();
    r = eeprom_read_byte((unsigned char *)adr);
    accept(TOKENIZER_RIGHTPAREN);
    return r;
}

/*---------------------------------------------------------------------------*/
int adc_expression() {
	int r=0;
	int channel;
	
    accept(TOKENIZER_ADC);
    accept(TOKENIZER_LEFTPAREN);
	channel=expr();
	if ((channel<0)||(channel>ADC_COUNT_MAX)) {
		//Fehlerfall
	    tokenizer_error_print(current_linenum, UNKNOWN_ADC_CHANNEL);
		ubasic_break();
	} else {
		// ADC_Kanal und Referenzspannung (hier Avcc) einstellen
		ADMUX =  channel;
		ADMUX |= (1<<REFS0);
		// Prescaler 8:1 und ADC aktivieren
		ADCSRA = (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0);
		// eine ADC-Wandlung fuer eine Dummy-Messung
		ADCSRA |= (1<<ADSC);              
		while (ADCSRA & (1<<ADSC));
		r = ADCW;
		// eigentliche Messung
		ADCSRA |= (1<<ADSC);  
		while (ADCSRA & (1<<ADSC));
		r = ADCW;
		// ADC wieder ausschalten
		ADCSRA=0;
	}	
    accept(TOKENIZER_RIGHTPAREN);
    return r;
}

/*---------------------------------------------------------------------------*/
int pin_in_expression() {
	int r=0;
	const char *port;
	char pin;
	
    accept(TOKENIZER_IN);
    accept(TOKENIZER_LEFTPAREN);
    accept(TOKENIZER_STRING);
	port=tokenizer_last_string_ptr();
	accept(TOKENIZER_COMMA);
	pin=expr();
    accept(TOKENIZER_RIGHTPAREN);
		switch (*port) {
		#if HAVE_PORTA
		case 'a':
		case 'A':
			if (bit_is_clear(PINA, pin)) r=0; else r=1;
			break;
		#endif
		#if HAVE_PORTB
		case 'b':
		case 'B':
			if (bit_is_clear(PINB, pin)) r=0; else r=1;
			break;
		#endif
		#if HAVE_PORTC
		case 'c':
		case 'C':
			if (bit_is_clear(PINC, pin)) r=0; else r=1;
			break;
		#endif
		#if HAVE_PORTD
		case 'd':
		case 'D':
			if (bit_is_clear(PIND, pin)) r=0; else r=1;
			break;
		#endif
		default:
		    tokenizer_error_print(current_linenum, UNKNOWN_IO_PORT);
			ubasic_break();
		}
	return r;
}

/*---------------------------------------------------------------------------*/
void dir_statement(void) {
	const char *port;
	char pin, val;
	
	accept(TOKENIZER_DIR);
    accept(TOKENIZER_LEFTPAREN);
    accept(TOKENIZER_STRING);
	port=tokenizer_last_string_ptr();
	accept(TOKENIZER_COMMA);
	pin=expr();
    accept(TOKENIZER_RIGHTPAREN);
    accept(TOKENIZER_EQ);
	if (expr()) val=1; else val=0;
	switch (*port) {
		#if HAVE_PORTA
		case 'a':
		case 'A':
			if (val) DDRA |= (1 << pin); else DDRA &= ~(1 << pin);
			break;
		#endif
		#if HAVE_PORTB
		case 'b':
		case 'B':
			if (val) DDRB |= (1 << pin); else DDRB &= ~(1 << pin);
			break;
		#endif
		#if HAVE_PORTC
		case 'c':
		case 'C':
			if (val) DDRC |= (1 << pin); else DDRC &= ~(1 << pin);
			break;
		#endif
		#if HAVE_PORTD
		case 'd':
		case 'D':
			if (val) DDRD |= (1 << pin); else DDRD &= ~(1 << pin);
			break;
		#endif
		default:
		    tokenizer_error_print(current_linenum, UNKNOWN_IO_PORT);
			ubasic_break();
	}
	tokenizer_next();
}

/*---------------------------------------------------------------------------*/
void out_statement(void) {
	char const *port;
	char pin, val;
	
	accept(TOKENIZER_OUT);
    accept(TOKENIZER_LEFTPAREN);
    accept(TOKENIZER_STRING);
	port=tokenizer_last_string_ptr();
	accept(TOKENIZER_COMMA);
	pin=expr();
    accept(TOKENIZER_RIGHTPAREN);
    accept(TOKENIZER_EQ);
	val = expr();
	switch (*port) {
		#if HAVE_PORTA
		case 'a':
		case 'A':
			if (val) PORTA |= (1 << pin); else PORTA &= ~(1 << pin);
			break;
		#endif
		#if HAVE_PORTB
		case 'b':
		case 'B':
			if (val) PORTB |= (1 << pin); else PORTB &= ~(1 << pin);
			break;
		#endif
		#if HAVE_PORTC
		case 'c':
		case 'C':
			if (val) PORTC |= (1 << pin); else PORTC &= ~(1 << pin);
			break;
		#endif
		#if HAVE_PORTD
		case 'd':
		case 'D':
			if (val) PORTD |= (1 << pin); else PORTD &= ~(1 << pin);
			break;
		#endif
		default:
			tokenizer_error_print(current_linenum, UNKNOWN_IO_PORT);
			ubasic_break();
	}
	tokenizer_next();
}

/*---------------------------------------------------------------------------*/
void epoke_statement(void)
{
	int adr, val;
	accept(TOKENIZER_EPOKE);
    accept(TOKENIZER_LEFTPAREN);
    adr = expr();
    accept(TOKENIZER_RIGHTPAREN);
    accept(TOKENIZER_EQ);
	val = expr();
	eeprom_write_byte((unsigned char *)adr, val);
	tokenizer_next();
}

/*---------------------------------------------------------------------------*/
void wait_statement(void) {
	int delay;

	accept(TOKENIZER_WAIT);
	delay=expr();
	for (int i=0; i<delay; i++) _delay_ms(1);
	tokenizer_next();
}

#endif
