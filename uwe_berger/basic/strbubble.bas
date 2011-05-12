b=100
rem ***Zufallsbuchstaben erzeugen***
dim a$(b)
srand
for i=0 to b-1
	a$(i)=chr$(rand(25)+65)
	next i
gosub 100

rem ***Bubblesort***
for i=0 to b-1
	for j=0 to b-1
		if a$(j) > a$(i) gosub 200
	next j
next i

print 
print "******"
gosub 100
print
end

100:
rem ***Ausgabe***
for i=0 to b-1
print a$(i);
next i
return

200:
rem ***Swapping***
t$=a$(i)
a$(i)=a$(j)
a$(j)=t$
return

