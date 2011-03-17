/*--------------------------------------------------------
*                Deklarationen
*  Basic-Testprogramme & entspr. Zugriffsroutinen 
*               fuer AVR-PROGMEM
*  ==============================================
*         Uwe Berger (bergeruw@gmx.net); 2010
* 
*
* 
* ---------
* Have fun!
*
----------------------------------------------------------*/

#ifndef __UBASIC_TESTS_H__
#define __UBASIC_TESTS_H__

struct progs_t {
	char name[MAX_PROG_NAME_LEN];
	const char *prog;
};

signed char get_program_pgm_idx(char *);
const char* get_program_pgm_ptr(unsigned char);
signed char get_program_pgm_count(void);
const char* get_program_pgm_name(unsigned char);


#endif /* __UBASIC_TESTS_H__ */
