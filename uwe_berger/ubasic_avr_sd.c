/*--------------------------------------------------------
*      AVR-Basic-Interpreter mit SD-Karte
*    =====================================
*     Uwe Berger (bergeruw@gmx.net); 2010
* 
* Basic-Programme liegen auf einer SD-Karte. Fuer die SD-
* Kartenzugriffe wurde folgende Lib verwendet:
*
* http://www.roland-riegel.de/sd-reader/
* 
*
* Die "Shell-Befehle" wurden moeglichst einfach gehalten:
*
* m             : Speicherinformationen (SRAM)
* s             : einige Statusinformationen zur SD-Karte
* d             : Dateien auf SD-Karte auflisten
* l <Dateiname> : Programmlisting
* r <Dateiname> : Programm starten
* t             : Runtime ausgeben
*
* Die Dateien muessen im Root-Verzeichnis der SD-Karte liegen.
* 
* Ein-/Ausgaben erfolgen ueber die serielle Schnittstelle (57600, 8, n, 1)
*
*
*
* ---------
* Have fun!
*
----------------------------------------------------------*/

#include <stdlib.h>
#include <avr/io.h>

#include "sd_card/fat.h"
#include "sd_card/my_fat.h"
#include "sd_card/partition.h"
#include "sd_card/sd_raw.h"

#include "mem_check/mem_check.h"

#include "uart/usart.h"
#include "avr_basic/ubasic_config.h"
#include "avr_basic/tokenizer_access.h"
#include "avr_basic/ubasic_ext_proc.h"
#include "avr_basic/ubasic.h"

//Taktfrequenz
#ifndef F_CPU
#define F_CPU 16000000UL
#endif

//Baudrate der seriellen Schnittstelle
#define BAUDRATE 57600

#define PRELOAD_T0 6

volatile uint16_t time_row;
uint16_t time_all;

struct fat_fs_struct* fs;
struct fat_dir_struct* dd;


//************************************************************************
ISR(TIMER0_OVF_vect)
{
	TCNT0 = PRELOAD_T0;						// Preloadwert setzen
	time_row++;
}


//*****************************************************************************************************************
int main(void){
	char buffer[20];
	extern struct fat_file_struct* fd;

#if UBASIC_EXT_PROC
	extern char current_proc[MAX_PROG_NAME_LEN];
#endif


    usart_init(BAUDRATE);

    // Timer 0 initialisieren (runtime-Messung)
    TCNT0  = PRELOAD_T0;							// Preloadwert Timer0 setzen
	TCCR0B |= (1 << CS01)|(1 << CS00);				// Prescaler Timer0 auf 64 stellen
	TIMSK0 |= (1 << TOIE0);
	sei();
	
	usart_write("AVR-Basic-Computer; Uwe Berger, 2010\r\n");
	usart_write("Compiliert am "__DATE__" um "__TIME__"\r\n");

	/* setup sd card slot */
	while (!sd_raw_init()){
		usart_write("MMC/SD initialization failed\n\r");
	}	
	usart_write("--> SD-Card ok...\r\n");

	/* open first partition (read only) */
	struct partition_struct* partition; 
	partition = partition_open(sd_raw_read, sd_raw_read_interval, 0, 0, 0);
	while(!partition) {
		/* If the partition did not open, assume the storage device
		 * is a "superfloppy", i.e. has no MBR.
		 */
		partition = partition_open(sd_raw_read, sd_raw_read_interval, 0, 0, -1);
		if(!partition) {
                usart_write("opening partition failed\n\r");
		}
	}
	usart_write("--> Partition ok...\r\n");

	/* open file system */
	fs = fat_open(partition);
	while(!fs) {
		usart_write("opening filesystem failed\n\r");
	}
	usart_write("--> FAT ok...\r\n");

	/* open root directory */
	struct fat_dir_entry_struct directory;
	fat_get_dir_entry_of_path(fs, "/", &directory);
	dd = fat_open_dir(fs, &directory);
	while(!dd) {
		usart_write("opening root directory failed\n\r");
	}
	usart_write("--> Root-directory ok...\r\n");

	usart_write("\r\nunused mem: %i\r\n", get_mem_unused());
	usart_write("free mem..: %i\r\n", get_mem_free());
    usart_write("\r\n>");


	while(1){
		char* command = buffer;
        if(usart_read_line(command, sizeof(buffer)) < 1) continue;
        // Speicherinformationen (SRAM)
        if (strcmp_P(command, PSTR("m")) == 0) {
			usart_write("\r\nunused mem: %i\r\n", get_mem_unused());
			usart_write("free mem..: %i\r\n", get_mem_free());
		} else
        // SD-Karten-Informationen
        if (strcmp_P(command, PSTR("s")) == 0) {
		    struct sd_raw_info disk_info;
			sd_raw_get_info(&disk_info);
			usart_write("manuf:  0x%x\n\r", disk_info.manufacturer);
			usart_write("oem:    %s\n\r", (char*) disk_info.oem);
			usart_write("prod:   %s\n\r", (char*) disk_info.product);
			usart_write("rev:    %x\n\r", disk_info.revision);
			usart_write("serial: 0x%x \n\r", disk_info.serial);
			usart_write("date:   %i/%i\n\r", disk_info.manufacturing_month, disk_info.manufacturing_year);
			usart_write("size:   %i MB\n\r", disk_info.capacity / 1024 / 1024);
			usart_write("format: %i\n\r", disk_info.format);
		} else
		// Root-Verzeichnisinhalt auflisten
		if (strcmp_P(command, PSTR("d")) == 0) {
			struct fat_dir_entry_struct dir_entry;
			while(fat_read_dir(dd, &dir_entry)) {
				if ((dir_entry.attributes != FAT_ATTRIB_DIR) && (dir_entry.attributes != FAT_ATTRIB_VOLUME)) {
					usart_write("%s\n\r", dir_entry.long_name);
				}
			}
		} else
		// Runtime ausgeben
        if (strcmp_P(command, PSTR("t")) == 0) {
			usart_write("\r\nruntime: %ims\r\n", time_all);
		} else
        // Basic-Programm auflisten
        if (strncmp_P(command, PSTR("l "), 2) == 0) {
			usart_write("\r\n");
			command += 2;
			/* search file in current directory and open it */
			fd = open_file_in_dir(fs, dd, command);
			if(fd) {
                uint8_t c=0;
                while(fat_read_file(fd, &c, 1) > 0) {
					if (c=='\n') usart_write_char('\r');
					usart_write_char(c);
				}
				fat_close_file(fd);
			} else {
                usart_write("error opening %s\n\r ", command);
			}
		} else
        // Basic-Programm ausfuehren
        if (strncmp_P(command, PSTR("r "), 2) == 0) {
			usart_write("\r\n");
			command += 2;
			/* search file in current directory and open it */
			fd = open_file_in_dir(fs, dd, command);
			if(fd) {
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
						usart_write("\r\nProgramm unterbrochen...!\r\n");
						break;
					}
					//usart_write("--> in while run...\n\r");
				} while(!ubasic_finished());
				//usart_write("--> hinter finish...\n\r");
				fat_close_file(fd);
			} else {
                usart_write("error opening %s\n\r ", command);
			}
        } else 	usart_write("\r\nKommando %s unbekannt!\r\n", command);
        // Prompt
        usart_write("\r\n>");
    }
	return 0;
}
