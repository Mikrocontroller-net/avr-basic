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
#include "avr_basic/ubasic_config.h"
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

const char prog29[] PROGMEM=
"\
10 dim a(10)\n\
20 for i=0 to 9\n\
30 a(i) = i\n\
40 next i\n\
50 for i=0 to 9\n\
60 print a(i)\n\
70 next i\n\
80 end\n\
";

const char prog30[] PROGMEM=
"\
10 dir(\"b\",5)=1\n\
20 out(\"b\",5)=1\n\
25 print \"An...\"\n\
30 wait 1000\n\
40 out(\"b\",5)=0\n\
45 print \"Aus...\"\n\
50 wait 1000\n\
60 goto 20\n\
70 end\n\
";

const char prog31[] PROGMEM=
"\
10  b=10\n\
20  c=200\n\
30  dim a(b)\n\
40  for i=0 to b-1\n\
50  a(i)=0\n\
60  next i\n\
70  srand\n\
80  for i=1 to c\n\
90  z=rand(b-1)\n\
100  a(z)=a(z)+1\n\
110 next i\n\
120 for i=0 to b-1\n\
130 print a(i),\n\
140 next i\n\
150 print \n\
160 end\n\
";

const char prog32[] PROGMEM=
"\
10   b=20\n\
20   rem ***Zufallszahen erzeugen***\n\
30   dim a(b)\n\
40   srand\n\
50   for i=0 to b-1\n\
60   a(i)=rand(100)\n\
80   next i\n\
90   gosub 1000\n\
100  print\n\
110  print \"**********\"\n\
120  rem ***Bubblesort***\n\
130  for i=0 to b-1\n\
140  for j=0 to b-1\n\
150  if a(j) > a(i) gosub 1100\n\
160  next j\n\
170  next i\n\
180  gosub 1000\n\
190  print\n\
200  end\n\
1000 rem ***Array ausgeben***\n\
1010 for i=0 to b-1\n\
1020 print a(i),\n\
1030 next i\n\
1040 return\n\
1100 rem ***Swapping***\n\
1110 t=a(i)\n\
1120 a(i)=a(j)\n\
1130 a(j)=t\n\
1140 return\n\
";

const char prog33[] PROGMEM=
"\
print \"Hallo Uwe\"\n\
for i=1 to 10\n\
print i\n\
next i\n\
gosub 20\n\
goto 10\n\
print \"hier nicht!\"\n\
10: rem ...\n\
print \"hier ist korrekt...\"\n\
end\n\
20: rem UP...\n\
print \"Unterprogramm\"\n\
return\n\
";

const char prog34[] PROGMEM=
"\
for a=0 to 32\n\
 	gosub 100\n\
 	print \" \",\n\
 	gosub 200\n\
 	print\n\
next a\n\
end\n\
rem ***** Ende *****\n\
rem ***** Unterprogramme *****\n\
100:\n\
dim b(32)\n\
i=-1\n\
t=a\n\
print \"0b\";\n\
10: i=i+1\n\
	b(i) = t mod 2\n\
	t=t/2\n\
	if t>0 goto 10\n\
for t=i downto 0\n\
	print b(t);\n\
next t\n\
return\n\
200:\n\
dim b(32)\n\
i=-1\n\
t=a\n\
print \"0x\";\n\
20: i=i+1\n\
	b(i) = t mod 16\n\
	t=t/16\n\
	if t>0 goto 20\n\
for t=i downto 0\n\
	if b(t) = 0  print \"0\";\n\
	if b(t) = 1  print \"1\";\n\
	if b(t) = 2  print \"2\";\n\
	if b(t) = 3  print \"3\";\n\
	if b(t) = 4  print \"4\";\n\
	if b(t) = 5  print \"5\";\n\
	if b(t) = 6  print \"6\";\n\
	if b(t) = 7  print \"7\";\n\
	if b(t) = 8  print \"8\";\n\
	if b(t) = 9  print \"9\";\n\
	if b(t) = 10 print \"A\";\n\
	if b(t) = 11 print \"B\";\n\
	if b(t) = 12 print \"C\";\n\
	if b(t) = 13 print \"D\";\n\
	if b(t) = 14 print \"E\";\n\
	if b(t) = 15 print \"F\";\n\
next t\n\
return\n\
";

const char prog35[] PROGMEM=
"\
input \"zwei Eingabe: \"; a, b\n\
print\n\
print\n\
print a, b\n\
end\n\
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
	{"ablink", prog30},
	{"zufall", prog31},
	{"bubble", prog32},
	{"no_ln",  prog33},
	{"hexbin", prog34},
	{"input",  prog35},
	{"extgo",  prog27},
	{"up1",    prog28},
	{"array",  prog29}
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
	return progs[i].name;
}
