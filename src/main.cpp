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
#include <cmath>
#include <cstdlib>
#include <unistd.h>
#include <list>
#include "keyboard.h"
#include "luksdevice.h"
#include "config.h"
#include "util.h"
#include "tooltip.h"
#include "draw_helpers.h"


#define TICK_INTERVAL 32


using namespace std;

Uint32 EVENT_RENDER;
bool lastUnlockingState = false;
bool showPasswordError = false;
static string ErrorText = "Incorrect passphrase";


int main(int argc, char **args) {
  list<string> passphrase;
  Opts opts;
  Config config;
  SDL_Event event;
  SDL_Window *display = NULL;
  SDL_Surface *screen = NULL;
  SDL_Renderer *renderer = NULL;
  Tooltip *tooltip = NULL;
  int WIDTH = 480;
  int HEIGHT = 800;
  int repeat_delay_ms = 100;    // Keyboard key repeat rate in ms
  int prev_keydown_ticks = 0;   // Two sep. prev_ticks required for handling
  int prev_text_ticks = 0;      // textinput & keydown event types
  int cur_ticks = 0;

  static SDL_Event renderEvent{
    .type = EVENT_RENDER
  };

  if (fetchOpts(argc, args, &opts)){
    exit(1);
  }

  if(!config.Read(opts.confPath)){
    fprintf(stderr, "No valid config file specified, use -c [path]");
    exit(1);
  }

  LuksDevice *luksDev = new LuksDevice(&opts.luksDevName, &opts.luksDevPath);

  if (opts.verbose){
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
  }else{
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_ERROR);
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER) < 0) {
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
    fprintf(stderr, "ERROR: Could not create window/display: %s\n",
            SDL_GetError());
    atexit(SDL_Quit);
    exit(1);
  }

  renderer = SDL_CreateRenderer(display, -1, SDL_RENDERER_SOFTWARE);

  if (renderer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                "ERROR: Could not create renderer: %s\n", SDL_GetError());
    atexit(SDL_Quit);
    exit(1);
  }

  screen = SDL_GetWindowSurface(display);

  if (screen == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                "ERROR: Could not get window surface: %s\n", SDL_GetError());
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
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                "ERROR: Could not fill background color: %s\n",
                SDL_GetError());
    atexit(SDL_Quit);
    exit(1);
  }

  // Initialize virtual keyboard
  Keyboard *keyboard = new Keyboard(0, 1, WIDTH, keyboardHeight, &config);
  keyboard->setKeyboardColor(0, 30, 30, 30);
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
  long inputBoxRadius = strtol(config.inputBoxRadius.c_str(), NULL, 10);
  if(inputBoxRadius >= BEZIER_RESOLUTION || inputBoxRadius > inputHeight / 1.5){
    fprintf(stderr, "inputbox-radius must be below %f and %f, it is %ld\n",
            BEZIER_RESOLUTION, inputHeight / 1.5, inputBoxRadius);
    inputBoxRadius = 0;
  }
  argb wallpaperColor;
  wallpaperColor.a = 255;
  if(sscanf(config.wallpaper.c_str(), "#%02x%02x%02x", &wallpaperColor.r, &wallpaperColor.g,
     &wallpaperColor.b) != 3){

      fprintf(stderr, "Could not parse color code %s\n",
              config.wallpaper.c_str());
      //to avoid akward colors just remove the radius
      inputBoxRadius = 0;
  }

  argb inputBoxColor = argb{255, 255, 255, 255};

  SDL_Surface* inputBox = make_input_box(WIDTH * 0.9, inputHeight,
                                        &inputBoxColor, inputBoxRadius);
  SDL_Texture* inputBoxTexture = SDL_CreateTextureFromSurface(renderer,
                                                              inputBox);

  int topHalf = (HEIGHT - (keyboard->getHeight() * keyboard->getPosition()));
  SDL_Rect inputBoxRect = SDL_Rect{
    .x = WIDTH / 20,
      .y = (int)(topHalf / 3.5),
      .w = (int)((double)WIDTH * 0.9),
      .h = inputHeight
  };

  if(inputBoxTexture == NULL){
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO,
                "ERROR: Could not create input box texture: %s\n",
                SDL_GetError());
    atexit(SDL_Quit);
    exit(1);
  }

  // Start drawing keyboard when main loop starts
  SDL_PushEvent(&renderEvent);

  // The Main Loop.
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
          showPasswordError = false;
          prev_keydown_ticks = cur_ticks;
          switch (event.key.keysym.sym) {
            case SDLK_RETURN:
              if (!passphrase.empty() && !luksDev->unlockRunning()){
                string pass = strList2str(&passphrase);
                luksDev->setPassphrase(&pass);
                luksDev->unlock();
              }
              break; // SDLK_RETURN
            case SDLK_BACKSPACE:
              if (!passphrase.empty() && !luksDev->unlockRunning()){
                passphrase.pop_back();
                SDL_PushEvent(&renderEvent);
                continue;
              }
              break; // SDLK_BACKSPACE
            case SDLK_ESCAPE:
              goto QUIT;
              break; // SDLK_ESCAPE
          }
          SDL_PushEvent(&renderEvent);
          break; // SDL_KEYDOWN
          // handle touchscreen
        case SDL_FINGERUP:
          unsigned int xTouch, yTouch, offsetYTouch;
          showPasswordError = false;
          // x and y values are normalized!
          xTouch = event.tfinger.x * WIDTH;
          yTouch = event.tfinger.y * HEIGHT;
          if (opts.verbose)
            printf("xTouch: %u\tyTouch: %u\n", xTouch, yTouch);
          offsetYTouch = yTouch - (int)(HEIGHT - (keyboard->getHeight() *
                                                  keyboard->getPosition()));
          tapped = keyboard->getCharForCoordinates(xTouch, offsetYTouch);
          if (!luksDev->unlockRunning()){
            handleVirtualKeyPress(tapped, keyboard, luksDev, &passphrase);
          }
          SDL_PushEvent(&renderEvent);
          break; // SDL_FINGERUP
          // handle the mouse
        case SDL_MOUSEBUTTONUP:
          unsigned int xMouse, yMouse, offsetYMouse;
          showPasswordError = false;
          xMouse = event.button.x;
          yMouse = event.button.y;
          if (opts.verbose)
            printf("xMouse: %u\tyMouse: %u\n", xMouse, yMouse);
          offsetYMouse = yMouse - (int)(HEIGHT - (keyboard->getHeight() *
                                                  keyboard->getPosition()));
          tapped = keyboard->getCharForCoordinates(xMouse, offsetYMouse);
          if (!luksDev->unlockRunning()){
            handleVirtualKeyPress(tapped, keyboard, luksDev, &passphrase);
          }
          SDL_PushEvent(&renderEvent);
          break; // SDL_MOUSEBUTTONUP
          // handle physical keyboard
        case SDL_TEXTINPUT:
          /*
           * Only register text input if time since last text input has exceeded
           * the keyboard repeat delay rate
           */
          showPasswordError = false;
          // Enable key repeat delay
          if ((cur_ticks - repeat_delay_ms) > prev_text_ticks){
            prev_text_ticks = cur_ticks;
            if (!luksDev->unlockRunning()){
              passphrase.push_back(event.text.text);
              if (opts.verbose)
                printf("Phys Keyboard Key Entered %s\n", event.text.text);
            }
          }
          SDL_PushEvent(&renderEvent);
          break; // SDL_TEXTINPUT
      } // switch event.type
      // Render event handler
      if (event.type == EVENT_RENDER){
        int topHalf;

        SDL_RenderCopy(renderer, wallpaperTexture, NULL, NULL);
        // Hide keyboard if unlock luks thread is running
        keyboard->setTargetPosition(!luksDev->unlockRunning());
        keyboard->draw(renderer, HEIGHT);

        if(lastUnlockingState != luksDev->unlockRunning()){
          if(luksDev->unlockRunning() == false && luksDev->isLocked()){
            // Luks is finished and the password was wrong
            showPasswordError = true;
            passphrase.clear();
            SDL_PushEvent(&renderEvent);
          }
          lastUnlockingState = luksDev->unlockRunning();
        }
        topHalf = (HEIGHT - (keyboard->getHeight() * keyboard->getPosition()));
        // Only show either error box or password input box, not both
        if(showPasswordError){
          int tooltipPosition = topHalf / 4;
          tooltip->draw(renderer, WIDTH / 20, tooltipPosition);
        }else{
          inputBoxRect.y = (int)(topHalf / 3.5);
          SDL_RenderCopy(renderer, inputBoxTexture, NULL, &inputBoxRect);
          draw_password_box_dots(renderer, inputHeight, WIDTH,
                                passphrase.size(), inputBoxRect.y,
                                luksDev->unlockRunning());
        }
        SDL_RenderPresent(renderer);
        // If any animations are running, continue to push render events to the
        // event queue
        bool keyboardAnimate = (int)floor(keyboard->getTargetPosition()*100) !=
          (int)floor(keyboard->getPosition()*100);
        if (luksDev->unlockRunning() || keyboardAnimate){
          SDL_PushEvent(&renderEvent);
        }
      }
    } // event handle loop
  }   // main loop

QUIT:
  SDL_QuitSubSystem(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_TIMER);
  atexit(SDL_Quit);
  delete keyboard;
  delete luksDev;
  return 0;
}
