
print "Hallo Uwe"

for i=1 to 10
	print i
next i

gosub 20

goto 10

print "hier nicht!"

10:
print "hier ist korrekt..."
end

20:
print "Unterprogramm"
return
