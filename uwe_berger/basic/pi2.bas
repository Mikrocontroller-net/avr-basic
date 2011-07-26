0 rem digits --> z
1 rem desiredprecision --> y
2 rem olda1 --> x
3 rem oldb1 --> w
4 rem a1 --> e
5 rem b1 --> f
6 rem d1 --> g
7 rem
10 k = 2
20 a = 4
30 b = 1
40 e = 12
50 f = 4
60 z = 0
70 y = 200
80 rem while z<y
81 if z >= y goto 290
90  p = k*k
100 q = (2*k)+1
110 k = k+1
120 x = e
130 w = f
140 e = (p*a)+(q*e)
150 f = (p*b)+(q*f)
160 a = x
170 b = w
180 d = a / b
190 g = e / f
200 rem while d = g
201 if d <> g goto 280
202 print d, g
210 print d
220 z = z + 1
230 if (z % 50)=0 then print
240 a = 10*(a % b)
250 e = 10*(e % f)
260 d = a/b
270 g = e/f
280 rem wend 200
281 if d=g goto 200
290 rem wend 80
291 if z<y goto 80
300 end
