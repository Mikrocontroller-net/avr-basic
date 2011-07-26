b$ = "Uwe ist auch cool!"
print b$
dim a$(10)
a$(0) = "Hallo..."
a$(1) = left$(b$, 3)
print left$(a$(0), 5), a$(1)
print a$(0) + a$(1)
end

