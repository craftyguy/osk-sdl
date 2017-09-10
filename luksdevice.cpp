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

#include "luksdevice.h"


using namespace std;


LuksDevice::LuksDevice(){
  this->deviceName = "";
  this->devicePath = "";
  this->passphrase = "";
  this->running = false;
  this->locked = true;
}


LuksDevice::LuksDevice(string deviceName, string devicePath){
  this->deviceName = deviceName;
  this->devicePath = devicePath;
  this->passphrase = "";
  this->running = false;
  this->locked = true;
}


LuksDevice::~LuksDevice(){

}


void LuksDevice::setPassphrase(string passphrase){
  this->passphrase = passphrase;
}


int LuksDevice::unlock(){
  SDL_CreateThread(unlock, "lukscryptdevice_unlock", this);
  return 0;
}


int LuksDevice::unlock(void *luksDev){
  struct crypt_device *cd;
  int ret;
  LuksDevice *lcd = (LuksDevice*) luksDev;

  // Note: no mutex here, since this function makes a blocking call later on.
  // Careful!
  lcd->running = true;

  usleep(MIN_UNLOCK_TIME_MS * 1000);

  // Initialize crypt device
  ret = crypt_init(&cd, lcd->devicePath.c_str());
  if (ret < 0) {
    printf("crypt_init() failed for %s.\n", lcd->devicePath.c_str());
    lcd->running = false;
    return ret;
  }

  // Load header
  ret = crypt_load(cd, CRYPT_LUKS1, NULL);
  if (ret < 0) {
    printf("crypt_load() failed on device %s.\n", crypt_get_device_name(cd));
    crypt_free(cd);
    lcd->running = false;
    return ret;
  }

  ret = crypt_activate_by_passphrase(
                                      cd, lcd->deviceName.c_str(),
                                      CRYPT_ANY_SLOT,
                                      lcd->passphrase.c_str(),
                                      lcd->passphrase.size(),
                                      CRYPT_ACTIVATE_ALLOW_DISCARDS // Enable TRIM support
                                      );
  if (ret < 0){
    printf("crypt_activate_by_passphrase failed on device. Errno %i\n", ret);
    crypt_free(cd);
    lcd->running = false;
    return ret;
  }
  printf("Successfully unlocked device %s\n", lcd->devicePath.c_str());
  crypt_free(cd);
  lcd->locked = false;
  lcd->running = false;
  return 0;
}


bool LuksDevice::isLocked(){
  return this->locked;
}


bool LuksDevice::unlockRunning(){
  return this->running;
}


