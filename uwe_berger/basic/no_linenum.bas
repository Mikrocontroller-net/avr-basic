
print "Hallo Uwe"

for i=1 to 10
	print i
next i

gosub 20

goto 10

print "hier nicht!"

10: rem ...
print "hier ist korrekt..."
end

20: rem UP...
print "Unterprogramm"
return
