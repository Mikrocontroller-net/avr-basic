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
*        list n --> gibt das Basic-Programm mit der Nummer n aus
*                   (siehe auch ubasic_tests.h)
*        run n  --> startet das Basic-Programm mit der Nummer n aus
*                   (siehe auch ubasic_tests.h)
*                   laufende Programm kann man mit einem [ESC] abbrechen
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
#include "tokenizer_access.h"
#include "ubasic.h"
#include "ubasic_tests.h"
#include "mem_check.h"

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

//----------------------------------------------------------------------------
//Hier startet das Hauptprogramm
int main(void)
{  
	char buffer[10];
	char* command = buffer;
	const char *p;
	
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
    usart_write("\r\n>");

	while(1){

        if(usart_read_line(command, sizeof(buffer)) < 1) continue;
        // Programm aus Programmflash starten
        if (strncmp_P(command, PSTR("run "), 4) == 0) {
			command += 4;
			if(command[0] == '\0') continue;
			//load_from_flash(program, atoi(command));
			usart_write_str("\r\n");
			time_all=0;
			ubasic_init((const char*)(pgm_read_word(&(progs[atoi(command)]))));
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
        // ein paar Infos ueber den RAM
        if (strcmp_P(command, PSTR("mem")) == 0) {
			usart_write("\r\nunused mem: %i\r\n", get_mem_unused());
			usart_write("free mem..: %i\r\n", get_mem_free());
        } else
        // Infos ueber Laufzeit des Basic-Programmes
        if (strcmp_P(command, PSTR("time")) == 0) {
			usart_write("\r\nruntime: %ims\r\n", time_all);
        } else
        // Basic-Programm aus Programmflash auflisten
        if (strncmp_P(command, PSTR("list "), 5) == 0) {
			usart_write_str("\r\n");
			command += 5;
			p = (const char*)(pgm_read_word(&(progs[atoi(command)])));
			while (pgm_read_byte(p)) {
				if (pgm_read_byte(p) == '\n') usart_write_char('\r');
				usart_write("%c", pgm_read_byte(p));
				p++;
			}
        } else 	usart_write("\r\nKommando >%s< unbekannt bzw. nicht vollstaendig!\r\n", command);
        usart_write("\r\n>");
    }
		
return(0);
}

