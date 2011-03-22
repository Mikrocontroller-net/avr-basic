10  REM Program to calculate up to 15000 digits of PI 
20  REM Richard Russell, 16th March 2006 
21  REM geht nur auf Plattformen mit 32-Bit-Integer!!!
30  n=400
40  n = (n/4)*14
50  DIM f(n+1) 
60  FOR i=1 TO n 
70  f(i)=2000 
80  NEXT I 
90  rem a=10000 
100 e=0 
110 FOR c=n DOWNTO 14 STEP 14
120 d=0 
130 FOR b=c DOWNTO 1 STEP 1 
140 d=d*b+f(b)*10000 
150 g=b*2-1 
160 f(b)=d%g 
170 d=d/g 
180 NEXT b 
190 z=e+(d/10000)
200 IF z<1000 PRINT 0;
210 IF z<100  PRINT 0;
220 IF z<10   PRINT 0;
230 PRINT z;
240 e=d%10000
250 NEXT c
260 PRINT
270 END 
