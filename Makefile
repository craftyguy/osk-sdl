osk-sdl: main.cpp keyboard.cpp keyboard.h
	g++ -o osk-sdl main.cpp keyboard.cpp -std=c++14 -Werror -w -lcryptsetup -lSDL_ttf `sdl-config --cflags --libs`
