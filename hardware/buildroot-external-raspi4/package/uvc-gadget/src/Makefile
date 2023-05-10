CXXFLAGS		:= -fpermissive -Wno-write-strings -Ofast
LDFLAGS			:= -g

all: uvc-gadget

%.o: %.cpp
	$(CXX) -c $(CXXFLAGS) -o $@ $< -I $(STAGING_DIR)/usr/include/opencv4

uvc-gadget: uvc-gadget.o
	$(CXX) $(LDFLAGS) -o $@ $^ -lopencv_videoio -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc -lopencv_core -ljpeg

install:
	install -D -m 0755 $(@D)/uvc-gadget $(TARGET_DIR)/usr/bin

clean:
	rm -f *.o
	rm -f uvc-gadget
