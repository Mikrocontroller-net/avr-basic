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
 *
 * ---------------------------------------------------------------------------
 * Source heavily modified by Rene Böllhoff (reneboellhoff@gmx.de); Jun 2010
 *
 *  - Removed DEBUG_PRINTF-Option
 *  - Added Char-Device-Functionality for flexible input
 *  - Added Compiler-Features & Changes for parallel use (Text/Bin)
 *  - Added Extensions for Access to C-Functions/Arrays/Variables
 *  - Added Preprocessor Generated Code with common Config (basic_cfg.h)
 *
 * ---------------------------------------------------------------------------
 */


/*--- Allgemeine Header ---*/

#include <stdio.h> /* printf() */
#include <string.h> /* memset() */
#ifdef WIN32
	#include <conio.h>
#endif


/*--- uBasic Header & Platformen ---*/

#include "platform.h"
#include "ubasic.h"
#include "tokenizer.h"

#include "uart.h"


/*--- Datentyp für For-Schleifen ---*/

typedef struct 
{
  I16 line_after_for;		// Zeile/Tokenposition nach dem for-Statement
  I16 to;								// Schleifenzähler bis
	I8 for_variable;			// Schleifenvariable
} T_FOR_STATE;



/*--- Lokale Variablen ---*/

I16					gosub_stack	[MAX_GOSUB_STACK_DEPTH];	// Stack für Gosub/Return
T_FOR_STATE for_stack		[MAX_FOR_STACK_DEPTH];		// Stack für For-Schleifen
I16					variables		[MAX_VARNUM];							// Variablen
U8					gosub_stack_ptr;											// Gosub-Stackpointer
U8					for_stack_ptr;												// For-Stackpointer
I8					ended;																// Basic-Prg beendet

/*--- Externe Variablen wenn Compiler benutzt wird ---*/

#ifdef BASIC_USES_COMPILER
	extern I8		runmode;				// Run-Modus (RM_TEXT/RM_COMPILED)
	extern I16	gototokenpos;		// zuletzt ermittelte Goto/Gosub Tokenposition (nur runmode == RM_COMPILED)
#endif

/*--- Forward Deklarationen ---*/

I16		expr						(void);		// Ausdruck holen
void	statement				(void);		// Statement bearbeiten




/*--- uBasic Resetten (Variablen, Stacks, Stackpointer initialisieren) ---*/

void ubasic_reset(void)
{
	// Stackpointer resetten
  for_stack_ptr = gosub_stack_ptr = 0;
	// For-Stack Löschen
	memset (for_stack,		0, sizeof (for_stack)		);
	// Variablen Löschen
	memset (variables,		0, sizeof (variables)		);
	// Gosub-Stack Löschen
	memset (gosub_stack,	0, sizeof (gosub_stack)	);
}


/*--- Zuletzt ermitteltes Token prüfen und wenn ok neues Token ermitteln ---*/

void accept(TOKEN token)
{
	// aktuelles Token mit dem zu erwartenden Token vergleichen
  if(token != tokenizer_token()) {
		// bei Fehler -> Fatal Abort
    FATAL_ABORT(FA_UNEXPECTED_TOKEN);
  }
	// wenn ok -> nächstes Token holen
  tokenizer_next();
}


/*--- Hilfsfunktionen für Extensions ---*/

#ifdef BASIC_USES_EXTENSIONS
	
	/*--- Parameter für C-Funktionsaufruf holen ---*/

	I8 get_parameters(T_USER_FUNC_DATAS *params, I16 numparams, I8 next)
	{
		U8 ptr = 0;
		// wenn keine Parameter erwartet werden, nächstes Token holen und tschüss
		if (numparams == 0) { tokenizer_next(); return 0;} 

		// Solange noch parameter zu holen sind
		while (ptr < numparams)
		{
			switch (tokenizer_token())
			{
				// String ablegen
				case TOKENIZER_STRING 	:
					tokenizer_string (((*params) [ptr].text), MAX_USER_KEYWORD_LEN);
					(*params) [ptr].type = 2;
					tokenizer_next ();
					break;
        // Integer-Wert (Zahl/Ausdruck/Variable) ablegen
				case TOKENIZER_NUMBER 	:
				case TOKENIZER_VARIABLE :
				case TOKENIZER_NAME 		:
					(*params) [ptr].value = expr();
					(*params) [ptr].type = 1;
					break;
				default :
        // alles andere -> Fehler
					FATAL_ABORT(FA_ERROR_IN_PARAMETER_LIST);
			}
			// Komma holen (als Separator für Parameter)
			if (++ptr < numparams) { accept (TOKENIZER_COMMA); }
		}
		// Optional nächstes Token schon holen
		if (next) { tokenizer_next(); }
		return 0;
	}


	/*--- Array-Index für ext. C-Array holen ---*/

	I16 get_arrayindex (void)
	{
		I16 i;
		accept (TOKENIZER_LEFTBRACK);		// Eckige Klammer auf
		i = expr();											// Index als Ausdruck holen
		accept (TOKENIZER_RIGHTBRACK);	// Eckige Klammer zu
		return i;
	}
