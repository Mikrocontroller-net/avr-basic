/*
 * 
 * http://www.rn-wissen.de/index.php/Speicherverbrauch_bestimmen_mit_avr-gcc
 * 
 * 
 */


#ifndef _MEM_CHECK_H_
#define _MEM_CHECK_H_

#ifdef AVR
	extern unsigned short get_mem_unused (void);
	extern unsigned int get_mem_free (void);
#endif

#endif  /* _MEM_CHECK_H_ */
