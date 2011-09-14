/*----------------------------------------------------------------------------
*             Testprogramm Zugriff auf ext. Dataflash 
*             =======================================
*               Uwe Berger (bergeruw@gmx.net); 2011
* 
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

#include "df/spi.h"
#include "df/df.h"
#include "df/fs.h"
#include "df/common.h"

//Taktfrequenz
#ifndef F_CPU
#define F_CPU 20000000UL
#endif

//Baudrate der seriellen Schnittstelle
#define BAUDRATE 57600

extern fs_t fs;
extern fs_inode_t prog_inode;

#define BUFF_LEN 32


#define PRELOAD_T0 178

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
	char p_name[MAX_PROG_NAME_LEN];
	const char *p;

#if UBASIC_EXT_PROC
	extern char current_proc[MAX_PROG_NAME_LEN];
#endif
	
    usart_init(BAUDRATE); // setup the UART
    
    // Timer 0 initialisieren (runtime-Messung)
    TCNT0  = PRELOAD_T0;							// Preloadwert Timer0 setzen
	TCCR0B |= (1 << CS02);							// Prescaler Timer0 auf 256 stellen
	TIMSK0 |= (1 << TOIE0);
	sei();
    
    
	usart_write("AVR-Basic-Computer; Uwe Berger; 2010, 2011\r\n");
	usart_write("compiled "__DATE__", "__TIME__"\r\n");
	usart_write("\r\n");

	usart_write("initializing SPI... \r\n");
	spi_init();

    usart_write("initializing filesystem...\r\n");
    fs_init();
    //usart_write("fs: root page is 0x%x\r\n", fs.root);
    //usart_write("done\r\n");
	//usart_write("df status: 0x%x\r\n", df_status(NULL));
    usart_write("\r\n>");

	// Hauptschleife    
	while(1){
		char* command = buffer;
        if(usart_read_line(command, sizeof(buffer)) < 1) continue;
        
        // Speicherinformationen (SRAM)
        if (strcmp_P(command, PSTR("m")) == 0) {
			usart_write("\r\nunused mem: %i\r\n", get_mem_unused());
			usart_write("free mem..: %i\r\n", get_mem_free());
		} else
				
        // Filesystem formatieren...!
        if (strcmp_P(command, PSTR("f")) == 0) {
			usart_write("fs format...\n\t");
			fs_format(&fs);
			//fs_init(&fs, NULL);
			fs_init();
		} else

		// BASIC-Dateien auf externen Dataflash erzeugen
        if (strcmp_P(command, PSTR("n")) == 0) {
			for (uint8_t i=0; i<get_program_pgm_count(); i++) {
				strcpy_P(p_name, get_program_pgm_name(i));
				usart_write("\r\n%s", p_name);
				if (fs_create(&fs, p_name) == FS_OK) {
					p=(const char*)get_program_pgm_ptr(i);
					fs_inode_t inode = fs_get_inode(&fs, p_name);
					fs_size_t offset = 0;
					uint8_t *data = malloc(BUFF_LEN);
					uint16_t idx=0;
					while (pgm_read_byte(p)) {
						idx++;
						data[idx-1]=pgm_read_byte(p);
						if (idx==BUFF_LEN) {
							fs_write(&fs, inode, data, offset, idx);
							offset=offset+idx;
							idx=0;
						}
						p++;
					}
					//... den letzten Rest schreiben
					if (idx) fs_write(&fs, inode, data, offset, idx);
					free(data);
					usart_write(" --> created!");
				} else {
					usart_write(" --> error!");	
				}
			}
		} else

		// Dateiinhalt ausgeben (von externen Dataflash)
		if (strncmp_P(command, PSTR("c "), 2) == 0) {
			usart_write("\r\n");
			command += 2;
			fs_inode_t inode = fs_get_inode(&fs, command);
			if (inode == 0xffff) {
				usart_write("File %s not found!\r\n", command);			
			} else {
				fs_size_t offset = 0;
				fs_size_t size, idx;
				uint8_t *data = malloc(BUFF_LEN);
				size = fs_read(&fs, inode, data, offset, BUFF_LEN);
				while (size > 0) {
					for (idx=0; idx<size; idx++) {
						usart_write("%c", data[idx]);
						if (data[idx] == '\n') usart_write("\r");
					}
					if (size == BUFF_LEN) {
						offset=offset+size;
						size = fs_read(&fs, inode, data, offset, BUFF_LEN);
					} else {
						size = 0;
					} 
				}
				free(data);
			}
		} else
        
		// BASIC-Programm ausfuehren...
        if (strncmp_P(command, PSTR("r "), 2) == 0) {
			usart_write("\r\n");
			command += 2;
			prog_inode = fs_get_inode(&fs, command);
			if (prog_inode == 0xffff) {
				usart_write("Error opening file %s!\n\r", command);			
			} else {
				#if 0
				SET_PROG_PTR_ABSOLUT(0);
				while (!END_OF_PROG_TEXT) {
					usart_write("%c", GET_CONTENT_PROG_PTR);
					INCR_PROG_PTR;	
				}
				#endif
				
				#if 1
				time_all=0;
				#if UBASIC_EXT_PROC
					strncpy(current_proc, command, MAX_PROG_NAME_LEN);
				#endif
				ubasic_init(0);
				do {
					time_row=0;
					ubasic_run();
					time_all += time_row;
					if (usart_is_receive() && (usart_receive_char() == 0x1b)) {
						usart_write("\r\nInterrupted program!\r\n");
						break;
					}
				} while(!ubasic_finished());
				#endif
			}
		} else
		
		// Runtime ausgeben
        if (strcmp_P(command, PSTR("t")) == 0) {
			usart_write("\r\nruntime: %ims\r\n", time_all);
		} else
        
        // Dateinamen auflisten (von externen Dataflash)
        if (strcmp_P(command, PSTR("l")) == 0) {
			char name[FS_FILENAME+1];
			fs_index_t id = 0;
			while ( fs_list(&fs, NULL, name, id++) == FS_OK) {
				name[FS_FILENAME] = '\0';
				usart_write("%s\n\r", name);
			}
			
        } else 	usart_write("\r\nUnknown command!\r\n");
        // Prompt
        usart_write("\r\n>");
    }

	return(0);
}

