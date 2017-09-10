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

#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <map>

using namespace std;

class Config {
public:
  Config();
  bool Read(string path);

  string keyboardBackground;
  string wallpaper;
  string keyboardFont;
  string keyboardMap;
  string inputBoxRadius;
  string keyRadius;
private:
  bool Parse(istream & file);

  map<string, string> options;
};

#endif // CONFIG_H
