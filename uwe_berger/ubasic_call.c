/*--------------------------------------------------------
*    Implementierung Basic-Befehl "call()"
*    =====================================
*     Uwe Berger (bergeruw@gmx.net); 2010
* 
* Dokumentation call_referenz.txt...!
*
* 
* Have fun!
* ---------
*
----------------------------------------------------------*/
#include "ubasic.h"
#include "tokenizer.h"
#include "ubasic_config.h"
#include "ubasic_call.h"


#if USE_AVR
	#include "usart.h"
#else
	#include <string.h>
	#include <stdio.h> 
#endif

#if UBASIC_CALL

#define DEBUG 0
#if DEBUG
	#define DEBUG_PRINTF(...)  usart_write(__VA_ARGS__)
#else
	#define DEBUG_PRINTF(...)
#endif


//------------------------------------------
// ein paar Testfunktionen fuer call...
void a(void) {
#if USE_AVR
	DDRB |= (1 << PB1);
	PORTB |= (1 << PB1);
#endif
}

void b(int p1) {
#if USE_AVR
	DDRB |= (1 << PB1);
	if (p1) PORTB |= (1 << PB1); else PORTB &= ~(1 << PB1);
#endif
}

int c(int p1) {
	int r=0;	
#if USE_AVR
	ADMUX =  p1;
	ADMUX |= (1<<REFS0);
	ADCSRA = (1<<ADEN) | (1<<ADPS1) | (1<<ADPS0);
	ADCSRA |= (1<<ADSC);              
	while (ADCSRA & (1<<ADSC));
	r = ADCW;
	ADCSRA |= (1<<ADSC);  
	while (ADCSRA & (1<<ADSC));
	r = ADCW;
	ADCSRA=0;

#endif
	return r;
}


//--------------------------------------------

// Funktionspointertabelle
callfunct_t callfunct[] = {
    {"a",	.funct_ptr.VoidFuncVoid=a,	VOID_FUNC_VOID},
    {"b",	.funct_ptr.VoidFuncInt=b,	VOID_FUNC_INT},
    {"c",	.funct_ptr.IntFuncInt=c,	INT_FUNC_INT},
    {NULL, {NULL},	255}
};


int call_statement(void) {
	//static char funct_name[MAX_NAME_LEN+1];
	uint8_t idx=0;

	int p1=0;
	int r=0;
	
	accept(TOKENIZER_CALL);
	// Parameterliste wird durch linke Klammer eingeleitet
    accept(TOKENIZER_LEFTPAREN);
	// Funktionsname ermitteln
	if(tokenizer_token() == TOKENIZER_STRING) {
		//tokenizer_string(funct_name, sizeof(funct_name));
		DEBUG_PRINTF("funct_name: %s\n\r", tokenizer_last_string_ptr());
		tokenizer_next();
	}
	// Funktionsname in Tabelle suchen
	while(strncasecmp(callfunct[idx].funct_name, tokenizer_last_string_ptr(), MAX_NAME_LEN) &&
		  callfunct[idx].funct_name != 0)
    {
    	idx++;
    }
    // einen Tabelleneintrag gefunden!
    if (callfunct[idx].funct_name == 0) {
    	DEBUG_PRINTF("funct_name: %s nicht gefunden!\n\r", tokenizer_last_string_ptr());
    	tokenizer_error_print(current_linenum, UNKNOWN_CALL_FUNCT);
		ubasic_break();
    } else {
	    DEBUG_PRINTF("funct_name: %s hat Index: %i\n\r", tokenizer_last_string_ptr(), idx);
		// je nach Funktionstyp (3.Spalte in Funktionspointertabelle) 
		// Parameterliste aufbauen und Funktion aufrufen
		switch (callfunct[idx].typ){
			case VOID_FUNC_VOID: 
						// ohne Parameter/Rueckgabewert
						callfunct[idx].funct_ptr.VoidFuncVoid();
						break;
			case VOID_FUNC_INT:	
						// ein Integer und kein Rueckgabewert
						accept(TOKENIZER_COMMA);
						p1=expr();
						callfunct[idx].funct_ptr.VoidFuncInt(p1);
						break;
			case INT_FUNC_INT: 
						// ein Integer und Rueckgabewert
						accept(TOKENIZER_COMMA);
						p1=expr();
						r=callfunct[idx].funct_ptr.IntFuncInt(p1);
						break;
			default:	DEBUG_PRINTF("Funktionspointertyp %i nicht gefunden\n\r", callfunct[idx].typ);
						tokenizer_error_print(current_linenum, UNKNOWN_CALL_FUNCT_TYP);
						ubasic_break();
		}
	}
	// abschliessende rechte Klammer
    accept(TOKENIZER_RIGHTPAREN);
    // bei Funktionspointertypen ohne Rueckgabewert ein Token weitergelesen...
    if ((callfunct[idx].typ == VOID_FUNC_VOID) ||
    	(callfunct[idx].typ == VOID_FUNC_INT) 
    	) tokenizer_next();
	return r;
}
#endif
