"""Run the embedded renderer and existing logic tests with mocked board I/O."""
from pathlib import Path
import subprocess,shutil,sys
root=Path(__file__).resolve().parents[3];port=root/'ports/stm32';stage=port/'build-project';host=port/'host-check';host.mkdir(exist_ok=True)
header='''#pragma once
#include <stdint.h>
typedef int GPIO_TypeDef;
#define GPIO_PIN_RESET 0
#define GPIO_PIN_SET 1
#define __DMB() ((void)0)
uint32_t HAL_GetTick(void);
void HAL_Delay(uint32_t n);
int HAL_GPIO_ReadPin(GPIO_TypeDef*p,uint16_t n);
void HAL_GPIO_WritePin(GPIO_TypeDef*p,uint16_t n,int v);
'''
for i,name in enumerate(['Up1','Down1','Left1','Right1','A1','B1','Start1','Select1','vibration']):header+=f'#define {name}_GPIO_Port ((GPIO_TypeDef*)0)\n#define {name}_Pin {i}\n'
(host/'main.h').write_text(header)
(host/'ILI9341_STM32_Driver.h').write_text('''#pragma once
#include <stdint.h>
#define SCREEN_HORIZONTAL_1 1
void ILI9341_Init(void);
void ILI9341_Set_Rotation(int);
void ILI9341_Draw_RGB565(uint16_t,uint16_t,uint16_t,uint16_t,const uint16_t*);
''')
(host/'mock.c').write_text('''#include "main.h"
#include "audio.h"
#include <stdio.h>
#include <stdlib.h>
static uint32_t now;static int guard;static uint16_t screen[320*240];
void DA_InputTick(void);
static void save(void){FILE*f=fopen("screen.ppm","wb");fprintf(f,"P6\\n320 240\\n255\\n");for(int i=0;i<320*240;i++){uint16_t c=screen[i];fputc((c>>11)*255/31,f);fputc(((c>>5)&63)*255/63,f);fputc((c&31)*255/31,f);}fclose(f);}
uint32_t HAL_GetTick(void){now++;if(!guard){guard=1;DA_InputTick();guard=0;}if(now>4000){save();exit(0);}return now;}
void HAL_Delay(uint32_t n){now+=n;}
int HAL_GPIO_ReadPin(GPIO_TypeDef*p,uint16_t n){if(n==6&&now>600&&now<850)return 0;return 1;}
void HAL_GPIO_WritePin(GPIO_TypeDef*p,uint16_t n,int v){}
void ILI9341_Init(void){}
void ILI9341_Set_Rotation(int n){}
void ILI9341_Draw_RGB565(uint16_t x,uint16_t y,uint16_t w,uint16_t h,const uint16_t*p){for(int j=0;j<h;j++)for(int i=0;i<w;i++)screen[(y+j)*320+x+i]=p[j*w+i];}
void DA_AudioSelect(int track,_Bool paused){}
void DA_AudioPump(void){}
''')
(host/'run.c').write_text('int DogAlive_Run(void);int main(void){return DogAlive_Run();}\n')
gcc='C:/mingw64/bin/gcc.exe'
common=[gcc,'-std=c11','-O1','-I'+str(host),'-I'+str(stage/'Core/Inc/dogalive'),'-I'+str(stage/'Core/Inc'),str(port/'platform.c'),str(host/'mock.c'),str(stage/'Core/Src/fonts.c')]
def run(name,sources):
 exe=host/(name+'.exe');r=subprocess.run([*common,*map(str,sources),'-lm','-o',str(exe)],capture_output=True,text=True)
 if r.returncode:print(name,r.stderr);return False
 r=subprocess.run([str(exe)],cwd=host,capture_output=True,text=True,timeout=30)
 print(name,'PASS' if r.returncode==0 else 'FAIL',r.stdout.strip(),r.stderr.strip());return r.returncode==0
ok=run('game_preview',[stage/'Core/Src/game.c',host/'run.c'])
for name in ['potion_test','bow_input_test','bow_charges_test','charge_attack_test','charged_parry_test','enemy_combat_test','enemy_group_test','flesh_coffin_test','melee_timing_test','player_invincibility_test','room_interactions_test','slime_king_test','stages_test','story_test','enemy_layouts_test']:
    source=root/'tests'/(name+'.c')
    if name=='story_test':
        # Keep all gameplay assertions; replace the desktop contact-sheet
        # export with the actual embedded 320x240 renderer.
        text=source.read_text(encoding='utf-8')
        text=text[:text.index('    assert(SDL_Init(0));')]+'''    assert(SDL_Init(0));
    SDL_Renderer *r=SDL_CreateRenderer(NULL,NULL);StoryArt art={0};assert(StoryArt_Init(&art,r));
    for(int i=0;i<3;i++) { SDL_SetRenderDrawColor(r,0,0,0,255);SDL_RenderClear(r);
      if(i==0)Story_Background(r,&art,1,1,0);
      else {s.mode=i==1?STORY_SLIDES:STORY_EPILOGUE;s.timer=12;s.interaction=2;Story_Draw(&s,&art,r,&p);}
      SDL_RenderPresent(r);
    }
    puts("Story logic and embedded background rendering passed");return 0;
}
'''
        source=host/'story_embedded.c';source.write_text(text,encoding='utf-8')
    ok=run(name,[source,stage/'Core/Inc/dogalive/src/core/game_core.c']) and ok
try:
 from PIL import Image
 Image.open(host/'screen.ppm').save(host/'screen.png')
except ImportError:pass
sys.exit(0 if ok else 1)
