10 REM *** BASIC Version of 99 Bottles of beer ***
20 FOR i=100 DOWNTO 1 
30 PRINT i,"Bottle";
31 if i>1 THEN PRINT "s";
32 PRINT " of beer on the wall,",i,"bottle";
33 if i>1 THEN PRINT "s";
34 PRINT " of beer"
40 PRINT "Take one down and pass it around,",i-1,"bottle";
41 if (i-1)>1 THEN PRINT "s";
42 PRINT " of beer on the wall"
50 PRINT
60 NEXT i
61 PRINT "No more bottles of beer on the wall, no more bottles of beer."
62 PRINT "Go to the store and buy some more, 99 bottles of beer on the wall."
70 END







