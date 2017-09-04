#include "draw_helpers.h"

SDL_Point* bezier_corner (SDL_Point*offset,SDL_Point *p1, SDL_Point *p2, SDL_Point *p3) {
  SDL_Point*pts = (SDL_Point*)malloc(sizeof(SDL_Point) * BEZIER_RESOLUTION);
  int i = 0;
  for(double t=0.0; t<=1.0; t+=1/BEZIER_RESOLUTION){
    pts[i].x = ((1-t)*(1-t)*p1->x+2*(1-t)*t*p2->x+t*t*p3->x) + offset->x;
    pts[i].y = ((1-t)*(1-t)*p1->y+2*(1-t)*t*p2->y+t*t*p3->y) + offset->y;
    i++;
  }
  return pts;
}

void smooth_corners(SDL_Rect *rect, int radius,function<void(int,int)> draw_cb){
    //Top Left
    auto corner = bezier_corner(new SDL_Point{rect->x-1,rect->y-1}, new SDL_Point{0,radius},
      new SDL_Point{0,0},new SDL_Point{radius,0});
    for(int i = 0; i < BEZIER_RESOLUTION; i++){
      for(int x =rect->x; x < corner[i].x; x++){
        draw_cb(x,corner[i].y);
      }
    }
    free(corner);

    //Top Right
    corner = bezier_corner(new SDL_Point{rect->x + rect->w+1,rect->y-1}, new SDL_Point{0,radius},
      new SDL_Point{0,0},new SDL_Point{-radius,0});
    for(int i = 0; i < BEZIER_RESOLUTION; i++){
      for(int x = rect->x+rect->w; x > corner[i].x; x--){
        draw_cb(x,corner[i].y);
      }
    }

    //Bottom Left
    corner = bezier_corner(new SDL_Point{rect->x-1,rect->y + rect->h+1}, new SDL_Point{0,-radius},
      new SDL_Point{0,0},new SDL_Point{radius,0});
    for(int i = 0; i < BEZIER_RESOLUTION; i++){
      for(int x =rect->x; x < corner[i].x; x++){
        draw_cb(x,corner[i].y);
      }
    }
    free(corner);

    //Bottom Right
    corner = bezier_corner(new SDL_Point{rect->x + rect->w + 1,rect->y + rect->h + 1}, new SDL_Point{0,-radius},
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
void smooth_corners_renderer_to_image(SDL_Renderer*renderer,Uint8*pixels,int w,int h,SDL_Rect*rect,int radius){
  smooth_corners(rect,radius,[&](int x,int y){
    if(x < w && y < h) {
      SDL_SetRenderDrawColor(renderer,pixels[4 * (w * y + x)],pixels[4 * (w * y + x)+1],
        pixels[4 * (w * y + x)+2],pixels[4 * (w * y + x)+3]);
      SDL_RenderDrawPoint(renderer,x,y);
    }
  });
}
SDL_Surface* scale_surface(SDL_Surface* surface, SDL_Rect* rect){
  if(rect == NULL || surface == NULL || rect->w == 0 || rect->h == 0){
    return NULL;
  }
  SDL_Surface * scaled = SDL_CreateRGBSurface(surface->flags,rect->w,rect->h,surface->format->BitsPerPixel,
    surface->format->Rmask,surface->format->Gmask,surface->format->Bmask,surface->format->Amask);
  if(!scaled){
    return NULL;
  }
  if(SDL_BlitScaled(surface,NULL,scaled,NULL)){
    return NULL;
  }
  return scaled;
}