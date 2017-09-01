#ifndef TOOLTIP_H
#define TOOLTIP_H

#include "SDL2/SDL.h"
#include <SDL2/SDL_ttf.h>
#include "config.h"
#include <string>

class Tooltip {
public:
  Tooltip(int width, int height, Config *config);
  int init(SDL_Renderer *renderer, string text);
  void draw(SDL_Renderer *renderer, int x, int y);
private:
  SDL_Texture *texture;
  string text;
  Config *config;
  int width;
  int height;
};

#endif
