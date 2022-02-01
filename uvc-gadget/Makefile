CROSS_COMPILE	?= 
ARCH			?= x86
KERNEL_DIR		?= /usr/src/linux

CC				:= $(CROSS_COMPILE)g++
CFLAGS			:= -W -Wall -g $(KERNEL_INCLUDE) -fpermissive
LDFLAGS			:= -g
KERNEL_INCLUDE	:= -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)/arch/$(ARCH)/include

all: uvc-gadget

uvc-gadget.o: uvc-gadget.cpp
	$(CC) $(CFLAGS) -c $^

uvc-gadget: uvc-gadget.o
	$(CC) $(LDFLAGS) -o $@ $^ `pkg-config --libs opencv libjpeg`

clean:
	rm -f *.o
	rm -f uvc-gadget
