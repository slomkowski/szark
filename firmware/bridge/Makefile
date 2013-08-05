#
# SZARK - motor driver
# 2013 Michał Słomkowski
# This code is distributed under the terms of GNU General Public License version 3.0.
#
 
AVR-CC=avr-gcc
AVR-CPP=avr-g++
AVR-OBJCOPY=avr-objcopy
AVR-SIZE=avr-size
AVRDUDE=avrdude

OUTPUT=bridge

CPU-GCC=atmega168
CPU-AVRDUDE=m168

COMMONFLAGS=-mmcu=$(CPU-GCC) -Os
COMMONFLAGS+=-Wall 
COMMONFLAGS+=-I../common -I. -Iusbdrv
CFLAGS=$(COMMONFLAGS) --std=gnu99
CPPFLAGS=$(COMMONFLAGS) --std=c++11

# config for USB programmer
PROGRAMMER=stk500v2
PORT=avrdoper

FUSE_LOW=0xff
FUSE_HIGH=0xdd
FUSE_EXTENDED=0xf9

EXTRA_DEPS = global.h

BIN_DIR=obj

SRC_CPP = $(shell ls *.cpp)
SRC_C=$(shell ls usbdrv/*.c)
SRC_ASM = $(shell ls usbdrv/*.S)

.SUFFIXES:
.SUFFIXES: .h .cpp .c .o .inc .S .asm

HEADERS = $(SRC_CPP:%.cpp=%.h) 
OBJ = $(SRC_C:%.c=$(BIN_DIR)/%.o)
OBJ += $(SRC_CPP:%.cpp=$(BIN_DIR)/%.o)
OBJ += $(SRC_ASM:%.S=$(BIN_DIR)/%.o)

all : $(OUTPUT).hex

directories:
	mkdir -p $(BIN_DIR)/usbdrv

# generic code
$(OUTPUT).hex : directories $(BIN_DIR)/$(OUTPUT).elf
	$(AVR-OBJCOPY) -O ihex $(BIN_DIR)/$(OUTPUT).elf $(OUTPUT).hex
	$(AVR-SIZE) $(OUTPUT).hex

$(BIN_DIR)/$(OUTPUT).elf : $(OBJ) 
	$(AVR-CC) $(CFLAGS) -o $(BIN_DIR)/$(OUTPUT).elf $(OBJ)

$(BIN_DIR)/main.o : main.cpp $(EXTRA_DEPS) directories
	$(AVR-CPP) $(CPPFLAGS) -c $< -o $@

$(BIN_DIR)/%.o : %.c %.h $(EXTRA_DEPS) directories
	$(AVR-CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o : %.cpp %.h $(EXTRA_DEPS) directories
	$(AVR-CPP) $(CPPFLAGS) -c $< -o $@
	
# Generic rule for assembling Assembler source files:
$(BIN_DIR)/%.o : %.S
	$(AVR-CC) $(COMMONFLAGS) -x assembler-with-cpp -c $< -o $@

run: burn

burn: $(OUTPUT).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT) -U flash:w:"$(OUTPUT).hex":a -U flash:v:"$(OUTPUT).hex":a -e

burn_fuses:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT) -U lfuse:w:$(FUSE_LOW):m -U hfuse:w:$(FUSE_HIGH):m -U efuse:w:$(FUSE_EXTENDED):m
	

clean:
	rm -rf $(OUTPUT).* $(BIN_DIR)

.PHONY: all clean burn burn_fuses run

