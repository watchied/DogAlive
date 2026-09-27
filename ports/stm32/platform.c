#include "SDL3/SDL.h"
#include "main.h"
#include "ILI9341_STM32_Driver.h"
#include "fonts.h"
#include "audio.h"
#include <stdarg.h>
static uint16_t framebuffer[320*240],unpacked[128*128];
static const uint16_t *lastAsset;
static SDL_Renderer renderer;
static SDL_Window window;
static bool keys[SDL_SCANCODE_COUNT];
static bool inputReady;
static volatile uint32_t rumbleUntil;
void DA_Feedback(int hp,bool playing){static int previous;static bool active;if(playing&&active&&hp<previous){rumbleUntil=HAL_GetTick()+100;HAL_GPIO_WritePin(vibration_GPIO_Port,vibration_Pin,GPIO_PIN_SET);}if(!playing){rumbleUntil=0;HAL_GPIO_WritePin(vibration_GPIO_Port,vibration_Pin,GPIO_PIN_RESET);}previous=hp;active=playing;}
volatile char dogalive_error[160];
const uint16_t *DA_Unpack(const uint16_t*p){
 if(!p||p[0]!=0xDA7A||p[1]!=0xC0DE)return p;
 if(p==lastAsset)return unpacked;
 uint32_t n=p[2]|((uint32_t)p[3]<<16),i=0;p+=4;
 if(n>128*128){SDL_Log("Asset too large");return unpacked;}
 const uint16_t *origin=p-4;
 while(i<n){uint32_t count=*p++;uint16_t c=*p++;if(!count||count>n-i){SDL_Log("Invalid RLE");break;}while(count--)unpacked[i++]=c;}
 lastAsset=origin;return unpacked;
}
void SDL_Log(const char*f,...){va_list a;va_start(a,f);vsnprintf((char*)dogalive_error,sizeof dogalive_error,f,a);va_end(a);}
bool SDL_Init(int f){ILI9341_Init();ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);inputReady=true;return true;}
SDL_Window *SDL_CreateWindow(const char*s,int w,int h,int f){return &window;}
SDL_Renderer *SDL_CreateRenderer(SDL_Window*w,const char*s){return &renderer;}
uint64_t SDL_GetTicks(void){return HAL_GetTick();}
uint64_t SDL_GetPerformanceCounter(void){return HAL_GetTick()+0x1234567u;}
void SDL_Delay(uint32_t n){uint32_t start=HAL_GetTick();do{DA_AudioPump();}while((uint32_t)(HAL_GetTick()-start)<n);}
typedef struct {GPIO_TypeDef *port;uint16_t pin;int key;bool raw,stable;uint32_t changed;} Button;
static Button buttons[]={
 {Up1_GPIO_Port,Up1_Pin,SDL_SCANCODE_UP},{Down1_GPIO_Port,Down1_Pin,SDL_SCANCODE_DOWN},
 {Left1_GPIO_Port,Left1_Pin,SDL_SCANCODE_LEFT},{Right1_GPIO_Port,Right1_Pin,SDL_SCANCODE_RIGHT},
 {A1_GPIO_Port,A1_Pin,SDL_SCANCODE_SPACE},{B1_GPIO_Port,B1_Pin,SDL_SCANCODE_J},
 {Start1_GPIO_Port,Start1_Pin,SDL_SCANCODE_RETURN},{Select1_GPIO_Port,Select1_Pin,SDL_SCANCODE_M}};
