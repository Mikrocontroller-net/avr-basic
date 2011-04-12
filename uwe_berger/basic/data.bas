dim a(10)
read a(0), a(1), a(3)
gosub 100
data 42, 23, 4711
end

100:
for i=0 to 9
	print a(i)
next i
return
