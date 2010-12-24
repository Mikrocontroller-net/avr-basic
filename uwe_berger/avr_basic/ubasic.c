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


#include "tokenizer_access.h"
#include "ubasic.h"
#include "tokenizer.h"
#include "ubasic_config.h"
#include "ubasic_ext_proc.h"

#if !USE_AVR
	#include <stdio.h>
	#include <string.h>
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
	#include "../uart/usart.h"
	#include <avr/io.h>
	#include "ubasic_avr.h"
#endif

#if UBASIC_EXT_PROC
	extern char current_proc[MAX_PROG_NAME_LEN];
#endif

#if !BREAK_NOT_EXIT
	#include <stdlib.h> /* exit() */
#endif

PTR_TYPE program_ptr;

struct gosub_stack_t {
#if UBASIC_EXT_PROC
	char p_name[MAX_PROG_NAME_LEN];
#endif
	PTR_TYPE p_ptr;
};

static struct gosub_stack_t gosub_stack[MAX_GOSUB_STACK_DEPTH];
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
#if UBASIC_EXT_PROC
	char p_name[MAX_PROG_NAME_LEN];
#endif
	int linenum;
	PTR_TYPE next_line_ptr;
};

static struct linenum_cache_t linenum_cache[MAX_LINENUM_CACHE_DEPTH];
static int linenum_cache_ptr;
#endif


static int variables[MAX_VARNUM];

static unsigned char ended;

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
	unsigned char i;
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
    tokenizer_error_print(current_linenum, SYNTAX_ERROR);
    ubasic_break();
  }
  tokenizer_next();
}
/*---------------------------------------------------------------------------*/
static int
varfactor(void)
{
  int r;
  r = ubasic_get_variable(tokenizer_variable_num());
  accept(TOKENIZER_VARIABLE);
  return r;
}
/*---------------------------------------------------------------------------*/
static int
factor(void)
{
  int r=0;
  #if UBASIC_RND
  int b;
  #endif
  
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
    r=epeek_expression();
    break;
  #endif
  
  #if AVR_ADC
  case TOKENIZER_ADC:
	r=adc_expression();
    break;
  #endif
    
  #if AVR_IN
  case TOKENIZER_IN:
	r=pin_in_expression();
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
  while(op == TOKENIZER_ASTR  ||
		op == TOKENIZER_SLASH ||
		op == TOKENIZER_MOD) {
    tokenizer_next();
    f2 = factor();
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
  while(op == TOKENIZER_LT ||
		op == TOKENIZER_GT ||
		op == TOKENIZER_GE ||
		op == TOKENIZER_LE ||
		op == TOKENIZER_NE ||
		op == TOKENIZER_EQ) {
    tokenizer_next();
    r2 = expr();
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
	unsigned char i;
	// zuerst die Zeilennummer im Cache suchen
	for (i=0; i<linenum_cache_ptr; i++){
		//PRINTF("DEBUG (jump_linenum): l=%i, n=%s\n\r", linenum_cache[i].linenum, linenum_cache[i].p_name);
		if (linenum_cache[i].linenum == linenum
			#if UBASIC_EXT_PROC
				&& strncmp(current_proc, linenum_cache[i].p_name, MAX_PROG_NAME_LEN) == 0
			#endif 		
			) {
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
	}

#if USE_LINENUM_CACHE	
	// wenn noch im Zeilennummern-Cache Platz ist, Zeilennummer/-Pointer
	// merken
	if (linenum_cache_ptr < MAX_LINENUM_CACHE_DEPTH) {
		linenum_cache[linenum_cache_ptr].next_line_ptr=get_prog_text_pointer();
		linenum_cache[linenum_cache_ptr].linenum=linenum;
		#if UBASIC_EXT_PROC
		strncpy(linenum_cache[linenum_cache_ptr].p_name, current_proc, MAX_PROG_NAME_LEN);
		#endif
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
	unsigned char nl;
	accept(TOKENIZER_PRINT);
	do {
		nl=1;
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
	tokenizer_next();
}
#endif
/*---------------------------------------------------------------------------*/
static void
if_statement(void)
{
	int r;
	unsigned char no_then=0;
  
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
	tokenizer_next();
}
/*---------------------------------------------------------------------------*/
static void
gosub_statement(void)
{
	int linenum;
	accept(TOKENIZER_GOSUB);
	
#if UBASIC_EXT_PROC
	char p_name[MAX_PROG_NAME_LEN]="";
	if (tokenizer_token() == TOKENIZER_STRING) {
		strncpy(p_name, tokenizer_last_string_ptr(), MAX_PROG_NAME_LEN);
		jump_to_next_linenum();
	} else
#endif	
	if (tokenizer_token() == TOKENIZER_STRING) {
	    tokenizer_error_print(current_linenum, GOSUB_NO_EXT_SUBPROC);
    	ubasic_break();		
	}
	linenum = expr();
	// es muss bis zum Zeilenende gelesen werden, um die Rueck-
	// sprungzeile fuer return zu ermitteln
	//jump_to_next_linenum();
	// ... wg. expr() steht der Parser schon am Zeilenende CR (wenn Syntax 
	// ok ist), deshalb nur noch ein Token weiterlesen!
	tokenizer_next();
	if(gosub_stack_ptr < MAX_GOSUB_STACK_DEPTH) {
		gosub_stack[gosub_stack_ptr].p_ptr = get_prog_text_pointer();
		#if UBASIC_EXT_PROC
			strncpy(gosub_stack[gosub_stack_ptr].p_name, current_proc, MAX_PROG_NAME_LEN);
		#endif
		gosub_stack_ptr++;
#if UBASIC_EXT_PROC
		if (p_name[0]) {
			switch_proc(p_name);
		} else
#endif		
		jump_linenum(linenum);
	} else {
	    tokenizer_error_print(current_linenum, GOSUB_STACK_DETH);
    	ubasic_break();
	}
}
/*---------------------------------------------------------------------------*/
static void
return_statement(void)
{
	accept(TOKENIZER_RETURN);
	if(gosub_stack_ptr > 0) {
		gosub_stack_ptr--;
		#if UBASIC_EXT_PROC
			// wenn nicht gleiches Programm, dann erstmal umschalten...
			if (strncmp(current_proc, gosub_stack[gosub_stack_ptr].p_name, MAX_PROG_NAME_LEN)){
				switch_proc(gosub_stack[gosub_stack_ptr].p_name);
			}
		#endif		
		jump_to_prog_text_pointer(gosub_stack[gosub_stack_ptr].p_ptr);
	} else {
	    tokenizer_error_print(current_linenum, GOSUB_STACK_INVALID);
    	ubasic_break();
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
    for_stack_ptr++;
  } else {
    tokenizer_error_print(current_linenum, FOR_STACK_DETH);
   	ubasic_break();
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
#if UBASIC_RND
#if USE_AVR
long unsigned int seed = 0;
static void srand_statement(void) {
	unsigned int *p = (unsigned int*) (RAMEND+1);
	extern unsigned int __heap_start;
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
    tokenizer_error_print(current_linenum, UNKNOWN_STATEMENT);
    ubasic_break();
  }
}
/*---------------------------------------------------------------------------*/
static void
line_statement(void)
{
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