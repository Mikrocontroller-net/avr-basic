
10   b=100
20   rem ***Zufallszahen erzeugen***
30   dim a(b)
40   srand

50   for i=0 to b-1
60   a(i)=rand(100)
80   next i
90   gosub 1000
100  print 
110  print "**********"
120  rem ***Bubblesort***
130  for i=0 to b-1
140  for j=0 to b-1
150  if a(j) > a(i) gosub 1100
160  next j
170  next i
180  gosub 1000
190  print
200  end

1000 rem ***Array ausgeben***
1010 for i=0 to b-1
1020 print a(i),
1030 next i
1040 return

1100 rem ***Swapping***
1110 t=a(i)
1120 a(i)=a(j)
1130 a(j)=t
1140 return

