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
  string keyboardBackground = "#333333";
  string wallpaper = "#FF9900";
  string keyboardFont = "DejaVu";
  string keyboardMap = "us";
  string inputBoxRadius = "0";
  string keyRadius = "0";

  /**
    Constructor for Config
    */
  Config();
   /**
    Read from config file
    @path Path to config file
    */
  bool Read(string path);
private:
  map<string, string> options;

  /**
    Parse configuration file
    @file File to parse
    */
  bool Parse(istream & file);
};

#endif // CONFIG_H
