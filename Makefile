# File:   Makefile
# Author: Ryan Croucher (rcr69), Jeremy Roberts (jro162), M. P. Hayes, UCECE
# Date:   18 Oct 2021
# Descr:  Makefile for game

# Definitions.
CC = avr-gcc
CFLAGS = -mmcu=atmega32u2 -Os -Wall -Wstrict-prototypes -Wextra -g -I. -I../../utils -I../../fonts -I../../drivers -I../../drivers/avr
OBJCOPY = avr-objcopy
SIZE = avr-size
DEL = rm


# Default target.
all: game.out


# Compile: create object files from C source files.
game.o: game.c ../../drivers/avr/system.h
	$(CC) -c $(CFLAGS) $< -o $@

system.o: ../../drivers/avr/system.c ../../drivers/avr/system.h
	$(CC) -c $(CFLAGS) $< -o $@

timer.o: ../../drivers/avr/timer.c ../../drivers/avr/timer.h
	$(CC) -c $(CFLAGS) $< -o $@

led.o: ../../drivers/led.c ../../drivers/led.h
	$(CC) -c $(CFLAGS) $< -o $@

ledmat.o: ../../drivers/ledmat.c ../../drivers/ledmat.h
	$(CC) -c $(CFLAGS) $< -o $@

pacer.o: ../../utils/pacer.c ../../utils/pacer.h ../../drivers/avr/timer.h
	$(CC) -c $(CFLAGS) $< -o $@

display.o: ../../drivers/display.c ../../drivers/display.h ../../drivers/ledmat.h
	$(CC) -c $(CFLAGS) $< -o $@

font.o: ../../utils/font.c ../../utils/font.h
	$(CC) -c $(CFLAGS) $< -o $@

navswitch.o: ../../drivers/navswitch.c ../../drivers/navswitch.h ../../drivers/avr/system.h ../../drivers/avr/delay.h
	$(CC) -c $(CFLAGS) $< -o $@

tinygl.o: ../../utils/tinygl.c ../../utils/tinygl.h ../../drivers/avr/system.h ../../drivers/display.h ../../utils/font.h
	$(CC) -c $(CFLAGS) $< -o $@

player.o: player.c ./player.h
	$(CC) -c $(CFLAGS) $< -o $@

platforms.o: platforms.c ./platforms.h ../../drivers/ledmat.h
	$(CC) -c $(CFLAGS) $< -o $@

interface.o: interface.c ./interface.h ../../utils/tinygl.h ../../drivers/avr/system.h ../../utils/uint8toa.h
	$(CC) -c $(CFLAGS) $< -o $@

powerup.o: powerup.c ./powerup.h
	$(CC) -c $(CFLAGS) $< -o $@

button.o: ../../drivers/button.c ../../drivers/button.h
	$(CC) -c $(CFLAGS) $< -o $@

uint8toa.o: ../../utils/uint8toa.c ../../utils/uint8toa.h
	$(CC) -c $(CFLAGS) $< -o $@



# Link: create ELF output file from object files.
game.out: game.o system.o pacer.o led.o timer.o ledmat.o display.o font.o navswitch.o tinygl.o player.o platforms.o interface.o powerup.o button.o uint8toa.o
	$(CC) $(CFLAGS) $^ -o $@ -lm
	$(SIZE) $@


# Target: clean project.
.PHONY: clean
clean:
	-$(DEL) *.o *.out *.hex


# Target: program project.
.PHONY: program
program: game.out
	$(OBJCOPY) -O ihex game.out game.hex
	dfu-programmer atmega32u2 erase; dfu-programmer atmega32u2 flash game.hex; dfu-programmer atmega32u2 start
