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

#if ACCESS_VIA_SDCARD
	#include "sd_card/fat.h"
	
	static unsigned char c;
	static unsigned char eof;
	struct fat_file_struct* fd;

	//------------------------------------------
	char get_content(void) {
		return c;
	}
	//------------------------------------------
	void set_ptr(PTR_TYPE offset) {
		ptr = offset;
		fat_seek_file(fd, &offset, FAT_SEEK_SET);
		if (fat_read_file(fd, &c, 1)<=0) eof=1; else eof=0;
	}
	//------------------------------------------
	void incr_ptr(void) {
		ptr++;
		if (fat_read_file(fd, &c, 1)<=0) eof=1; else eof=0;
	}
	//------------------------------------------
	char is_eof(void) {
		//if (eof) return 1; else return 0;
		return eof;
	}
#endif

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
		c=(char)fgetc(f);
	}
	//------------------------------------------
	void incr_ptr(void) {
		ptr++;
		c=(char)fgetc(f);
	}
	//------------------------------------------
	char is_eof(void) {
		return (char)feof(f);	
	}
#endif
