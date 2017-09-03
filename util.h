#ifndef UTIL_H
#define UTIL_H
#include "SDL2/SDL.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include <list>
#include "config.h"
#include "keyboard.h"
#include "luksdevice.h"
#include <math.h>
#include <functional>

using namespace std;


const string DEFAULT_LUKSDEVPATH = "/home/user/disk";
const string DEFAULT_LUKSDEVNAME = "root";
const string DEFAULT_CONFPATH = "/etc/osk.conf";
const float  BEZIER_RESOLUTION = 50;

struct Opts{
  string luksDevPath;
  string luksDevName;
  string confPath;
  bool testMode;
};


/**
  Fetch command line options
  @param argc Number of arguments passed on cmdline
  @param args Arguments from cmdline
  @param opts Structure for storing options from cmdline
  @return 0 on success, non-zero on failure
*/
int fetchOpts(int argc, char **args, Opts *opts);

/**
  Calculate difference between current time and future time
  @param now Time now
  @param next_time Future time
  @return Difference between times
*/
Uint32 time_left(Uint32 now, Uint32 next_time);

/**
  Convert list of strings into a single string
  @param strList List of strings
  @return String with all elements of strList concatenated together
*/
string strList2str(const list<string> *strList);

/**
  Create wallpaper
  @param renderer Initialized SDL_Renderer
  @param config Config paramters
  @param width Width of wallpaper to generate
  @param height Height of wallpaper to generate
  @return Initialized SDL_Surface, else NULL on failure
*/
SDL_Surface* make_wallpaper(SDL_Renderer *renderer, Config *config,
                            int width, int height);

/**
  Draw a circle
  @param renderer Initialized SDL_Renderer object
  @param center Center position of circle
  @param radius Radius of circle
*/
void draw_circle(SDL_Renderer *renderer, SDL_Point center, int radius);

/**
  Draw input box for passwords
  @param renderer Initialized SDL_Renderer object
  @param numDots Number of password 'dots' to draw
  @param screenHeight Height of overall screen
  @param screenWidth Width of overall screen
  @param inputHeight Height of input box
  @param keyboardHeight Height of associated keyboard
  @param keyboardPos Position of associated keyboard, between 0 (0%) and 1 (100%)
*/
void draw_password_box(SDL_Renderer *renderer, int numDots, int screenHeight,
                       int screenWidth, int inputHeight, int keyboardHeight,
                       float keyboardPos, bool busy);

/**
  Handle keypresses for virtual keyboard
  @param tapped Character tapped on keyboard
  @param kbd Initialized Keyboard obj
  @param lkd Initialized LuksDevice obj
  @param lkd passphrase Passphrase to modify
*/
void handleVirtualKeyPress(string tapped, Keyboard *kbd, LuksDevice *lkd,
                           list<string> *passphrase);
/**
  Curve the corneres of a rectange
  @param rect the rectange to smooth
  @param radius the distance from a corner where the curve will start
  @param draw_cb callback, with coordinates to the next pixel to draw
*/
void smooth_corners(SDL_Rect *rect, int radius,function<void(int,int)> draw_cb);

/**
  Draw rounded corneres for a rectangle directly onto a surface
  @param surface the surface to draw on
  @param color the color to draw
  @param rect the rectange to smooth
  @param radius the distance from a corner where the curve will start
*/
void smooth_corners_surface(SDL_Surface*surface,Uint32 color,SDL_Rect*rect,int radius);

/**
  Draw rounded corneres for a rectangle with a renderer
  @param renderer the renderer to draw on
  @param color the color to draw
  @param rect the rectange to smooth
  @param radius the distance from a corner where the curve will start
*/
void smooth_corners_renderer(SDL_Renderer*renderer,argb*color,SDL_Rect*rect,int radius);

#endif