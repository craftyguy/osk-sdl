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

#include "keyboard.h"
#include "draw_helpers.h"
using namespace std;

Keyboard::Keyboard(int pos, int targetPos, int width,
                   int height, Config *config){

  this->position = pos;
  this->targetPosition = targetPos;
  this->keyboardWidth = width;
  this->keyboardHeight = height;
  this->activeLayer = 0;
  this->config = config;
}


Keyboard::~Keyboard(){
  list<KeyboardLayer>::iterator layer;
  for (layer = keyboard.begin(); layer != keyboard.end(); ++layer){
    delete (*layer).surface;
  }
}


int Keyboard::init(SDL_Renderer *renderer){
  list<KeyboardLayer>::iterator layer;

  loadKeymap("");
  long keyLong = strtol(config->keyRadius.c_str(),NULL,10);
  if(keyLong >= BEZIER_RESOLUTION || keyLong > (keyboardHeight/5)/1.5){
    fprintf(stderr,"key-radius must be below %f and %f, it is %ld\n",BEZIER_RESOLUTION,(keyboardHeight/5)/1.5,keyLong);
    keyRadius = 0;
  }else{
    keyRadius = keyLong;
  }
  for (layer = this->keyboard.begin(); layer != this->keyboard.end(); ++layer){
    (*layer).surface = makeKeyboard(&(*layer));
    if (!(*layer).surface){
      fprintf(stderr, "ERROR: Unable to generate keyboard surface\n");
      return 1;
    }
    (*layer).texture = SDL_CreateTextureFromSurface(renderer, layer->surface);
    if (!(*layer).texture){
      fprintf(stderr, "ERROR: Unable to generate keyboard texture\n");
      return 1;
    }
  }

  return 0;
}




float Keyboard::getPosition(){
  return this->position;
}


void Keyboard::setPosition(float p){
  this->position = p;
  return;
}


float Keyboard::getTargetPosition(){
  return this->targetPosition;
}


void Keyboard::setTargetPosition(float p){
  this->targetPosition = p;
  return;
}


void Keyboard::setKeyboardColor(int r, int g, int b){
  this->keyboardColor.r = r;
  this->keyboardColor.g = g;
  this->keyboardColor.b = b;
}


float Keyboard::getHeight(){
  return this->keyboardHeight;
}


void Keyboard::draw(SDL_Renderer *renderer, int screenHeight){
  list<KeyboardLayer>::iterator layer;
  SDL_Rect keyboardRect, srcRect;

  if (this->position != this->targetPosition){
    if (this->position > this->targetPosition){
      this->position -= (this->position - this->targetPosition) / 10;
    }else
      this->position += (this->targetPosition - this->position) / 10;

    keyboardRect.x = 0;
    keyboardRect.y = (int)(screenHeight - (this->keyboardHeight * this->position));
    keyboardRect.w = this->keyboardWidth;
    keyboardRect.h = (int)(this->keyboardHeight * this->position);

    srcRect.x = 0;
    srcRect.y = 0;
    srcRect.w = this->keyboardWidth;
    srcRect.h = keyboardRect.h;

    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);


    for (layer = keyboard.begin(); layer != keyboard.end(); ++layer){
      if ((*layer).layerNum == this->activeLayer){
        SDL_RenderCopy(renderer, (*layer).texture,
                       &srcRect, &keyboardRect);
      }
    }
  }
  return;
}


