#include "SDL2/SDL.h"
#include <iostream>
#include <string>
#include <cstdlib>
#include <unistd.h>
#include <list>
#include "keyboard.h"
#include "luksdevice.h"
#include "config.h"
#include "util.h"
#include "tooltip.h"
#include "draw_helpers.h"


#define TICK_INTERVAL 16


using namespace std;


int main(int argc, char **args) {
  list<string> passphrase;
  static Uint32 next_time;
  Opts opts;
  Config config;
  SDL_Event event;
  SDL_Window *display = NULL;
  SDL_Surface *screen = NULL;
  SDL_Renderer *renderer = NULL;
  int WIDTH = 480;
  int HEIGHT = 800;
  int repeat_delay_ms = 100;    // Keyboard key repeat rate in ms
  int prev_keydown_ticks = 0;   // Two sep. prev_ticks required for handling
  int prev_text_ticks = 0;      // textinput & keydown event types
  int cur_ticks = 0;

  if (fetchOpts(argc, args, &opts)){
    exit(1);
  }

  if(!config.Read(opts.confPath)){
    fprintf(stderr, "No valid config file specified, use -c [path]");
    exit(1);
  }

  // This is a workaround for: https://bugzilla.libsdl.org/show_bug.cgi?id=3751
  putenv(const_cast<char *>("SDL_DIRECTFB_LINUX_INPUT=1"));

  LuksDevice *luksDev = new LuksDevice(opts.luksDevName, opts.luksDevPath);

  if (opts.verbose){
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);
  }else{
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_ERROR);
  }

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS |
               SDL_INIT_TIMER | SDL_INIT_JOYSTICK) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_ERROR, "SDL_Init failed: %s", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  if (!opts.testMode) {
    // Switch to the resolution of the framebuffer if not running
    // in test mode.
    SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0};
    if(SDL_GetDisplayMode(0, 0, &mode) != 0){
      SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "SDL_GetDisplayMode failed: %s",
                   SDL_GetError());
      SDL_Quit();
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
    SDL_Quit();
    exit(1);
  }

  renderer = SDL_CreateRenderer(display, -1, SDL_RENDERER_SOFTWARE);

  if (renderer == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "ERROR: Could not create renderer: %s\n",
                 SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  screen = SDL_GetWindowSurface(display);

  if (screen == NULL) {
    SDL_LogError(SDL_LOG_CATEGORY_VIDEO, "ERROR: Could not get window surface: %s\n",
                 SDL_GetError());
    SDL_Quit();
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
    SDL_Quit();
    exit(1);
  }

  // Initialize virtual keyboard
  Keyboard *keyboard = new Keyboard(0, 1, WIDTH, keyboardHeight, &config);
  keyboard->setKeyboardColor(30, 30, 30);
  if (keyboard->init(renderer)){
    fprintf(stderr, "ERROR: Failed to initialize keyboard!\n");
    SDL_Quit();
    exit(1);
  }

  // Initialize tooltip for password error
  string ErrorText ="The password is incorrect";
  Tooltip *tooltip = new Tooltip(WIDTH*0.9, inputHeight, &config);
  tooltip->init(renderer, ErrorText);

  next_time = SDL_GetTicks() + TICK_INTERVAL;

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
  bool showPasswordError = false;
  int lastUnlockingState = false;
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
  SDL_Rect inputRect;
  while (luksDev->isLocked()) {
    SDL_RenderCopy(renderer, wallpaperTexture, NULL, NULL);
    while (SDL_PollEvent(&event)) {
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
          showPasswordError = false;
          if (passphrase.size() > 0 && !luksDev->unlockRunning()){
              passphrase.pop_back();
              continue;
          }
          break;
        case SDLK_ESCAPE:
          exit(1);
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
        showPasswordError = false;
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
        showPasswordError = false;
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
        showPasswordError = false;
        break;
      }
    }
    // Hide keyboard if unlock luks thread is running
    keyboard->setTargetPosition(!luksDev->unlockRunning());

    keyboard->draw(renderer, HEIGHT);

    if(lastUnlockingState != luksDev->unlockRunning()){
      if(luksDev->unlockRunning() == false && luksDev->isLocked()){
        // Luks is finished and the password was wrong
        showPasswordError = true;
        passphrase.clear();
      }
      lastUnlockingState = luksDev->unlockRunning();
    }

    int topHalf = (HEIGHT - (keyboard->getHeight() * keyboard->getPosition()));
    int passwordPosition;
    int tooltipPosition;
    if(showPasswordError){
      passwordPosition = topHalf / 3 * 2;
      tooltipPosition = topHalf / 3;
    }else{
      passwordPosition = topHalf / 3;
    }

    draw_password_box(renderer, passphrase.size(), HEIGHT, WIDTH, inputHeight,
                      passwordPosition, luksDev->unlockRunning());

    if(showPasswordError){
      tooltip->draw(renderer, WIDTH/20, tooltipPosition);
    }

    if(inputBoxRadius > 0){
      int topHalf = HEIGHT - (keyboard->getHeight() * keyboard->getPosition());
      inputRect.x = WIDTH / 20;
      inputRect.y = (topHalf / 2) - (inputHeight / 2);
      inputRect.w = WIDTH * 0.9;
      inputRect.h = inputHeight;
      smooth_corners_renderer(renderer,&wallpaperColor,&inputRect,inputBoxRadius);
    }

    SDL_Delay(time_left(SDL_GetTicks(), next_time));
    next_time += TICK_INTERVAL;
    // Update
    SDL_RenderPresent(renderer);
  }
  SDL_Quit();
  delete keyboard;
  delete luksDev;
  return 0;
}
