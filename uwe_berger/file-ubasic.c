/*----------------------------------------------------------------------------
*             Testprogramm fuer ubasic_avr-Interpreter 
*             ========================================
*               Uwe Berger (bergeruw@gmx.net); 2010
* 
* ==>> Basic-Programmtext steht in einer Datei <<==
*      ---------------------------------------
* 
* Basic-Programm wird direkt via Filesystemzugriffe gelesen.
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


/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
/*---------------------------------------------------------------------------*/
int main(int argc, char* argv[]) {

	// ist in tokenizer_access.c deklariert...
	extern FILE *f;

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

	// Basic-Programm interpretieren
	ubasic_init(0);
	do {
		ubasic_run();
	} while(!ubasic_finished());

	fclose(f);
	exit(0);
}
