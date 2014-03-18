#
# SZARK - motor driver
# 2014 Michał Słomkowski
# This code is distributed under the terms of GNU General Public License version 3.0.
#
# Makefile based on LUFA library Makefile
# LUFA Library
# Copyright (C) Dean Camera, 2014.
# dean [at] fourwalledcubicle [dot] com www.lufa-lib.org
#

MCU          = atmega32u4
ARCH         = AVR8
BOARD        = NONE
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = bridge
SRC          = $(shell ls *.cpp)
SRC          += $(LUFA_SRC_USB)
LUFA_PATH    = ../LUFA
CC_FLAGS     = -I../common -Wall -Wextra -DUSE_LUFA_CONFIG_HEADER 
LD_FLAGS     =
CPP_STANDARD=c++11
OBJDIR=obj

AVRDUDE_PROGRAMMER=avr109
AVRDUDE_PORT=/dev/ttyACM0

FUSE_LOW=0xde
FUSE_HIGH=0xd8
FUSE_EXTENDED=0xf9

# Default target
all: hex

include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk
include $(LUFA_PATH)/Build/lufa_avrdude.mk

burn_fuses:
	avrdude -c stk500v2 -p $(MCU) -P avrdoper -U lfuse:w:$(FUSE_LOW):m -U hfuse:w:$(FUSE_HIGH):m -U efuse:w:$(FUSE_EXTENDED):m