#endif


/*--- Ausdrucks-Funktionen ---*/


/*--- Zahl/Variable/ext. Faktor inkl Klammerausdrücke ---*/

I16 factor(void)
{
  I16								r = 0;		// Result/Temporäre Variable
#ifdef BASIC_USES_EXTENSIONS
  I16								i,				// Index für Array
										*arr;			// Zeiger auf Array
	T_USER_FUNC_DATAS temp;			// Parameter für ext. Funktionsaufruf
	T_BASIC_USER_FUNC func;			// Funktionszeiger auf ext. Funktion
	TOKEN							idx;			// Index für ext. Aufruf
	U8								type;			// ext. Aufruftyp (1=Fkt 2=Var 3=Arr)
	U16								data;			// zus. Daten für ext. Aufruf
	void							*ptr;			// Zeiger für ext. Aufruf (Fkt/Var/Array)
#endif

  switch(tokenizer_token()) {
		//---------------------------------------------------------------
		// Zahl verarbeiten
		case TOKENIZER_NUMBER:
			r = tokenizer_num();
			accept(TOKENIZER_NUMBER);
			break;
#ifdef BASIC_USES_EXTENSIONS
		//---------------------------------------------------------------
		// Name verarbeiten
		case TOKENIZER_NAME:
			idx = tokenizer_userdata (&type, &data, &ptr);
			if (idx == -1) { FATAL_ABORT (FA_EXTENSION_WORD_NOT_FOUND); }
			switch (type)
			{
				case 1 : 
				  // C-Funktion aufrufen
					accept (TOKENIZER_NAME);
					get_parameters (&temp, data, 0);
					func = ptr;
					r = func (temp);
					break;
				case 2 :
				  // C-Variable holen 
					accept(TOKENIZER_NAME);
					r = *((I16 *) ptr);
					break;
				case 3 : 
				  // Wert aus C-Array holen
					accept(TOKENIZER_NAME);
					i = get_arrayindex ();
					arr = (I16 *) ptr;
					if ((i >= 0) && (i < data))
					{
						r = arr [i];
					}
					break;
			}
			break;
#endif
		//---------------------------------------------------------------
		//  Klammer-ausdrücke verarbeiten
		case TOKENIZER_LEFTPAREN:
			accept(TOKENIZER_LEFTPAREN);
			r = expr();
			accept(TOKENIZER_RIGHTPAREN);
			break;
		//---------------------------------------------------------------
		case TOKENIZER_ERROR :
			FATAL_ABORT (FA_ERROR_IN_FACTOR);
			break;
		//---------------------------------------------------------------
		// Basic-Variable holen
		default:
			r = ubasic_get_variable(tokenizer_variable_num());
			accept(TOKENIZER_VARIABLE);
			break;
  }
  return r;
}


/*--- Für Terme/Produkte/Vergleiche den Abschnitt BASIC_TOKEN_OPS einblenden ---*/

#define BASIC_TOKEN_OPS

/*--- Term holen ---*/

I16 term(void)
{
  I16 f1, f2;
  TOKEN tk;

	// 1. Faktor holen
  f1 = factor();
	// Operation holen
  tk = tokenizer_token();
	// Solange wie TERM-Operationen ausgeführt werden sollen ...
	while (
		#define TOKEN_CHAR_TERM(token,op,func)	(tk == token) ||
		#define TOKEN_CHAR_EXPR(token,op,func)
		#define TOKEN_CHAR_REL(token,op,func)
			#include BASIC_CONFIG_FILE
		#undef TOKEN_CHAR_TERM
		#undef TOKEN_CHAR_EXPR
		#undef TOKEN_CHAR_REL
		(0)) {
		// nächstes Token
    tokenizer_next();
		// weiteren Faktor holen
    f2 = factor();
		// ausführen
		switch (tk)
		{
		  // Umschaltung Aufruf der operation via Funktion / Textersetzung als Direkt-Anweisung
#ifndef BASIC_USES_FUNC_AS_OPS
			#define TOKEN_CHAR_TERM(token,op,func)	case token : f1 = f1 op f2; break;
#else
			#define TOKEN_CHAR_TERM(token,op,func)	case token : f1 = func (f1, f2); break;
#endif
			#define TOKEN_CHAR_EXPR(token,op,func)
			#define TOKEN_CHAR_REL(token,op,func)
				#include BASIC_CONFIG_FILE
			#undef TOKEN_CHAR_TERM
			#undef TOKEN_CHAR_EXPR
			#undef TOKEN_CHAR_REL
		  case TOKENIZER_ERROR :
				FATAL_ABORT (FA_ERROR_IN_TERM);
				break;
		}

    tk = tokenizer_token();
  }
  return f1;
}


