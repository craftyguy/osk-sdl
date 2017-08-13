osk-sdl: main.cpp keyboard.cpp keyboard.h config.h config.cpp Makefile
	g++ -o osk-sdl main.cpp keyboard.cpp config.cpp -std=c++14 -Werror -g -lcryptsetup `sdl2-config --cflags --libs` -lSDL2_ttf
