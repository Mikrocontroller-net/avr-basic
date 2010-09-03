/*----------------------------------------------------------------------------
* Testprogramm fuer ubasic_avr-Interpreter....
* 
* Uwe Berger (bergeruw@gmx.net); 2010
* 
* 
* Ein-/Ausgaben erfolgen ueber die serielle Schnittstelle (57600, 8, n, 1)
*
* Kommandos:
*        list   --> gibt das aktuelle Basic-Programm aus
*        run    --> startet das aktuell geladene Basic-Programm; das
*                   laufende Programm kann man mit einem [ESC] abbrechen
*        load   --> die folgenden Eingaben werden als Basic-Programm gewertet
*                   mit einem [ESC] beendet man den Eingabe-Mode
*        load n --> laed das Programm mit der angegenen Nummer n (siehe
*                   auch ubasic_tests.h)
*        mem    --> Infos ueber RAM (unused, free)
*        time   --> Laufzeit des letzten ausgefuehrten Basic-Programms
*                   in Millisekunden
*
* Have fun!
* ---------
* 
* ----------------------------------------------------------------------------*/

#include <avr/io.h>
#include "usart.h"
#include "ubasic.h"
#include "ubasic_tests.h"
#include "mem_check.h"

//Taktfrequenz
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//Baudrate der seriellen Schnittstelle
#define BAUDRATE 57600



#define MAX_PROGRAM_LEN	200

static char program[MAX_PROGRAM_LEN] = "";

#define PRELOAD_T0 6

volatile uint16_t time_row;
uint16_t time_all;

//************************************************************************
ISR(TIMER0_OVF_vect)
{
	TCNT0 = PRELOAD_T0;						// Preloadwert setzen
	time_row++;
}


//----------------------------------------------------------------------------
void basic_load( char* Buffer){
	int i = 0;
	char c;

	while((c=usart_receive_char()) != 0x1b && i < MAX_PROGRAM_LEN - 1) {
		if (c=='\r' || c=='\n') {
			c='\n';
			usart_write("\r\n");
		} else {
			usart_write_char(c);
		}
		Buffer[i++]=c;
	}
	Buffer[i] = '\0';
	usart_write_str("\r\nEnde Eingabemodus...\r\n");
}

//----------------------------------------------------------------------------
void basic_list(char *c) {
	while (*c) {
		if (*c == '\n') usart_write_char('\r');
		usart_write_char(*c++);
	}
}

//----------------------------------------------------------------------------
void load_from_flash(char* Buffer, int prog_idx) {
	const char *pstrflash;
	if (prog_idx >= (sizeof(progs)/sizeof(progs[0]))) {
		usart_write_str("\r\nProgramm existiert nicht!\r\n");
	} else {
		pstrflash = (const char*)(pgm_read_word(&(progs[prog_idx])));
		strncpy_P(Buffer, pstrflash, MAX_PROGRAM_LEN);
		usart_write("\n\rProgramm %i geladen.\n\r", prog_idx);
	}
}

//----------------------------------------------------------------------------
//Hier startet das Hauptprogramm
int main(void)
{  
	char buffer[10];
	
    usart_init(BAUDRATE); // setup the UART
    
    // Timer 0 initialisieren (runtime-Messung)
    TCNT0  = PRELOAD_T0;							// Preloadwert Timer0 setzen
	TCCR0B |= (1 << CS01)|(1 << CS00);				// Prescaler Timer0 auf 64 stellen
	TIMSK0 |= (1 << TOIE0);
	sei();
	
	usart_write("AVR-uBasic-Interpreter; Uwe Berger, 2010\r\n");
	usart_write("Compiliert am "__DATE__" um "__TIME__"\r\n");
	usart_write("\r\nunused mem: %i\r\n", get_mem_unused());
	usart_write("free mem..: %i\r\n", get_mem_free());
    // Prompt
    usart_write("\r\n>");
	
	while(1){

		char* command = buffer;
        if(usart_read_line(command, sizeof(buffer)) < 1) continue;
        // Basic-Programm starten
        if (strcmp_P(command, PSTR("run")) == 0) {
			usart_write_str("\r\n");
			time_all=0;
			ubasic_init(program);
			do {
				time_row=0;
				ubasic_run();
				time_all += time_row;
				// Basic-Programm unterbrechen, wenn [ESC] empfangen
				if (usart_is_receive() && (usart_receive_char() == 0x1b)) {
					usart_write_str("\r\nProgramm unterbrochen...!\r\n");
					break;
				}
			} while(!ubasic_finished());
        } else
        // Programm aus Programmflash laden
        if (strncmp_P(command, PSTR("load "), 5) == 0) {
			command += 5;
			if(command[0] == '\0') continue;
			load_from_flash(program, atoi(command));
        } else
        // Programmeingabe via serieller Schnittstelle
        if (strcmp_P(command, PSTR("load")) == 0) {
			usart_write_str("\r\nBeginn manueller Eingabemodus, Ende mit [ESC].\r\n");
			basic_load(program);			
        } else
        // ein paar Infos ueber den RAM
        if (strcmp_P(command, PSTR("mem")) == 0) {
			usart_write("\r\nunused mem: %i\r\n", get_mem_unused());
			usart_write("free mem..: %i\r\n", get_mem_free());
        } else
        // Infos ueber Laufzeit des Basic-Programmes
        if (strcmp_P(command, PSTR("time")) == 0) {
			usart_write("\r\nruntime: %ims\r\n", time_all);
        } else
        // Basic-Programm auflisten
        if (strcmp_P(command, PSTR("list")) == 0) {
			usart_write_str("\r\n");
			basic_list(program);
        } else 	usart_write("\r\nKommando %s unbekannt!\r\n", command);
        // Prompt
        usart_write("\r\n>");
    }
		
return(0);
}

