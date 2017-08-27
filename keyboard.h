#include "SDL2/SDL.h"
#include <SDL2/SDL_ttf.h>
#include "config.h"
#include <string>
#include <vector>
#include <list>

const string KEYCAP_BACKSPACE = "\u2190";
const string KEYCAP_SHIFT     = "\u2191";

struct keyboardLayout{
  /* Stub */
};

struct touchArea {
  string keyChar;
  int x1;
  int x2;
  int y1;
  int y2;
};

struct rgb {
  int r;
  int g;
  int b;
};

struct argb{
  int a;
  int r;
  int g;
  int b;
};


class Keyboard {

  public:
    Keyboard(int pos, int targetPos, int width,
             int height, int inputHeight, Config *config,
             SDL_Renderer *renderer);
    ~Keyboard();
    string getCharForCoordinates(int x, int y);
    int setLayout(keyboardLayout *layout);
    int setKeyboardColor(int r, int g, int b);
    int setInputColor(int r, int g, int b);
    int setDotColor(int r, int g, int b);
    float getPosition();
    void setPosition(float p);
    float getTargetPosition();
    void setTargetPosition(float p);
    float getHeight();
    void draw(SDL_Renderer *renderer, int screenHeight, int numDots);


  private:
    vector<touchArea> keyList;
    rgb keyboardColor;
    rgb inputColor;
    rgb dotColor;
    float position;
    float targetPosition;
    int keyboardWidth;
    int keyboardHeight;
    int inputHeight;
    SDL_Surface *keyboard;
    SDL_Texture *keyboardTexture;

    void drawRow(SDL_Surface *surface, int x, int y, int width, int height,
                 list<string> *keys, int padding, TTF_Font *font);
    void drawKey(SDL_Surface *surface, int x, int y, int width, int height,
                 char *cap, string key, int padding, TTF_Font *font);
    void draw_circle(SDL_Renderer *renderer, SDL_Point center, int radius);
    SDL_Surface *makeKeyboard(int width, int height, Config *config);

};

