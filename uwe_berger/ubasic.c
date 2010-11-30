/*
 * Copyright (c) 2006, Adam Dunkels
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the author nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * ------------------------------------------------------
 * Source modified by Uwe Berger (bergeruw@gmx.net); 2010
 * ------------------------------------------------------
 */

#define DEBUG 0

#if DEBUG
	#define DEBUG_PRINTF(...)  usart_write(__VA_ARGS__)
//	#define DEBUG_PRINTF(...)  printf(__VA_ARGS__)
#else
	#define DEBUG_PRINTF(...)
#endif

#include "tokenizer_access.h"
#include "ubasic.h"
#include "tokenizer.h"
#include "ubasic_config.h"

#if !USE_AVR
	#include <stdio.h>
	#if UBASIC_RND
		#include <time.h>
	#endif
#endif

#if UBASIC_CALL
	#include "ubasic_call.h"
#endif

#if UBASIC_CVARS
	#include "ubasic_cvars.h"
#endif

#if USE_AVR
	#include "usart.h"
#endif

#if AVR_EPEEK || AVR_EPOKE
	#include <avr/eeprom.h>
#endif

#if AVR_WAIT
	#include <util/delay.h>
#endif

#if !BREAK_NOT_EXIT
	#include <stdlib.h> /* exit() */
#endif

static PTR_TYPE program_ptr;

//#if UBASIC_PRINT
//static char string[MAX_STRINGLEN];
//#endif

static PTR_TYPE gosub_stack[MAX_GOSUB_STACK_DEPTH];
static int gosub_stack_ptr;

struct for_state {
  PTR_TYPE next_line_ptr;
  int for_variable;
  int to;
  int step;
  char downto;
};

static struct for_state for_stack[MAX_FOR_STACK_DEPTH];
static int for_stack_ptr;

#if USE_LINENUM_CACHE
struct linenum_cache_t {
	int linenum;
	PTR_TYPE next_line_ptr;
};

static struct linenum_cache_t linenum_cache[MAX_LINENUM_CACHE_DEPTH];
static int linenum_cache_ptr;
#endif


static int variables[MAX_VARNUM];

static uint8_t ended;

int expr(void);
static void line_statement(void);
static void statement(void);
#if UBASIC_RND && USE_AVR
static long unsigned int rand31_next(void);
#endif

/*---------------------------------------------------------------------------*/
void
ubasic_init(PTR_TYPE program)
{
	uint8_t i;
	program_ptr = program;
	for_stack_ptr = gosub_stack_ptr = 0;
#if USE_LINENUM_CACHE
	linenum_cache_ptr = 0;
#endif
	for (i=0; i<MAX_VARNUM; i++) variables[i]=0;
	tokenizer_init(program);
	ended = 0;
}
/*---------------------------------------------------------------------------*/
void
ubasic_break(void)
{
	#if BREAK_NOT_EXIT
	// zum Ende der Basic-Zeile und Ende-Merker setzen
	ended = 1;
	jump_to_next_linenum();
	#else
	exit(1);
	#endif
}

