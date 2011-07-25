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
#define VOID_FUNC_2INT		3
#define VOID_FUNC_4INT		4
#define VOID_FUNC_2INT_CHAR	5
#define UCHAR_FUNC_UCHAR	6

// Strukturdefinition fuer Funktionspointertabelle
typedef struct {
#if USE_PROGMEM
	  char funct_name[MAX_NAME_LEN+1];
#else	
      char *funct_name;
#endif

      union ftp {
        void (*VoidFuncVoid)	(void);
        void (*VoidFuncInt)		(int);
        void (*VoidFunc2Int)	(int, int);
        void (*VoidFunc4Int)	(int, int, int, int);
        void (*VoidFunc2IntChar)(int, int, char*);
        int  (*IntFuncInt)		(int);
        unsigned char  (*UCharFuncUChar)		(unsigned char);
      } funct_ptr;
      unsigned char typ;
} callfunct_t;


// exportierbare Prototypen
int call_statement(void);

#endif /* __UBASIC_CALL_H__ */