void Keyboard::drawRow(SDL_Surface *surface, vector<touchArea> *keyList, int x,
                       int y, int width, int height, list<string> *keys,
                       int padding, TTF_Font *font) {

  auto keyBackground = SDL_MapRGB(surface->format, 15, 15, 15);
  SDL_Color textColor = {255, 255, 255, 0};

  auto background = SDL_MapRGB(surface->format,keyboardColor.r ,keyboardColor.g, keyboardColor.b);
  int i = 0;
  list<string>::const_iterator keyCap;
  for (keyCap = keys->begin(); keyCap != keys->end(); ++keyCap) {
    SDL_Rect keyRect;
    keyRect.x = x + (i * width) + padding;
    keyRect.y = y + padding;
    keyRect.w = width - (2 * padding);
    keyRect.h = height - (2 * padding);
    SDL_FillRect(surface, &keyRect, keyBackground);
    if(keyRadius > 0){
      smooth_corners_surface(surface,background,&keyRect,keyRadius);
    }
    SDL_Surface *textSurface;
    keyList->push_back(
        {*keyCap, x + (i * width), x + (i * width) + width, y, y + height});

    textSurface = TTF_RenderUTF8_Blended(font, keyCap->c_str(), textColor);

    SDL_Rect keyCapRect;
    keyCapRect.x = keyRect.x + ((keyRect.w / 2) - (textSurface->w / 2));
    keyCapRect.y = keyRect.y + ((keyRect.h / 2) - (textSurface->h / 2));
    keyCapRect.w = keyRect.w;
    keyCapRect.h = keyRect.h;
    SDL_BlitSurface(textSurface, NULL, surface, &keyCapRect);

    i++;
  }
}


void Keyboard::drawKey(SDL_Surface *surface, vector<touchArea> *keyList, int x,
                       int y, int width, int height, char *cap, string key,
                       int padding, TTF_Font *font){
  auto keyBackground = SDL_MapRGB(surface->format, 15, 15, 15);
  SDL_Color textColor = {255, 255, 255, 0};

  SDL_Rect keyRect;
  keyRect.x = x + padding;
  keyRect.y = y + padding;
  keyRect.w = width - (2 * padding);
  keyRect.h = height - (2 * padding);
  SDL_FillRect(surface, &keyRect, keyBackground);
  SDL_Surface *textSurface;

  keyList->push_back({key, x, x + width, y, y + height});

  textSurface = TTF_RenderText_Blended(font, cap, textColor);

  SDL_Rect keyCapRect;
  keyCapRect.x = keyRect.x + ((keyRect.w / 2) - (textSurface->w / 2));
  keyCapRect.y = keyRect.y + ((keyRect.h / 2) - (textSurface->h / 2));
  keyCapRect.w = keyRect.w;
  keyCapRect.h = keyRect.h;
  SDL_BlitSurface(textSurface, NULL, surface, &keyCapRect);
}

SDL_Surface *Keyboard::makeKeyboard(KeyboardLayer *layer) {
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

  surface = SDL_CreateRGBSurface(SDL_SWSURFACE, this->keyboardWidth,
                                 this->keyboardHeight, 32, rmask, gmask,
                                 bmask, amask);

  if (surface == NULL) {
    fprintf(stderr, "CreateRGBSurface failed: %s\n", SDL_GetError());
    return NULL;
  }

  SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, this->keyboardColor.r,
                                         this->keyboardColor.g,
                                         this->keyboardColor.b));

  int rowHeight = this->keyboardHeight / 5;

  if (TTF_Init() == -1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    return NULL;
  }

  TTF_Font *font = TTF_OpenFont(config->keyboardFont.c_str(), 24);
  if (!font) {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    return NULL;
  }

  drawRow(surface, &layer->keyList, 0, 0, this->keyboardWidth / 10, rowHeight,
          &layer->row1, this->keyboardWidth / 100, font);
  drawRow(surface, &layer->keyList, 0, rowHeight, this->keyboardWidth / 10,
          rowHeight, &layer->row2, this->keyboardWidth / 100, font);
  drawRow(surface, &layer->keyList, this->keyboardWidth / 20, rowHeight * 2,
          this->keyboardWidth / 10, rowHeight, &layer->row3, this->keyboardWidth / 100, font);
  drawRow(surface, &layer->keyList, this->keyboardWidth / 20, rowHeight * 3,
          this->keyboardWidth / 10,
          rowHeight, &layer->row4, this->keyboardWidth / 100, font);

  // Divide the bottom row in 20 columns and use that for calculations
  int colw = this->keyboardWidth/20;

  /* Draw symbols or ABC key based on which layer we are creating */
  if (layer->layerNum < 2){
    char symb[] = "=\\<";
    drawKey(surface, &layer->keyList, colw, rowHeight * 4, colw*3, rowHeight,
            symb, KEYCAP_SYMBOLS, this->keyboardWidth/100, font);
  }else if (layer->layerNum == 2){
    char abc[] = "abc";
    drawKey(surface, &layer->keyList, colw, rowHeight * 4, colw*3, rowHeight, abc,
            KEYCAP_ABC, this->keyboardWidth/100, font);
  }

  char space[] = " ";
  drawKey(surface, &layer->keyList, colw*5, rowHeight * 4, colw*10, rowHeight, space,
          KEYCAP_SPACE, this->keyboardWidth/100, font);

  char enter[] = "OK";
  drawKey(surface, &layer->keyList, colw*15, rowHeight * 4,  colw*5, rowHeight, enter,
          KEYCAP_RETURN, this->keyboardWidth/100, font);

  return surface;
}

