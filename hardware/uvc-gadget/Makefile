CROSS_COMPILE		?= 
ARCH			?= x86
KERNEL_DIR		?= /usr/src/linux

CXX			:= $(CROSS_COMPILE)g++
CXXFLAGS		:= -fpermissive -Wno-write-strings -Ofast $(KERNEL_INCLUDE) # -std=c++11
LDFLAGS			:= -g
KERNEL_INCLUDE		:= -I$(KERNEL_DIR)/include -I$(KERNEL_DIR)/arch/$(ARCH)/include -I/usr/include/opencv4

all: uvc-gadget

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< -I/usr/include/opencv4

uvc-gadget: uvc-gadget.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lopencv_videoio -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_core -ljpeg

clean:
	rm -f *.o
	rm -f uvc-gadget
