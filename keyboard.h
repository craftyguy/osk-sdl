#include "SDL2/SDL.h"
#include "config.h"

SDL_Surface *makeKeyboard(int width, int height, Config *config);
string getCharForCoordinates(int x, int y);
