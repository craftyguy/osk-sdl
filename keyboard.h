#include "SDL2/SDL.h"
#include "config.h"

const string KEYCAP_BACKSPACE = "\u2190";
const string KEYCAP_SHIFT     = "\u2191";

SDL_Surface *makeKeyboard(int width, int height, Config *config);
string getCharForCoordinates(int x, int y);
