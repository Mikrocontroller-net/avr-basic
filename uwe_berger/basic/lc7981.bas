10  call("clear")
20  call("line", 10, 10, 149, 10)
30  call("line", 10, 10, 10, 69)
40  call("line", 10, 69, 149, 69)
50  call("line", 149, 10, 149, 69)
60  call("puts", 12, 12, "AVR-Basic")
70  call("puts", 12, 24, "mit LC7981")
80  call("puts", 12, 36, "Pixel gehen auch")
90  call("pset", 120, 40)
100 call("pset", 123, 40)
110 call("puts", 12, 48, "Pixel/Rect-Loeschen")
120 call("pclear", 10, 69)
130 call("rclear", 140, 60, 15, 15)
200 end