/*---------------------------------------------------------------------------*/
void
accept(int token)
{
  if(token != tokenizer_token()) {
    DEBUG_PRINTF("Token not what was expected (expected %i, got %i)\r\n",
		 token, tokenizer_token());
    tokenizer_error_print(current_linenum, SYNTAX_ERROR);
    ubasic_break();
  }
  DEBUG_PRINTF("Expected %i, got it\r\n", token);
  tokenizer_next();
}
/*---------------------------------------------------------------------------*/
static int
varfactor(void)
{
  int r;
  DEBUG_PRINTF("varfactor: obtaining %i from variable %i\r\n", variables[tokenizer_variable_num()], tokenizer_variable_num());
  r = ubasic_get_variable(tokenizer_variable_num());
  accept(TOKENIZER_VARIABLE);
  return r;
}
/*---------------------------------------------------------------------------*/
static int
factor(void)
{
  int r=0;
  #if AVR_IN || AVR_ADC || AVR_EPEEK 
  int adr;
  const char *port;
  char pin;
  #endif
  #if UBASIC_RND
  int b;
  #endif
  
  DEBUG_PRINTF("factor: token %i\n\r", tokenizer_token());
  switch(tokenizer_token()) {
  case TOKENIZER_NUMBER:
    r = tokenizer_num();
    accept(TOKENIZER_NUMBER);
    break;
    
  case TOKENIZER_MINUS:
    accept(TOKENIZER_MINUS);
    r = expr()*(-1);
    break;

  case TOKENIZER_PLUS:
    accept(TOKENIZER_PLUS);
    r = expr();
    break;

  case TOKENIZER_LEFTPAREN:
    accept(TOKENIZER_LEFTPAREN);
    r = expr();
    accept(TOKENIZER_RIGHTPAREN);
    break;
  
  #if UBASIC_RND
  case TOKENIZER_RND:
    accept(TOKENIZER_RND);
    accept(TOKENIZER_LEFTPAREN);
   	b = expr();
    #if USE_AVR
		r = rand31_next() % (b+1);
	#else
		r = rand() % (b+1);
	#endif
    accept(TOKENIZER_RIGHTPAREN);
    break;
  #endif
  
  #if UBASIC_ABS
  case TOKENIZER_ABS:
    accept(TOKENIZER_ABS);
    accept(TOKENIZER_LEFTPAREN);
    r = expr();
    if (r<0) r=r*(-1);
    accept(TOKENIZER_RIGHTPAREN);
    break;
  #endif

  #if UBASIC_NOT
  case TOKENIZER_NOT:
    accept(TOKENIZER_NOT);
    accept(TOKENIZER_LEFTPAREN);
    r = ~expr();
    accept(TOKENIZER_RIGHTPAREN);
    break;
  #endif
  
  #if UBASIC_CALL
  case TOKENIZER_CALL:
  	r=call_statement();
  	break;	
  #endif
    
  #if UBASIC_CVARS
  case TOKENIZER_VPEEK:
  	r=vpeek_expression();
  	break;	
  #endif

  #if AVR_EPEEK
  case TOKENIZER_EPEEK:
    accept(TOKENIZER_EPEEK);
    accept(TOKENIZER_LEFTPAREN);
    adr = expr();
    r = eeprom_read_byte((unsigned char *)adr);
    accept(TOKENIZER_RIGHTPAREN);
    break;
  #endif
  
  #if AVR_ADC
  case TOKENIZER_ADC:
    accept(TOKENIZER_ADC);
    accept(TOKENIZER_LEFTPAREN);
	pin=expr();
	if ((pin<0)||(pin>ADC_COUNT_MAX)) {
		//Fehlerfall
		DEBUG_PRINTF("adc_token: unknown channel %c\r\n", pin);
	    tokenizer_error_print(current_linenum, UNKNOWN_ADC_CHANNEL);
		ubasic_break();
	} else {
		// ADC_Kanal und Referenzspannung (hier Avcc) einstellen
		ADMUX =  pin;
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
    break;
  #endif
    
  #if AVR_IN
  case TOKENIZER_IN:
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
			DEBUG_PRINTF("in_token: unknown port %c\r\n", port);
		    tokenizer_error_print(current_linenum, UNKNOWN_IO_PORT);
			ubasic_break();
		}
    break;
  #endif
  
  default:
    r = varfactor();
    break;
  }
  return r;
}
/*---------------------------------------------------------------------------*/
static int
term(void)
{
  int f1, f2;
  int op;

  f1 = factor();
  op = tokenizer_token();
  DEBUG_PRINTF("term: token %i\n\r", op);
  while(op == TOKENIZER_ASTR  ||
		op == TOKENIZER_SLASH ||
		op == TOKENIZER_MOD) {
    tokenizer_next();
    f2 = factor();
    DEBUG_PRINTF("term: %i %i %i\r\n", f1, op, f2);
    switch(op) {
    case TOKENIZER_ASTR:
      f1 = f1 * f2;
      break;
    case TOKENIZER_SLASH:
      f1 = f1 / f2;
      break;
    case TOKENIZER_MOD:
      f1 = f1 % f2;
      break;
    }
    op = tokenizer_token();
  }
  DEBUG_PRINTF("term: %i\r\n", f1);
  return f1;
}
/*---------------------------------------------------------------------------*/
int
expr(void)
{
  int t1, t2;
  int op;
  
  t1 = term();
  op = tokenizer_token();
  DEBUG_PRINTF("expr: token %i\r\n", op);
  while(op == TOKENIZER_PLUS  ||
		op == TOKENIZER_MINUS ||
		op == TOKENIZER_AND   ||
		#if UBASIC_XOR
		op == TOKENIZER_XOR   ||
		#endif
		#if UBASIC_SHL
		op == TOKENIZER_SHL   ||
		#endif
		#if UBASIC_SHR
		op == TOKENIZER_SHR   ||
		#endif
		op == TOKENIZER_OR) {
    tokenizer_next();
    t2 = term();
    DEBUG_PRINTF("expr: %i %i %i\r\n", t1, op, t2);
    switch(op) {
    case TOKENIZER_PLUS:
      t1 = t1 + t2;
      break;
    case TOKENIZER_MINUS:
      t1 = t1 - t2;
      break;
    case TOKENIZER_AND:
      t1 = t1 & t2;
      break;
    case TOKENIZER_OR:
      t1 = t1 | t2;
      break;
#if UBASIC_XOR
    case TOKENIZER_XOR:
      t1 = t1 ^ t2;
      break;
#endif
#if UBASIC_SHL
    case TOKENIZER_SHL:
      t1 = t1 << t2;
      break;
#endif
#if UBASIC_SHR
    case TOKENIZER_SHR:
      t1 = t1 >> t2;
      break;
#endif
	  
    }
    op = tokenizer_token();
  }
  DEBUG_PRINTF("expr: %i\r\n", t1);
  return t1;
}
/*---------------------------------------------------------------------------*/
static int
relation(void)
{
  int r1, r2;
  int op;
  
  r1 = expr();
  op = tokenizer_token();
  DEBUG_PRINTF("relation: token %i\r\n", op);
  while(op == TOKENIZER_LT ||
		op == TOKENIZER_GT ||
		op == TOKENIZER_GE ||
		op == TOKENIZER_LE ||
		op == TOKENIZER_NE ||
		op == TOKENIZER_EQ) {
    tokenizer_next();
    r2 = expr();
    DEBUG_PRINTF("relation: %i %i %i\r\n", r1, op, r2);
    switch(op) {
    case TOKENIZER_LT:
      r1 = r1 < r2;
      break;
    case TOKENIZER_GT:
      r1 = r1 > r2;
      break;
    case TOKENIZER_EQ:
      r1 = r1 == r2;
      break;
    case TOKENIZER_LE:
      r1 = r1 <= r2;
      break;
    case TOKENIZER_GE:
      r1 = r1 >= r2;
      break;
    case TOKENIZER_NE:
      r1 = r1 != r2;
      break;
    }
    op = tokenizer_token();
  }
  return r1;
}
/*---------------------------------------------------------------------------*/
static void
jump_linenum(int linenum)
{
	
#if USE_LINENUM_CACHE	
	uint8_t i;
	// zuerst die Zeilennummer im Cache suchen
	for (i=0; i<linenum_cache_ptr; i++){
		if (linenum_cache[i].linenum == linenum) {
			jump_to_prog_text_pointer(linenum_cache[i].next_line_ptr);
			return;	
		}
	}
#endif	
	tokenizer_init(program_ptr);
	while(tokenizer_num() != linenum) {
		do {
			jump_to_next_linenum();
		} while(tokenizer_token() != TOKENIZER_NUMBER);
		DEBUG_PRINTF("jump_linenum: Found line %i\r\n", tokenizer_num());
	}

#if USE_LINENUM_CACHE	
	// wenn noch im Zeilennummern-Cache Platz ist, Zeilennummer/-Pointer
	// merken
	if (linenum_cache_ptr < MAX_LINENUM_CACHE_DEPTH) {
		linenum_cache[linenum_cache_ptr].next_line_ptr=get_prog_text_pointer();
		linenum_cache[linenum_cache_ptr].linenum=linenum;
		linenum_cache_ptr++;
	}
#endif
}
/*---------------------------------------------------------------------------*/
static void
goto_statement(void)
{
  accept(TOKENIZER_GOTO);
  jump_linenum(expr());
}
/*---------------------------------------------------------------------------*/
#if UBASIC_PRINT
static void
print_statement(void)
{
	uint8_t nl;
	accept(TOKENIZER_PRINT);
	do {
		nl=1;
		DEBUG_PRINTF("Print loop\r\n");
		if(tokenizer_token() == TOKENIZER_STRING) {
			PRINTF("%s", tokenizer_last_string_ptr());
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_COMMA) {
			nl=0;
			PRINTF(" ");
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_SEMICOLON) {
			nl=0;
			tokenizer_next();
		} else if(tokenizer_token() == TOKENIZER_VARIABLE  ||
			tokenizer_token() == TOKENIZER_LEFTPAREN ||
			tokenizer_token() == TOKENIZER_MINUS     ||
			tokenizer_token() == TOKENIZER_PLUS      ||
			#if UBASIC_RND
			tokenizer_token() == TOKENIZER_RND       ||
			#endif
			#if UBASIC_ABS
			tokenizer_token() == TOKENIZER_ABS       ||
			#endif
			#if UBASIC_NOT
			tokenizer_token() == TOKENIZER_NOT       ||
			#endif
			#if UBASIC_CALL
			tokenizer_token() == TOKENIZER_CALL      ||
			#endif
			#if UBASIC_CVARS
			tokenizer_token() == TOKENIZER_VPEEK     ||
			#endif
			#if AVR_EPEEK
			tokenizer_token() == TOKENIZER_EPEEK     ||
			#endif
			#if AVR_IN
			tokenizer_token() == TOKENIZER_IN        ||
			#endif
			#if AVR_ADC
			tokenizer_token() == TOKENIZER_ADC       ||
			#endif
			tokenizer_token() == TOKENIZER_NUMBER ) {
				PRINTF("%i", expr());
		} else {
			break;
		}
	} while(tokenizer_token() != TOKENIZER_CR &&
			tokenizer_token() != TOKENIZER_ENDOFINPUT);
	// wenn "," oder ";" am Zeilenende, dann kein Zeilenvorschub
	if (nl) PRINTF("\n\r");
	DEBUG_PRINTF("End of print\r\n");
	tokenizer_next();
}
#endif
/*---------------------------------------------------------------------------*/
static void
if_statement(void)
{
	int r;
	uint8_t no_then=0;
  
	accept(TOKENIZER_IF);
	r = relation();
	// Kurzform (IF ohne THEN/ELSE)?
	if (tokenizer_token() == TOKENIZER_THEN) accept(TOKENIZER_THEN); else no_then=1;
	if(r) {
		statement();
		// bei Kurzform darf kein ELSE kommen!
		if (no_then && (tokenizer_token() != TOKENIZER_NUMBER)) {
		    tokenizer_error_print(current_linenum, SHORT_IF_WITH_ELSE);
			ubasic_break();			
		}
		// hmm..., hier ist man schon ein Token zu weit...
		if(tokenizer_token() == TOKENIZER_NUMBER) return;
		jump_to_next_linenum();
	} else {
		do {
			tokenizer_next();
		} while(tokenizer_token() != TOKENIZER_ELSE &&
				tokenizer_token() != TOKENIZER_CR &&
				tokenizer_token() != TOKENIZER_ENDOFINPUT);
		if(tokenizer_token() == TOKENIZER_ELSE) {
			// bei Kurzform darf kein ELSE kommen!
			if (no_then) {
				tokenizer_error_print(current_linenum, SHORT_IF_WITH_ELSE);
				ubasic_break();
			}
			tokenizer_next();
			statement();
		} else	if(tokenizer_token() == TOKENIZER_CR) tokenizer_next();
	}
}

