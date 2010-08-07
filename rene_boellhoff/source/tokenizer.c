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





/*  --- Header --- */

#include "platform.h"

#include "tokenizer.h"
#include <string.h>
#include <ctype.h>
#include <stdlib.h>


/*  --- Funktionsprototypen --- */

TOKEN singlechar									(void);
TOKEN get_token_from_memory 			(void);
TOKEN get_next_token_from_memory 	(void);



/*  --- Lokale Typen (alle Modi) --- */

typedef struct 
{
  char *keyword;
  TOKEN token;
	T_BASIC_FUNCTION func;
} T_KEYWORD_TOKEN;


/*  --- Lokale Typen für Zeichengerät --- */
#ifdef BASIC_USES_CHARDEVICE

	typedef void (* T_SET_TEXT)         (void *pvData, CPOS len);
	typedef char (* T_WORK_CHAR)				(void);
	typedef void (* T_NEXT_CHAR)				(void);
	typedef CPOS (* T_GET_RPOS)					(void);
	typedef void (* T_SET_RPOS)					(CPOS stRel);

	typedef struct
	{
	  T_SET_TEXT  pfSetText;		// Setze Text im Zeichengerät
		T_WORK_CHAR	pfWorkChar;		// Aktuelles Zeichen
		T_NEXT_CHAR	pfNextChar;		// Nächstes Zeichen
		T_GET_RPOS 	pfGetRPos;		// Hole Position
		T_SET_RPOS 	pfSetRPos;		// Setze Position
	} T_CHAR_DEVICE;

#endif


/*  --- Lokale Variablen (alle Modi) --- */

TOKEN current_token = TOKENIZER_ERROR;  // zuletzt erkanntes Token
int 	lastvalue;		// zuletzt erkannter Wert    (current_token = TOKEN_NUMBER)
int 	lastvarnum;		// zuletzt erkannte Variable (current_token = TOKEN_VARIABLE)
CPOS 	laststrpos;		// zuletzt erkannter String  (current_token = TOKEN_STRING)



/* --- Lokale Variablen für Zeichengerät (CharDevice) --- */

#ifdef BASIC_USES_CHARDEVICE
	/* --- Funktionszeiger (Variablen) für CharDevice Funktionen  --- */
	T_SET_TEXT 	pfSetText;		// Setzt Text im Zeichengerät
	T_WORK_CHAR	pfGetChar;		// Holt aktuelles Zeichen
	T_NEXT_CHAR	pfNextChar;		// Setzt Zeiger auf das nächste Zeichen
	T_GET_RPOS	pfGetRPos;		// Holt Zeichenzeiger relativ zum Anfang des Codes
	T_SET_RPOS	pfSetRPos;		// Setzt Zeichenzeiger relativ zum Anfang des Codes
#endif


/*  --- Lokale Variablen für Compiler Funktionalität --- */

#ifdef BASIC_USES_COMPILER
	char				 	runmode = RM_TEXT;								// Setzt den Verarbeitungsmodus (Text oder Binär)
	int  					gototokenpos = 0;									// zuletzt erkannte Goto/Gosub "Zeile" innerhalb des Streams
	unsigned char stringlength;											// aktuelle String-länge
	char 					tokendata [MAX_USER_KEYWORD_LEN];	// Buffer für aktuelles Token
#endif


/*  --- Lokale Variablen für die Extensions --- */

#ifdef BASIC_USES_EXTENSIONS
	char  userkeyword [MAX_USER_KEYWORD_LEN];		// Buffer für zu suchendes Keyword für Extensions
	TOKEN	userkeywordindex 	= 0;								// Index des erkannten Keywords (-1 = nicht gefunden)
	char 	userkeywordtype  	= 0;								// Typ des erkannten Keywords 1 : Funktion 2 : Variable 3 : Array)
	int 	userkeyworddata  	= 0;								// zus. Daten des erk. Keywords (Index für Array)
	void *userkeywordptr 		= NULL;							// Zeiger auf Fkt/Var/Array (je nach Typ)
#endif





/*  --- Lokale Variablen für Text/Symbol-Tokens  --- */
// Die Daten zu den Tokens werden per Präprozessor generiert.
// Die Daten selbst stehen in der basic_cfg.h und werden hier
// "schablonenartig" in die Arrays eingeblendet (bzw erzeugen
// selbst Arrays)

