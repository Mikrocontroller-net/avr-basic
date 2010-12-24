/*--------------------------------------------------------
*    Implementierung Basic-Befehl "vpeek" und "vpoke"
*    =====================================
*     Uwe Berger (bergeruw@gmx.net); 2010
* 
* 
* Have fun!
* ---------
*
----------------------------------------------------------*/
#ifndef __UBASIC_CVARS_H__
#define __UBASIC_CVARS_H__


// Strukturdefinition fuer Variablenpointertabelle
typedef struct {
	char *var_name;
	int *pvar;
} cvars_t;


// exportierbare Prototypen
void vpoke_statement(void);		//setzen
int vpeek_expression(void);		//lesen

#endif /* __UBASIC_CVARS_H__ */
