CXXFLAGS=-std=c++14 -Wall -g -lcryptsetup `sdl2-config --cflags --libs` -lSDL2_ttf
osk-sdl: main.cpp keyboard.cpp util.cpp keyboard.h config.h config.cpp luksdevice.h luksdevice.cpp tooltip.h tooltip.cpp Makefile
	g++ -o osk-sdl main.cpp keyboard.cpp util.cpp config.cpp luksdevice.cpp tooltip.cpp $(CXXFLAGS)

clean:
	rm osk-sdl
	rm *.gch
