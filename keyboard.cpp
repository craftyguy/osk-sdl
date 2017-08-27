#include "keyboard.h"

using namespace std;

Keyboard::Keyboard(int pos, int targetPos, int width,
                   int height, int inputHeight, Config *config,
                   SDL_Renderer *renderer){

  this->keyboard = makeKeyboard(width, height, config);
  this->keyboardTexture = SDL_CreateTextureFromSurface(renderer, keyboard);
  this->position = pos;
  this->targetPosition = targetPos;
  this->keyboardWidth = width;
  this->keyboardHeight = height;
  this->inputHeight = inputHeight;

}


Keyboard::~Keyboard(){
  delete this->keyboard;
  this->keyboard = NULL;
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


int Keyboard::setKeyboardColor(int r, int g, int b){
  this->keyboardColor.r = r;
  this->keyboardColor.g = g;
  this->keyboardColor.b = b;
  return 0;
}


int Keyboard::setInputColor(int r, int g, int b){
  this->inputColor.r = r;
  this->inputColor.g = g;
  this->inputColor.b = b;
  return 0;
}


int Keyboard::setDotColor(int r, int g, int b){
  this->dotColor.r = r;
  this->dotColor.g = g;
  this->dotColor.b = b;
  return 0;
}


float Keyboard::getHeight(){
  return this->keyboardHeight;
}


void Keyboard::draw(SDL_Renderer *renderer, int screenHeight, int numDots){
  SDL_Rect keyboardRect, srcRect, inputRect;

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
    SDL_RenderCopy(renderer, this->keyboardTexture, &srcRect, &keyboardRect);
  }

  // Draw empty password box
  int topHalf = screenHeight - (this->keyboardHeight * this->position);
  inputRect.x = this->keyboardWidth / 20;
  inputRect.y = (topHalf / 2) - (this->inputHeight / 2);
  inputRect.w = this->keyboardWidth * 0.9;
  inputRect.h = this->inputHeight;
  SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
  SDL_RenderFillRect(renderer, &inputRect);

  // Draw password dots
  int dotSize = this->keyboardWidth * 0.02;
  for (int i = 0; i < numDots; i++) {
    SDL_Point dotPos;
    dotPos.x = (this->keyboardWidth / 10) + (i * dotSize * 3);
    dotPos.y = (topHalf / 2);
    draw_circle(renderer, dotPos, dotSize);
  }
  return;
}


void Keyboard::drawRow(SDL_Surface *surface, int x, int y, int width, int height, list<string> *keys, int padding, TTF_Font *font) {

  auto keyBackground = SDL_MapRGB(surface->format, 15, 15, 15);
  auto keyColor = SDL_MapRGB(surface->format, 200, 200, 200);
  SDL_Color textColor = {255, 255, 255, 0};

  int i = 0;
  list<string>::const_iterator keyCap;
  for (keyCap = keys->begin(); keyCap != keys->end(); ++keyCap) {
    SDL_Rect keyRect;
    keyRect.x = x + (i * width) + padding;
    keyRect.y = y + padding;
    keyRect.w = width - (2 * padding);
    keyRect.h = height - (2 * padding);
    SDL_FillRect(surface, &keyRect, keyBackground);
    SDL_Surface *textSurface;

    this->keyList.push_back(
        {*keyCap, x + (i * width), x + (i * width) + width, y, y + height});

    textSurface = TTF_RenderUTF8_Blended(font, keyCap->c_str(), textColor);

    SDL_Rect keyCapRect;
    keyCapRect.x = keyRect.x + ((keyRect.w / 2) - (textSurface->w / 2));
    keyCapRect.y = keyRect.y;
    keyCapRect.w = keyRect.w;
    keyCapRect.h = keyRect.h;
    SDL_BlitSurface(textSurface, NULL, surface, &keyCapRect);

    i++;
  }
}


void Keyboard::drawKey(SDL_Surface *surface, int x, int y, int width, int height, char *cap, string key, int padding, TTF_Font *font){
  auto keyBackground = SDL_MapRGB(surface->format, 15, 15, 15);
  auto keyColor = SDL_MapRGB(surface->format, 200, 200, 200);
  SDL_Color textColor = {255, 255, 255, 0};

  SDL_Rect keyRect;
  keyRect.x = x + padding;
  keyRect.y = y + padding;
  keyRect.w = width - (2 * padding);
  keyRect.h = height - (2 * padding);
  SDL_FillRect(surface, &keyRect, keyBackground);
  SDL_Surface *textSurface;

  this->keyList.push_back({key, x, x + width, y, y + height});

  textSurface = TTF_RenderText_Blended(font, cap, textColor);

  SDL_Rect keyCapRect;
  keyCapRect.x = keyRect.x + ((keyRect.w / 2) - (textSurface->w / 2));
  keyCapRect.y = keyRect.y;
  keyCapRect.w = keyRect.w;
  keyCapRect.h = keyRect.h;
  SDL_BlitSurface(textSurface, NULL, surface, &keyCapRect);
}

SDL_Surface *Keyboard::makeKeyboard(int width, int height, Config *config) {
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
    SDL_Quit();
    exit(1);
  }

  SDL_FillRect(surface, NULL, SDL_MapRGB(surface->format, keyboardColor.r,
                                         keyboardColor.g, keyboardColor.b));

  int rowHeight = height / 5;
  list<string> row1 {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0"};
  list<string> row2 {"q", "w", "e", "r", "t", "y", "u", "i", "o", "p"};
  list<string> row3 {"a", "s", "d", "f", "g", "h", "j", "k", "l"};
  list<string> row4 {KEYCAP_SHIFT, "z", "x", "c", "v", "b", "n", "m", KEYCAP_BACKSPACE};

  if (TTF_Init() == -1) {
    printf("TTF_Init: %s\n", TTF_GetError());
    SDL_Quit();
    exit(1);
  }

  TTF_Font *font = TTF_OpenFont(config->keyboardFont.c_str(), 24);
  if (!font) {
    printf("TTF_OpenFont: %s\n", TTF_GetError());
    exit(1);
  }

  drawRow(surface, 0, 0, width / 10, rowHeight, &row1, width / 100, font);
  drawRow(surface, 0, rowHeight, width / 10, rowHeight, &row2, width / 100,
          font);
  drawRow(surface, width / 20, rowHeight * 2, width / 10, rowHeight, &row3,
          width / 100, font);
  drawRow(surface, width / 20, rowHeight * 3, width / 10, rowHeight, &row4,
          width / 100, font);

  // Divide the bottom row in 20 columns and use that for calculations
  int colw = width/20;
  char space[] = " ";
  drawKey(surface, colw*5, rowHeight * 4, colw*10, rowHeight, space, " ", width/100, font);

  char enter[] = "OK";
  drawKey(surface, colw*15, rowHeight * 4,  colw*5, rowHeight, enter, "\n", width/100, font);

  return surface;
}


string Keyboard::getCharForCoordinates(int x, int y) {
  for (vector<touchArea>::iterator it = this->keyList.begin(); it != this->keyList.end();
       ++it) {
    if(x > it->x1 && x < it->x2 && y > it->y1 && y < it->y2){
      return it->keyChar;
    }
  }
  return "\0";
}


void Keyboard::draw_circle(SDL_Renderer *renderer, SDL_Point center, int radius) {
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
