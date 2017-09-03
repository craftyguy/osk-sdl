CXXFLAGS=-std=c++14 -Wall -g -lcryptsetup `sdl2-config --cflags --libs` -lSDL2_ttf
osk-sdl: main.cpp draw_helpers.cpp keyboard.cpp util.cpp draw_helpers.h keyboard.h config.h config.cpp luksdevice.h luksdevice.cpp Makefile
	g++ -o osk-sdl main.cpp draw_helpers.cpp keyboard.cpp util.cpp config.cpp luksdevice.cpp $(CXXFLAGS)