/*--- Ausdruck holen ---*/

I16 expr(void)
{
  I16 t1, t2;
  TOKEN tk;
  
	// 1. Term holen
  t1 = term();
	// Operation holen
  tk = tokenizer_token();
	// Solange wie EXPR-Operationen ausgeführt werden sollen ...
	while (
		#define TOKEN_CHAR_TERM(token,op,func) 
		#define TOKEN_CHAR_EXPR(token,op,func)	(tk == token) ||
		#define TOKEN_CHAR_REL(token,op,func)
			#include BASIC_CONFIG_FILE
		#undef TOKEN_CHAR_TERM
		#undef TOKEN_CHAR_EXPR
		#undef TOKEN_CHAR_REL
		(0)) {
		// nächstes Token
    tokenizer_next();
		// weiteren Faktor holen
    t2 = term();
		// ausführen
		switch (tk)
		{
			#define TOKEN_CHAR_TERM(token,op,func)	
		  // Umschaltung Aufruf der operation via Funktion / Textersetzung als Direkt-Anweisung
#ifndef BASIC_USES_FUNC_AS_OPS
			#define TOKEN_CHAR_EXPR(token,op,func)	case token : t1 = t1 op t2; break;
#else
			#define TOKEN_CHAR_EXPR(token,op,func)	case token : t1 = func (t1, t2); break;
#endif
			#define TOKEN_CHAR_REL(token,op,func)
				#include BASIC_CONFIG_FILE
			#undef TOKEN_CHAR_TERM
			#undef TOKEN_CHAR_EXPR
			#undef TOKEN_CHAR_REL
		  case TOKENIZER_ERROR :
				FATAL_ABORT (FA_ERROR_IN_EXPRESSION);
				break;
		}
    tk = tokenizer_token();
  }
  return t1;
}


/*--- Vergleich ---*/

I16 relation(void)
{
  I16 r1, r2;
  TOKEN tk;
  
	// 1. Operanden holen
  r1 = expr();
	// Operation holen
  tk = tokenizer_token();
	// Solange wie Vergleichs-Operationen ausgeführt werden sollen ...
	while (
		#define TOKEN_CHAR_TERM(token,op,func) 
		#define TOKEN_CHAR_EXPR(token,op,func)
		#define TOKEN_CHAR_REL(token,op,func)		(tk == token) ||
			#include BASIC_CONFIG_FILE
		#undef TOKEN_CHAR_TERM
		#undef TOKEN_CHAR_EXPR
		#undef TOKEN_CHAR_REL
		(0)) {
		// nächstes Token
    tokenizer_next();
		// weiteren Faktor holen
    r2 = expr();
		// ausführen
		switch (tk)
		{
			#define TOKEN_CHAR_TERM(token,op,func)	
			#define TOKEN_CHAR_EXPR(token,op,func)
		  // Umschaltung Aufruf der operation via Funktion / Textersetzung als Direkt-Anweisung
#ifndef BASIC_USES_FUNC_AS_OPS
			#define TOKEN_CHAR_REL(token,op,func)		case token : r1 = r1 op r2; break;
#else
			#define TOKEN_CHAR_REL(token,op,func)		case token : r1 = func (r1, r2); break;
#endif
				#include BASIC_CONFIG_FILE
			#undef TOKEN_CHAR_TERM
			#undef TOKEN_CHAR_EXPR
			#undef TOKEN_CHAR_REL
		}
    tk = tokenizer_token();
  }
  return r1;
}


#undef BASIC_TOKEN_OPS


/*--- Gehe zu Zeile (für Goto/Gosub/For) ---*/

