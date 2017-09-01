#include "tooltip.h"

using namespace std;

Tooltip::Tooltip(int width, int height, Config *config){
  this->config = config;
  this->width = width;
  this->height = height;
}

int Tooltip::init(SDL_Renderer *renderer, string text){
  SDL_Surface* surface;
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
  surface = SDL_CreateRGBSurface(SDL_SWSURFACE, this->width,
                                 this->height, 32, rmask, gmask,
                                 bmask, amask);
  if (surface == NULL) {
     fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
     return -1;
  }

  SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, 239, 59, 59));

  TTF_Font *font = TTF_OpenFont(config->keyboardFont.c_str(), 24);
  SDL_Surface *textSurface;
  SDL_Color textColor = {255, 255, 255, 0};
  textSurface = TTF_RenderText_Blended(font, text.c_str(), textColor);

  SDL_Rect textRect;
  textRect.x = (this->width / 2) - (textSurface->w / 2);
  textRect.y = (this->height / 2) - (textSurface->h / 2);
  textRect.w = textSurface->w;
  textRect.h = textSurface->h;
  SDL_BlitSurface(textSurface, NULL, surface, &textRect);

  this->texture = SDL_CreateTextureFromSurface(renderer, surface);
  return 0;
}

void Tooltip::draw(SDL_Renderer *renderer, int x, int y){
  SDL_Rect target;
  target.x = x;
  target.y = y;
  target.w = this->width;
  target.h = this->height;
  SDL_RenderCopy(renderer, this->texture, NULL, &target);
}