// Sektion BASIC_TOKENS einblenden
#define BASIC_TOKENS

	/*  --- Token-Texte erzeugen --- */
	#define TOKEN_WORD(name,text,func) char keyword__##name [] FLASHMEM = text;
	#define TOKEN_CHAR(name,char)
		#include BASIC_CONFIG_FILE
	#undef TOKEN_WORD
	#undef TOKEN_CHAR

	/*  --- Token-Texte, Token-Wert und dazugehörige Verarbeitungsfunktion (uBasic.c) --- */
	T_KEYWORD_TOKEN keywords[] FLASHMEM = {
		#define TOKEN_WORD(name,text,func) { keyword__##name, name, func, },
		#define TOKEN_CHAR(name,char)
			#include BASIC_CONFIG_FILE
		#undef TOKEN_WORD
		#undef TOKEN_CHAR
		{NULL, TOKENIZER_ERROR, NULL, }
	};

#undef BASIC_TOKENS



/*  --- Lokale Variablen für Erweiterungen (Extensions) --- */
// ToDo : Einzelne Einblendungen als Typ zusammenfassen
#ifdef BASIC_USES_EXTENSIONS

	// Sektion BASIC_USER_EXTENSIONS einblenden
	#define BASIC_USER_EXTENSIONS

		int user_dummy_var = 0;

		/*  --- Keyword-Texte für C-Funktionen/Variablen/Arrays erzeugen --- */
	  // Dummy-Variable für Extensions 
		char uservar_dummy [] FLASHMEM = "dummy";
		#define USER_FUNCTION(name,text,func,param)	char userfunc__##name [] FLASHMEM  = { text };
		#define USER_VARIABLE(name,text,var)				char uservar__##name  [] FLASHMEM  = { text };
		#define USER_ARRAY(name,text,var,size)			char userarr__##name  [] FLASHMEM  = { text };	
			#include BASIC_CONFIG_FILE
		#undef USER_FUNCTION
		#undef USER_VARIABLE
		#undef USER_ARRAY

		/*  --- Keyword-Liste für C-Funktionen/Variablen/Arrays erzeugen --- */
		char *userkeywords [] FLASHMEM  = {
			uservar_dummy, 
			#define USER_FUNCTION(name,text,func,param)	userfunc__##name,
			#define USER_VARIABLE(name,text,var)				uservar__##name, 
			#define USER_ARRAY(name,text,var,size)			userarr__##name, 	
				#include BASIC_CONFIG_FILE
			#undef USER_FUNCTION
			#undef USER_VARIABLE
			#undef USER_ARRAY
		};

		/*  --- Zeiger Liste für C-Funktionen/Variablen/Arrays erzeugen --- */
		void *userpointer [] FLASHMEM  = {
			&user_dummy_var,
			#define USER_FUNCTION(name,text,func,param)	(void *) func,	
			#define USER_VARIABLE(name,text,var)				(void *) &var,
			#define USER_ARRAY(name,text,var,size)			(void *) &var,
				#include BASIC_CONFIG_FILE
			#undef USER_FUNCTION
			#undef USER_VARIABLE
			#undef USER_ARRAY
		};

		/*  --- Typ Liste für C-Funktionen/Variablen/Arrays erzeugen --- */
		char usertypes [] FLASHMEM  = {
			2,
			#define USER_FUNCTION(name,text,func,param)	1,
			#define USER_VARIABLE(name,text,var)				2,
			#define USER_ARRAY(name,text,var,size)			3,
				#include BASIC_CONFIG_FILE
			#undef USER_FUNCTION
			#undef USER_VARIABLE
			#undef USER_ARRAY
		};

		/*  --- zus. Daten für C-Funktionen/Variablen/Arrays erzeugen  --- */
		int userdatas [] FLASHMEM  = {
			0,
			#define USER_FUNCTION(name,text,func,param)	param,
			#define USER_VARIABLE(name,text,var)				0,
			#define USER_ARRAY(name,text,var,size)			size,
				#include BASIC_CONFIG_FILE
			#undef USER_FUNCTION
			#undef USER_VARIABLE
			#undef USER_ARRAY
		};

	#undef BASIC_USER_EXTENSIONS

#endif



