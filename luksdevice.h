#ifndef LUKSDEVICE_H
#define LUKSDEVICE_H
#include "SDL2/SDL_thread.h"
#include <libcryptsetup.h>
#include <iostream>
#include <string>
#include <unistd.h>

using namespace std;

const int MIN_UNLOCK_TIME_MS  = 1000;


class LuksDevice{
  public:

    /**
      Default constructor
    */
    LuksDevice();
    /**
      Constructor
      @param deviceName Name of luks device
      @param devicePath Path to luks device
    */
    LuksDevice(string deviceName, string devicePath);
    ~LuksDevice();
    /**
      Unlock luks device
      @return 0 on success, non-zero on failure
    */
    int unlock();
    /**
      Query luks device lock status
      @return Bool indicating whether luks device is locked or not
    */
    bool isLocked();
    /**
      Query luks device unlocking status
      @return Bool indicating that unlock thread is running or not
    */
    bool unlockRunning();
    /**
      Configure passphrase for luks device
      @param passphrase Passphrase to pass to luks device when activating it
    */
    void setPassphrase(string passphrase);

  private:
    string devicePath;
    string deviceName;
    string passphrase;
    bool locked;
    bool running;

    /**
      Unlock luks device
      @param luksDev LuksDevice object to use, should represent 'this'
    */
    static int unlock(void *luksDev);

};
#endif
