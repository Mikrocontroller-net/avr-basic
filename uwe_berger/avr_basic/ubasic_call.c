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
#include "tokenizer_access.h"
#include "ubasic.h"
#include "tokenizer.h"
#include "ubasic_config.h"
#include "ubasic_call.h"

#if USE_AVR && USE_LC7981
	#include "../lc7981/lc7981.h"
#endif

#if USE_AVR && RTC_DS1307
	#include "../ds1307/ds1307.h"
#endif

#if USE_AVR
	#include "../uart/usart.h"
#else
	#include <string.h>
	#include <stdio.h> 
#endif

#if UBASIC_CALL

//--------------------------------------------

// Funktionspointertabelle
#if USE_PROGMEM
callfunct_t callfunct[] PROGMEM = {
#else
callfunct_t callfunct[] = {
#endif
#if USE_AVR && USE_LC7981
	// lc7981-Routinen...
    {"clear",	.funct_ptr.VoidFuncVoid=lcd_clear,		VOID_FUNC_VOID},
    {"pset",	.funct_ptr.VoidFunc2Int=lcd_pset,		VOID_FUNC_2INT},
    {"line",	.funct_ptr.VoidFunc4Int=line,			VOID_FUNC_4INT},
    {"rclear",	.funct_ptr.VoidFunc4Int=lcd_clear_rect,	VOID_FUNC_4INT},
    {"pclear",	.funct_ptr.VoidFunc2Int=lcd_pclear,		VOID_FUNC_2INT},
    {"puts",	.funct_ptr.VoidFunc2IntChar=lcd_puts,	VOID_FUNC_2INT_CHAR},
#endif
#if USE_AVR && RTC_DS1307
	// Auslesen RTC DS1307
    {"get_rtc",	.funct_ptr.UCharFuncUChar=get_DS1307,	UCHAR_FUNC_UCHAR},
#endif

    {"",		{NULL},									255}
};


int call_statement(void) {

	unsigned char f_typ_temp;
	uint8_t idx=0;

#if defined VOID_FUNC_INT || defined INT_FUNC_INT || defined VOID_FUNC_2INT || defined VOID_FUNC_4INT || defined VOID_FUNC_2INT_CHAR
	int p1=0;
	int p2=0;
	int p3=0;
	int p4=0;
	int r=0;
#endif 
	
	accept(TOKENIZER_CALL);
	// Parameterliste wird durch linke Klammer eingeleitet
    accept(TOKENIZER_LEFTPAREN);
	// Funktionsname ermitteln
	if(tokenizer_token() == TOKENIZER_STRING) {
		tokenizer_next();
	}
	// Funktionsname in Tabelle suchen
#if USE_PROGMEM
	while(pgm_read_byte(&callfunct[idx].typ) != 255 &&
	      strncasecmp_P(tokenizer_last_string_ptr(), callfunct[idx].funct_name, MAX_NAME_LEN)) {
    	idx++;
    }
    f_typ_temp = pgm_read_byte(&callfunct[idx].typ);
#else	
	while(callfunct[idx].typ != 255 &&
	      strncasecmp(callfunct[idx].funct_name, tokenizer_last_string_ptr(), MAX_NAME_LEN)) {
    	idx++;
    }
    f_typ_temp = callfunct[idx].typ;
#endif
    // keinen Tabelleneintrag gefunden!
    if (f_typ_temp == 255) {
    	tokenizer_error_print(current_linenum, UNKNOWN_CALL_FUNCT);
		ubasic_break();
    } else {
		// je nach Funktionstyp (3.Spalte in Funktionspointertabelle) 
		// Parameterliste aufbauen und Funktion aufrufen
		switch (f_typ_temp){
#ifdef VOID_FUNC_VOID
			case VOID_FUNC_VOID:
						;
						#if USE_PROGMEM
							void (* f0)(void) = (void (*)(void)) pgm_read_word(&callfunct[idx].funct_ptr.VoidFuncVoid);
							f0();
						#else
							callfunct[idx].funct_ptr.VoidFuncVoid();
						#endif
						break;
#endif
#ifdef VOID_FUNC_INT
			case VOID_FUNC_INT:	
						accept(TOKENIZER_COMMA);
						p1=expr();
						#if USE_PROGMEM
							void (* f1)(int) = (void (*)(int)) pgm_read_word(&callfunct[idx].funct_ptr.VoidFuncInt);
							f1(p1);
						#else
							callfunct[idx].funct_ptr.VoidFuncInt(p1);
						#endif
						break;
#endif
#ifdef VOID_FUNC_2INT
			case VOID_FUNC_2INT:	
						accept(TOKENIZER_COMMA);
						p1=expr();
						accept(TOKENIZER_COMMA);
						p2=expr();
						#if USE_PROGMEM
							void (* f2)(int, int) = (void (*)(int, int)) pgm_read_word(&callfunct[idx].funct_ptr.VoidFunc2Int);
							f2(p1, p2);
						#else
							callfunct[idx].funct_ptr.VoidFunc2Int(p1, p2);
						#endif
						break;
#endif
#ifdef VOID_FUNC_4INT
			case VOID_FUNC_4INT:	
						accept(TOKENIZER_COMMA);
						p1=expr();
						accept(TOKENIZER_COMMA);
						p2=expr();
						accept(TOKENIZER_COMMA);
						p3=expr();
						accept(TOKENIZER_COMMA);
						p4=expr();
						#if USE_PROGMEM
							void (* f3)(int, int, int, int) = (void (*)(int, int, int, int)) pgm_read_word(&callfunct[idx].funct_ptr.VoidFunc4Int);
							f3(p1, p2, p3, p4);
						#else
							callfunct[idx].funct_ptr.VoidFunc4Int(p1, p2, p3, p4);
						#endif
						break;
#endif
#ifdef VOID_FUNC_2INT_CHAR
			case VOID_FUNC_2INT_CHAR:	
						accept(TOKENIZER_COMMA);
						p1=expr();
						accept(TOKENIZER_COMMA);
						p2=expr();
						accept(TOKENIZER_COMMA);
						accept(TOKENIZER_STRING);
						#if USE_PROGMEM
							void (* f4)(int, int, char*) = (void (*)(int, int, char*)) pgm_read_word(&callfunct[idx].funct_ptr.VoidFunc2IntChar);
							f4(p1, p2, (char*)tokenizer_last_string_ptr());
						#else
							callfunct[idx].funct_ptr.VoidFunc2IntChar(p1, p2, (char*)tokenizer_last_string_ptr());
						#endif
						break;
#endif
#ifdef UCHAR_FUNC_UCHAR
			case UCHAR_FUNC_UCHAR: // <-- naja, koennte ins Auge gehen...
#endif
#ifdef INT_FUNC_INT
			case INT_FUNC_INT: 
						accept(TOKENIZER_COMMA);
						p1=expr();
						#if USE_PROGMEM
							int (* f5)(int) = (int (*)(int)) pgm_read_word(&callfunct[idx].funct_ptr.IntFuncInt);
							r=f5(p1);
						#else
							r=callfunct[idx].funct_ptr.IntFuncInt(p1);
						#endif
						break;
#endif
			default:	tokenizer_error_print(current_linenum, UNKNOWN_CALL_FUNCT_TYP);
						ubasic_break();
		}
	}
	// abschliessende rechte Klammer
    accept(TOKENIZER_RIGHTPAREN);
	return r;
}
#endif
