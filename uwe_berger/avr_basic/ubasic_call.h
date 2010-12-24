/*--------------------------------------------------------
*    Implementierung Basic-Befehl "call()"
*    =====================================
*     Uwe Berger (bergeruw@gmx.net); 2010
* 
* Dokumentation call_referenz.txt...!
*
* 
* Have fun!
* ---------
*
----------------------------------------------------------*/
#ifndef __UBASIC_CALL_H__
#define __UBASIC_CALL_H__

// Funktionspointertypen
#define VOID_FUNC_VOID		0
#define VOID_FUNC_INT		1
#define INT_FUNC_INT		2

// Strukturdefinition fuer Funktionspointertabelle
typedef struct {
      char *funct_name;
      union ftp {
        void (*VoidFuncVoid)(void);
        void (*VoidFuncInt)	(int);
        int  (*IntFuncInt)	(int);
      } funct_ptr;
      unsigned char typ;
} callfunct_t;


// exportierbare Prototypen
int call_statement(void);

#endif /* __UBASIC_CALL_H__ */
