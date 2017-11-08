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

#ifndef LUKSDEVICE_H
#define LUKSDEVICE_H
#include "SDL2/SDL.h"
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
    Constructor
    @param deviceName Name of luks device
    @param devicePath Path to luks device
    */
  LuksDevice(const string *devName, const string *devPath);
  /**
    Destructor
    */
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
  void setPassphrase(string *passphrase);

private:
  string devicePath;
  string deviceName;
  string passphrase = "";
  bool locked = true;
  bool running = false;

  /**
    Unlock luks device
    @param luksDev LuksDevice object to use, should represent 'this'
    */
  static int unlock(void *luksDev);

};
#endif
