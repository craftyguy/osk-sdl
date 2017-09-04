#ifndef DRAW_HELPERS_H
#define DRAW_HELPERS_H
#include "SDL2/SDL.h"
#include "keyboard.h"
#include <functional>

const float  BEZIER_RESOLUTION = 100;

/**
  Curve the corneres of a rectangle
  @param rect the rectangle to smooth
  @param radius the distance from a corner where the curve will start
  @param draw_cb callback, with coordinates to the next pixel to draw
*/
void smooth_corners(SDL_Rect *rect, int radius,function<void(int,int)> draw_cb);

/**
  Draw rounded corneres for a rectangle directly onto a surface
  @param surface the surface to draw on
  @param color the color to draw
  @param rect the rectangle to smooth
  @param radius the distance from a corner where the curve will start
*/
void smooth_corners_surface(SDL_Surface*surface,Uint32 color,SDL_Rect*rect,int radius);

/**
  Draw rounded corneres for a rectangle with a renderer
  @param renderer the renderer to draw on
  @param color the color to draw
  @param rect the rectangle to smooth
  @param radius the distance from a corner where the curve will start
*/
void smooth_corners_renderer(SDL_Renderer*renderer,argb*color,SDL_Rect*rect,int radius);

/**
  Get each pixel pixel of bezier curve based on three points
  @param offset the points offset from (0,0)
  @param p1 the first point
  @param p2 the second point
  @param p3 the third point
  @returns an array of pixel coordinates (length equal to BEZIER_RESOLUTION)
*/
SDL_Point* bezier_corner (SDL_Point*offset,SDL_Point *p1, SDL_Point *p2, SDL_Point *p3);

/**
  Stretch a surface to a specific size
  @param surface the surface to modify
  @param rect the "w" and "h" properties are the size to scale to, "x" and "y" are ignored
*/
SDL_Surface* scale_surface(SDL_Surface* surface, SDL_Rect* rect);

/**
  Draw rounded corneres for a rectangle with a renderer. Instead of drawing with a single color, get each pixel's color from a buffer
  @param renderer the renderer to draw on
  @param pixels the array of pixels to draw from
  @param w the width of pixels
  @param h the height of pixels
  @param radius the distance from a corner where the curve will start
*/
void smooth_corners_renderer_to_image(SDL_Renderer*renderer,Uint8*pixels,int w, int h,SDL_Rect*rect,int radius);
#endif