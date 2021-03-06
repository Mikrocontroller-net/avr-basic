/*--------------------------------------------------------
*    Plattformabhaengige Zugriffsroutinen fuer externe
*                    Unterprogramme 
*    =================================================
*          Uwe Berger (bergeruw@gmx.net); 2010
* 
* Folgendes ist je nach Zugriffsmethode auf den Basic-Programm-Text 
* anzupassen:
*
* current_proc  --> Definition des aktuellen Programms; diese Variable
*                   muss auch initial fuer das erste Hauptprogramm ge-
*                   setzt werden
* 
* switch_proc() --> - Schliessen des derzeitigen Programms;
*                   - Oeffnen des Programms aus Parameter;
*                   - Setzen von PROG_PTR auf Anfang des Programms;
*                   - tokenizer_init() mit neuen Programm;
*                   - Setzen von current_proc auf Programm aus Parameter
* 
*
* ---------
* Have fun!
*
----------------------------------------------------------*/

#include "ubasic_config.h"

#define __UBASIC_EXT_PROC_C__
	#include "tokenizer_access.h"
#undef __UBASIC_EXT_PROC_C__

#include "ubasic.h"
#include "tokenizer.h"
#include "ubasic_ext_proc.h"



#if USE_AVR
	#include "../uart/usart.h"
#else
	#include <string.h>
	#include <stdio.h> 
#endif


#if UBASIC_EXT_PROC

extern PTR_TYPE program_ptr;

// aktueller Programmname
char current_proc[MAX_PROG_NAME_LEN];

//**********************************************************************
#if ACCESS_VIA_PGM
	#include "../ubasic_tests.h"
	// Umschalten des Programm-Kontextes
	void switch_proc(char *p_name) {
		unsigned char i=0;
		i=get_program_pgm_idx(p_name);	
		//Fehlerpruefung (Programm nicht vorhanden)
		if (i<0) {
			tokenizer_error_print(current_linenum, UNKNOWN_SUBPROC);
			ubasic_break();
		} else {
			PROG_PTR=(const char*)get_program_pgm_ptr(i);
			program_ptr=(const char*)get_program_pgm_ptr(i);
			tokenizer_init(program_ptr);
			strncpy(current_proc, p_name, MAX_PROG_NAME_LEN);
		}
	}
#endif

//**********************************************************************
#if ACCESS_VIA_SDCARD
	#include "sd_card/fat.h"
	#include "sd_card/my_fat.h"
	// Filediscriptor
	extern struct fat_file_struct* fd;
	extern struct fat_fs_struct* fs;
	extern struct fat_dir_struct* dd;
	// Umschalten des Programm-Kontextes
	void switch_proc(char *p_name) {
		// aktuelle Datei schliessen
		fat_close_file(fd);
		// neue Datei oeffnen
		fd = open_file_in_dir(fs, dd, p_name);
		if(fd) {
			// diverse Pointer und Tokenizer initialisieren
			PROG_PTR=0;
			program_ptr=0;
			tokenizer_init(program_ptr);
			strncpy(current_proc, p_name, MAX_PROG_NAME_LEN);
		} else {
			tokenizer_error_print(current_linenum, UNKNOWN_SUBPROC);
			ubasic_break();
		}
	}
#endif

//**********************************************************************
#if ACCESS_VIA_FILE
	// Filediscriptor
	extern FILE *f;
	// Umschalten des Programm-Kontextes
	void switch_proc(char *p_name) {
		fclose(f);
		f = fopen(p_name, "rb");
		if (!f) {
			tokenizer_error_print(current_linenum, UNKNOWN_SUBPROC);
			ubasic_break();
		} else {
			PROG_PTR=0;
			program_ptr=0;
			tokenizer_init(program_ptr);
			strncpy(current_proc, p_name, MAX_PROG_NAME_LEN);
		}
	}
#endif

//**********************************************************************
#if ACCESS_VIA_DF
	#include "df/fs.h"
	#include "tokenizer_access.h"
	extern fs_t fs;
	extern fs_inode_t prog_inode;
	// Umschalten des Programm-Kontextes
	void switch_proc(char *p_name) {
		prog_inode = fs_get_inode(&fs, p_name);
		if (prog_inode == 0xffff) {
			tokenizer_error_print(current_linenum, UNKNOWN_SUBPROC);
			ubasic_break();
		} else {
			destroy_prog_buf();
			create_prog_buf(fs_size(&fs, prog_inode));
			PROG_PTR=0;
			program_ptr=0;
			tokenizer_init(program_ptr);
			strncpy(current_proc, p_name, MAX_PROG_NAME_LEN);
		}
	}
#endif


#endif
