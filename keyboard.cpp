#include "SDL2/SDL.h"
#include <SDL2/SDL_ttf.h>
#include <string>
#include "config.h"

using namespace std;

void drawRow(SDL_Surface *surface, int x, int y, int width, int height,
             char *keys, int padding, TTF_Font *font) {

  auto keyBackground = SDL_MapRGB(surface->format, 15, 15, 15);
  auto keyColor = SDL_MapRGB(surface->format, 200, 200, 200);
  SDL_Color textColor = {255, 255, 255, 0};

  int i = 0;

  string chars(keys);

  for (char &keyCap : chars) {
    SDL_Rect keyRect;
    keyRect.x = x + (i * width) + padding;
    keyRect.y = y + padding;
    keyRect.w = width - (2 * padding);
    keyRect.h = height - (2 * padding);
    SDL_FillRect(surface, &keyRect, keyBackground);
    SDL_Surface *surface;
    char text[] = "a";
    text[0] = keyCap;
    surface = TTF_RenderText_Solid(font, text, textColor);
    i++;
  }
}

SDL_Surface *makeKeyboard(int width, int height,Config *config) {
  SDL_Surface *surface;
  Uint32 rmask, gmask, bmask, amask;

/* SDL interprets each pixel as a 32-bit number, so our masks must depend
   on the endianness (byte order) of the machine */
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
  rmask = 0xff000000;
  gmask = 0x00ff0000;
  bmask = 0x0000ff00;
  amask = 0x000000ff;
#else
  rmask = 0x000000ff;
  gmask = 0x0000ff00;
  bmask = 0x00ff0000;
  amask = 0xff000000;
#endif

  surface = SDL_CreateRGBSurface(SDL_SWSURFACE, width, height, 32, rmask, gmask,
                                 bmask, amask);

  if (surface == NULL) {
    fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
    exit(1);
  }

  auto keyboardColor = SDL_MapRGB(surface->format, 30, 30, 30);
  SDL_FillRect(surface, NULL, keyboardColor);

  int rowHeight = height / 5;
  char row1[] = "1234567890";
  char row2[] = "qwertyuiop";
  char row3[] = "asdfghjkl";
  char row4[] = "zxcvbnm";

  TTF_Font *font = TTF_OpenFont(config->keyboardFont.c_str(), 24);

  drawRow(surface, 0, 0, width / 10, rowHeight,row1, width / 100, font);
  drawRow(surface, 0, rowHeight, width / 10, rowHeight, row2,
          width / 100, font);
  drawRow(surface, width / 20, rowHeight * 2, width / 10, rowHeight,
          row3, width / 100, font);
  drawRow(surface, width / 10, rowHeight * 3, width / 10, rowHeight, row4,
          width / 100, font);
  return surface;
}
