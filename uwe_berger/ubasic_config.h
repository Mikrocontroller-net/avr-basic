/*
 *           Uwe Berger (bergeruw@gmx.net); 2010
 *           ===================================
 * Einige Defines, die hauptsaechlich den uBasic-Sprachumfang und die 
 * Zielplattform steuern.
 *
 *
 */
#ifndef __UBASIC_CONFIG_H__
#define __UBASIC_CONFIG_H__


// AVR-spezifischen einschalten
#ifndef USE_AVR
	#define USE_AVR		1
#endif

// regulaere Standardausgabe
#if USE_AVR
	#define PRINTF(...)  usart_write(__VA_ARGS__)
#else
	#define PRINTF(...)  printf(__VA_ARGS__)
#endif

// grml..., sollte man besser loesen!
#if !USE_AVR
	#define uint8_t unsigned char
#endif

// max. Stringlaenge (Basic)
#ifndef MAX_STRINGLEN
	#define MAX_STRINGLEN 20
#endif

// max. Schachtelungstiefe fuer GOSUB (Basic)
#define MAX_GOSUB_STACK_DEPTH 2

// max. Schachtelungstiefe fuer FOR-NEXT (Basic)
#define MAX_FOR_STACK_DEPTH 4

// Zeilennummern-Cache verwenden (für goto und gosub)
#define USE_LINENUM_CACHE		1

// max. Anzahl der gebufferten Zeilennummern
#define MAX_LINENUM_CACHE_DEPTH	8

// max. Anzahl Variablen (Basic)
#define MAX_VARNUM 26

// max. Laenge von Funktions- und Variablennamen in call(), vpeek() und vpoke()
#define MAX_NAME_LEN	8

// bei Verwendung des PROGMEM muess die Laenge des Schluesselwordfeldes
// fest vorgegeben werden (Tabelle keywords in tokenenizer.c)
#define MAX_KEYWORD_LEN	8

// einige Basic-Erweiterungen/-Befehle/-Anweisungen, die man nicht immer unbedingt benoetigt
#define UBASIC_ABS		1
#define UBASIC_NOT		1
#define UBASIC_CALL 	1
#define UBASIC_CVARS	1
#define UBASIC_REM		1
#define UBASIC_XOR		1
#define UBASIC_SHL		1
#define UBASIC_SHR		1
#define UBASIC_PRINT	1
#define UBASIC_RND		1

// exit(1) in Fehlersituationen macht sich bei AVRs etwas schlecht...
#ifndef BREAK_NOT_EXIT
	#define BREAK_NOT_EXIT	1
#endif

// die folgenden Defines nur, wenn USE_AVR gesetzt ist
#if USE_AVR
	// Verwendung des AVR-PROGMEM fuer einige Daten
	#define USE_PROGMEM		1
	// AVR-spezifischen Befehle an-/abwaehlen
	#define AVR_WAIT		1
	#define AVR_EPEEK		1
	#define AVR_EPOKE		1
	#define AVR_DIR			1
	#define AVR_IN			1
	#define AVR_OUT			1
	#define AVR_ADC			1
	// AVR-Ports fuer Basic-Befehle dir, in, out
	#define HAVE_PORTA		0
	#define HAVE_PORTB		1
	#define HAVE_PORTC		1
	#define HAVE_PORTD		1
	// AVR: Anzahl der ADC-Eingaenge (0...ACD_COUNT_MAX)
	#define ADC_COUNT_MAX	4
#endif // USE_AVR

#endif /* __UBASIC_CONFIG_H__ */