void jump_linenum(I16 linenum)
{
	// wenn Compiler benutzt wird Unterscheidung RunMode berücksichtigen
#ifdef BASIC_USES_COMPILER
	if (runmode == RM_TEXT)
	{
#endif
		// Im Text Modus : zu Anfang des Quelltextes gehen (und Zeilennummer holen)
		tokenizer_start_pos();
		// Solange Zeile noch nicht gefunden ...
		while(tokenizer_num() != linenum) {
			do {
				do {
					// Wenn Ende des Textes oder Fehler ...
					tokenizer_next();
					if ((tokenizer_token() == TOKENIZER_ERROR) || 
						 (tokenizer_token() == TOKENIZER_ENDOFINPUT)) 
					{
						// Fehler : Zeilennummer nicht gefunden
						FATAL_ABORT(FA_ERROR_JUMP_DESTINATION_NOT_FOUND);
					}
					// bis Ende der Zeile oder Text-Ende Lesen
				} while(tokenizer_token() != TOKENIZER_CR &&
					tokenizer_token() != TOKENIZER_ENDOFINPUT);
				// War es Zeilenende ?!
				if(tokenizer_token() == TOKENIZER_CR) {
					// Dann weiter
					tokenizer_next();
				}
				// Solange wir Nummern haben
			} while(tokenizer_token() != TOKENIZER_NUMBER);
		}
#ifdef BASIC_USES_COMPILER
	}
	else
	{
		// Im Compiled Mode -> Einfach Zeiger setzen
		tokenizer_jump_to_pos (linenum);
	}
#endif
}


/*--- Goto-Statement ---*/

void goto_statement(void)
{
	// Erwarte Goto
  accept(TOKENIZER_GOTO);
#ifdef BASIC_USES_COMPILER
	// Bei Compilerunterstützung : Je nach RunMode Zeilennummer oder Position holen und springen
	jump_linenum(runmode == RM_TEXT ? tokenizer_num() : gototokenpos);
#else
	// Ohne Compilerunterstützung : Zeilennummer holen und springen
  jump_linenum(tokenizer_num());
#endif
}


/*--- Print-Statement  ---*/

void print_statement(void)
{
	// Temporärer String
	char string [MAX_STRINGLEN];

	// Erwarte Print
  accept(TOKENIZER_PRINT);
  do {
		// Soll ein String gedruckt werden ? 
    if(tokenizer_token() == TOKENIZER_STRING) {
      tokenizer_string(string, sizeof(string));
      BASIC_PRINTF_P (PSTR("%s"), string);
      tokenizer_next();
		// Soll ein Space gedruckt werden ? 
    } else if(tokenizer_token() == TOKENIZER_COMMA) {
      BASIC_PRINTF_P (PSTR(" "));
      tokenizer_next();
    // Platzhalter ? 
    } else if(tokenizer_token() == TOKENIZER_SEMICOLON) {
      tokenizer_next();
    // Soll der Wert einer Variablen gedruckt werden ? 
    } else if((tokenizer_token() == TOKENIZER_VARIABLE) ||
#ifdef BASIC_USES_EXTENSIONS
    // Soll der Wert einer Variablen (Extensions) gedruckt werden ? 
				(tokenizer_token() == TOKENIZER_NAME)  ||
#endif
	      (tokenizer_token() == TOKENIZER_NUMBER)) 
				 {
      BASIC_PRINTF_P (PSTR("%d"), expr());
    } else {
      break;
    }
		// Solange ausführen bis Ende des Textes oder der Zeile
  } while(tokenizer_token() != TOKENIZER_CR &&
	  tokenizer_token() != TOKENIZER_ENDOFINPUT);
	// Neue Zeile nicht vergessen
  PRINTF_P (PSTR("\n\r"));
	// Nächstes Token Bitte
  tokenizer_next();
}


/*--- If-Statement ---*/

void if_statement(void)
{
  I16 r;
  
	// Erwarte If
  accept(TOKENIZER_IF);
	// Vergleich
  r = relation();
	// Erwarte Then
  accept(TOKENIZER_THEN);
	// Wenn vergleich TRUE then Zweig ausführen
  if(r) {
    statement();
  } else {
		// Ansonsten Then überlesen bis Ende der Zeile, Ende des Textes oder eben Else
    do {
      tokenizer_next();
    } while(tokenizer_token() != TOKENIZER_ELSE &&
	    tokenizer_token() != TOKENIZER_CR &&
	    tokenizer_token() != TOKENIZER_ENDOFINPUT);
		// Bei Else -> Else Zweig ausführen
    if(tokenizer_token() == TOKENIZER_ELSE) {
      tokenizer_next();
      statement();
    // Bei Zeilenende -> Nächstes Token
    } else if(tokenizer_token() == TOKENIZER_CR) {
      tokenizer_next();
    }
  }
}


