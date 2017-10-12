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

#include "util.h"
#include "draw_helpers.h"

int fetchOpts(int argc, char **args, Opts *opts){
  int opt;

  while ((opt = getopt(argc, args, "td:n:c:v")) != -1)
    switch (opt) {
    case 't':
      opts->luksDevPath = DEFAULT_LUKSDEVPATH;
      opts->luksDevName = DEFAULT_LUKSDEVNAME;
      opts->testMode = true;
      break;
    case 'd':
      opts->luksDevPath = optarg;
      break;
    case 'n':
      opts->luksDevName = optarg;
      break;
    case 'c':
      opts->confPath = optarg;
      break;
    case 'v':
      opts->verbose = true;
      break;
    default:
      fprintf(stdout, "Usage: osk-sdl [-t] [-d /dev/sda] [-n device_name] [-c /etc/osk.conf] -v\n");
      return 1;
    }
  if (opts->luksDevPath.empty()) {
    fprintf(stderr, "No device path specified, use -d [path] or -t\n");
    return 1;
  }
  if (opts->luksDevName.empty()) {
    fprintf(stderr, "No device name specified, use -n [name] or -t\n");
    return 1;
  }
  if (opts->confPath.empty()){
    opts->confPath = DEFAULT_CONFPATH;
  }
  return 0;
}


string strList2str(const list<string> *strList){
  string str = "";
  list<string>::const_iterator it;
  for (it = strList->begin(); it != strList->end(); ++it){
    str.append(*it);
  }
  return str;
}


SDL_Surface* make_wallpaper(SDL_Renderer *renderer, Config *config,
                            int width, int height){
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

  if(config->wallpaper[0] == '#'){
    int r, g, b;
    if(sscanf(config->wallpaper.c_str(), "#%02x%02x%02x", &r, &g, &b)!=3){
      fprintf(stderr, "Could not parse color code %s\n", config->wallpaper.c_str());
      exit(1);
    }
    SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, r, g, b));
  }else{
    // Implement image loading
    fprintf(stderr, "Image loading not supported yet\n");
    exit(1);
  }
  return surface;
}


void draw_circle(SDL_Renderer *renderer, SDL_Point center, int radius) {
  SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
  for (int w = 0; w < radius * 2; w++) {
    for (int h = 0; h < radius * 2; h++) {
      int dx = radius - w; // horizontal offset
      int dy = radius - h; // vertical offset
      if ((dx * dx + dy * dy) <= (radius * radius)) {
        SDL_RenderDrawPoint(renderer, center.x + dx, center.y + dy);
      }
    }
  }
}


void draw_password_box(SDL_Renderer *renderer, int numDots, int screenHeight,
                       int screenWidth, int inputHeight, int y, argb *color,
                       int inputBoxRadius, bool busy){

  SDL_Rect inputRect;
  // Draw empty password box
  inputRect.x = screenWidth / 20;
  inputRect.y = y;
  inputRect.w = screenWidth * 0.9;
  inputRect.h = inputHeight;
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderFillRect(renderer, &inputRect);

  if(inputBoxRadius > 0){
    smooth_corners_renderer(renderer, color, &inputRect, inputBoxRadius);
  }
  int deflection = inputHeight / 4;
  int ypos = y + inputHeight / 2;
  float tick = (float) SDL_GetTicks();
  // Draw password dots
  int dotSize = screenWidth * 0.02;
  for (int i = 0; i < numDots; i++) {
    SDL_Point dotPos;
    dotPos.x = (screenWidth / 10) + (i * dotSize * 3);
    if (busy) {
      dotPos.y = ypos + sin((tick / 100.0)+(i)) * deflection;
    } else {
      dotPos.y = ypos;
    }
    draw_circle(renderer, dotPos, dotSize);
  }
  return;
}

void draw_password_box_dots(SDL_Renderer* renderer, int inputHeight, int screenWidth, 
                              int numDots, int y, bool busy){
  int deflection = inputHeight / 4;
  int ypos = y + inputHeight / 2;
  float tick = (float) SDL_GetTicks();
  // Draw password dots
  int dotSize = screenWidth * 0.02;
  for (int i = 0; i < numDots; i++) {
    SDL_Point dotPos;
    dotPos.x = (screenWidth / 10) + (i * dotSize * 3);
    if (busy) {
      dotPos.y = ypos + sin((tick / 100.0)+(i)) * deflection;
    } else {
      dotPos.y = ypos;
    }
    draw_circle(renderer, dotPos, dotSize);
  }
  return;
}
void handleVirtualKeyPress(string tapped, Keyboard *kbd, LuksDevice *lkd,
                           list<string> *passphrase){
  // return pressed
  if(tapped.compare("\n") == 0){
    lkd->setPassphrase(strList2str(passphrase));
    lkd->unlock();
  }
  // Backspace pressed
  else if (tapped.compare(KEYCAP_BACKSPACE) == 0){
    if (passphrase->size() > 0){
      passphrase->pop_back();
    }
  }
  // Shift pressed
  else if (tapped.compare(KEYCAP_SHIFT) == 0){
    if (kbd->getActiveLayer() > 1){
      kbd->setActiveLayer(0);
    }else{
      kbd->setActiveLayer(!kbd->getActiveLayer());
    }
  }
  // Symbols key pressed
  else if (tapped.compare(KEYCAP_SYMBOLS) == 0){
    kbd->setActiveLayer(2);
  }
  // ABC key was pressed
  else if (tapped.compare(KEYCAP_ABC) == 0){
    kbd->setActiveLayer(0);
  }
  // handle other key presses
  else if (tapped.compare("\0") != 0){
    passphrase->push_back(tapped);
  }
  return;
}

