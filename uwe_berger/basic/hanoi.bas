1  REM * The Towers Of Hanoi
2  REM * BASIC
3  REM * Copyright (C) 1998 Amit Singh. All Rights Reserved.
4  REM * http://hanoi.kernelthread.com
5  REM * Tested under Chipmunk BASIC 1.0, Microsoft QBasic 1.1
   
   rem ---------------------------------------------
   rem modifed for AVR-BASIC by Uwe Berger; 2011
   rem ---------------------------------------------
   
6  REM --- S is the stack
10 DIM S(4096)
15 REM --- b is the stack pointer
20 b = 1
25 REM --- _sfTO
30 t = 2
35 REM --- _sfFR
40 f = 0
45 REM --- _sfUS
50 u = 1
55 REM --- _sfNU
60 n = 3
65 REM --- _sfEN
70 e = 0
75 REM --- remain
80 m = 0
82 rem --- movecounter
83 i = 0
85 REM --- the stack variable
90 INPUT "Number of disks (0 < disks < 16)"; v
91 IF v>0 AND v<16 THEN GOTO 100 ELSE GOTO 92
92 PRINT "Illegal number of disks"
93 GOTO 90
95 REM --- PUSH N (=v here)
100 GOSUB 2000
110 v = 1
120 GOSUB 2000
130 v = 3
140 GOSUB 2000
150 v = 0
160 GOSUB 2000
170 rem WHILE b > 1
171 if b <= 1 goto 580
180 GOSUB 3000
190 e = v
200 GOSUB 3000
210 t = v
220 GOSUB 3000
230 f = v
240 GOSUB 3000
250 n = v
260 m = 6 - f - t
270 IF e <> 0 THEN GOTO 280 ELSE GOTO 380
280 GOSUB 4000
290 v = n - 1
300 GOSUB 2000
310 v = m
320 GOSUB 2000
330 v = t
340 GOSUB 2000
350 v = 0
360 GOSUB 2000
370 GOTO 570
380 IF n <> 1 THEN GOTO 390 ELSE GOTO 560
390 v = n
400 GOSUB 2000
410 v = f
420 GOSUB 2000
430 v = t
440 GOSUB 2000
450 v = 1
460 GOSUB 2000
470 v = n - 1
480 GOSUB 2000
490 v = f
500 GOSUB 2000
510 v = m
520 GOSUB 2000
530 v = 0
540 GOSUB 2000
550 GOTO 570
560 GOSUB 4000
570 rem WEND
571 goto 170
580:
print "------------"
print i, "Move(s)"
END

1999 REM --- procedure push(v)
2000 S(b) = v
2010 b = b + 1
2020 RETURN

2999 REM --- procedure pop()
3000 IF b = 0 THEN GOTO 3030 ELSE GOTO 3010
3010 b = b - 1
3020 v = S(b)
3030 RETURN

3999 REM --- procedure movedisk(from, to)
4000 rem PRINT USING "move # --> #"; f; t;
4001 PRINT "move #"; f, "--> #"; t
4010 rem PRINT " "
4015 i = i+1
4020 RETURN

