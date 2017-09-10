/*
Copyright (C) 2017 Martijn Braam & Clayton Craft <clayton@craftyguy.net>

This file is part of osk-sdl.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

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
