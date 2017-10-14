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

#include "SDL2/SDL.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <list>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include "keyboard.h"
#include "luksdevice.h"
#include "config.h"
#include "util.h"
#include "tooltip.h"
#include "draw_helpers.h"


#define TICK_INTERVAL 16


using namespace std;

struct uiRenderData {
  SDL_Renderer *renderer;
  Keyboard *keyboard;
  Tooltip *tooltip;
  list<string> *passphrase;
  LuksDevice *luksDev;
  SDL_Texture* wallpaperTexture;
  argb *wallpaperColor;
  int HEIGHT;
  int WIDTH;
  int inputHeight;
  int inputBoxRadius;
  bool backbuf_wa;
};

bool lastUnlockingState = false;
bool showPasswordError = false;

SDL_mutex *renderMutex;

Uint32 uiRenderCB(Uint32 i, void *data){
  const uiRenderData *urd = (uiRenderData*) data;
  int topHalf;
  int passwordPosition;
  int tooltipPosition;

  SDL_RenderCopy(urd->renderer, urd->wallpaperTexture, NULL, NULL);
  // Hide keyboard if unlock luks thread is running
  urd->keyboard->setTargetPosition(!urd->luksDev->unlockRunning());
  urd->keyboard->draw(urd->renderer, urd->HEIGHT);

  SDL_LockMutex(renderMutex);
  if(lastUnlockingState != urd->luksDev->unlockRunning()){
    if(urd->luksDev->unlockRunning() == false && urd->luksDev->isLocked()){
      // Luks is finished and the password was wrong
      showPasswordError = true;
      urd->passphrase->clear();
    }
    lastUnlockingState = urd->luksDev->unlockRunning();
  }
  SDL_UnlockMutex(renderMutex);

  topHalf = (urd->HEIGHT - (urd->keyboard->getHeight() * urd->keyboard->getPosition()));

  // Only show either error box or password input box, not both
  if(showPasswordError){
    tooltipPosition = topHalf / 4;
    urd->tooltip->draw(urd->renderer, urd->WIDTH/20, tooltipPosition);
  }else{
   passwordPosition = topHalf / 3.5;
   draw_password_box(urd->renderer, urd->passphrase->size(), urd->HEIGHT,
                    urd->WIDTH, urd->inputHeight, passwordPosition,
                    urd->wallpaperColor, urd->inputBoxRadius,
                    urd->luksDev->unlockRunning());
  }

  SDL_RenderPresent(urd->renderer);

  // Workaround for double buffering FB implementation on some devices
  if (urd->backbuf_wa){
    struct fb_var_screeninfo vinfo;
    int fbd;
    FILE *fbf = fopen("/dev/fb0", "r+");
    if (fbf == NULL) {
        fprintf(stderr,"Error: cannot open framebuffer device\n");
        return 0;
    }
    fbd = fileno(fbf);
    // Get variable screen information
    if (ioctl(fbd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
        fprintf(stderr, "Error reading screen info\n");
        return 0;
    }
    // Put variable screen information
    if (ioctl(fbd, FBIOPUT_VSCREENINFO, &vinfo) == -1) {
        fprintf(stderr, "Error applying screen info\n");
        return 0;
    }
    fclose(fbf);
  }

  return i;
}


int main(int argc, char **args) {
  list<string> passphrase;
  Opts opts;
  Config config;
  SDL_Event event;
  SDL_Window *display = NULL;
  SDL_Surface *screen = NULL;
  SDL_Renderer *renderer = NULL;
  SDL_TimerID uiRenderTimerID = 0;
  Tooltip *tooltip = NULL;
  uiRenderData urd;
  int WIDTH = 480;
  int HEIGHT = 800;
  int repeat_delay_ms = 100;    // Keyboard key repeat rate in ms
  int prev_keydown_ticks = 0;   // Two sep. prev_ticks required for handling
  int prev_text_ticks = 0;      // textinput & keydown event types
  int cur_ticks = 0;
  string ErrorText = "Incorrect passphrase";



  renderMutex = SDL_CreateMutex();
  if (!renderMutex){
    fprintf(stderr, "Unable to initialize rendering mutex\n");
    exit(1);
  }

  if (fetchOpts(argc, args, &opts)){
    exit(1);
  }

  if(!config.Read(opts.confPath)){
    fprintf(stderr, "No valid config file specified, use -c [path]");
    exit(1);
  }

  LuksDevice *luksDev = new LuksDevice(opts.luksDevName, opts.luksDevPath);

  if (opts.verbose){
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
  }else{
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_ERROR);
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS |
               SDL_INIT_TIMER) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed: %s", SDL_GetError());
    atexit(SDL_Quit);
    exit(1);
  }

  if (!opts.testMode) {
    // Switch to the resolution of the framebuffer if not running
    // in test mode.
    SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0};
    if(SDL_GetDisplayMode(0, 0, &mode) != 0){
      SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "SDL_GetDisplayMode failed: %s",
                   SDL_GetError());
      atexit(SDL_Quit);
      exit(1);
    }
    WIDTH = mode.w;
    HEIGHT = mode.h;
  }

  /*
   * Set up display, renderer, & screen
   * Use windowed mode in test mode and device resolution otherwise
   */
  int windowFlags = 0;
  if (opts.testMode) {
    windowFlags = SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN;
  } else {
    windowFlags = SDL_WINDOW_FULLSCREEN;
  }

  display = SDL_CreateWindow("OSK SDL", SDL_WINDOWPOS_UNDEFINED,
                             SDL_WINDOWPOS_UNDEFINED, WIDTH,
                             HEIGHT, windowFlags);
  if (display == NULL) {
    fprintf(stderr, "ERROR: Could not create window/display: %s\n", SDL_GetError());
    atexit(SDL_Quit);
    exit(1);
  }

  renderer = SDL_CreateRenderer(display, -1, SDL_RENDERER_SOFTWARE);

  if (renderer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "ERROR: Could not create renderer: %s\n",
                 SDL_GetError());
    atexit(SDL_Quit);
    exit(1);
  }

  screen = SDL_GetWindowSurface(display);

  if (screen == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "ERROR: Could not get window surface: %s\n",
                 SDL_GetError());
    atexit(SDL_Quit);
    exit(1);
  }

  int keyboardHeight = HEIGHT / 3 * 2;
  if (HEIGHT > WIDTH) {
    // Keyboard height is screen width / max number of keys per row * rows
    keyboardHeight = WIDTH / 2;
  }

  int inputHeight = WIDTH / 10;
  auto backgroundColor = SDL_MapRGB(screen->format, 255, 128, 0);

  if (SDL_FillRect(screen, NULL, backgroundColor) != 0) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "ERROR: Could not fill background color: %s\n", SDL_GetError());
    atexit(SDL_Quit);
    exit(1);
  }

  // Initialize virtual keyboard
  Keyboard *keyboard = new Keyboard(0, 1, WIDTH, keyboardHeight, &config);
  keyboard->setKeyboardColor(30, 30, 30);
  if (keyboard->init(renderer)){
    fprintf(stderr, "ERROR: Failed to initialize keyboard!\n");
    atexit(SDL_Quit);
    exit(1);
  }

  // Initialize tooltip for password error
  tooltip = new Tooltip(WIDTH*0.9, inputHeight, &config);
  tooltip->init(renderer, ErrorText);

  // Disable mouse cursor if not in testmode
  if (SDL_ShowCursor(opts.testMode) < 0) {
    fprintf(stderr, "Setting cursor visibility failed: %s\n", SDL_GetError());
    // Not stopping here, this is a pretty recoverable error.
  }

  // Make SDL send text editing events for textboxes
  SDL_StartTextInput();

  SDL_Surface* wallpaper = make_wallpaper(renderer, &config, WIDTH, HEIGHT);
  SDL_Texture* wallpaperTexture = SDL_CreateTextureFromSurface(renderer,
                                                               wallpaper);

  string tapped;
  long inputBoxRadius = strtol(config.inputBoxRadius.c_str(),NULL,10);
  if(inputBoxRadius >= BEZIER_RESOLUTION || inputBoxRadius > inputHeight/1.5){
    fprintf(stderr,"inputbox-radius must be below %f and %f, it is %ld\n",
            BEZIER_RESOLUTION, inputHeight/1.5, inputBoxRadius);
    inputBoxRadius = 0;
  }
  argb wallpaperColor;
  wallpaperColor.a = 255;
  if(sscanf(config.wallpaper.c_str(), "#%02x%02x%02x",
            &wallpaperColor.r, &wallpaperColor.g, &wallpaperColor.b)!=3){
      fprintf(stderr, "Could not parse color code %s\n",
              config.wallpaper.c_str());
      //to avoid akward colors just remove the radius
      inputBoxRadius = 0;
  }

  //Set up and start render callback
  urd.keyboard = keyboard;
  urd.renderer = renderer;
  urd.tooltip = tooltip;
  urd.wallpaperTexture = wallpaperTexture;
  urd.passphrase = &passphrase;
  urd.luksDev = luksDev;
  urd.wallpaperColor = &wallpaperColor;
  urd.HEIGHT = HEIGHT;
  urd.WIDTH = WIDTH;
  urd.inputHeight = inputHeight;
  urd.inputBoxRadius = inputBoxRadius;
  urd.backbuf_wa = opts.backbuf_wa;

  uiRenderTimerID = SDL_AddTimer(TICK_INTERVAL, uiRenderCB, &urd);
  if (uiRenderTimerID == 0){
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "ERROR: Could not start render timer: %s\n",
                 SDL_GetError());
    atexit(SDL_Quit);
    exit(1);
  }

  while (luksDev->isLocked()) {
    if (SDL_WaitEvent(&event)) {
      cur_ticks = SDL_GetTicks();
      // an event was found
      switch (event.type) {
      // handle the keyboard
      case SDL_KEYDOWN:
        // handle repeat key events
        if ((cur_ticks - repeat_delay_ms) < prev_keydown_ticks){
          continue;
        }
        prev_keydown_ticks = cur_ticks;
        switch (event.key.keysym.sym) {
        case SDLK_RETURN:
          if (passphrase.size() > 0 && !luksDev->unlockRunning()){
            luksDev->setPassphrase(strList2str(&passphrase));
            luksDev->unlock();
          }
          break;
        case SDLK_BACKSPACE:
          if (passphrase.size() > 0 && !luksDev->unlockRunning()){
              passphrase.pop_back();
              continue;
          }
          break;
        case SDLK_ESCAPE:
          goto QUIT;
          break;
        }
        break;
      // handle touchscreen
      case SDL_FINGERUP:
        unsigned int xTouch, yTouch, offsetYTouch;
        // x and y values are normalized!
        xTouch = event.tfinger.x * WIDTH;
        yTouch = event.tfinger.y * HEIGHT;
        if (opts.verbose)
          printf("xTouch: %i\tyTouch: %i\n", xTouch, yTouch);
        offsetYTouch = yTouch - (int)(HEIGHT - (keyboard->getHeight() * keyboard->getPosition()));
        tapped = keyboard->getCharForCoordinates(xTouch, offsetYTouch);
        if (!luksDev->unlockRunning()){
          handleVirtualKeyPress(tapped, keyboard, luksDev, &passphrase);
        }
        break;
      // handle the mouse
      case SDL_MOUSEBUTTONUP:
        unsigned int xMouse, yMouse, offsetYMouse;
        xMouse = event.button.x;
        yMouse = event.button.y;
        if (opts.verbose)
          printf("xMouse: %i\tyMouse: %i\n", xMouse, yMouse);
        offsetYMouse = yMouse - (int)(HEIGHT - (keyboard->getHeight() * keyboard->getPosition()));
        tapped = keyboard->getCharForCoordinates(xMouse, offsetYMouse);
        if (!luksDev->unlockRunning()){
          handleVirtualKeyPress(tapped, keyboard, luksDev, &passphrase);
        }
        break;
      // handle physical keyboard
      case SDL_TEXTINPUT:
        /*
         * Only register text input if time since last text input has exceeded
         * the keyboard repeat delay rate
         */
        // Enable key repeat delay
        if ((cur_ticks - repeat_delay_ms) > prev_text_ticks){
          prev_text_ticks = cur_ticks;
          if (!luksDev->unlockRunning()){
            passphrase.push_back(event.text.text);
            if (opts.verbose)
              printf("Phys Keyboard Key Entered %s\n", event.text.text);
          }
        }
        break;
      }
      SDL_LockMutex(renderMutex);
      showPasswordError = false;
      SDL_UnlockMutex(renderMutex);
    }
  }

QUIT:
  SDL_RemoveTimer(uiRenderTimerID);
  SDL_DestroyMutex(renderMutex);
  SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
  atexit(SDL_Quit);
  delete keyboard;
  delete luksDev;
  return 0;
}