/*  --- Fehlertexte für den gesamten Komplex (Tokenizer/Interpreter/Compiler) --- */
#ifdef BASIC_USES_ERROR_TEXTS


	// Text einzeln erzeugen
	char errortext__unknown_error [] FLASHMEM = { "unknown error" };
	#define BASIC_ERROR(name,text) char errortext__##name [] FLASHMEM = { text };
		#include BASIC_CONFIG_FILE
	#undef BASIC_ERROR


	// Texte als Liste von char * aufbauen
	char *errortext_list [] FLASHMEM = {
		#define BASIC_ERROR(name,text)		errortext__##name,
			#include BASIC_CONFIG_FILE
		#undef BASIC_ERROR
	};

	// Gewünsche Fehlernummer als Text holen (Zeiger auf FLASH-Speicher wenn AVR)
	char *tokenizer_geterror_text (U8 error)
	{
		if (error >= TOKENIZER_NUMBER_ERRORS)
		{
			return (char *) errortext__unknown_error;
		}
		return pgm_read_ptr (&(errortext_list [error]));
	}


#else

	// Wenn Texte nicht übersetzt werden, immer Leer-String zurückgeben

 	char errortext__none [] FLASHMEM = { "" };

	char *tokenizer_geterror_text (U8 error)
	{
		return errortext__none;
	}

#endif





// Wird das Zeichengerät verwendet so werden die elementaren Zugriffe
// (hole zeichen, nächstes zeichen, hole/setze Position) über Funktionszeiger und Makros gemappt
// Die letztendendes definierten Makros stellen dann die Elementarzugriffe dar.
#ifdef BASIC_USES_CHARDEVICE

	T_CHAR_DEVICE astCharDevices [] FLASHMEM = {
		#define TOKENIZER_MEDIUM(name,st,cc,nc,gp,sp) { st, cc, nc, gp, sp, }, 
			#include BASIC_CONFIG_FILE
		#undef TOKENIZER_MEDIUM
	};


	/* --- Makros zum Aufruf der Funktion via Funktionszeiger --- */
	#define tokenizer_settext(x,l)	pfSetText(x,l)
	#define tokenizer_getchar()			pfGetChar()
	#define tokenizer_nextchar()		pfNextChar()
	#define tokenizer_startpos()		pfSetRPos((CPOS) 0)
	#define tokenizer_setrelpos(x)	pfSetRPos(x)
	#define tokenizer_getrelpos()		pfGetRPos()

	/*  --- Setzen der Funktionszeiger für Zeichengerät  --- */
	void  tokenizer_chardevice    (U8 device)
	{
	  if (device < TOKENIZER_NUM_CHARDEVICES)
		{
		  pfSetText   = (T_SET_TEXT ) pgm_read_funcptr (&(astCharDevices [device].pfSetText ));
			pfGetChar		= (T_WORK_CHAR) pgm_read_funcptr (&(astCharDevices [device].pfWorkChar));
			pfNextChar	= (T_NEXT_CHAR) pgm_read_funcptr (&(astCharDevices [device].pfNextChar));
			pfSetRPos 	= (T_SET_RPOS ) pgm_read_funcptr (&(astCharDevices [device].pfSetRPos ));
			pfGetRPos		= (T_GET_RPOS ) pgm_read_funcptr (&(astCharDevices [device].pfGetRPos ));
		}
	}

#else



	/* --- Zeichengerät "RAM" (wenn die Zeichengeräte-Funktionalität nicht aktiviert ist --- */

	/* --- ein ganz normaler char-Zeiger --- */
	char *charptr, *charptrbak;

	/* --- Anpassungsmakros für WIN32/AVR (32/16Bit-Zeigerlänge)  --- */
	#ifdef WIN32
		#define PTRTYPE void *
	#endif
	#ifdef AVR
		#define PTRTYPE unsigned int
	#endif

	/* --- Makros zum direkten Zugriff auf die Zeiger --- */
	// Mit diesem Trick werden die Elementaroperationen direkt
	// in Zeigeroperationen auf dem SRAM umgesetzt. Dadurch
	// wird der Geschwindigkeitsvorteil enorm, gegenüber 
	// der über Funktionszeiger abstrahierten Implementierung.
	// Zudem sind die selben Elementarfunktionsmakros definiert
	// wie auch bei einem Zeichengerät, sprich die ganze
	// Zeichen-Quelle ist transparent aufgebaut und der Tokenizer
	// bedient sich nur dieser 6 Makros.
	#define tokenizer_settext(x,l)  charptr = charptrbak = (char * x);
	#define tokenizer_getchar()			*charptr
	#define tokenizer_nextchar()		charptr++
	#define tokenizer_startpos()		charptr = charptrbak;
	#define tokenizer_setrelpos(x)	charptr = &(charptrbak [(unsigned int) (x)])
	#define tokenizer_getrelpos()		(CPOS) ((PTRTYPE) (charptr - charptrbak))


