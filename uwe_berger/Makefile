# 
# Makefile for avr_ethernet webserver
#
#

MCU		= atmega168
#MCU		= atmega644p
TARGET_ARCH     = -mmcu=$(MCU)
OBJECTS         = main.o usart.o ubasic.o tokenizer.o ubasic_call.o ubasic_cvars.o mem_check.o
AVRDUDE_PROGRAMMER = usbasp
TARGET	= main

# Uncomment this for Pollin NetIO Board with 16MHz Quarz
#POLLIN_NETIO = 1


CC      = avr-gcc
CFLAGS  = -Os -g -Wall -std=gnu99 -I. 
CFLAGS  += -fno-strict-aliasing
CFLAGS  += -finline-limit=4 -fno-if-conversion -mcall-prologues
CFLAGS  += -funsigned-char  -funsigned-bitfields -fpack-struct -fshort-enums
# -fno-tree-scev-cprop and -fno-split-wide-types since avr-gcc 4.3.0
CFLAGS  += -fno-tree-scev-cprop -fno-split-wide-types 

CFLAGS  += -DF_CPU=16000000UL
ASFLAGS = -Os -g -Wall -I.
LDFLAGS = -g
#LDFLAGS += -Wl,--relax
MODULES = $(OBJECTS)


all:	main.hex

clean:
	rm -f main.hex main.elf *.o *.d 

main.elf: $(MODULES)
	$(LINK.o) -o $@ $(MODULES)

main.hex: main.elf
	avr-objcopy -j .text -j .data -O ihex main.elf main.hex
	avr-size main.hex

disasm: main.elf
	avr-objdump -S main.elf

#flash:	main.hex
#	$(FLASH_CMD) -U flash:w:main.hex
	
flash:
	avrdude -p $(MCU) -c $(AVRDUDE_PROGRAMMER) -U flash:w:$(TARGET).hex	

main.o: main.c
