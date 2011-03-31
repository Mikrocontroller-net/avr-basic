 b=20
 c=2000
 dim a(b)
 srand
 for i=1 to c
 z=rand(b-1)
 a(z)=a(z)+1
 next i
 for i=0 to b-1
 print a(i),
 next i
 print ""
 end