#endif


/*  --- Text Setzen für alle Modi & Zeichengeräte (Makro aufrufen)  --- */
void tokenizer_set_text (void *program, CPOS length)
{
	tokenizer_settext (program, length);
}


/* --- strncmp_P mit Elementarfunktionen  --- */
char tokenizer_strncmp_P (char *string, int len)
{
	CPOS pos = tokenizer_getrelpos ();
	char c1, c2;
	for (; len > 0; len --)
	{
		c1 = tokenizer_getchar (); tokenizer_nextchar ();
		c2 = pgm_read_byte (string); string ++;
		if (c1 > c2) { tokenizer_setrelpos (pos); return  1; }
		if (c1 < c2) { tokenizer_setrelpos (pos); return -1; }
	}
	return 0;
}

/* --- Ext. Zugriff auf "Setze Zeiger auf Anfang des Codes" --- */
void  tokenizer_start_pos   (void)
{
	tokenizer_startpos ();
}

/* --- Ext. Zugriff auf "Setze Zeiger relativ zum Anfang des Codes"  --- */
void  tokenizer_set_rel_pos (CPOS stPos)
{
	tokenizer_setrelpos (stPos);
}

/* --- Ext. Zugriff auf "Hole Zeiger relativ zum Anfang des Codes"  --- */
CPOS  tokenizer_get_rel_pos   (void)
{
	return tokenizer_getrelpos ();
}







/*  --- Funktionen für die Extensions --- */
// Hier wird nach den Bezeichnern der Erweiterungen 
// gesucht und bei Fund die entsprechenden Daten
// (Zeiger, Anzahl Parameter, Index innerhalb des Arrays,
// Typ) bereitgestellt
#ifdef BASIC_USES_EXTENSIONS

	/*  --- Simple suche nach Extension-Keyword in den Extension-Daten --- */
	TOKEN searchuserkeyword (void)
	{
		unsigned char c;
		char *keyword;

		// einfach Extension-Keyword-Liste
		// ToDo : Optimierung !!!!
		// Hinweis : + 1 wegen dummy 
		for (c = 0; c < BASIC_NUM_USER_EXTENSIONS + 1; c++)
		{
			keyword = pgm_read_ptr (&(userkeywords [c]));
			if ((strcmp_P (userkeyword, keyword) == 0) && (c != 0))
			{
				// wenn Vergleich zutrifft, betreffende Daten holen
				userkeywordindex 	= c;
				userkeyworddata 	= pgm_read_byte (&(userdatas 	[c]));
				userkeywordtype 	= pgm_read_byte (&(usertypes 	[c]));
				userkeywordptr  	= pgm_read_ptr  (&(userpointer [c]));
				return c;
			}
		}
		// ansonsten betreffende Daten resetten
		userkeywordindex 	= -1;
		userkeyworddata 	= 0;
		userkeywordtype 	= 0;
		userkeywordptr 		= NULL;
		return -1;
	}

	/*  --- Liefert die Daten zum aktuellen Extension Aufruf --- */
	TOKEN tokenizer_userdata (U8 *type, U16 *data, void **ptr)
	{
		*type = userkeywordtype;
		*data = userkeyworddata;
		*ptr  = userkeywordptr;
		return userkeywordindex;
	}

	/*  --- Liefert den Index des Extension Aufrufs --- */
	TOKEN tokenizer_userdata_idx (void)
	{
		return userkeywordindex;
	}
#endif






