#
# SZARK - motor driver
# 2014 Michał Słomkowski
# This code is distributed under the terms of GNU General Public License version 3.0.
#
 
AVR-CC=avr-gcc
AVR-CPP=avr-g++
AVR-OBJCOPY=avr-objcopy
AVR-SIZE=avr-size
AVRDUDE=avrdude

OUTPUT=bridge

CPU-GCC=atmega32u4
CPU-AVRDUDE=atmega32u4

COMMONFLAGS=-mmcu=$(CPU-GCC) -Os
COMMONFLAGS+=-Wall
COMMONFLAGS+=-I../common -I.
CPPFLAGS=$(COMMONFLAGS) --std=c++11 -Wextra

# config for USB programmer
PROGRAMMER=stk500v2
PORT=avrdoper

FUSE_LOW=0xde
FUSE_HIGH=0xd8
FUSE_EXTENDED=0xf9

EXTRA_DEPS = global.hpp

BIN_DIR=obj

SRC_CPP = $(shell ls *.cpp)

.SUFFIXES:
.SUFFIXES: .hpp .h .cpp .c .o .inc .S .asm

HEADERS = $(SRC_CPP:%.cpp=%.hpp) 
OBJ += $(SRC_CPP:%.cpp=$(BIN_DIR)/%.o)

all : $(OUTPUT).hex

directories:
	mkdir -p $(BIN_DIR)

# generic code
$(OUTPUT).hex : directories $(BIN_DIR)/$(OUTPUT).elf
	$(AVR-OBJCOPY) -O ihex $(BIN_DIR)/$(OUTPUT).elf $(OUTPUT).hex
	$(AVR-SIZE) --mcu=$(CPU-GCC) -C $(BIN_DIR)/$(OUTPUT).elf

$(BIN_DIR)/$(OUTPUT).elf : $(OBJ) 
	$(AVR-CPP) $(CPPFLAGS) -o $(BIN_DIR)/$(OUTPUT).elf $(OBJ)

$(BIN_DIR)/main.o : main.cpp $(EXTRA_DEPS) directories
	$(AVR-CPP) $(CPPFLAGS) -c $< -o $@

$(BIN_DIR)/%.o : %.cpp %.hpp $(EXTRA_DEPS) directories
	$(AVR-CPP) $(CPPFLAGS) -c $< -o $@
	
run: burn

reset:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT)

burn: $(OUTPUT).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT) -U flash:w:"$(OUTPUT).hex":a -e

burn_fuses:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT) -U lfuse:w:$(FUSE_LOW):m -U hfuse:w:$(FUSE_HIGH):m -U efuse:w:$(FUSE_EXTENDED):m
	

clean:
	rm -rf $(OUTPUT).* $(BIN_DIR)

.PHONY: all clean burn burn_fuses run reset directories

