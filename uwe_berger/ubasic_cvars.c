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
#include "ubasic.h"
#include "tokenizer.h"
#include "ubasic_config.h"
#include "ubasic_cvars.h"

#include "usart.h"

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


//--------------------------------------------

// Variablenpointertabelle
cvars_t cvars[] = {
    {"a", &va},
    {NULL, NULL}
};

int search_cvars(char *var_name) {
	int idx=0;
	// Variablenname in Tabelle suchen
	while(strncasecmp(cvars[idx].var_name, var_name, MAX_NAME_LEN) &&
		  cvars[idx].var_name != 0)
    {
    	idx++;
    }
    // keinen Tabelleneintrag gefunden!
    if (cvars[idx].var_name == 0) {
    	tokenizer_error_print(current_linenum, UNKNOWN_CVAR_NAME);
		ubasic_break();
    }
	return idx;
}

void vpoke_statement(void) {
	static char var_name[MAX_NAME_LEN+1];
	int idx=0;
	
	accept(TOKENIZER_VPOKE);
    accept(TOKENIZER_LEFTPAREN);
	// Funktionsname ermitteln
	if(tokenizer_token() == TOKENIZER_STRING) {
		tokenizer_string(var_name, sizeof(var_name));
		DEBUG_PRINTF("funct_name: %s\n\r", var_name);
		tokenizer_next();
	}
	idx=search_cvars(var_name);
	accept(TOKENIZER_RIGHTPAREN);
	accept(TOKENIZER_EQ);
	*cvars[idx].pvar = expr();
	tokenizer_next();
}

int vpeek_expression(void) {
	static char var_name[MAX_NAME_LEN+1];
	int idx=0;
	int r=0;

	accept(TOKENIZER_VPEEK);
	// Parameterliste wird durch linke Klammer eingeleitet
    accept(TOKENIZER_LEFTPAREN);
	// Funktionsname ermitteln
	if(tokenizer_token() == TOKENIZER_STRING) {
		tokenizer_string(var_name, sizeof(var_name));
		DEBUG_PRINTF("funct_name: %s\n\r", var_name);
		tokenizer_next();
	}
	idx=search_cvars(var_name);
	r = *cvars[idx].pvar;
    accept(TOKENIZER_RIGHTPAREN);
	return r;
}
#endif