/*  --- Holt Token (wenn möglich) an aktueller Zeichenposition --- */
TOKEN get_next_token_from_text(void)
{
	char *keyword,											// Extension Keyword aus Liste zum suchen 
				ch = tokenizer_getchar ();		// aktuelles Zeichen
  unsigned char i;										// Zähler für Extension Keyword
	int val = 0;												// "zusammengebauter Wert" für Zeilennummer/Zahl
	CPOS pos;														// aktuelle Zeichenposition merken
	
	// 0x00 ? -> Ende des Textes
	if (ch == 0) {
    return TOKENIZER_ENDOFINPUT;
  }
	

	// Zahlen ?
	if(isdigit(ch)) {
		pos = tokenizer_getrelpos ();
		// Maximale Zahlenlänge durchgehen
    for(i = 0; i < MAX_NUMLEN; ++i) {
			ch = tokenizer_getchar (); 
			// wenn keine Zahl mehr ...
      if(!isdigit(ch)) {
				// waren schon welche da ?
				if(i > 0) {
					// ok, Zahl gefunden
					lastvalue = val;
					return TOKENIZER_NUMBER;
				}
			}
			val *= 10; val += ch - '0'; // Dezimalzahl bilden
			tokenizer_nextchar ();			// nächste Stelle
    }
		// mehr als 5 Ziffern ? -> Fehler
		tokenizer_setrelpos (pos);
    return TOKENIZER_ERROR;


  // Strings ?
  } else if(ch == '"') {
		// Position Merken (inkl. erstem ")
		laststrpos = tokenizer_getrelpos ();
#ifdef BASIC_USES_COMPILER
		stringlength = 0;
#endif
    do {
			tokenizer_nextchar ();
#ifdef BASIC_USES_COMPILER
			if (tokenizer_getchar () != '"') { stringlength++; }
#endif
    } while(tokenizer_getchar () != '"');
		tokenizer_nextchar ();
    return TOKENIZER_STRING;


	// Basic-Schlüsselwörter aus basic_cfg.h ?
  } else {
		// Einfach Liste durchgehen
		// ToDo : Optimierung !!!!
    for(i = 0; i < (sizeof (keywords)/sizeof (keywords[0]));i++) {
		  keyword = pgm_read_ptr (&(keywords [i].keyword));
			if (keyword == NULL) break;
      if(tokenizer_strncmp_P(keyword, strlen_P(keyword)) == 0) {
				return pgm_read_word (&(keywords [i].token));
      }
    }
	}


	// Einzelzeichen-Token ? (außerhalb von Alpha)
	// +,-,*,/,(,) usw ...
  if(singlechar()) {
		ch = singlechar();
		tokenizer_nextchar ();
    return ch;
	}

	// Nochmals Alpha...
  if (ch >= 'a' && ch <= 'z') {
		// Position merken
		pos = tokenizer_getrelpos ();
		tokenizer_nextchar ();
		// nächstes Zeichen kein Alpha mehr ?
		if (!((tokenizer_getchar () >= 'a') && (tokenizer_getchar () <= 'z')))
		{
			// dann Variable -> Variablennummer holen
			lastvarnum = ch - 'a' + 1;
			return TOKENIZER_VARIABLE;
		} 

		// wenn nächstes Zeichen doch Alpha, dann Extension-Keyword holen
#ifdef BASIC_USES_EXTENSIONS		
		else {
			// zurück auf erstes Alpha Zeichen
			tokenizer_setrelpos (pos);
			memset (userkeyword, 0, sizeof (userkeyword));
			// solange prüfen bis MAX_USER_KEYWORD_LEN erreicht ist und jedes Zeichen
			// in userkeyword speichern
			for (i = 0; i < MAX_USER_KEYWORD_LEN - 1; i++)
			{
				ch = tokenizer_getchar ();
				if (isalpha (ch) || (ch == '_'))  { userkeyword [i] = ch; tokenizer_nextchar (); }
																		else	{ break; }

			}
			// kann eig. weg ....
			if (i == 0)  { tokenizer_setrelpos (pos); return TOKENIZER_ERROR; }
			// Extension-Keyword suchen, und ggf Daten setzen
			searchuserkeyword ();
			return TOKENIZER_NAME;
		}
#endif
	}
	// wenn hier immer noch nichts gefunden ... dann ist nicht mehr zu helfen
  return TOKENIZER_ERROR;
}



/*  --- Führe Basic-Keyword-Funktion aus --- */
void tokenizer_exec_token_func (void)
{
	unsigned char temp;  // Index für Funktionszegerarray
	T_BASIC_FUNCTION func;
	if ((current_token >= TOKENIZER_KEYWORDS) && (current_token < TOKENIZER_KEYCHARS))
	{
		// Index berechnen
	  temp = current_token - TOKENIZER_KEYWORDS - 1;
		// Funktion holen
	  func = (T_BASIC_FUNCTION) pgm_read_ptr (&(keywords [temp].func));
		// Ausführen
	  func();
	}
}



/*  --- auf Einzelzeichen-Token prüfen --- */
TOKEN singlechar(void)
{
	char x = tokenizer_getchar ();
	#define BASIC_TOKENS
																				if (0) { }
		#define TOKEN_WORD(name,text,func)
		#define TOKEN_CHAR(name,char)				else if (x == char) 	return name;
			#include BASIC_CONFIG_FILE
		#undef TOKEN_CHAR
		#undef TOKEN_WORD
																				else 										return 0;
	#undef BASIC_TOKENS
}


