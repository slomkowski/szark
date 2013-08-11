#
# SZARK - arm driver
# 2013 Michał Słomkowski
# This code is distributed under the terms of GNU General Public License version 3.0.
#
 
AVR-CC=avr-g++
AVR-OBJCOPY=avr-objcopy
AVR-SIZE=avr-size
AVRDUDE=avrdude

OUTPUT=arm_driver

CPU-GCC=atmega88
CPU-AVRDUDE=m88

CFLAGS=-mmcu=$(CPU-GCC) -Os
CFLAGS+=-std=c++11 -Wall
CFLAGS+=-I../common

# config for USB programmer
PROGRAMMER=stk500v2
PORT=avrdoper

FUSE_LOW=0xe2
FUSE_HIGH=0xdf
FUSE_EXTENDED=0xf9

EXTRA_DEPS = global.h

BIN_DIR=obj

SRC_CPP = $(shell ls *.cpp)

.SUFFIXES:
.SUFFIXES: .h .cpp .c .o

HEADERS = $(SRC_CPP:%.cpp=%.h) 
OBJ = $(SRC_CPP:%.cpp=$(BIN_DIR)/%.o)

all : $(OUTPUT).hex

directories:
	if [ ! -e $(BIN_DIR) ]; then mkdir -p $(BIN_DIR); fi

# generic code
$(OUTPUT).hex : directories $(BIN_DIR)/$(OUTPUT).elf
	$(AVR-OBJCOPY) -O ihex $(BIN_DIR)/$(OUTPUT).elf $(OUTPUT).hex
	$(AVR-SIZE) $(OUTPUT).hex

$(BIN_DIR)/$(OUTPUT).elf : $(OBJ) 
	$(AVR-CC) $(CFLAGS) -o $(BIN_DIR)/$(OUTPUT).elf $(OBJ)

$(BIN_DIR)/main.o : main.cpp $(EXTRA_DEPS) directories
	$(AVR-CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o : %.cpp %.h $(EXTRA_DEPS) directories
	$(AVR-CC) $(CFLAGS) -c $< -o $@

run: burn

burn: $(OUTPUT).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT) -U flash:w:"$(OUTPUT).hex":a -U flash:v:"$(OUTPUT).hex":a -e

burn_fuses:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT) -U lfuse:w:$(FUSE_LOW):m -U hfuse:w:$(FUSE_HIGH):m -U efuse:w:$(FUSE_EXTENDED):m

clean:
	rm -rf $(OUTPUT).* $(BIN_DIR)

.PHONY: all clean burn burn_fuses run

