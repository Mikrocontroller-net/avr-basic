/*----------------------------------------------------------------------------
*             Testprogramm fuer ubasic_avr-Interpreter 
*             ========================================
*               Uwe Berger (bergeruw@gmx.net); 2010
* 
* ==>> Basic-Programmtext im RAM <<==
*      -------------------------
* 
* Basic-Programm wird aus Filesystem in den RAM geladen und ausgefuehrt.
* 
* Syntax: ubasic <Dateiname_des_Basic-Programms>
* 
* 
* Have fun!
* ---------
* 
* ----------------------------------------------------------------------------*/

#include "tokenizer_access.h"
#include "ubasic.h"
#include <stdio.h>
#include <stdlib.h>

#define MAX_PROGRAM_LEN	1000
static char program[MAX_PROGRAM_LEN] = "";

/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {

	FILE *f;

	printf("uBasic-Interpreter; Uwe Berger, 2010\r\n");
	printf("Compiliert am "__DATE__" um "__TIME__"\r\n");
  
	// Programm-Parameter behandlen
	if(argc < 2) {
		printf("Parameterfehler!\n");
		printf("Syntax: ubasic <Dateiname_des_Basic-Programms>\n");
		exit(1);
	}

	// Basic-Programm von Datei einlesen
	f = fopen(argv[1], "r");
	if (!f) {
		printf("Fehler beim Oeffnen der Datei: %s\n", argv[1]);
		exit(1);
	}
	fread(program, sizeof(char), sizeof(program), f);
	fclose(f);

	// Basic-Programm interpretieren
	ubasic_init(program);
	do {
		ubasic_run();
	} while(!ubasic_finished());

	exit(0);
}
