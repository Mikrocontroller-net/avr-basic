print "swapping via stack"
x=333
y=444
print x, y
push x, y
pop x, y
print x, y
print "push mit expressions"
push x+12, 1234
pop a, b
print a, b 
print "feldelement auf stack"
dim a(2)
a(0)=123
a(1)=42
print a(0), a(1)
push a(0)
push a(1)
pop a(0)
pop a(1)
print a(0), a(1)
print "datenbackup/-restore vor gosup"
c=42
gosub 1000
print c
end
rem *******UPs**********
1000 rem UP
push c
c=23
print 23
pop c
return