/*  --- Token-Funktionen für Compiler-Funktionalität --- */
#ifdef BASIC_USES_COMPILER

	/*  --- Byte aus Stream holen --- */
	unsigned char get_token_byte (void)
	{
		unsigned char temp = tokenizer_getchar ();
		tokenizer_nextchar ();
		return temp;
	}


	/*  --- Word aus Stream holen --- */
	unsigned short get_token_word (void)
	{
		unsigned short temp = (unsigned char) tokenizer_getchar ();
		tokenizer_nextchar ();
		temp = temp | ((unsigned short) tokenizer_getchar () << 8);
		tokenizer_nextchar ();
		return temp;
	}


	/*  --- String aus Stream holen und nach destbuffer kopieren --- */
	void get_token_string (char *destbuffer)
	{
		unsigned char len = get_token_byte ();

		for (; len > 0; len --)
		{
			*destbuffer++ = get_token_byte ();
		}
		*destbuffer = 0;
	}


	/*  --- Selbe Funktion wie get_token_from_text nur aus dem Binär-Stream (also vorcompiliert) --- */
	TOKEN get_next_token_from_memory (void)
	{
	// für die Extensions brauchen wir den Index
	#ifdef BASIC_USES_EXTENSIONS
		unsigned char c;  
	#endif
		// Temporäre Variable für Word-Operationen (Zahlen und Zeiger (64kByte !))
		short word;

		// aktuelles Token holen
		tokendata [0] = get_token_byte ();
		// je nach Token unterschiedliche Daten holen/aufbereiten
		switch (tokendata [0])
		{
			// Zahl 16 Bit holen (als Operand, Zeilennummer oder Zeiger)
			case TOKENIZER_NUMBER		:
				word = get_token_word ();
				memcpy (&(tokendata [1]), &word, 2);
				break;
      // bei eingeschalteten Extensions spart man sich ja
			// das searchkeywords. Dementsprechend ist nur der Index von Interesse
			// und es werden hier automatisch die entprechenden Extension-Daten
			// bereitgestellt.
	#ifdef BASIC_USES_EXTENSIONS
			case TOKENIZER_NAME			:
				tokendata [1] = get_token_byte ();
				memcpy (&c, &(tokendata [1]), 1);
				userkeywordindex 	= c;
				userkeyworddata 	= pgm_read_byte (&(userdatas 	[c]));
				userkeywordtype 	= pgm_read_byte (&(usertypes 	[c]));
				userkeywordptr  	= pgm_read_ptr  (&(userpointer [c]));
				break;
	#endif
			// 
			case TOKENIZER_VARIABLE :
				tokendata [1] = get_token_byte ();
				break;
			case TOKENIZER_STRING		:
				get_token_string (&(tokendata [1]));
				break;
			// goto und gosub benötigen hier "extra-behandlung" (siehe uBasic.c)
			case TOKENIZER_GOTO  		:
			case TOKENIZER_GOSUB 		:
				gototokenpos = get_token_word ();
				memcpy (&(tokendata [1]), &gototokenpos, 2);
				break;
		}
		return tokendata [0];
	}
#endif


/*  --- Hauptfunktion zur Bestimmung des nächsten Tokens (beide Modi) --- */
TOKEN get_next_token (void)
{
// Bei Compilerunterstützung runmode berücksichtigen
#ifdef BASIC_USES_COMPILER
	if (runmode == RM_COMPILED)
	{
		// vorcompiliert
		return get_next_token_from_memory();
	}
	else
	{
#endif
		// Text
		return get_next_token_from_text();
#ifdef BASIC_USES_COMPILER
	}
#endif
}


/*  ---  Setze Zeiger auf Position und hole aktuelles Token --- */
#ifdef BASIC_USES_COMPILER
	void tokenizer_jump_to_pos (CPOS jumptodest)
	{
		tokenizer_setrelpos ((CPOS) jumptodest);
		tokenizer_next();
	}
#endif


/*  --- Runmode setzen (wenn Compiler-Benutzt wird) und aktuelles Token holen --- */
// Wichtig	: Je nach Modus muß das Zeichengerät auf den Anfang des
//						Codes/Textes gesetzt werden.
void tokenizer_init(char rmode)
{
#ifdef BASIC_USES_COMPILER
	runmode = rmode;
#endif
	current_token = get_next_token();
}


/*  --- holt das aktuelle Token --- */
TOKEN tokenizer_token(void)
{
  return current_token;
}


