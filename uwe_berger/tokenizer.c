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
//	#define DEBUG_PRINTF(...)  usart_write(__VA_ARGS__)
	#define DEBUG_PRINTF(...)  printf(__VA_ARGS__)

#else
	#define DEBUG_PRINTF(...)
#endif


#include "tokenizer.h"
#include "ubasic_config.h"


#if USE_AVR
	#include "usart.h"
#else
	#include <string.h>
	#include <ctype.h>
	#include <stdlib.h>
	#include <stdio.h>
#endif

#if USE_PROGMEM
	#include <avr/pgmspace.h>
#endif

static char const *ptr;
static char const *last_num_ptr;

static char last_string[MAX_STRINGLEN+1];
static int  last_value;
static int  last_var_num;

#define MAX_NUMLEN 5

struct keyword_token {
#if USE_PROGMEM
	// um via strxxx_P zugreifen zu koennen, muss eine feste Laenge vorgegeben
	// werden
	char keyword[MAX_KEYWORD_LEN+1];
#else	
	char *keyword;
#endif
  int token;
};

static int current_token = TOKENIZER_ERROR;

#if USE_PROGMEM
static const struct keyword_token keywords[] PROGMEM = {
#else
static const struct keyword_token keywords[] = {
#endif
	{"let", TOKENIZER_LET},
	#if UBASIC_PRINT
	{"print", TOKENIZER_PRINT},
	#endif
	{"if", TOKENIZER_IF},
	{"then", TOKENIZER_THEN},
	{"else", TOKENIZER_ELSE},
	{"for", TOKENIZER_FOR},
	{"to", TOKENIZER_TO},
	{"downto", TOKENIZER_DOWNTO},
	{"step", TOKENIZER_STEP},
	{"next", TOKENIZER_NEXT},
	{"goto", TOKENIZER_GOTO},
	{"gosub", TOKENIZER_GOSUB},
	{"return", TOKENIZER_RETURN},
	{"end", TOKENIZER_END},
	#if UBASIC_ABS
	{"abs", TOKENIZER_ABS},
	#endif
	#if UBASIC_NOT
	{"not", TOKENIZER_NOT},
	#endif
	#if UBASIC_XOR
	{"xor", TOKENIZER_XOR},
	#endif
 	#if UBASIC_SHL
	{"shl", TOKENIZER_SHL},
	#endif
	#if UBASIC_SHR
	{"shr", TOKENIZER_SHR},
	#endif
	#if UBASIC_CALL
	{"call", TOKENIZER_CALL},
	#endif
	#if UBASIC_CVARS
	{"vpoke", TOKENIZER_VPOKE},
	{"vpeek", TOKENIZER_VPEEK},
	#endif
	#if UBASIC_REM
	{"rem", TOKENIZER_REM},
	#endif
	#if UBASIC_RND
	{"srand", TOKENIZER_SRND},
	{"rand", TOKENIZER_RND},
	#endif
	#if AVR_EPOKE
	{"epoke", TOKENIZER_EPOKE},
	#endif
	#if AVR_EPEEK
	{"epeek", TOKENIZER_EPEEK},
	#endif
	#if AVR_WAIT
	{"wait", TOKENIZER_WAIT},
	#endif
	#if AVR_DIR
	{"dir", TOKENIZER_DIR},
	#endif
	#if AVR_IN
	{"in", TOKENIZER_IN},
	#endif
	#if AVR_OUT
	{"out", TOKENIZER_OUT},
	#endif
	#if AVR_ADC
	{"adc", TOKENIZER_ADC},
	#endif
	{"or", TOKENIZER_OR},
	{"and", TOKENIZER_AND},
	{"mod", TOKENIZER_MOD},
	{"<=", TOKENIZER_LE},
	{">=", TOKENIZER_GE},
	{"<>", TOKENIZER_NE},
	{"", TOKENIZER_ERROR}
};

/*---------------------------------------------------------------------------*/
const char *get_prog_text_pointer(void) {
	//return ptr;
	return last_num_ptr;
}

/*---------------------------------------------------------------------------*/
void jump_to_prog_text_pointer(const char *jump_ptr) {
	ptr=jump_ptr;
	tokenizer_init(ptr);	
}

/*---------------------------------------------------------------------------*/
void jump_to_next_linenum(void) {
	while(*ptr != '\n' && *ptr != 0) {
		++ptr;  
	}
	if(*ptr == '\n') ++ptr;
	tokenizer_init(ptr);	
}

/*---------------------------------------------------------------------------*/
static int singlechar(void) {
	if(*ptr == '\n') {
		return TOKENIZER_CR;
	} else if(*ptr == ',') {
		return TOKENIZER_COMMA;
	} else if(*ptr == ';') {
		return TOKENIZER_SEMICOLON;
	} else if(*ptr == '+') {
		return TOKENIZER_PLUS;
	} else if(*ptr == '-') {
		return TOKENIZER_MINUS;
	} else if(*ptr == '&') {
		return TOKENIZER_AND;
	} else if(*ptr == '|') {
		return TOKENIZER_OR;
	} else if(*ptr == '*') {
		return TOKENIZER_ASTR;
	} else if(*ptr == '/') {
		return TOKENIZER_SLASH;
	} else if(*ptr == '%') {
		return TOKENIZER_MOD;
	} else if(*ptr == '(') {
		return TOKENIZER_LEFTPAREN;
	} else if(*ptr == ')') {
		return TOKENIZER_RIGHTPAREN;
	} else if(*ptr == '<') {
		return TOKENIZER_LT;
	} else if(*ptr == '>') {
		return TOKENIZER_GT;
	} else if(*ptr == '=') {
		return TOKENIZER_EQ;
  	}
	return 0;
}
/*---------------------------------------------------------------------------*/
static int get_next_token(void) {
#if !USE_PROGMEM
	struct keyword_token const *kt;
#endif
	uint8_t i;
	int temp_token;
  
	DEBUG_PRINTF("get_next_token(): '%s'\n", ptr);
	if(*ptr == 0) {
		return TOKENIZER_ENDOFINPUT;
	}
	if(isdigit(*ptr)) {
		last_value=0;
		for(i = 0; i <= MAX_NUMLEN; ++i) {
			if(!isdigit(*ptr)) {
				if(i > 0) {
					last_num_ptr = ptr - i; //??? doof!
					return TOKENIZER_NUMBER;
				} else {
				DEBUG_PRINTF("get_next_token: error due to too short number\r\n");
				return TOKENIZER_ERROR;
				}
			}
			if(!isdigit(*ptr)) {
				DEBUG_PRINTF("get_next_token: error due to malformed number\r\n");
				return TOKENIZER_ERROR;
			}
			last_value = 10 * last_value + *ptr - '0';
			ptr++;
    	}
		DEBUG_PRINTF("get_next_token: error due to too long number\r\n");
		return TOKENIZER_ERROR;
	} else if(*ptr == '"') {
						ptr++;
						i=0;
						do {
							last_string[i] = *ptr;
							++ptr;
							i++;
							// max. zulaessige Stringlaenge?
							if (i >= MAX_STRINGLEN) return TOKENIZER_ERROR;
						} while(*ptr != '"');
						// String null-terminieren
						last_string[i]=0;
						++ptr;
						return TOKENIZER_STRING;
				} else {
#if USE_PROGMEM  	
					for (i=0; i < sizeof(keywords)/sizeof(keywords[0])-1; i++) {
						if (strncasecmp_P(ptr, keywords[i].keyword, strlen_P(keywords[i].keyword)) == 0) {
							ptr = ptr + strlen_P(keywords[i].keyword);
							return pgm_read_word(&keywords[i].token);
						}
					}
#else  	
					for(kt = keywords; kt->token != TOKENIZER_ERROR; ++kt) {
						if(strncasecmp(ptr, kt->keyword, strlen(kt->keyword)) == 0) {
							ptr = ptr + strlen(kt->keyword);
							return kt->token;
						}
					}
#endif
				} 
				
	temp_token = singlechar();
	if (temp_token) {
		ptr = ptr + 1;
		return temp_token;
	}
	if((*ptr >= 'a' && *ptr <= 'z') || (*ptr >= 'A' && *ptr <= 'Z')) {
		if (*ptr >= 'a') last_var_num=*ptr-'a'; else last_var_num=*ptr-'A';
		ptr = ptr + 1;
		return TOKENIZER_VARIABLE;
	}
	return TOKENIZER_ERROR;
}
/*---------------------------------------------------------------------------*/
void tokenizer_init(const char *program) {
	ptr = program;
	current_token = get_next_token();
}
/*---------------------------------------------------------------------------*/
int tokenizer_token(void) {
	return current_token;
}
/*---------------------------------------------------------------------------*/
void tokenizer_next(void) {
	if(tokenizer_finished()) return;
	DEBUG_PRINTF("tokenizer_next: %p\r\n", nextptr);
	while(*ptr == ' ') ++ptr;
	current_token = get_next_token();
	DEBUG_PRINTF("tokenizer_next: '%s', %i\r\n", ptr, current_token);
	return;
}
/*---------------------------------------------------------------------------*/
int tokenizer_num(void) {
	return last_value;
}
/*---------------------------------------------------------------------------*/
const char * tokenizer_last_string_ptr(void) {
	return (const char*)&last_string[0];
}
/*---------------------------------------------------------------------------*/
void tokenizer_error_print(int linenum, int error_nr) {
	PRINTF("error %i at line: '%i'\r\n", error_nr, linenum);
}
/*---------------------------------------------------------------------------*/
int tokenizer_finished(void) {
	return *ptr == 0 || current_token == TOKENIZER_ENDOFINPUT;
}
/*---------------------------------------------------------------------------*/
int tokenizer_variable_num(void) {
	return last_var_num;
}
