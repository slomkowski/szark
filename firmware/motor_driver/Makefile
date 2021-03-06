#
# SZARK - motor driver
# 2013 Michał Słomkowski
# This code is distributed under the terms of GNU General Public License version 3.0.
#
 
AVR-CC=avr-g++
AVR-OBJCOPY=avr-objcopy
AVR-SIZE=avr-size
AVRDUDE=avrdude

OUTPUT=motor_driver

CPU-GCC=attiny2313
CPU-AVRDUDE=t2313

CFLAGS=-mmcu=$(CPU-GCC) -Os 
CFLAGS+=-Wall -std=c++11
CFLAGS+=-I../common

# config for USB programmer
PROGRAMMER=stk500v2
PORT=avrdoper

FUSE_LOW=0xc2
FUSE_HIGH=0xd9
FUSE_EXTENDED=0xff

EXTRA_DEPS = global.hpp

BIN_DIR=obj

SRC_CPP = $(shell ls *.cpp)

.SUFFIXES:
.SUFFIXES: .hpp .cpp .c .o .h

HEADERS = $(SRC_CPP:%.cpp=%.hpp) 
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

$(BIN_DIR)/main.o : main.cpp $(EXTRA_DEPS)
	$(AVR-CC) $(CFLAGS) -c $< -o $@

$(BIN_DIR)/%.o : %.cpp %.hpp $(EXTRA_DEPS)
	$(AVR-CC) $(CFLAGS) -c $< -o $@

run: burn

burn: $(OUTPUT).hex
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT) -U flash:w:"$(OUTPUT).hex":a -U flash:v:"$(OUTPUT).hex":a -e

burn_fuses:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(CPU-AVRDUDE) -P $(PORT) -U lfuse:w:$(FUSE_LOW):m -U hfuse:w:$(FUSE_HIGH):m -U efuse:w:$(FUSE_EXTENDED):m

clean:
	rm -rf $(OUTPUT).* $(BIN_DIR)

.PHONY: all clean burn burn_fuses run

