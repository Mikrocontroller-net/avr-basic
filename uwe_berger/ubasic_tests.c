/*--------------------------------------------------------
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

#include <avr/pgmspace.h>
#include "avr_basic/ubasic_ext_proc.h"
#include "ubasic_tests.h"


const char prog0[] PROGMEM=
"\
10 gosub 100\n\
15 a=30\n\
20 for i = 1 to a step 10\n\
30 print \"i=\", i\n\
40 next i\n\
50 end\n\
100 print \"subroutine\"\n\
110 return\n\
";

const char prog1[] PROGMEM=
"\
5  c=9\n\
10 print \"davor\", c\n\
20 print in(\"c\",0)\n\
30 print \"danach\"\n\
60 end\n\
";

const char prog2[] PROGMEM=
"\
10 epoke(2)=55\n\
25 epoke(3)=11\n\
20 a=epeek(2)\n\
25 print a\n\
26 print epeek(3)+3\n\
30 print \"end\"\n\
40 end\n\
";

const char prog3[] PROGMEM=
"\
10 a=1\n\
20 print a\n\
30 wait 1000\n\
40 a=a+1\n\
50 goto 20\n\
60 end\n\
";

const char prog4[] PROGMEM=
"\
10 dir(\"b\",1)=1\n\
20 a=0\n\
30 out(\"b\",1)=a\n\
40 if a=1 then a=0 else a=1\n\
50 wait 1000\n\
60 goto 30\n\
70 end\n\
";

const char prog5[] PROGMEM=
"\
10 dir(\"b\",1)=1\n\
20 out(\"b\",1)=1\n\
30 wait 1000\n\
40 out(\"b\",1)=0\n\
50 wait 1000\n\
60 goto 20\n\
70 end\n\
";

const char prog6[] PROGMEM=
"\
10 dir(\"b\",1)=1\n\
20 a=0\n\
30 a=a+1\n\
40 if a>1 a=0\n\
50 if (a%2)=1 then out(\"b\",1)=0 else out(\"b\",1)=1\n\
60 print \"a=\";a;\", adc(0)=\";adc(0)\n\
70 wait 1000\n\
80 goto 30\n\
90 end\n\
";

const char prog7[] PROGMEM=
"\
10 print abs(-8)\n\
20 print not(7)\n\
30 end\n\
";

const char prog8[] PROGMEM=
"\
10 srand\n\
20 for v=1 to 200\n\
30 y=rand(4)\n\
40 if y=0 then a=a+1\n\
50 if y=1 then b=b+1\n\
60 if y=2 then c=c+1\n\
70 if y=3 then d=d+1\n\
80 if y=4 then e=e+1\n\
100 next v\n\
110 print a,b,c,d,e\n\
120 end\n\
";

const char prog9[] PROGMEM=
"\
40 if 0=0 then print 0 else print 1\n\
100 print 2\n\
120 end\n\
";

const char prog10[] PROGMEM=
"\
10 call(\"a\")\n\
15 print call(\"c\",0)\n\
20 wait 500\n\
30 call(\"b\", 0)\n\
40 wait 500\n\
50 goto 10\n\
60 end\n\
";

const char prog11[] PROGMEM=
"\
10 for i=1 to 20\n\
20 print i\n\
30 next i\n\
40 end\n\
";

const char prog12[] PROGMEM=
"\
10 for i=0 to 20\n\
20 if (i%2) then print i else print \"i ist gerade...\"\n\
30 next i\n\
40 end\n\
";

const char prog13[] PROGMEM=
"\
10 for i=0 to 20\n\
20 if (i%2) then print i\n\
30 next i\n\
40 end\n\
";

const char prog14[] PROGMEM=
"\
20 if 0=0 then print \"0\"\n\
30 print \"1\"\n\
40 end\n\
";

const char prog15[] PROGMEM=
"\
20 if 0=0 then a=1\n\
40 end\n\
";

const char prog16[] PROGMEM=
"\
10 srand\n\
20 for v=1 to 200\n\
30 y=rand(1)\n\
90 print v\n\
100 next v\n\
120 end\n\
";

const char prog17[] PROGMEM=
"\
1 y=rand(4)\n\
2 print a\n\
3 print b\n\
4 print c\n\
5 print d\n\
6 print e\n\
7 print f\n\
8 print g\n\
9 print h\n\
10 print i\n\
11 print j\n\
12 print k\n\
13 print l\n\
14 print m\n\
15 print n\n\
16 print o\n\
17 print p\n\
18 print q\n\
19 print r\n\
20 print s\n\
21 print t\n\
22 print u\n\
23 print v\n\
24 print w\n\
25 print x\n\
26 print x\n\
27 print z\n\
120 end\n\
";

const char prog18[] PROGMEM=
"\
10 print \"vor Kommentar...\"\n\
20 rem DAS ist EIN kommentar\n\
30 print \"...nach Kommentar...\"\n\
40 end\n\
";

const char prog19[] PROGMEM=
"\
10 a=123\n\
30 print A\n\
40 end\n\
";

const char prog20[] PROGMEM=
"\
10 print vpeek(\"a\")\n\
20 vpoke(\"a\")=321\n\
30 print vpeek(\"a\")\n\
40 end\n\
";

const char prog21[] PROGMEM=
"\
10 a=10\n\
20 if a >= 10 then print \"a .GE. 10\"\n\
30 if a <= 10 then print \"a .LE. 10\"\n\
40 if a <> 10 then print \"a .NE. 10\" else print \"...else-Zweig...\"\n\
50 end\n\
";

const char prog22[] PROGMEM=
"\
10 print (10 mod 3)\n\
11 print 10 | 3\n\
12 print 10 or 3\n\
13 print 10 & 3\n\
14 print 10 and 3\n\
20 print 15 xor 1\n\
30 print 1 shl 2\n\
40 print 4 shr 2\n\
50 end\n\
";


const char prog23[] PROGMEM=
"\
10 gosub 100\n\
20 for i = 1 to 10\n\
30 print i\n\
40 next i\n\
50 end\n\
100 print \"sub\"\n\
110 return\n\
";

const char prog24[] PROGMEM=
"\
10 a=1\n\
20 if a=1 then gosub 40 else print \"-->else\"\n\
25 print \"-->main\"\n\
30 end\n\
40 print \"-->sub\"\n\
50 return\n\
";

const char prog25[] PROGMEM=
"\
10 a=1\n\
20 if a=1 then gosub 40\n\
25 print \"-->main\"\n\
30 end\n\
40 print \"-->sub\"\n\
50 return\n\
";

const char prog26[] PROGMEM=
"\
10 for i = 1 to 100\n\
20 print i\n\
30 gosub 100\n\
40 next i\n\
50 print a\n\
60 end\n\
100 a=a+1\n\
110 return\n\
";

const char prog27[] PROGMEM=
"\
10 print \"Hauptprogramm\"\n\
20 a=42\n\
30 gosub \"up1\"\n\
40 print \"wieder Hauptprogramm\"\n\
50 print a\n\
60 end\n\
";

const char prog28[] PROGMEM=
"\
10 print \"Unterprogramm\"\n\
30 print a\n\
40 a=4711\n\
50 return\n\
";


static const struct progs_t progs[] PROGMEM = {
	{"prog0",  prog0 },
	{"prog1",  prog1 },
	{"prog2",  prog2 },
	{"prog3",  prog3 },
	{"prog4",  prog4 },
	{"prog5",  prog5 },
	{"prog6",  prog6 },
	{"prog7",  prog7 },
	{"prog8",  prog8 },
	{"prog9",  prog9 },
	{"prog10", prog10},
	{"prog11", prog11},
	{"prog12", prog12},
	{"prog13", prog13},
	{"prog14", prog14},
	{"prog15", prog15},
	{"prog16", prog16},
	{"prog17", prog17},
	{"prog18", prog18},
	{"prog19", prog19},
	{"prog20", prog20},
	{"prog21", prog21},
	{"prog22", prog22},
	{"prog23", prog23},
	{"prog24", prog24},
	{"prog25", prog25},
	{"prog26", prog26},
	{"extgo",  prog27},
	{"up1",    prog28},
	{"extgo1",  prog27}
};


//************************************************************************
signed char get_program_pgm_idx(char *p_name) {
	unsigned char i=0;
	while (i<sizeof(progs)/sizeof(progs[0]) &&
	       strncmp_P(p_name, progs[i].name, MAX_PROG_NAME_LEN)) {
		i++;
	}
	if (i<sizeof(progs)/sizeof(progs[0])) return i; else return -1;
}

//************************************************************************
const char* get_program_pgm_ptr(unsigned char i) {
	return (const char*)(pgm_read_word(&(progs[i].prog)));
}

//************************************************************************
signed char get_program_pgm_count(void) {
	return (sizeof(progs)/sizeof(progs[0]));
}

//************************************************************************
const char* get_program_pgm_name(unsigned char i) {
	//strcpy_P(p_name, progs[i].name);
	return progs[i].name;
}