/*---------------------------------------------------------------------------*/
static void
let_statement(void)
{
	int var;
	var = tokenizer_variable_num();
	accept(TOKENIZER_VARIABLE);
	accept(TOKENIZER_EQ);
	ubasic_set_variable(var, expr());
	DEBUG_PRINTF("let_statement: assign %i to %i\r\n", variables[var], var);
	tokenizer_next();
}
/*---------------------------------------------------------------------------*/
static void
gosub_statement(void)
{
	int linenum;
	accept(TOKENIZER_GOSUB);
	linenum = expr();
	// es muss bis zum Zeilenende gelesen werden, um die Rueck-
	// sprungzeile fuer return zu ermitteln
	//jump_to_next_linenum();
	// ... wg. expr() steht der Parser schon am Zeilenende CR (wenn Syntax 
	// ok ist), deshalb nur noch ein Token weiterlesen!
	tokenizer_next();
	if(gosub_stack_ptr < MAX_GOSUB_STACK_DEPTH) {
		gosub_stack[gosub_stack_ptr] = get_prog_text_pointer();
		DEBUG_PRINTF("gosub_statement: gosub_stack=%i\r\n", gosub_stack[gosub_stack_ptr]);
		gosub_stack_ptr++;
		jump_linenum(linenum);
	} else {
		DEBUG_PRINTF("gosub_statement: gosub stack exhausted\r\n");
	}
}
/*---------------------------------------------------------------------------*/
static void
return_statement(void)
{
	accept(TOKENIZER_RETURN);
	if(gosub_stack_ptr > 0) {
		gosub_stack_ptr--;
		jump_to_prog_text_pointer(gosub_stack[gosub_stack_ptr]);
	} else {
		DEBUG_PRINTF("return_statement: non-matching return\r\n");
	}
}
/*---------------------------------------------------------------------------*/
static void
next_statement(void)
{
  int var;
  
  accept(TOKENIZER_NEXT);
  var = tokenizer_variable_num();
  accept(TOKENIZER_VARIABLE);
  if(for_stack_ptr > 0 && var == for_stack[for_stack_ptr - 1].for_variable) {
    ubasic_set_variable(var, ubasic_get_variable(var) + for_stack[for_stack_ptr - 1].step);
    if(((ubasic_get_variable(var) <= for_stack[for_stack_ptr - 1].to) && !for_stack[for_stack_ptr - 1].downto)||
       ((ubasic_get_variable(var) >= for_stack[for_stack_ptr - 1].to) && for_stack[for_stack_ptr - 1].downto)
      ) {
      jump_to_prog_text_pointer(for_stack[for_stack_ptr - 1].next_line_ptr);
    } else {
      for_stack_ptr--;
      accept(TOKENIZER_CR);
    }
  } else {
    DEBUG_PRINTF("next_statement: non-matching next (expected %i, found %i)\r\n", for_stack[for_stack_ptr - 1].for_variable, var);
    accept(TOKENIZER_CR);
  }

}
/*---------------------------------------------------------------------------*/
static void
for_statement(void)
{
  int for_variable, to, step; 
  unsigned char downto;
  
  accept(TOKENIZER_FOR);
  for_variable = tokenizer_variable_num();
  accept(TOKENIZER_VARIABLE);
  accept(TOKENIZER_EQ);
  ubasic_set_variable(for_variable, expr());
  if (tokenizer_token() == TOKENIZER_TO) {
  	downto = 0;
  } else if (tokenizer_token() == TOKENIZER_DOWNTO) {
  	downto = 1;
  } else tokenizer_error_print(current_linenum, FOR_WITHOUT_TO);
  tokenizer_next();
  to = expr();
  if(tokenizer_token() == TOKENIZER_STEP) {
  	tokenizer_next();
  	step = expr();
  } else {
  	step = 1;
  }
  if (downto) step *= -1;
  accept(TOKENIZER_CR);

  if(for_stack_ptr < MAX_FOR_STACK_DEPTH) {
    for_stack[for_stack_ptr].next_line_ptr = get_prog_text_pointer();
    for_stack[for_stack_ptr].for_variable = for_variable;
    for_stack[for_stack_ptr].to = to;
    for_stack[for_stack_ptr].step = step;
    for_stack[for_stack_ptr].downto = downto;
    
    DEBUG_PRINTF("for_statement: new for, var %i to %i\r\n",
		 for_stack[for_stack_ptr].for_variable,
		 for_stack[for_stack_ptr].to);
		 
    for_stack_ptr++;
  } else {
    DEBUG_PRINTF("for_statement: for stack depth exceeded\r\n");
  }
}
/*---------------------------------------------------------------------------*/
static void
end_statement(void)
{
	accept(TOKENIZER_END);
	ended = 1;
}

