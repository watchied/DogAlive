#ifndef DOGALIVE_STM32_PLATFORM_H
#define DOGALIVE_STM32_PLATFORM_H
/* Small source-compatibility interface, not the desktop SDL library. */
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
typedef uint8_t Uint8;
typedef uint32_t Uint32;
typedef struct {float x,y,w,h;} SDL_FRect;
typedef struct {float x,y;} SDL_FPoint;
typedef struct {uint8_t r,g,b,a;int blend;} SDL_Renderer;
typedef struct {int unused;} SDL_Window;
typedef struct {int w,h,pitch;void *pixels;} SDL_Surface;
typedef struct {int w,h;const uint16_t *rle;uint16_t *pixels;uint8_t r,g,b,a;} SDL_Texture;
enum {SDL_SCANCODE_W,SDL_SCANCODE_A,SDL_SCANCODE_S,SDL_SCANCODE_D,
 SDL_SCANCODE_UP,SDL_SCANCODE_DOWN,SDL_SCANCODE_LEFT,SDL_SCANCODE_RIGHT,
 SDL_SCANCODE_SPACE,SDL_SCANCODE_J,SDL_SCANCODE_RETURN,SDL_SCANCODE_M,
 SDL_SCANCODE_ESCAPE,SDL_SCANCODE_R,SDL_SCANCODE_COUNT};
typedef int SDL_Scancode;
enum {SDL_EVENT_KEY_DOWN=1,SDL_EVENT_KEY_UP,SDL_EVENT_QUIT,SDL_EVENT_WINDOW_FOCUS_LOST};
typedef struct {int type;struct {int scancode;bool repeat;uint64_t timestamp;} key;} SDL_Event;
enum {SDL_BLENDMODE_NONE,SDL_BLENDMODE_BLEND};
#define SDL_INIT_VIDEO 0
#define SDL_WINDOW_RESIZABLE 0
#define SDL_WINDOW_INPUT_FOCUS 1
#define SDL_LOGICAL_PRESENTATION_INTEGER_SCALE 0
#define SDL_PIXELFORMAT_RGBA32 0
#define SDL_SCALEMODE_NEAREST 0
#define SDL_FLIP_NONE 0
const uint16_t *DA_Unpack(const uint16_t *p);
void DA_Feedback(int hp,bool playing);
SDL_Texture *DA_TextureRLE(const uint16_t *p,int w,int h);
bool SDL_Init(int flags);
SDL_Window *SDL_CreateWindow(const char*,int,int,int);
SDL_Renderer *SDL_CreateRenderer(SDL_Window*,const char*);
static inline bool SDL_SetRenderLogicalPresentation(SDL_Renderer*r,int w,int h,int m){return true;}
static inline const char *SDL_GetError(void){return "STM32 platform error";}
void SDL_Log(const char*,...);
uint64_t SDL_GetTicks(void);
uint64_t SDL_GetPerformanceCounter(void);
void SDL_Delay(uint32_t);
bool SDL_PollEvent(SDL_Event*);
const bool *SDL_GetKeyboardState(int*);
static inline int SDL_GetWindowFlags(SDL_Window*w){return SDL_WINDOW_INPUT_FOCUS;}
static inline void SDL_SetWindowTitle(SDL_Window*w,const char*s){}
static inline void SDL_DestroyWindow(SDL_Window*w){}
static inline void SDL_DestroyRenderer(SDL_Renderer*r){}
static inline void SDL_Quit(void){}
SDL_Surface *SDL_CreateSurface(int,int,int);
void SDL_DestroySurface(SDL_Surface*);
SDL_Texture *SDL_CreateTextureFromSurface(SDL_Renderer*,SDL_Surface*);
void SDL_DestroyTexture(SDL_Texture*);
static inline void SDL_SetTextureScaleMode(SDL_Texture*t,int m){}
static inline void SDL_SetTextureBlendMode(SDL_Texture*t,int m){}
static inline void SDL_SetTextureColorMod(SDL_Texture*t,uint8_t r,uint8_t g,uint8_t b){t->r=r;t->g=g;t->b=b;}
static inline void SDL_GetTextureColorMod(SDL_Texture*t,uint8_t*r,uint8_t*g,uint8_t*b){*r=t->r;*g=t->g;*b=t->b;}
static inline void SDL_SetTextureAlphaMod(SDL_Texture*t,uint8_t a){t->a=a;}
static inline void SDL_SetRenderDrawColor(SDL_Renderer*r,uint8_t a,uint8_t b,uint8_t c,uint8_t d){r->r=a;r->g=b;r->b=c;r->a=d;}
static inline void SDL_SetRenderDrawBlendMode(SDL_Renderer*r,int m){r->blend=m;}
void SDL_RenderFillRect(SDL_Renderer*,const SDL_FRect*);
void SDL_RenderFillRects(SDL_Renderer*,const SDL_FRect*,int);
void SDL_RenderRect(SDL_Renderer*,const SDL_FRect*);
void SDL_RenderLine(SDL_Renderer*,float,float,float,float);
void SDL_RenderLines(SDL_Renderer*,const SDL_FPoint*,int);
void SDL_RenderClear(SDL_Renderer*);
void SDL_RenderPresent(SDL_Renderer*);
void SDL_RenderTextureRotated(SDL_Renderer*,SDL_Texture*,const SDL_FRect*,const SDL_FRect*,double,const SDL_FPoint*,int);
void SDL_RenderTexture(SDL_Renderer*,SDL_Texture*,const SDL_FRect*,const SDL_FRect*);
void SDL_RenderDebugText(SDL_Renderer*,float,float,const char*);
void SDL_RenderDebugTextFormat(SDL_Renderer*,float,float,const char*,...);
static inline bool SDL_HasRectIntersectionFloat(const SDL_FRect*a,const SDL_FRect*b){return a->w>0&&a->h>0&&b->w>0&&b->h>0&&a->x<b->x+b->w&&b->x<a->x+a->w&&a->y<b->y+b->h&&b->y<a->y+a->h;}
bool SDL_GetRectAndLineIntersectionFloat(const SDL_FRect*,float*,float*,float*,float*);
#endif
