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

  SDL_LogSetAllPriority(SDL_LOG_PRIORITY_INFO);

  if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS |
               SDL_INIT_TIMER ) < 0) {
    SDL_Log("SDL_Init failed: %s", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  if (!opts.testMode) {
    // Switch to the resolution of the framebuffer if not running
    // in test mode.
    SDL_DisplayMode mode = { SDL_PIXELFORMAT_UNKNOWN, 0, 0, 0, 0};
    if(SDL_GetDisplayMode(0, 0, &mode) != 0){
      SDL_Log("SDL_GetDisplayMode failed: %s", SDL_GetError());
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
    fprintf(stderr, "ERROR: Could not create renderer: %s\n", SDL_GetError());
    SDL_Quit();
    exit(1);
  }

  screen = SDL_GetWindowSurface(display);

  if (screen == NULL) {
    fprintf(stderr, "ERROR: Could not get window surface: %s\n", SDL_GetError());
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
    fprintf(stderr, "ERROR: Could not fill background color: %s\n", SDL_GetError());
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

  next_time = SDL_GetTicks() + TICK_INTERVAL;

  // Disable mouse cursor if not in testmode
  if (SDL_ShowCursor(opts.testMode) < 0) {
    fprintf(stderr, "Setting cursor visibility failed: %s\n", SDL_GetError());
    // Not stopping here, this is a pretty recoverable error.
  }

  // Make SDL send text editing events for textboxes
  SDL_StartTextInput();

  SDL_Surface* wallpaper = make_wallpaper(renderer, &config, WIDTH, HEIGHT);
  SDL_Texture* wallpaperTexture = SDL_CreateTextureFromSurface(renderer, wallpaper);

  int offsetYMouse;
  string tapped;

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
          if (passphrase.size() > 0 && luksDev->unlockRunning()){
            luksDev->setPassphrase(strList2str(passphrase));
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
          exit(1);
          break;
        }
        break;
      // handle the mouse/touchscreen
      case SDL_MOUSEBUTTONUP:
        unsigned int xMouse, yMouse;
        xMouse = event.button.x;
        yMouse = event.button.y;
        printf("xMouse: %i\tyMouse: %i\n", xMouse, yMouse);
        offsetYMouse = yMouse - (int)(HEIGHT - (keyboard->getHeight() * keyboard->getPosition()));
        tapped = keyboard->getCharForCoordinates(xMouse, offsetYMouse);
        if (!luksDev->unlockRunning()){
          // return pressed
          if(tapped.compare("\n") == 0){
            luksDev->setPassphrase(strList2str(passphrase));
            luksDev->unlock();
            continue;
          }
          // Backspace pressed
          else if (tapped.compare(KEYCAP_BACKSPACE) == 0){
            if (passphrase.size() > 0){
              passphrase.pop_back();
            }
            continue;
          }
          // Shift pressed
          else if (tapped.compare(KEYCAP_SHIFT) == 0){
            if (keyboard->getActiveLayer() > 1){
              keyboard->setActiveLayer(0);
            }else{
              keyboard->setActiveLayer(!keyboard->getActiveLayer());
            }
          }
          // Symbols key pressed
          else if (tapped.compare(KEYCAP_SYMBOLS) == 0){
            keyboard->setActiveLayer(2);
          }
          // ABC key was pressed
          else if (tapped.compare(KEYCAP_ABC) == 0){
            keyboard->setActiveLayer(0);
          }
          // handle other key presses
          else if (tapped.compare("\0") != 0){
            passphrase.push_back(tapped);
            continue;
          }
        }
        break;
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
          }
        }
        break;
      }
    }
    // Hide keyboard if unlock luks thread is running
    keyboard->setTargetPosition(!luksDev->unlockRunning());

    keyboard->draw(renderer, HEIGHT);

    draw_password_box(renderer, passphrase.size(), HEIGHT, WIDTH, inputHeight,
                       keyboard->getHeight(), keyboard->getPosition());

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