/*--- Verkürztes Let-Statement ---*/

void let_statement(void)
{
  I8 var;

	// Variablennummer holen 
  var = tokenizer_variable_num();
	// Erwarte Variable
  accept(TOKENIZER_VARIABLE);
	// Erwarte =
  accept(TOKENIZER_EQ);
	// Hole Ausdruck und weise den Wert der Variablennummer zu
  ubasic_set_variable(var, expr());
	// Erwarte Ende der Zeile
  accept(TOKENIZER_CR);
}


/*--- Normales Let-Statement ---*/

void let_statement_2(void)
{
	// Erwarte Let
	accept (TOKENIZER_LET);
	// Der Rest wie gehabt
	let_statement ();
}


/*--- Gosub-Statement ---*/

void gosub_statement(void)
{
  I16 linenum = 0;
	CPOS currpos;
	accept(TOKENIZER_GOSUB);
#ifdef BASIC_USES_COMPILER
	if (runmode == RM_TEXT)
	{
#endif
		linenum = tokenizer_num();
		accept(TOKENIZER_NUMBER);
#ifdef BASIC_USES_COMPILER
	}
#endif
	currpos = tokenizer_get_rel_pos ();
  accept(TOKENIZER_CR);
  if(gosub_stack_ptr < MAX_GOSUB_STACK_DEPTH) {
#ifdef BASIC_USES_COMPILER
		gosub_stack[gosub_stack_ptr] = (runmode == RM_TEXT) ? tokenizer_num() : currpos;
#else
    gosub_stack[gosub_stack_ptr] = tokenizer_num();
#endif
		gosub_stack_ptr++;
#ifdef BASIC_USES_COMPILER
		jump_linenum((runmode == RM_TEXT) ? linenum : gototokenpos);
#else
    jump_linenum(linenum);
#endif
  } else {
  }
}


/*--- Return-Statement ---*/

void return_statement(void)
{
  accept(TOKENIZER_RETURN);
  if(gosub_stack_ptr > 0) {
    gosub_stack_ptr--;
		jump_linenum(gosub_stack[gosub_stack_ptr]);
  } else {

	}
}


/*--- For-Statement ---*/

void for_statement(void)
{
  I8 for_variable;
	I16  to;
  CPOS pos;

  accept(TOKENIZER_FOR);
  for_variable = tokenizer_variable_num();
  accept(TOKENIZER_VARIABLE);
  accept(TOKENIZER_EQ);
  ubasic_set_variable(for_variable, expr());
  accept(TOKENIZER_TO);
  to = expr();
	pos = tokenizer_get_rel_pos();
  accept(TOKENIZER_CR);

  if(for_stack_ptr < MAX_FOR_STACK_DEPTH) {
#ifdef BASIC_USES_COMPILER
		for_stack[for_stack_ptr].line_after_for = (runmode == RM_TEXT) ? tokenizer_num() : pos;
#else
    for_stack[for_stack_ptr].line_after_for = tokenizer_num ();
#endif
    for_stack[for_stack_ptr].for_variable = for_variable;
    for_stack[for_stack_ptr].to = to;
    for_stack_ptr++;
  } else {
  }
}


/*--- Next-Statement ---*/

void next_statement(void)
{
  I8 var;
  
  accept(TOKENIZER_NEXT);
  var = tokenizer_variable_num();
  accept(TOKENIZER_VARIABLE);
  if(for_stack_ptr > 0 &&
     var == for_stack[for_stack_ptr - 1].for_variable) {
    ubasic_set_variable(var,
			ubasic_get_variable(var) + 1);
    if(ubasic_get_variable(var) <= for_stack[for_stack_ptr - 1].to) {
			jump_linenum(for_stack[for_stack_ptr - 1].line_after_for);
    } else {
      for_stack_ptr--;
      accept(TOKENIZER_CR);
    }
  } else {
    accept(TOKENIZER_CR);
  }
}



/*--- End-Statement ---*/

void end_statement(void)
{
  accept(TOKENIZER_END);
  ended = 1;
}



/*--- Statement-Verteiler ---*/

