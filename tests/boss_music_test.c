#include <assert.h>
#include <stdio.h>
#include <wchar.h>
#include "src/audio/boss_music.h"
int main(void) {
    assert(SDL_Init(0));BossMusic m={0};wchar_t mode[64]={0};
    assert(BossMusic_Open(&m));
    assert(!m.command(L"setaudio coffin_theme volume to 0",NULL,0,NULL));
    BossMusic_Update(&m,true,false);assert(m.playing);
    assert(!m.command(L"status coffin_theme mode",mode,64,NULL));
    assert(wcscmp(mode,L"playing")==0);
    BossMusic_Update(&m,true,true);assert(m.paused);
    BossMusic_Update(&m,true,false);assert(m.playing && !m.paused);
    BossMusic_Update(&m,false,false);assert(!m.playing);
    assert(!m.command(L"status coffin_theme position",mode,64,NULL));
    assert(wcstol(mode,NULL,10)==0);
    BossMusic_Close(&m);assert(!m.library && !m.opened);
    SDL_Quit();puts("MP3 open, loop playback, pause/resume, stop/reset and cleanup passed");
}