/*---------------------------------------------------------------------------*/
#if UBASIC_REM
static void
rem_statement(void)
{
	accept(TOKENIZER_REM);
	jump_to_next_linenum();
}
#endif

/*---------------------------------------------------------------------------*/
#if AVR_EPOKE
static void
epoke_statement(void)
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
#endif

/*---------------------------------------------------------------------------*/
#if AVR_DIR
static void
dir_statement(void)
{
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
	//switch (port) {
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
			DEBUG_PRINTF("dir_statement: unknown port %c\r\n", *port);
		    tokenizer_error_print(current_linenum, UNKNOWN_IO_PORT);
			ubasic_break();
	}
	tokenizer_next();
}
#endif

/*---------------------------------------------------------------------------*/
#if AVR_OUT
static void
out_statement(void)
{
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
			DEBUG_PRINTF("out_statement: unknown port %c\r\n", port);
			tokenizer_error_print(current_linenum, UNKNOWN_IO_PORT);
			ubasic_break();
	}
	tokenizer_next();
}
#endif

/*---------------------------------------------------------------------------*/
#if AVR_WAIT
static void
wait_statement(void)
{
	int delay;
	accept(TOKENIZER_WAIT);
	delay=expr();
	for (int i=0; i<delay; i++) _delay_ms(1);
	tokenizer_next();

}
#endif

