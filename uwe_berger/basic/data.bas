dim a(10)
dim b$(1)
read a(0), a(1), a(3), b$(0)
gosub 100
data 42, 23, -4711, "MoinMoin"
end

100:
for i=0 to 9
	print a(i)
next i
print "*****"
print b$(0)
return
