rem Beispiel fuer Zahlenausgabe in 
rem Hex- und Binaerdarstellung

for a=0 to 32
 	gosub 100
 	print " ",
 	gosub 200
 	print
next a

end
rem ***** Ende *****

rem ***** Unterprogramme *****
100:
rem *** Zahl binaer ausgeben ***
rem Zahl steht in a
rem b wird als Array definiert und
rem manipuliert; t, i werden ebenfalls
rem veraendert
dim b(32)
i=-1
t=a
print "0b";

10: i=i+1 
	b(i) = t mod 2
	t=t/2
	if t>0 goto 10

for t=i downto 0
	print b(t);
next t
return

200:
rem *** Zahl hexadezimal ausgeben ***
dim b(32)
i=-1
t=a
print "0x";

20: i=i+1 
	b(i) = t mod 16
	t=t/16
	if t>0 goto 20

for t=i downto 0
	if b(t) = 0  print "0";
	if b(t) = 1  print "1";
	if b(t) = 2  print "2";
	if b(t) = 3  print "3";
	if b(t) = 4  print "4";
	if b(t) = 5  print "5";
	if b(t) = 6  print "6";
	if b(t) = 7  print "7";
	if b(t) = 8  print "8";
	if b(t) = 9  print "9";
	if b(t) = 10 print "A";
	if b(t) = 11 print "B";
	if b(t) = 12 print "C";
	if b(t) = 13 print "D";
	if b(t) = 14 print "E";
	if b(t) = 15 print "F";
next t
return
