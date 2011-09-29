rem *** Str-Bubble-Sort ***
a$=""
b=m_strlen
srand
for i=0 to b-1
  a$=a$+chr$(65+rand(25))
next i
print a$
gosub 1000
print a$
end
rem **** Unterprogramme ****
1000:
rem ***Bubblesort***
for i=0 to b-1
  for j=0 to b-1
    if mid$(a$,j,1) > mid$(a$,i,1) gosub 2000
  next j
next i
return
2000:
rem *** Swap ***
b$ = a$
if i<j a$=left$(b$,i)+mid$(b$,j,1)+mid$(b$,i+1,j-i-1)+mid$(b$,i,1)+right$(b$,b-j-1)
if j<i a$=left$(b$,j)+mid$(b$,i,1)+mid$(b$,j+1,i-j-1)+mid$(b$,j,1)+right$(b$,b-i-1)
return

