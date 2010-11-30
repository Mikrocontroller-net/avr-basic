/*--------------------------------------------------------
*     Zugriffsroutinen auf Speichermedium
*    =====================================
*     Uwe Berger (bergeruw@gmx.net); 2010
* 
*
* 
* Have fun!
* ---------
*
----------------------------------------------------------*/
#define __TOKENIZER_ACCESS_C__
	#include "tokenizer_access.h"
#undef __TOKENIZER_ACCESS_C__

#if ACCESS_VIA_FILE
	#include <stdio.h>
	static char c;
	FILE *f;
	//------------------------------------------
	char get_content(void) {
		return c;
	}
	//------------------------------------------
	void set_ptr(long offset) {
		ptr = offset;
		fseek(f, offset, SEEK_SET);
		c=fgetc(f);
	}
	//------------------------------------------
	void incr_ptr(void) {
		ptr++;
		c=fgetc(f);
	}
	//------------------------------------------
	char is_eof(void) {
		return feof(f);	
	}
#endif
