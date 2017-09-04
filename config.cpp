#include "config.h"
#include <fstream>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

Config::Config() {
  Config::wallpaper = "#FF9900";
  Config::keyboardBackground = "#333333";
  Config::keyboardMap = "us";
  Config::keyboardFont = "DejaVu";
  Config::allowMargins = "true";
}

bool Config::Read(string path) {
  std::ifstream is(path, std::ifstream::binary);
  if (!is) {
    fprintf(stderr, "Could not open config file: %s\n", path.c_str());
    return false;
  }
  if (!Config::Parse(is)) {
    fprintf(stderr, "Could not parse config file: %s\n", path.c_str());
    return false;
  }

  auto it = Config::options.find("wallpaper");
  if (it != Config::options.end()) {
    Config::wallpaper = Config::options["wallpaper"];
  }

  it = Config::options.find("keyboard-background");
  if (it != Config::options.end()) {
    Config::keyboardBackground = Config::options["keyboard-background"];
  }

  it = Config::options.find("keyboard-font");
  if (it != Config::options.end()) {
    Config::keyboardFont = Config::options["keyboard-font"];
  }

  it = Config::options.find("keyboard-map");
  if (it != Config::options.end()) {
    Config::keyboardMap = Config::options["keyboard-map"];
  }

  it = Config::options.find("key-radius");
  if (it != Config::options.end()) {
    Config::keyRadius = Config::options["key-radius"];
  }

  it = Config::options.find("inputbox-radius");
  if (it != Config::options.end()) {
    Config::inputBoxRadius = Config::options["inputbox-radius"];
  }
  it = Config::options.find("allow-wallpaper-margins");
  if (it != Config::options.end()) {
    Config::allowMargins = Config::options["allow-wallpaper-margins"];
  }
  return true;
}

bool Config::Parse(istream &file) {
  int lineno = 0;
  for (std::string line; std::getline(file, line);) {
    lineno++;

    std::istringstream iss(line);
    std::string id, eq, val;

    bool error = false;

    if (!(iss >> id)) {
      continue;
    } else if (id[0] == '#') {
      continue;
    } else if (id == "") {
      continue;
    } else if (!(iss >> eq >> val >> std::ws) || eq != "=" ||
               iss.get() != EOF) {
      error = true;
    }

    if (error) {
      fprintf(stderr, "Syntax error on line %d\n", lineno);
      return false;
    } else {
      Config::options[id] = val;
    }
  }
  return true;
}