static SDL_Event events[32];
static volatile unsigned writeEvent,readEvent;
void DA_InputTick(void){
 if(!inputReady)return;
 uint32_t now=HAL_GetTick();
 if(rumbleUntil&&(int32_t)(now-rumbleUntil)>=0){HAL_GPIO_WritePin(vibration_GPIO_Port,vibration_Pin,GPIO_PIN_RESET);rumbleUntil=0;}
 for(unsigned i=0;i<8;i++){Button*b=&buttons[i];bool v=HAL_GPIO_ReadPin(b->port,b->pin)==GPIO_PIN_RESET;
 if(v!=b->raw){b->raw=v;b->changed=now;}
 if(v!=b->stable&&(uint32_t)(now-b->changed)>=15){unsigned next=(writeEvent+1)%32;if(next!=readEvent){b->stable=v;events[writeEvent]=(SDL_Event){.type=v?SDL_EVENT_KEY_DOWN:SDL_EVENT_KEY_UP,.key={b->key,false,(uint64_t)now*1000000}};__DMB();writeEvent=next;}}}
}
bool SDL_PollEvent(SDL_Event*e){
 static unsigned pressed;
 DA_AudioPump();if(readEvent==writeEvent){pressed=0;return false;}
 SDL_Event next=events[readEvent];unsigned bit=1u<<next.key.scancode;
 if(next.type==SDL_EVENT_KEY_UP&&(pressed&bit)){pressed=0;return false;}
 *e=next;keys[e->key.scancode]=e->type==SDL_EVENT_KEY_DOWN;
 if(e->type==SDL_EVENT_KEY_DOWN)pressed|=bit;
 __DMB();readEvent=(readEvent+1)%32;return true;
}
const bool *SDL_GetKeyboardState(int*n){if(n)*n=SDL_SCANCODE_COUNT;return keys;}
SDL_Surface *SDL_CreateSurface(int w,int h,int f){SDL_Surface*s=calloc(1,sizeof *s);if(!s)return NULL;s->w=w;s->h=h;s->pitch=w*4;s->pixels=calloc((size_t)w*h,4);if(!s->pixels){free(s);return NULL;}return s;}
void SDL_DestroySurface(SDL_Surface*s){if(s){free(s->pixels);free(s);}}
SDL_Texture *DA_TextureRLE(const uint16_t*p,int w,int h){SDL_Texture*t=calloc(1,sizeof *t);if(t){t->w=w;t->h=h;t->rle=p;t->r=t->g=t->b=t->a=255;}return t;}
SDL_Texture *SDL_CreateTextureFromSurface(SDL_Renderer*r,SDL_Surface*s){SDL_Texture*t=DA_TextureRLE(NULL,s->w,s->h);if(!t)return NULL;t->pixels=malloc((size_t)t->w*t->h*2);if(!t->pixels){free(t);return NULL;}uint8_t*p=s->pixels;for(int i=0;i<t->w*t->h;i++,p+=4)t->pixels[i]=p[3]?((p[0]>>3)<<11)|((p[1]>>2)<<5)|(p[2]>>3):0x07e0;return t;}
void SDL_DestroyTexture(SDL_Texture*t){if(t){free(t->pixels);free(t);}}
static void pixel(int x,int y,uint8_t r,uint8_t g,uint8_t b,uint8_t a){if((unsigned)x>=320||(unsigned)y>=240)return;uint16_t*d=&framebuffer[y*320+x];if(a<255){r=(r*a+((*d>>11)*255/31)*(255-a))/255;g=(g*a+(((*d>>5)&63)*255/63)*(255-a))/255;b=(b*a+((*d&31)*255/31)*(255-a))/255;}*d=((r>>3)<<11)|((g>>2)<<5)|(b>>3);}
void SDL_RenderFillRect(SDL_Renderer*r,const SDL_FRect*q){int x0=fmaxf(0,floorf(q->x)),y0=fmaxf(0,floorf(q->y)),x1=fminf(320,ceilf(q->x+q->w)),y1=fminf(240,ceilf(q->y+q->h));for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)pixel(x,y,r->r,r->g,r->b,r->blend?r->a:255);}
void SDL_RenderFillRects(SDL_Renderer*r,const SDL_FRect*q,int n){while(n--)SDL_RenderFillRect(r,q++);}
void SDL_RenderClear(SDL_Renderer*r){uint16_t c=((r->r>>3)<<11)|((r->g>>2)<<5)|(r->b>>3);for(int i=0;i<320*240;i++)framebuffer[i]=c;}
void SDL_RenderLine(SDL_Renderer*r,float ax,float ay,float bx,float by){int n=(int)ceilf(fmaxf(fabsf(bx-ax),fabsf(by-ay)));if(n>2048)n=2048;for(int i=0;i<=n;i++){float t=n?(float)i/n:0;pixel(lroundf(ax+(bx-ax)*t),lroundf(ay+(by-ay)*t),r->r,r->g,r->b,r->blend?r->a:255);}}
void SDL_RenderLines(SDL_Renderer*r,const SDL_FPoint*p,int n){for(int i=1;i<n;i++)SDL_RenderLine(r,p[i-1].x,p[i-1].y,p[i].x,p[i].y);}
void SDL_RenderRect(SDL_Renderer*r,const SDL_FRect*q){SDL_RenderLine(r,q->x,q->y,q->x+q->w,q->y);SDL_RenderLine(r,q->x,q->y,q->x,q->y+q->h);SDL_RenderLine(r,q->x+q->w,q->y,q->x+q->w,q->y+q->h);SDL_RenderLine(r,q->x,q->y+q->h,q->x+q->w,q->y+q->h);}
void SDL_RenderPresent(SDL_Renderer*r){
 static uint32_t hashes[30];static bool valid;
 for(int strip=0;strip<30;strip++){
  uint16_t *p=framebuffer+strip*320*8;uint32_t hash=2166136261u;
  for(int i=0;i<320*8;i++)hash=(hash^p[i])*16777619u;
  if(!valid||hash!=hashes[strip]){ILI9341_Draw_RGB565(0,strip*8,320,8,p);hashes[strip]=hash;}
  DA_AudioPump();
 }
 valid=true;
}
void SDL_RenderTextureRotated(SDL_Renderer*r,SDL_Texture*t,const SDL_FRect*source,const SDL_FRect*d,double degrees,const SDL_FPoint*pivot,int flip){
 if(!t||d->w<=0||d->h<=0)return;
 SDL_FRect s=source?*source:(SDL_FRect){0,0,t->w,t->h};
 /* Story backgrounds use full-size, untranslated-in-X RLE textures. Stream
    their runs directly to the framebuffer, avoiding a second full frame. */
 if(t->rle&&degrees==0&&!source&&d->w==t->w&&d->h==t->h){
  const uint16_t *run=t->rle+4;uint32_t index=0,total=t->w*t->h;
  while(index<total){unsigned count=*run++;uint16_t col=*run++;if(!count||count>total-index)break;
   uint8_t cr=((col>>11)*255/31)*t->r/255,cg=(((col>>5)&63)*255/63)*t->g/255,cb=((col&31)*255/31)*t->b/255;
   while(count--){pixel((int)d->x+index%t->w,(int)d->y+index/t->w,cr,cg,cb,t->a);index++;}
  }return;
 }
 const uint16_t*p=t->rle?DA_Unpack(t->rle):t->pixels;float a=degrees*0.01745329252f,c=cosf(a),sn=sinf(a),cx=pivot?pivot->x:d->w/2,cy=pivot?pivot->y:d->h/2;
 float radius=hypotf(d->w,d->h);int x0=fmaxf(0,floorf(d->x+cx-radius)),x1=fminf(320,ceilf(d->x+cx+radius)),y0=fmaxf(0,floorf(d->y+cy-radius)),y1=fminf(240,ceilf(d->y+cy+radius));
 if(degrees==0){x0=fmaxf(0,floorf(d->x));x1=fminf(320,ceilf(d->x+d->w));y0=fmaxf(0,floorf(d->y));y1=fminf(240,ceilf(d->y+d->h));}
 for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++){float dx=x+0.5f-d->x-cx,dy=y+0.5f-d->y-cy,u=dx*c+dy*sn+cx,v=-dx*sn+dy*c+cy;if(u<0||v<0||u>=d->w||v>=d->h)continue;int tx=s.x+u*s.w/d->w,ty=s.y+v*s.h/d->h;if((unsigned)tx>=(unsigned)t->w||(unsigned)ty>=(unsigned)t->h)continue;uint16_t col=p[ty*t->w+tx];if(!t->rle&&col==0x07e0)continue;pixel(x,y,((col>>11)*255/31)*t->r/255,(((col>>5)&63)*255/63)*t->g/255,((col&31)*255/31)*t->b/255,t->a);}
}
void SDL_RenderTexture(SDL_Renderer*r,SDL_Texture*t,const SDL_FRect*s,const SDL_FRect*d){SDL_RenderTextureRotated(r,t,s,d,0,NULL,0);}
void SDL_RenderDebugText(SDL_Renderer*r,float x,float y,const char*s){int ox=x;for(;*s;s++){if(*s=='\n'){x=ox;y+=10;continue;}unsigned ch=(unsigned char)*s;if(ch>=32&&ch<=126)for(int row=0;row<10;row++){uint16_t bits=Font_7x10.data[(ch-32)*10+row];for(int col=0;col<7;col++)if((bits<<col)&0x8000)pixel(x+col,y+row,r->r,r->g,r->b,r->a);}x+=8;}}
void SDL_RenderDebugTextFormat(SDL_Renderer*r,float x,float y,const char*f,...){char b[128];va_list a;va_start(a,f);vsnprintf(b,sizeof b,f,a);va_end(a);SDL_RenderDebugText(r,x,y,b);}
bool SDL_GetRectAndLineIntersectionFloat(const SDL_FRect*r,float*x1,float*y1,float*x2,float*y2){float dx=*x2-*x1,dy=*y2-*y1,t0=0,t1=1,p[4]={-dx,dx,-dy,dy},q[4]={*x1-r->x,r->x+r->w-*x1,*y1-r->y,r->y+r->h-*y1};for(int i=0;i<4;i++){if(p[i]==0){if(q[i]<0)return false;}else{float t=q[i]/p[i];if(p[i]<0){if(t>t1)return false;if(t>t0)t0=t;}else{if(t<t0)return false;if(t<t1)t1=t;}}}float ox=*x1,oy=*y1;*x1=ox+t0*dx;*y1=oy+t0*dy;*x2=ox+t1*dx;*y2=oy+t1*dy;return true;}
