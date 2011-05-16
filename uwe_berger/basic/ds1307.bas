rem ================================
rem Programm gibt Datum/Uhrzeit
rem eines RTC DS1307 aus
rem (Register DS1307 siehe ds1307.c)
rem ================================

print "Datum..: ";
a=call("get_rtc", 4)
gosub 10
print ".";
a=call("get_rtc", 5)
gosub 10
print ".";
a=call("get_rtc", 6)
gosub 10
print

print "Uhrzeit: ";
a=call("get_rtc", 2)
gosub 10
print ".";
a=call("get_rtc", 1)
gosub 10
print ".";
a=call("get_rtc", 0)
gosub 10
print
end

10:
rem --------------------------------
rem UP zur Ausgabe von Zahlen
rem (auszugebene Zahl in Variable a)
rem --------------------------------
if a < 10 print "0";
print a;
return