void statement(void)
{
  TOKEN token;
#ifdef BASIC_USES_EXTENSIONS
	T_USER_FUNC_DATAS temp;
	T_BASIC_USER_FUNC func;
	TOKEN idx; 
	U8 type;
	I16 *arr, i;
	U16 data;
	void *ptr;
#endif
  
  token = tokenizer_token();

	if (0) { }
	// Wenn Extensions verwendet werden, werden diese zuerst prüfen
#ifdef BASIC_USES_EXTENSIONS
  else	if (token == TOKENIZER_NAME)
	{
		accept (TOKENIZER_NAME);
		// Daten zum TOKENIZER_NAME holen
		idx = tokenizer_userdata (&type, &data, &ptr);
		// Je nach Aufruftyp ...
		switch (type)
		{
			case 1 :
			  // ... Parameter für C-Fkt holen und ausführen
				get_parameters (&temp, data, 1);
				func = ptr;
				func (temp);
				break;
			case 2 :
			  // ... C-Variable zuweisen
				arr = ptr;
				accept (TOKENIZER_EQ);
				*arr = expr();
				tokenizer_next();
				break;
			case 3 :
			  // ... C-Array-Eintrag zuweisen
				i = get_arrayindex ();
				arr = ptr;
				accept (TOKENIZER_EQ);
				arr [i] = expr();
				tokenizer_next();
				break;
			default : 
				FATAL_ABORT(FA_UNIMPLEMENTED_FUNCTION);
				break;
		}
	}
#endif
	else if (token == TOKENIZER_VARIABLE)
	{
		let_statement ();
	} 
	else 
	{
		tokenizer_exec_token_func ();
	}
}


/*--- Zeile als Statement ---*/

void line_statement(void)
{
  accept(TOKENIZER_NUMBER);  
	statement();
  return;
}




/*--- uBasic Initialisieren ---*/

void ubasic_init(I8 rmode)
{
  // For/Gosub-Stack/Variablen/Modus initialisieren
	ubasic_reset ();
  tokenizer_init(rmode);
  ended = 0;
}

/*--- uBasic ausführen (nur 1 Statement !) ---*/

void ubasic_run(void)
{
  if(tokenizer_finished()) {
    return;
  }

  line_statement();
}


/*--- Abfrage ob uBasic-Prg beendet ist (Fehler oder reguläres Ende) ---*/

I8 ubasic_finished(void)
{
  return ended || tokenizer_finished();
}


/*--- Interface-Funktion für Variable setzen ---*/

void ubasic_set_variable(I8 varnum, I16 value)
{
  // Prüfen ob Variablennummer gültig
  if(varnum > 0 && varnum <= MAX_VARNUM) {
	  // Variablenwert setzen
    variables[varnum - 1] = value;
  }
}


/*--- Interface-Funktion für Variable holen ---*/

I16 ubasic_get_variable(I8 varnum)
{
  // Prüfen ob Variablennummer gültig
  if(varnum > 0 && varnum <= MAX_VARNUM) {
	  // Variablenwert holen
    return variables[varnum - 1];
  }
  return 0;
}



/*--- Dummy-Funktion für execbasicfunc ---*/

void dummyfunc (void)
{
}


/*--- Variablen Operatoren (ohne weitere Prüfung) ---*/

#ifdef BASIC_USES_FUNC_AS_OPS

	I16 basic_add (I16 a, I16 b)		{ return a+b; 	}
	I16 basic_sub (I16 a, I16 b) 		{ return a-b; 	}
	I16 basic_mul (I16 a, I16 b)		{ return a*b;		}
	I16 basic_div (I16 a, I16 b)		{ return a/b; 	}
	I16 basic_mod (I16 a, I16 b)		{ return a%b; 	}
	I16 basic_and (I16 a, I16 b)		{ return a&b; 	}
	I16 basic_or  (I16 a, I16 b)		{ return a|b; 	}
	I16 basic_xor (I16 a, I16 b)		{ return a^b; 	}
	I16 basic_rel_eq (I16 a, I16 b)	{ return a==b; 	}
	I16 basic_rel_gt (I16 a, I16 b)	{ return a>b; 	}
	I16 basic_rel_lt (I16 a, I16 b)	{ return a<b; 	}
	I16 basic_rel_le (I16 a, I16 b)	{ return a<=b; 	}
	I16 basic_rel_ge (I16 a, I16 b)	{ return a>=b; 	}
	I16 basic_rel_ne (I16 a, I16 b)	{ return a!=b; 	}
	I16 basic_shr (I16 a, I16 b)		{ return a>>b; 	}
	I16 basic_shl (I16 a, I16 b)		{ return a<<b; 	}

#endif
