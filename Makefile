CXX = g++
SDL2_CFLAGS = $(shell sdl2-config --cflags)
SDL2_LIBS= $(shell sdl2-config --libs)
CXXFLAGS = -std=c++14 -Wall -g $(SDL2_CFLAGS)
LIBS = -lcryptsetup $(SDL2_LIBS) -lSDL2_ttf
DEPS = keyboard.h config.h luksdevice.h util.h draw_helpers.h tooltip.h

%.o: %.cpp $(DEPS)
	$(CXX) -c -o $@ $< $(CXXFLAGS)

osk-sdl: main.o keyboard.o config.o util.o luksdevice.o
	$(CXX) -o $@ $^ $(CXXFLAGS) $(LIBS)

.PHONY: clean

clean:
	rm -f *.o osk-sdl
