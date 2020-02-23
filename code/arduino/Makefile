#
#             LUFA Library
#     Copyright (C) Dean Camera, 2015.
#
#  dean [at] fourwalledcubicle [dot] com
#           www.lufa-lib.org
#
# --------------------------------------
#         LUFA Project Makefile.
# --------------------------------------

# Run "make help" for target help.

#http://fourwalledcubicle.com/blog/2012/08/lufa-and-the-arduino-leonardo/

MCU          = atmega32u4
ARCH         = AVR8
BOARD        = LEONARDO
F_CPU        = 16000000
F_USB        = $(F_CPU)
OPTIMIZATION = s
TARGET       = firmware
SRC          = timer.c usblib.c Descriptors.c main.c $(LUFA_SRC_USB) $(LUFA_SRC_USBCLASS)
LUFA_PATH    = ./lufa-master/LUFA
CC_FLAGS     = -DUSE_LUFA_CONFIG_HEADER -IConfig/
LD_FLAGS     =
SERIALPORT = /dev/ttyACM0


all: build

install: build 
#	ssh 192.168.1.214 "echo 0 > /sys/class/gpio/gpio49/value && usleep 500000 && echo 1 > /sys/class/gpio/gpio49/value && usleep 1000000"
	avrdude -v -b 9600 -p $(MCU) -c avr109 -P $(SERIALPORT) -D -U flash:w:$(TARGET).hex:i

eeprom: build
#	ssh 192.168.1.214 "echo 0 > /sys/class/gpio/gpio49/value && usleep 500000 && echo 1 > /sys/class/gpio/gpio49/value && usleep 1000000"
	avrdude -v -b 9600 -p $(MCU) -c avr109 -P $(SERIALPORT) -D -U eeprom:w:$(TARGET).eep:i


# Default target
build:
include $(LUFA_PATH)/Build/lufa_core.mk
include $(LUFA_PATH)/Build/lufa_sources.mk
include $(LUFA_PATH)/Build/lufa_build.mk


