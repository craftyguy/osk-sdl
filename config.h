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
  string allowMargins;
private:
  bool Parse(istream & file);

  map<string, string> options;
};

#endif // CONFIG_H
