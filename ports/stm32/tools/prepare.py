"""Prepare an isolated STM32 source tree; never modify the PC game."""
from pathlib import Path
import re, itertools, shutil
root=Path(__file__).resolve().parents[3]
out=root/'ports/stm32/prepared'
out.mkdir(parents=True,exist_ok=True)
shutil.copytree(root/'src',out/'src',dirs_exist_ok=True)
shutil.copytree(root/'assets/sprites',out/'assets/sprites',dirs_exist_ok=True)
for p in (out/'assets/sprites').rglob('*.h'):
    p.chmod(0o666)
    def pack(m):
        values=[int(x,16) for x in re.findall(r'0x[0-9a-fA-F]+',m[2])]
        encoded=[0xDA7A,0xC0DE,len(values)&65535,len(values)>>16]
        decoded=[]
        for value,g in itertools.groupby(values):
            count=sum(1 for _ in g)
            while count:
                n=min(count,65535);encoded += [n,value];decoded.extend([value]*n);count-=n
        assert decoded==values
        return 'static const uint16_t '+m[1]+'[] = {'+','.join(hex(v) for v in encoded)+'};'
    s=p.read_text(encoding='utf-8-sig')
    s=re.sub(r'static const uint16_t\s+(\w+)\s*\[\s*\]\s*=\s*\{([^}]+)\}\s*;',pack,s,flags=re.S)
    p.write_text(s,encoding='utf-8')
# Decompress at draw entry, not in simulation or every pixel.
patches={
 'src/main.c': [('    for (int py = 0; py < h; py++)','    sprite=DA_Unpack(sprite);\n    for (int py = 0; py < h; py++)'),('int main(void)','int DogAlive_Run(void)'),('uint16_t color = sprite[','uint16_t color = DA_Unpack(sprite)[')],
 'src/ui/slime_king_draw.h':[('uint16_t c = pixels[','uint16_t c = DA_Unpack(pixels)[')],
 'src/ui/room_objects_draw.h':[('uint16_t c = sprite[','uint16_t c = DA_Unpack(sprite)['),('uint16_t color=pixels[','uint16_t color=DA_Unpack(pixels)[')],
 'src/effects/melee_slash.h':[('uint16_t c = sprite[','uint16_t c = DA_Unpack(sprite)[')],
 'src/effects/parry_effect.h':[('uint16_t color=pixels[','uint16_t color=DA_Unpack(pixels)[')],
 'src/ui/health_ui.h':[('heart_frames[heartFrame][','DA_Unpack(heart_frames[heartFrame])['),('return front[','return DA_Unpack(front)['),('return back[','return DA_Unpack(back)['),('return middle[','return DA_Unpack(middle)['),('icon[y *','DA_Unpack(icon)[y *')],
 'src/core/floor_tiles.h':[('Floor_Pixels(i)[','DA_Unpack(Floor_Pixels(i))[')],
 'src/story/story.h':[('mirrored[py*16+px]=pixels[','mirrored[py*16+px]=DA_Unpack(pixels)[')]
}
for name, pairs in patches.items():
    p=out/name;s=p.read_text(encoding='utf-8-sig')
    for a,b in pairs:s=s.replace(a,b)
    p.write_text(s,encoding='utf-8')
p=out/'src/story/story.h';s=p.read_text(encoding='utf-8')
a=s.index('static inline bool StoryArt_Init(');b=s.index('static inline void StoryArt_Close',a)
s=s[:a]+'''static inline bool StoryArt_Init(StoryArt *art,SDL_Renderer *r) {
    (void)r;
    for(int i=0;i<DUNGEON_ENTRANCE_FRAMES_COUNT;++i) {
        art->frames[i]=DA_TextureRLE(dungeon_entrance_frames[i],320,240);
        if(!art->frames[i]) return false;
    }
    return true;
}
'''+s[b:];s=s.replace('Space:','A:');p.write_text(s,encoding='utf-8')
p=out/'src/main.c';s=p.read_text(encoding='utf-8')
a=s.index('                if(gameState==GAME_PLAYING && story.mode==STORY_NORMAL && !story.helper');b=s.index('                if (gameState == GAME_PLAYING',a)
s=s[:a]+s[b:]
s=s.replace('key == SDL_SCANCODE_ESCAPE','key == SDL_SCANCODE_RETURN')
s=s.replace('(gameState != GAME_MENU && key == SDL_SCANCODE_R)','((gameState == GAME_PAUSED || gameState == GAME_OVER || gameState == GAME_VICTORY) && key == SDL_SCANCODE_SPACE)')
for a,b in [('Enter:', 'Start:'),('Esc:', 'Start:'),('Space:', 'A:'),('Tap J:', 'Tap B:'),('Hold J:', 'Hold B:'),(' J:', ' B:'),('R:', 'A:'),('M:', 'Select:'),('WASD / Arrows:', 'D-pad:')]:s=s.replace(a,b)
# Large persistent game state must not occupy the 4KB call stack.
for decl in ['Projectile projectiles','RunningEffect runningEffect','Player player;','EnemyGroup enemies;','StageProgress stage;','BowInput bowInput','SlimeShot slimeShots','Story story','StoryArt storyArt']:
    s=s.replace('    '+decl,'    static '+decl)
p.write_text(s,encoding='utf-8')
# Vibration follows damage; no stage-skip binding is generated.
s=p.read_text(encoding='utf-8');s=s.replace('        SDL_RenderPresent(renderer);','        DA_Feedback(player.hp,gameState==GAME_PLAYING);\n        SDL_RenderPresent(renderer);');p.write_text(s,encoding='utf-8')
# Route the PC music hooks to one embedded I2S stream.
for name,kind,track in [('boss_music.h','BossMusic',1),('ending_music.h','EndingMusic',2)]:
    (out/'src/audio'/name).write_text(f'''#pragma once
#include <stdbool.h>
#include "audio.h"
typedef struct {{bool active;}} {kind};
static inline void {kind}_Update({kind}*m,bool active,bool paused) {{
 if(active) DA_AudioSelect({track},paused);
 else if(m->active) DA_AudioSelect(0,false);
 m->active=active;
}}
static inline void {kind}_Close({kind}*m) {{if(m->active)DA_AudioSelect(0,false);m->active=false;}}
''')
print('Prepared compressed assets and game sources at',out)
