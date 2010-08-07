

const char prog0[] FLASHMEM=
"\
10 for i = 0 to 31\n\
20 print \"hallo\" , i, i xor 5\n\
25 if i >= 16 then print i or 32\n\
30 next i\n\
40 end\n\
";

const char prog1[] FLASHMEM=
"\
10 for i = 1 to half 20\n\
20 lcdout i*2, 3*i+2, \"leer\"\n\
30 next i\n\
40 end\n\
";

const char prog2[] FLASHMEM=
"\
10 for i = 0 to 7\n\
20 print adc [i]\n\
30 next i\n\
40 for i = 0 to 7\n\
50 adc [i] = 30 + i\n\
60 next i\n\
70 for i = 0 to 7\n\
80 print adc [i]\n\
90 next i\n\
99 end\n\
";

const char prog3[] FLASHMEM=
"\
10 z=1\n\
15 gosub 100\n\
20 for i = 1 to half 20\n\
30 adc [i - 1] = 40\n\
32 d = adc [i - 1]\n\
33 lcdout 30, d*5, \"test\"\n\
35 print d, systick, half systick, half z,  \"tescht\", z\n\
36 z = z * 2\n\
37 half 45\n\
40 next i\n\
50 print \"end\"\n\
60 end\n\
100 print \"subroutine\"\n\
110 return\n\
";


const char prog4[] FLASHMEM=
"\
10 print i\n\
20 i = i +1\n\
30 if i < 300 then goto 10\n\
40 end\n\
";


const char prog5[] FLASHMEM=
"\
10 gosub 100\n\
20 end\n\
100 for i = 1 to 10\n\
105 print \"piep\"\n\
110 gosub 200\n\
120 next i\n\
130 return\n\
200 for j = 1 to 10\n\
210 print i, j, i * j\n\
220 next j\n\
230 return\n\
";


const char prog6[] FLASHMEM=
"\
10 z = 1\n\
15 z = z * (0 - 1)\n\
20 print z\n\
30 z = z + 1\n\
40 print z\n\
50 z = z + 1\n\
60 print z\n\
70 end\n\
";


const char prog7[] FLASHMEM=
"\
5 for x = 1 to 10\n\
10 for i = 1 to 10\n\
15 for n = 1 to 10\n\
20 gosub 100\n\
30 next n\n\
30 next i\n\
35 next x\n\
40 end\n\
100 print n,i \n\
105 return\n\
";








const char prog_empty[] FLASHMEM=
"\
\
\
\
";

const char *progs[] FLASHMEM = 	{
								prog0, prog1, prog2, prog3, prog4, prog5, prog6, prog7
								};
