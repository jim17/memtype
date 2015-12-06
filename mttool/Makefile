# Name: Makefile
# Project: hid-data example
# Author: Christian Starkjohann
# Creation Date: 2008-04-11
# Tabsize: 4
# Copyright: (c) 2008 by OBJECTIVE DEVELOPMENT Software GmbH
# License: GNU GPL v2 (see License.txt), GNU GPL v3 or proprietary (CommercialLicense.txt)

# Please read the definitions below and edit them as appropriate for your
# system:
ifeq ($(OS),Windows_NT)
#Windows stuff
# Use the following 3 lines on Windows and comment out the 3 above:
USBFLAGS= -Iwindows_inc/
USBLIBS= -Lwindows_inc/ -llibusb0_x86 #if 64 bit windows, remove "_x86"
EXE_SUFFIX= .exe
else
#Linux stuff
# Use the following 3 lines on Unix and Mac OS X:
USBFLAGS=   `libusb-config --cflags`
USBLIBS=    `libusb-config --libs`
EXE_SUFFIX=
endif

CC= gcc
CFLAGS=	-g -O -static -Wall $(USBFLAGS)
LIBS= $(USBLIBS)
SXMLPATH = ./sxmlc/src/
SXMLOBJ = $(SXMLPATH)sxmlc.o $(SXMLPATH)sxmlsearch.o
OBJ= mttool.o hiddata.o NoekeonDirectRef.o $(SXMLOBJ)
PROGRAM= mttool$(EXE_SUFFIX)

all: $(PROGRAM)

$(PROGRAM): $(OBJ)
	$(CC) -o $(PROGRAM) $(OBJ) $(LIBS)

strip: $(PROGRAM)
	strip $(PROGRAM)

clean:
	rm -f $(OBJ) $(PROGRAM)

.c.o:
	$(CC) $(ARCH_COMPILE) $(CFLAGS) -c $*.c -o $*.o
