/*--------------------------------------------------------
*    Implementierung Basic-Befehl "call()"
*    =====================================
*     Uwe Berger (bergeruw@gmx.net); 2010
* 
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
#include "ubasic_cvars.h"


#if USE_AVR
	#include "usart.h"
#else
	#include <string.h>
	#include <stdio.h>
#endif

#if UBASIC_CVARS

#define DEBUG 0
#if DEBUG
	#define DEBUG_PRINTF(...)  usart_write(__VA_ARGS__)
#else
	#define DEBUG_PRINTF(...)
#endif

//------------------------------------------
// eine Testvariable in C...
int va = 123;
int vb = 456;


//--------------------------------------------

// Variablenpointertabelle
cvars_t cvars[] = {
    {"a", &va},
    {"b", &vb},
    {NULL, NULL}
};

int search_cvars(const char *var_name) {
	int idx=0;
	// Variablenname in Tabelle suchen
	while(cvars[idx].var_name != NULL &&
	      strncasecmp(cvars[idx].var_name, var_name, MAX_NAME_LEN)) {
    	idx++;
    }
    // keinen Tabelleneintrag gefunden!
    if (cvars[idx].var_name == NULL) {
    	tokenizer_error_print(current_linenum, UNKNOWN_CVAR_NAME);
		ubasic_break();
    }
	return idx;
}

void vpoke_statement(void) {
	int idx=0;
	
	accept(TOKENIZER_VPOKE);
    accept(TOKENIZER_LEFTPAREN);
	// Funktionsname ermitteln
	if(tokenizer_token() == TOKENIZER_STRING) {
		DEBUG_PRINTF("funct_name: %s\n\r", tokenizer_last_string_ptr());
		tokenizer_next();
	}
	idx=search_cvars(tokenizer_last_string_ptr());
	accept(TOKENIZER_RIGHTPAREN);
	accept(TOKENIZER_EQ);
	*cvars[idx].pvar = expr();
	tokenizer_next();
}

int vpeek_expression(void) {
	int idx=0;
	int r=0;

	accept(TOKENIZER_VPEEK);
	// Parameterliste wird durch linke Klammer eingeleitet
    accept(TOKENIZER_LEFTPAREN);
	// Funktionsname ermitteln
	if(tokenizer_token() == TOKENIZER_STRING) {
		DEBUG_PRINTF("funct_name: %s\n\r", tokenizer_last_string_ptr());
		tokenizer_next();
	}
	idx=search_cvars(tokenizer_last_string_ptr());
	r = *cvars[idx].pvar;
    accept(TOKENIZER_RIGHTPAREN);
	return r;
}
#endif
