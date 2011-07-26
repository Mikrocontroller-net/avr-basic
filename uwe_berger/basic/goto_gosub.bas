10   print "---> Anfang"
20   A=10
30   goto 100 + A
100  print "Zeile 100"
110  print "Zeile 110"
120  gosub 1000 + A
190  print "---> Ende"
200  end
1000 print "UP in Zeile 1000"
1010 print "UP in Zeile 1010"
1020 return
