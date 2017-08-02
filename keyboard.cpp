#include <string>
#include "SDL/SDL.h"
#include "SDL_ttf.h"

using namespace std;

void drawRow(SDL_Surface *surface, int x, int y, int width, int height,
             char *keys, int padding, TTF_Font* font) {

  auto keyBackground = SDL_MapRGB(surface->format, 15, 15, 15);
  auto keyColor = SDL_MapRGB(surface->format, 200,200, 200);

  int i=0;

  string chars(keys);

  for(char & keyCap : chars){
    SDL_Rect keyRect;
    keyRect.x = x + (i*width) + padding;
    keyRect.y = y + padding;
    keyRect.w = width - (2*padding);
    keyRect.h = height - (2*padding);
    SDL_FillRect(surface, &keyRect, keyBackground);

    SDL_Surface* textSurface = TTF_RenderText_Shaded(font, keyCap, keyColor, keyBackground);
    SDL_Rect textLocation = { keyRect.x+10, keyRect.y+10, 0, 0 };
    SDL_BlitSurface(textSurface, NULL, surface, &textLocation);
    i++;
  }
}


SDL_Surface *makeKeyboard(int width, int height) {
  SDL_Surface *surface;

  if (TTF_Init() == -1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    exit(2);
  }

  TTF_Font* font = TTF_OpenFont("ARIAL.TTF", 12);


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

  drawRow(surface, 0, 0, width / 10, rowHeight, "1234567890", width/100, font);
  drawRow(surface, 0, rowHeight, width / 10, rowHeight, "qwertyuiop", width/100, font);
  drawRow(surface, width/20, rowHeight*2, width/10, rowHeight, "asdfghjkl" , width/100, font);
  drawRow(surface, width/10, rowHeight*3, width/10, rowHeight, "zxcvbnm" , width/100, font);
  return surface;
}