/*  --- holt das nächste Token --- */
void tokenizer_next(void)
{

  if(tokenizer_finished()) {
    return;
  }
#ifdef BASIC_USES_COMPILER
	if (runmode == RM_TEXT)
	{
#endif
		while (tokenizer_getchar() == ' ')
		{
			tokenizer_nextchar ();
		}
#ifdef BASIC_USES_COMPILER
	}
#endif
  current_token = get_next_token();
  return;
}


/*  --- Sind wir am Ende des Textes/Codes ? --- */
char tokenizer_finished(void)
{
// Bei Compilerunterstützung runmode berücksichtigen
#ifdef BASIC_USES_COMPILER
	if (runmode == RM_TEXT)
	{
#endif
		// Je nach runmode ist das Zeichen \0 und/oder das Token TOKENIZER_ENDOFINPUT das Ende 
		return tokenizer_getchar () == 0 || current_token == TOKENIZER_ENDOFINPUT;
#ifdef BASIC_USES_COMPILER
	}
	else
	{
		// Im Compiled Mode ist das Ende mit TOKENIZER_ENDOFINPUT gekennzeichnet
		return current_token == TOKENIZER_ENDOFINPUT;
	}
#endif
}


/*  --- holt eine zuvor erkannte Zahl (TOKEN_NUMBER) --- */
int tokenizer_num(void)
{
#ifdef BASIC_USES_COMPILER
	short temp;
	if (runmode == RM_TEXT)
	{
#endif
		return lastvalue;
#ifdef BASIC_USES_COMPILER
	}
	else
	{
		memcpy (&temp, &(tokendata [1]), 2);
		return (int) temp;
	}
#endif
}


/*  --- holt einen zuvor erkannten String (TOKEN_STRING) --- */
void tokenizer_string(char *dest, char len)
{
  char ch, pc = 0;
	CPOS pos = tokenizer_getrelpos();
  
  if(tokenizer_token() != TOKENIZER_STRING) {
    return;
  }
#ifdef BASIC_USES_COMPILER
	if (runmode == RM_TEXT)
	{
#endif

		tokenizer_setrelpos (laststrpos);
    for (; len > 0; len--)
		{
			ch = tokenizer_getchar (); tokenizer_nextchar ();
			if (ch != '"') 
			{ 
				*dest = ch; dest++; 
			}
			else 
			{
				pc++; if (pc == 2) { *dest = 0; break; }
			}
			if (ch == 0) { break; }
		}
		tokenizer_setrelpos (pos);
#ifdef BASIC_USES_COMPILER
	}
	else
	{
		if ((U8) len > (U8) strlen (&(tokendata [1]))) { len = (I8) strlen (&(tokendata [1])) + 1; }
		strncpy (dest, &(tokendata [1]), len);
	}
#endif
}



/*  --- holt eine zuvor erkannte Variable (TOKEN_STRING) ---*/
char tokenizer_variable_num(void)
{
#ifdef BASIC_USES_COMPILER
	if (runmode == RM_TEXT)
	{
#endif
		return lastvarnum;
#ifdef BASIC_USES_COMPILER
	}
	else
	{
		return tokendata [1];
	}
#endif
}


/*  --- Compiler-Funktionalität  --- */

