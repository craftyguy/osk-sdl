#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <map>

using namespace std;

class Config {
public:
  Config();
  bool Read(string path);
private:
  bool Parse(istream & file);

  map<string, string> options;
};

#endif // CONFIG_H
