CROSS_COMPILE	?= 
ARCH			?= x86
KERNEL_DIR		?= /usr/src/linux

CXX				:= $(CROSS_COMPILE)g++
CXXFLAGS		:= -fpermissive -Wno-write-strings -g $(KERNEL_INCLUDE)
LDFLAGS			:= -g
KERNEL_INCLUDE	:= -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)/arch/$(ARCH)/include

all: uvc-gadget

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $<

uvc-gadget: uvc-gadget.o
	$(CXX) $(LDFLAGS) -o $@ $^ `pkg-config --libs opencv libjpeg`

clean:
	rm -f *.o
	rm -f uvc-gadget
