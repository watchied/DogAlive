#include <stdio.h>
#include <stdint.h>
#include "src/audio/game_sfx.h"

static void write_u16(FILE *file, uint16_t value) { fwrite(&value,2,1,file); }
static void write_u32(FILE *file, uint32_t value) { fwrite(&value,4,1,file); }

int main(int argc, char **argv)
{
    if(argc!=2) {fprintf(stderr,"Usage: render_sfx_preview output.wav\n");return 1;}
    const SfxCue cues[]={SFX_COFFIN_WAKE,SFX_COFFIN_EYE,SFX_SWORD_RELEASE,
        SFX_STAB,SFX_COFFIN_SLASH,SFX_COFFIN_SLAM,SFX_SWORD_BREAK,
        SFX_COFFIN_PHASE,SFX_COFFIN_DEATH};
    GameSfx sounds={0};
    SDL_SetHint(SDL_HINT_AUDIO_DRIVER,"dummy");
    if(!GameSfx_Init(&sounds,0)) {
        fprintf(stderr,"Could not initialize effects: %s\n",SDL_GetError());
        return 1;
    }
    const int pauseFrames=GAME_SFX_RATE/4;
    uint32_t dataBytes=0;
    for(size_t i=0;i<sizeof(cues)/sizeof(cues[0]);++i) {
        if(!sounds.recorded[cues[i]]) {
            fprintf(stderr,"Missing Flesh Coffin recording %d\n",cues[i]);
            GameSfx_Close(&sounds);return 1;
        }
        dataBytes+=(uint32_t)sounds.bytes[cues[i]]+pauseFrames*2;
    }
    FILE *file=fopen(argv[1],"wb");
    if(!file) {perror(argv[1]);GameSfx_Close(&sounds);return 1;}
    fwrite("RIFF",1,4,file);write_u32(file,36+dataBytes);
    fwrite("WAVEfmt ",1,8,file);write_u32(file,16);
    write_u16(file,1);write_u16(file,1);write_u32(file,GAME_SFX_RATE);
    write_u32(file,GAME_SFX_RATE*2);write_u16(file,2);write_u16(file,16);
    fwrite("data",1,4,file);write_u32(file,dataBytes);
    Sint16 silence=0;
    for(size_t i=0;i<sizeof(cues)/sizeof(cues[0]);++i) {
        fwrite(sounds.samples[cues[i]],1,sounds.bytes[cues[i]],file);
        for(int j=0;j<pauseFrames;++j) fwrite(&silence,2,1,file);
    }
    int result=fclose(file);
    GameSfx_Close(&sounds);
    return result==0?0:1;
}
