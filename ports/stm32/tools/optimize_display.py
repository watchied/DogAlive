from pathlib import Path
from datetime import datetime
import shutil

root=Path('D:/watt/cube/gameboy')
changes={}
p=root/'Core/Src/dogalive_platform.c'
s=p.read_text(encoding='utf-8-sig')
a=s.index('void SDL_RenderPresent(');b=s.index('void SDL_RenderTextureRotated(',a)
s=s[:a]+'''/* Debugger-visible display cost and payload for the latest frame. */
volatile uint32_t lcd_present_ms, lcd_frame_ms, lcd_changed_tiles, lcd_pixel_bytes;
void SDL_RenderPresent(SDL_Renderer*r){
 static uint32_t hashes[20*15];static bool valid;static uint32_t previous;
 uint32_t start=HAL_GetTick();
 lcd_frame_ms=previous?start-previous:0;previous=start;
 lcd_changed_tiles=0;lcd_pixel_bytes=0;
 uint16_t tile[16*16];
 for(int ty=0;ty<15;ty++)for(int tx=0;tx<20;tx++){
  uint32_t hash=2166136261u;
  for(int y=0;y<16;y++){
   const uint16_t *p=framebuffer+(ty*16+y)*320+tx*16;
   for(int x=0;x<16;x++)hash=(hash^p[x])*16777619u;
  }
  int index=ty*20+tx;
  if(!valid||hash!=hashes[index]){
   for(int y=0;y<16;y++)memcpy(tile+y*16,framebuffer+(ty*16+y)*320+tx*16,32);
   ILI9341_Draw_RGB565(tx*16,ty*16,16,16,tile);
   hashes[index]=hash;lcd_changed_tiles++;lcd_pixel_bytes+=sizeof(tile);
  }
  if(tx==19)DA_AudioPump();
 }
 valid=true;lcd_present_ms=HAL_GetTick()-start;
}
'''+s[b:]
old='uint16_t*d=&framebuffer[y*320+x];if(a<255)'
new='uint16_t*d=&framebuffer[y*320+x];if(a==255){*d=((r>>3)<<11)|((g>>2)<<5)|(b>>3);return;}if(a==0)return;if(a<255)'
assert old in s;s=s.replace(old,new,1)
old='for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)pixel(x,y,r->r,r->g,r->b,r->blend?r->a:255);'
new='''if(!r->blend||r->a==255){uint16_t c=((r->r>>3)<<11)|((r->g>>2)<<5)|(r->b>>3);for(int y=y0;y<y1;y++){uint16_t*p=framebuffer+y*320;for(int x=x0;x<x1;x++)p[x]=c;}return;}for(int y=y0;y<y1;y++)for(int x=x0;x<x1;x++)pixel(x,y,r->r,r->g,r->b,r->a);'''
assert old in s;s=s.replace(old,new,1)
changes[p]=s
p=root/'Core/Src/main.c';s=p.read_text(encoding='utf-8-sig')
anchor='  /* USER CODE BEGIN Init */'
assert anchor in s
s=s.replace(anchor,anchor+'\n  /* Instruction cache does not require DMA buffer coherency changes. */\n  SCB_EnableICache();',1)
changes[p]=s
backup=root/'Backup'/('display-tiles-'+datetime.now().strftime('%Y%m%d-%H%M%S'))
for p,s in changes.items():
 b=backup/p.relative_to(root);b.parent.mkdir(parents=True,exist_ok=True)
 shutil.copy2(p,b);p.write_text(s,encoding='utf-8')
print('Display optimization installed. Backup:',backup)