#ifdef BASIC_USES_COMPILER

	/*  --- legt ein Byte ab --- */
	void tokenizer_drop_byte (T_SAVE_BYTE drop, unsigned char data)
	{
		if (drop) drop (data);
	}


	/*  --- legt ein Word ab  --- */
	void tokenizer_drop_word (T_SAVE_BYTE drop, unsigned short data)
	{
		tokenizer_drop_byte (drop, (data >> 0) & 0xff);
		tokenizer_drop_byte (drop, (data >> 8) & 0xff);
	}


	/*  --- legt ein Long ab  --- */
	void tokenizer_drop_long (T_SAVE_BYTE drop, unsigned long data)
	{
		tokenizer_drop_word (drop, (data >>  0) & 0xffff);
		tokenizer_drop_word (drop, (data >> 16) & 0xffff);
	}


	/*  --- ermittelt die Größe des Tokens in Byte --- */
	char tokenizer_get_token_size (int token, int strlen)
	{
		char size = 1;
		if (token == TOKENIZER_NUMBER)   { size += 2; }
		if (token == TOKENIZER_VARIABLE) { size += 1; }
#ifdef BASIC_USES_EXTENSIONS
		if (token == TOKENIZER_NAME)     { size += 1; }
#endif
		if (token == TOKENIZER_STRING)   { size += 1 + strlen; }
		return size;
	}


	/*  --- Die eigentliche Compiler-Funktion --- */
	char tokenizer_translate(T_SAVE_BYTE drop)
	{	
		char					state = 1,											// Zustand bei Zeilennummern Caching
									orm = runmode;									// aktuell gesetzter RunMode merken
    unsigned int	linecnt = 0,										// Zähler für Cache
									tokenpos = 0,										// Position innerhalb des Token-Streams
									i;															// allg. Zähler (für Cache-Suche, String-Kopieren)
	  char					string [MAX_USER_KEYWORD_LEN];	// temp. String

		int linepos [MAX_LINE_BUFFER];								// Zeilen-Positions-Cache (der Einfachheit halber hier als lokale Variable)
		int linenum [MAX_LINE_BUFFER];								// Zeilen-Nummern-Cache (der Einfachheit halber hier als lokale Variable)

		runmode = RM_TEXT;								// runmode = Text
		tokenizer_startpos ();						// Anfang des Textes
		current_token = get_next_token(); // erstes Token holen

		// Statemachine :
		// Erwartet zuerst eine Zahl (Wert und Pos merken). Dann suchen bis CR oder Ende.
		// Dabei Token-Position im Stream mitzählen und bei CR/Ende nächsten Eintrag setzen
		do
		{
			switch (state)
			{
				case 1:
					switch (current_token)
					{
						case TOKENIZER_NUMBER			: state =		2; 
																				linepos [linecnt] = tokenpos;
																				linenum [linecnt] = tokenizer_num();
																				break;
						case TOKENIZER_ENDOFINPUT : state =   0; break;
						default :										state =  -1; break;
					}
					break;
				case 2:
					switch (current_token)
					{
						case TOKENIZER_CR					: state =		1; linecnt++; 
						 														if (linecnt >= MAX_LINE_BUFFER) { FATAL_ABORT (FA_TOO_MANY_LINES); }
																				break;
						case TOKENIZER_ENDOFINPUT : state =  -1; break;
						case TOKENIZER_ERROR			: state =  -1; break;
						case TOKENIZER_GOTO       :
						case TOKENIZER_GOSUB      :
							tokenpos += 2;
							tokenizer_next ();
							break;
					}
					break;


			}
			tokenizer_next ();
			tokenpos += tokenizer_get_token_size (current_token, stringlength);
		} while (state > 0);

		tokenizer_startpos ();

		current_token = get_next_token();
		state = 1;

		do
		{

			switch (current_token)
			{
				case TOKENIZER_GOSUB :
				case TOKENIZER_GOTO :
					tokenizer_drop_byte (drop, current_token);
					tokenizer_next (); 

					for (i = 0; i < linecnt; i++)
					{
						if (linenum [i] == tokenizer_num())
						{
							tokenizer_drop_word (drop, linepos [i]);
							break;
						}
					}
					if (i == linecnt) { FATAL_ABORT (FA_LINE_IN_CACHE_NOT_FOUND); }
					break;

#ifdef BASIC_USES_EXTENSIONS
				case TOKENIZER_NAME :
					tokenizer_drop_byte (drop, current_token);
					tokenizer_drop_byte (drop, tokenizer_userdata_idx());
					break;
#endif
				case TOKENIZER_VARIABLE :
					tokenizer_drop_byte (drop, current_token);
					tokenizer_drop_byte (drop, tokenizer_variable_num ());
					break;

				case TOKENIZER_NUMBER :
					tokenizer_drop_byte (drop, current_token);
					tokenizer_drop_word (drop, tokenizer_num ());
					break;

				case TOKENIZER_STRING :
					tokenizer_drop_byte (drop, current_token);
					tokenizer_drop_byte (drop, stringlength);
					tokenizer_string (string, MAX_USER_KEYWORD_LEN);
					for (i = 0; i < stringlength; i++)
					{
						tokenizer_drop_byte (drop, string [i]);
					}
					break;

				case TOKENIZER_ERROR			: 
					tokenizer_drop_byte (drop, current_token);
					state = -1; 
					break;
				case TOKENIZER_ENDOFINPUT : 
					tokenizer_drop_byte (drop, current_token);
					state = 0; 
					break;

				default : 
					tokenizer_drop_byte (drop, current_token);
					break;
			}

			tokenizer_next ();
		} while ((state > 0) && (!tokenizer_finished ()));

		tokenizer_drop_byte (drop, 0xff);

		runmode = orm;

		return 0;
	}

#endif
