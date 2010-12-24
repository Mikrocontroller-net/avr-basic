/*----------------------------------------------------------------------------
*             Testprogramm fuer ubasic_avr-Interpreter 
*             ========================================
*               Uwe Berger (bergeruw@gmx.net); 2010
* 
* ==>> Basic-Programmtext im PROGMEM eines AVR-Mikrocontrollers <<==
*      --------------------------------------------------------
* 
* Ein-/Ausgaben erfolgen ueber die serielle Schnittstelle (57600, 8, n, 1)
*
* Kommandos:
*        ls                --> listet alle verfuegbaren Programmnamen 
*                              <prog_name> auf
*        list <prog_name>  --> gibt den Quelltext des Basic-Programm aus
*        run  <prog_name>  --> startet das Basic-Programm; das laufende
*                              Programm kann man mit einem [ESC] abgebrochen 
*                              werden
*        mem               --> Infos ueber RAM (unused, free)
*        time              --> Laufzeit des letzten ausgefuehrten Basic-Programms
*                              in Millisekunden
*
*        <prog_name> siehe auch ubasic_tests.h
*
* ---------
* Have fun!
* 
* ----------------------------------------------------------------------------*/

#include <avr/io.h>
#include "uart/usart.h"
#include "avr_basic/ubasic_config.h"
#include "avr_basic/tokenizer_access.h"
#include "avr_basic/ubasic_ext_proc.h"
#include "avr_basic/ubasic.h"
#include "ubasic_tests.h"
#include "mem_check/mem_check.h"

//Taktfrequenz
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//Baudrate der seriellen Schnittstelle
#define BAUDRATE 57600

#define PRELOAD_T0 6

volatile uint16_t time_row;
uint16_t time_all;


//************************************************************************
ISR(TIMER0_OVF_vect)
{
	TCNT0 = PRELOAD_T0;						// Preloadwert setzen
	time_row++;
}


//************************************************************************
//************************************************************************
//************************************************************************
int main(void)
{  
	char buffer[20];
	char* command = buffer;
	const char *p;
	signed char i;
	char p_name[MAX_PROG_NAME_LEN];
	
#if UBASIC_EXT_PROC
	extern char current_proc[MAX_PROG_NAME_LEN];
#endif
	
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
	// Hauptschleife    
	while(1){
		// Prompt
        usart_write("\r\n>");
        // Eingabe...
        if(usart_read_line(command, sizeof(buffer)) < 1) continue;
        // Programm aus Programmflash starten
        if (strncmp_P(command, PSTR("run "), 4) == 0) {
			command += 4;
			if(command[0] == '\0') continue;
			i=get_program_pgm_idx(command);
			if (i<0) usart_write("Programm nicht vorhanden!\n\r");
			else {
				usart_write_str("\r\n");
				time_all=0;
				#if UBASIC_EXT_PROC
					strncpy(current_proc, command, MAX_PROG_NAME_LEN);
				#endif
				ubasic_init((const char*)get_program_pgm_ptr(i));
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
			}
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
        // Programmnamen auflisten
        if (strcmp_P(command, PSTR("ls")) == 0) {
			for (i=0; i<get_program_pgm_count(); i++) {
				strcpy_P(p_name, get_program_pgm_name(i));
				usart_write("%s\n\r", p_name);
			}
        } else
        // Basic-Programm aus Programmflash auflisten
        if (strncmp_P(command, PSTR("list "), 5) == 0) {
			usart_write_str("\r\n");
			command += 5;
			i=get_program_pgm_idx(command);
			if (i<0) usart_write("Programm nicht vorhanden!\n\r");
			else {
				p=(const char*)get_program_pgm_ptr(i);
				while (pgm_read_byte(p)) {
					if (pgm_read_byte(p) == '\n') usart_write_char('\r');
					usart_write("%c", pgm_read_byte(p));
					p++;
				}
			}
		// unbekanntes Kommando!	
        } else 	
        	usart_write("\r\nKommando >%s< unbekannt bzw. nicht vollstaendig!\r\n", command);
    }
	return(0);
}

