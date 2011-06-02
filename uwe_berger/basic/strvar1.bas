rem print "Uwe " + "Berger" + " BASIC " + left$("ABCDEF", 3)
rem input "ein Text: "; a$
rem print a$
rem a$ = ("Uwe" + "Berger")
rem print a$
rem print ("Uwe" + "Berger")

rem if 2 >= 1 then print "then-Zweig" else print "else-Zweig"
rem print "****"
rem if "aaaaaa" = "aaaaaa" then print "then-Zweig" else print "else-Zweig"

print ""
print m_strlen
a$=""
for i=1 to m_strlen
	if (i%10)=0 then b$ = "|" else b$ = "*"
	a$=a$+b$
next i
print a$

print upper$("basic")
print lower$("BASIC")
print upper$(left$("Uwe Berger", 3))
a$ = lower$("COOL")
print a$
print upper$("1234abcd5678")
print lower$("1234ZYXV5678")
print instr$("BASIC", "SI")


end

