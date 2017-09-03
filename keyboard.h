#ifndef KEYBOARD_H
#define KEYBOARD_H
#include "SDL2/SDL.h"
#include <SDL2/SDL_ttf.h>
#include "config.h"
#include <string>
#include <vector>
#include <list>


const string KEYCAP_BACKSPACE = "\u2190";
const string KEYCAP_SHIFT     = "\u2191";
const string KEYCAP_SYMBOLS   = "SYM";
const string KEYCAP_ABC       = "abc";
const string KEYCAP_SPACE     = " ";
const string KEYCAP_RETURN    = "\n";

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

struct KeyboardLayer{
  SDL_Surface *surface;
  SDL_Texture *texture;
  list<string> row1;
  list<string> row2;
  list<string> row3;
  list<string> row4;
  vector<touchArea> keyList;
  int layerNum;
};

class Keyboard {

  public:
    /**
      Constructor for Keyboard
      @param pos Starting position (e.g. 0 for hidden)
      @param targetPos Final position (e.g. 1 for max height)
      @param width Width to draw keyboard
      @param height Height to draw keyboard
      @param config Pointer to Config
    */
    Keyboard(int pos, int targetPos, int width,
             int height, Config *config);
    ~Keyboard();
    /**
      Get the character/key at the given coordinates
      @param x X-axis coordinate
      @param y Y-axis coordinate
      @return String with value of key at the given coordinates
    */
    string getCharForCoordinates(int x, int y);
    /**
      Set keyboard color
      @param r Red value
      @param g Green value
      @param b Blue value
    */
    void setKeyboardColor(int r, int g, int b);
    /**
      Get position of keyboard
      @return Position as a value between 0 and 1 (0% and 100%)
    */
    float getPosition();
    /**
      Set keyboard position
      @param p Position between 0 (0%) and 1 (100%)
    */
    void setPosition(float p);
    /**
      Get keyboard target position
      @return Target position of keyboard, between 0 (0%) and 1 (100%)
    */
    float getTargetPosition();
    /**
      Set keyboard target position
      @param p Position between 0 (0%) and 1 (100%)
    */
    void setTargetPosition(float p);
    /**
      Get keyboard height
      @return configured height of keyboard
    */
    float getHeight();
    /**
      Draw/update keyboard on the screen
      @param renderer An initialized SDL_Renderer object
      @param screenHeight Height of screen
    */
    void draw(SDL_Renderer *renderer, int screenHeight);
    /**
      Get the active keyboard layer
      @return Index of active keyboard layer
    */
    int getActiveLayer();
    /**
      Set the active keyboard layer
      @param layerNum Index of layer to activate
    */
    void setActiveLayer(int layerNum);
    /**
      Set layout for keyboard
      @param layoutNum Index of layout
    */
    int setLayout(int layoutNum);
    /**
      Initialize keyboard object
      @param renderer Initialized SDL_Renderer object
      @return 0 on success, non-zero on error
    */
    int init(SDL_Renderer *renderer);

  private:
    rgb keyboardColor;
    rgb inputColor;
    rgb dotColor;
    int keyRadius;
    float position;
    float targetPosition;
    int keyboardWidth;
    int keyboardHeight;
    int activeLayer;
    list<KeyboardLayer> keyboard;
    Config *config;

    /**
      Draw keyboard row
      @param surface Surface to draw on
      @param keyList List of keys for keyboard layout
      @param x X-axis coord. for start of row
      @param y Y-axis coord. for start of row
      @param width Width of row
      @param height Height of row
      @param cap Key cap
      @param key Key text
      @param padding Spacing to reserve around the key
      @param font Font to use for key character
    */
    void drawRow(SDL_Surface *surface, vector<touchArea> *keyList, int x, int y,
                 int width, int height, list<string> *keys, int padding,
                 TTF_Font *font);
    /**
      Draw key for keyboard
      @param surface Surface to draw on
      @param keyList List of keys for keyboard layout
      @param x X-axis coord. for start of row
      @param y Y-axis coord. for start of row
      @param width Width of row
      @param height Height of row
      @param cap Key cap
      @param key Key text
      @param padding Spacing to reserve around the key
      @param font Font to use for key character
    */
    void drawKey(SDL_Surface *surface, vector<touchArea> *keyList, int x, int y,
                 int width, int height, char *cap, string key, int padding,
                 TTF_Font *font);
    /**
      Prepare new keyboard
      @param layer Keyboard layer to use
      @return New SDL_Surface, or NULL on error
    */
    SDL_Surface *makeKeyboard(KeyboardLayer *layer);
    /**
      Load a keymap into the keyboard
      @param keymapPath Path to keymap file
    */
    void loadKeymap(string keymapPath);
};
#endif
