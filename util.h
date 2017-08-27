#include "SDL2/SDL.h"
#include <unistd.h>
#include <iostream>
#include <string>
#include <list>
#include "config.h"


using namespace std;


const string DEFAULT_LUKSDEVPATH = "/home/user/disk";
const string DEFAULT_LUKSDEVNAME = "root";
const string DEFAULT_CONFPATH = "/etc/osk.conf";


struct Opts{
  string luksDevPath;
  string luksDevName;
  string confPath;
  bool testMode;
};


int fetchOpts(int argc, char **args, Opts *opts);

Uint32 time_left(Uint32 now, Uint32 next_time);

string strList2str(list<string> strList);

SDL_Surface* make_wallpaper(SDL_Renderer *renderer, Config *config,
                            int width, int height);