void setLayout(int layoutNum){
  /* Stub */
}

void Keyboard::setActiveLayer(int layerNum){
  if (layerNum >= 0){
    if ((string::size_type)layerNum <= keyboard.size() - 1 ){
      this->activeLayer = layerNum;
      return;
    }
  }
  fprintf(stderr, "Unknown layer number: %i\n", layerNum);
  return;
}


int Keyboard::getActiveLayer(){
  return this->activeLayer;
}

/*
 * This function is not actually parsing any external keymaps, it's currently
 * filling in the keyboardKeymap object with a US/QWERTY layout until parsing
 * from a file is implemented
 */
void Keyboard::loadKeymap(string keymapPath){
  KeyboardLayer layer0, layer1, layer2;
  list<string> row1, row2, row3, row4;
  this->keyboard.clear();

  layer0.row1 = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
  layer0.row2 = {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"};
  layer0.row3 = {"a", "s", "d", "f", "g", "h", "j", "k", "l"};
  layer0.row4 = {KEYCAP_SHIFT, "z", "x", "c", "v", "b", "n", "m", KEYCAP_BACKSPACE};
  layer0.layerNum = 0;

  layer1.row1 = {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")"};
  layer1.row2 = {"Q", "W", "E", "R", "T", "Y", "U", "I", "O", "P"};
  layer1.row3 = {"A", "S", "D", "F", "G", "H", "J", "K", "L"};
  layer1.row4 = {KEYCAP_SHIFT, "Z", "X", "C", "V", "B", "N", "M", KEYCAP_BACKSPACE};
  layer1.layerNum = 1;

  layer2.row1 = {"!", "@", "#", "$", "%", "^", "&", "*", "(", ")"};
  layer2.row2 = {";", ":", "'", "\"", ",", ".", "<", ">", "/", "?"};
  layer2.row3 = {"-", "_", "=", "+", "[", "]", "{", "}", "\\"};
  layer2.row4 = {KEYCAP_SHIFT, "|", "\u20a4", "\u20ac", "\u2211", "\u221e", "\u221a", "\u2248", KEYCAP_BACKSPACE};
  layer2.layerNum = 2;

  this->keyboard.push_back(layer0);
  this->keyboard.push_back(layer1);
  this->keyboard.push_back(layer2);

  return;
}


string Keyboard::getCharForCoordinates(int x, int y) {
  list<KeyboardLayer>::iterator layer;
  vector<touchArea>::iterator it;

  for (layer = this->keyboard.begin(); layer != this->keyboard.end(); ++layer){
    if ((*layer).layerNum == this->activeLayer){
      for (it = (*layer).keyList.begin(); it != (*layer).keyList.end(); ++it) {
        if(x > it->x1 && x < it->x2 && y > it->y1 && y < it->y2){
          return it->keyChar;
        }
      }
    }
  }
  return "\0";
}
