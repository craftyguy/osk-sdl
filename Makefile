osk-sdl: main.cpp keyboard.cpp util.cpp keyboard.h config.h config.cpp luksdevice.h luksdevice.cpp Makefile
	g++ -o osk-sdl main.cpp keyboard.cpp util.cpp config.cpp luksdevice.cpp -std=c++14 -Werror -g -lcryptsetup `sdl2-config --cflags --libs` -lSDL2_ttf