/*---------------------------------------------------------------------------*/
#if UBASIC_RND
#if USE_AVR
long unsigned int seed = 0;
static void srand_statement(void) {
	uint16_t *p = (uint16_t*) (RAMEND+1);
	extern uint16_t __heap_start;
	accept(TOKENIZER_SRND);
	while (p >= &__heap_start + 1)
		seed ^= * (--p);
	tokenizer_next();
}
#else
static void srand_statement(void) {
	time_t t;
	accept(TOKENIZER_SRND);
	time(&t);
	srand((unsigned int)t);
	tokenizer_next();
}
#endif
#endif

/*---------------------------------------------------------------------------*/
static void
statement(void)
{
  int token;
  
  token = tokenizer_token();
  
  switch(token) {
  #if UBASIC_PRINT
  case TOKENIZER_PRINT:
    print_statement();
    break;
  #endif
  case TOKENIZER_IF:
    if_statement();
    break;
  case TOKENIZER_GOTO:
    goto_statement();
    break;
  case TOKENIZER_GOSUB:
    gosub_statement();
    break;
  case TOKENIZER_RETURN:
    return_statement();
    break;
  case TOKENIZER_FOR:
    for_statement();
    break;
  case TOKENIZER_NEXT:
    next_statement();
    break;
  case TOKENIZER_END:
    end_statement();
    break;
	
  #if UBASIC_CALL
  case TOKENIZER_CALL:
    call_statement();
    break;
  #endif

  #if UBASIC_REM
  case TOKENIZER_REM:
    rem_statement();
    break;
  #endif

  #if UBASIC_CVARS
  case TOKENIZER_VPOKE:
    vpoke_statement();
    break;
  #endif

  #if AVR_EPOKE
  case TOKENIZER_EPOKE:
    epoke_statement();
    break;
  #endif

  #if AVR_WAIT
  case TOKENIZER_WAIT:
    wait_statement();
    break;
  #endif
  
  #if AVR_DIR
  case TOKENIZER_DIR:
    dir_statement();
    break;
  #endif

  #if AVR_OUT
  case TOKENIZER_OUT:
    out_statement();
    break;
  #endif
  
  #if UBASIC_RND
  case TOKENIZER_SRND:
    srand_statement();
    break;
  #endif

  case TOKENIZER_LET:
    accept(TOKENIZER_LET);
    /* Fall through. */
  case TOKENIZER_VARIABLE:
    let_statement();
    break;
    
  default:
    DEBUG_PRINTF("ubasic.c: statement(): not implemented %i\r\n", token);
    tokenizer_error_print(current_linenum, UNKNOWN_STATEMENT);
    ubasic_break();
  }
}
/*---------------------------------------------------------------------------*/
static void
line_statement(void)
{
  DEBUG_PRINTF("----------- Line number %i ---------\r\n", tokenizer_num());
  current_linenum = tokenizer_num();
  accept(TOKENIZER_NUMBER);
  statement();
  return;
}
/*---------------------------------------------------------------------------*/
void
ubasic_run(void)
{
  if(tokenizer_finished()) {
    DEBUG_PRINTF("uBASIC program finished\r\n");
    return;
  }

  line_statement();
}
/*---------------------------------------------------------------------------*/
int
ubasic_finished(void)
{
  return ended || tokenizer_finished();
}
/*---------------------------------------------------------------------------*/
void
ubasic_set_variable(int varnum, int value)
{
  if(varnum >= 0 && varnum < MAX_VARNUM) {
    variables[varnum] = value;
  }
}
/*---------------------------------------------------------------------------*/
int
ubasic_get_variable(int varnum)
{
  if(varnum >= 0 && varnum < MAX_VARNUM) {
    return variables[varnum];
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
// Park-Miller "minimal standard" 31Bit pseudo-random generator
// http://www.firstpr.com.au/dsp/rand31/
#if UBASIC_RND && USE_AVR
long unsigned int rand31_next(void)
{
	long unsigned int hi, lo;
	lo  = 16807 * (seed & 0xffff);
	hi  = 16807 * (seed >> 16);
	lo += (hi & 0x7fff) << 16;
	lo += hi >> 15;
	if (lo > 0x7fffffff) lo -= 0x7fffffff;
	return (seed = (long)lo);
}
#endif
/*---------------------------------------------------------------------------*/
