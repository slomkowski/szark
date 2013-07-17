#
# SZARK - motor driver
# 2013 Michał Słomkowski
# This code is distributed under the terms of GNU General Public License version 3.0.
#
 
AVR-CC=avr-gcc
AVR-OBJCOPY=avr-objcopy
AVR-SIZE=avr-size
AVRDUDE=avrdude

OUTPUT=motor_driver

CPU-GCC=attiny2313
CPU-AVRDUDE=t2313

CFLAGS=-mmcu=$(CPU-GCC) -std=c99 -Os
CFLAGS+=-Wall
CFLAGS+=-I../common

# config for USB programmer
PROGRAMMER=stk500v2
PORT=avrdoper

FUSE_LOW=0xe2
FUSE_HIGH=0xdf
FUSE_EXTENDED=0xff

EXTRA_DEPS = global.h

BIN_DIR=obj

SRC_C = $(shell ls *.c)

.SUFFIXES:
.SUFFIXES: .h .c .o

HEADERS = $(SRC_C:%.c=%.h) 
OBJ = $(SRC_C:%.c=$(BIN_DIR)/%.o)

all : $(OUTPUT).hex

directories:
	if [ ! -e $(BIN_DIR) ]; then mkdir -p $(BIN_DIR); fi

# generic code
$(OUTPUT).hex : $(BIN_DIR)/$(OUTPUT).elf
	$(AVR-OBJCOPY) -O ihex $(BIN_DIR)/$(OUTPUT).elf $(OUTPUT).hex
	$(AVR-SIZE) $(OUTPUT).hex

$(BIN_DIR)/$(OUTPUT).elf : $(OBJ) 
	$(AVR-CC) $(CFLAGS) -o $(BIN_DIR)/$(OUTPUT).elf $(OBJ)

$(BIN_DIR)/main.o : main.c $(EXTRA_DEPS) directories
	$(AVR-CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o : %.c %.h $(EXTRA_DEPS) directories
	$(AVR-CC) $(CFLAGS) -c $< -o $@

run: burn

burn: $(OUTPUT).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT) -U flash:w:"$(OUTPUT).hex":a -U flash:v:"$(OUTPUT).hex":a -e

burn_fuses:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT) -U lfuse:w:$(FUSE_LOW):m -U hfuse:w:$(FUSE_HIGH):m -U efuse:w:$(FUSE_EXTENDED):m

clean:
	rm -rf $(OUTPUT).* $(BIN_DIR)

.PHONY: all clean burn burn_fuses run

