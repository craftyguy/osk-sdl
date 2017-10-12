/*
Copyright (C) 2017  Ian Shehadeh, Martijn Braam & Clayton Craft <clayton@craftyguy.net>

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

#include "draw_helpers.h"

SDL_Point* bezier_corner (SDL_Point*pts, SDL_Point*offset,SDL_Point *p1, SDL_Point *p2, SDL_Point *p3) {
  int i = 0;
  static float increment = 1/BEZIER_RESOLUTION;

  for(double t = increment; t <= 1.0; t += increment){
    pts[i].x = ((1-t)*(1-t)*p1->x+2*(1-t)*t*p2->x+t*t*p3->x) + offset->x;
    pts[i].y = ((1-t)*(1-t)*p1->y+2*(1-t)*t*p2->y+t*t*p3->y) + offset->y;
    ++i;
  }
  return pts;
}

void smooth_corners(SDL_Rect *rect, int radius,function<void(int,int)> draw_cb){
    SDL_Point* corner = (SDL_Point*)malloc(sizeof(SDL_Point)*BEZIER_RESOLUTION);
    //Top Left
    bezier_corner(corner, new SDL_Point{rect->x-1,rect->y-1}, new SDL_Point{0,radius},
      new SDL_Point{0,0},new SDL_Point{radius,0});
    for(int i = 0; i < BEZIER_RESOLUTION; i++){
      for(int x =rect->x; x < corner[i].x; x++){
        draw_cb(x,corner[i].y);
      }
    }

    //Top Right
    bezier_corner(corner, new SDL_Point{rect->x + rect->w+1,rect->y-1}, new SDL_Point{0,radius},
      new SDL_Point{0,0},new SDL_Point{-radius,0});
    for(int i = 0; i < BEZIER_RESOLUTION; i++){
      for(int x = rect->x+rect->w; x > corner[i].x; x--){
        draw_cb(x,corner[i].y);
      }
    }

    //Bottom Left
    bezier_corner(corner, new SDL_Point{rect->x-1,rect->y + rect->h+1}, new SDL_Point{0,-radius},
      new SDL_Point{0,0},new SDL_Point{radius,0});
    for(int i = 0; i < BEZIER_RESOLUTION; i++){
      for(int x =rect->x; x < corner[i].x; x++){
        draw_cb(x,corner[i].y);
      }
    }

    //Bottom Right
    bezier_corner(corner, new SDL_Point{rect->x + rect->w + 1,rect->y + rect->h + 1}, new SDL_Point{0,-radius},
      new SDL_Point{0,0},new SDL_Point{-radius,0});
    for(int i = 0; i < BEZIER_RESOLUTION; i++){
      for(int x = rect->x+rect->w; x > corner[i].x; x--){
        draw_cb(x,corner[i].y);
      }
    }
    free(corner);
}
void smooth_corners_surface(SDL_Surface*surface,Uint32 color,SDL_Rect*rect,int radius){
  smooth_corners(rect,radius,[&](int x,int y){
    if(x >= surface->w || y >= surface->h || y < 0 || x < 0)
      return;
    Uint8 * pixel = (Uint8*)surface->pixels;
    pixel += (y * surface->pitch) + (x * sizeof(Uint32));
    *((Uint32*)pixel) = color;
  });
}
void smooth_corners_renderer(SDL_Renderer*renderer,argb*color,SDL_Rect*rect,int radius){
    SDL_SetRenderDrawColor(renderer,color->r,color->g,color->b,color->a);
    smooth_corners(rect,radius,[&](int x,int y){
      SDL_RenderDrawPoint(renderer,x,y);
    });
}


SDL_Surface* make_input_box(int inputWidth, int inputHeight, argb *color, int inputBoxRadius){

  SDL_Rect inputRect;
  inputRect.x = 1;
  inputRect.y = 1;
  inputRect.w = inputWidth + 3;
  inputRect.h = inputHeight + 3;

  SDL_Surface* surf;
  #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    surf = SDL_CreateRGBSurface(SDL_SWSURFACE, inputRect.w, inputRect.h, 32, 0xff000000, 0x00ff0000, 0x0000ff00, 0x000000ff);
  #else
    surf = SDL_CreateRGBSurface(SDL_SWSURFACE, inputRect.w, inputRect.h, 32, 0x000000ff, 0x0000ff00, 0x00ff0000, 0xff000000);
  #endif
  SDL_FillRect(surf, &inputRect, SDL_MapRGBA(surf->format, color->r, color->g, color->b, color->a));

  inputRect.w -= 2;
  inputRect.h -= 2;
  
  if(inputBoxRadius > 0)
    smooth_corners_surface(surf, SDL_MapRGBA(surf->format,0,0,0,0), &inputRect, inputBoxRadius);

  return surf;
}