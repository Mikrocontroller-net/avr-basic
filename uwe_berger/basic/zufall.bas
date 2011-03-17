10  b=20
20  c=2000
30  dim a(b)
40  srand
50  for i=1 to c
60  z=rand(b-1)
70  a(z)=a(z)+1
80  next i
90  for i=0 to b-1
100 print a(i),
110 next i
120 print ""
130 end
