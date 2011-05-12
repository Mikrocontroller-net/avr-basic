rem Beispiel fuer Zahlenausgabe in 
rem Hex- und Binaerdarstellung

for a=0 to 32
 	gosub 100
 	print "    ", chr$(9),
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
z$="0123456789ABCDEF"
i=-1
t=a
print "0x";

20: i=i+1 
	b(i) = t mod 16
	t=t/16
	if t>0 goto 20

for t=i downto 0
	print mid$(z$, b(t), 1);
next t
return
