#include "SDL2/SDL_thread.h"
#include <libcryptsetup.h>
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

const int MIN_UNLOCK_TIME_MS  = 1000;


class LuksDevice{
  public:

    LuksDevice();
    LuksDevice(string deviceName, string devicePath);
    ~LuksDevice();
    int unlock();
    bool isLocked();
    bool unlockRunning();
    void setPassphrase(string passphrase);

  private:
    string devicePath;
    string deviceName;
    string passphrase;
    bool locked;
    bool running;

    static int unlock(void *luksDev);

};
