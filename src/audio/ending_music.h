#ifndef ENDING_MUSIC_H
#define ENDING_MUSIC_H
#include <SDL3/SDL.h>
#define ENDING_MUSIC_PATH "assets/music/Ending song.mp3"
#define ENDING_MUSIC_VOLUME 600 // 0-1000.
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
typedef MCIERROR (WINAPI *EndingMusicCommand)(LPCWSTR,LPWSTR,UINT,HWND);
typedef struct {
    SDL_SharedObject *library;
    EndingMusicCommand command;
    bool attempted,opened,playing,paused;
} EndingMusic;

static inline bool EndingMusic_Open(EndingMusic *m) {
    if(m->attempted) return m->opened;
    m->attempted=true;
    m->library=SDL_LoadObject("winmm.dll");
    if(!m->library) { SDL_Log("Ending music: %s",SDL_GetError());return false; }
    m->command=(EndingMusicCommand)SDL_LoadFunction(m->library,"mciSendStringW");
    if(!m->command) return false;
    char path[2048];
    // Support launch from either the repository or bin directory.
    SDL_IOStream *file=SDL_IOFromFile(ENDING_MUSIC_PATH,"rb");
    if(file) { SDL_CloseIO(file);SDL_snprintf(path,sizeof(path),"%s",ENDING_MUSIC_PATH); }
    else SDL_snprintf(path,sizeof(path),"%s../%s",SDL_GetBasePath(),ENDING_MUSIC_PATH);
    wchar_t widePath[2048],absolutePath[2048],command[2200];
    if(!MultiByteToWideChar(CP_UTF8,0,path,-1,widePath,2048)) return false;
    for(wchar_t *c=widePath;*c;++c) if(*c==L'/') *c=L'\\';
    DWORD length=GetFullPathNameW(widePath,2048,absolutePath,NULL);
    if(!length || length>=2048) return false;
    _snwprintf(command,2200,L"open \"%ls\" type mpegvideo alias ending_theme",absolutePath);
    MCIERROR error=m->command(command,NULL,0,NULL);
    if(error) {
        typedef BOOL (WINAPI *ErrorText)(MCIERROR,LPWSTR,UINT);
        ErrorText describe=(ErrorText)SDL_LoadFunction(m->library,"mciGetErrorStringW");
        wchar_t message[256]={0};if(describe) describe(error,message,256);
        char text[1024];WideCharToMultiByte(CP_UTF8,0,message,-1,text,sizeof(text),NULL,NULL);
        SDL_Log("Ending music: %s (MCI error %lu)",text,(unsigned long)error);return false;
    }
    m->opened=true;
    _snwprintf(command,2200,L"setaudio ending_theme volume to %d",ENDING_MUSIC_VOLUME);
    m->command(command,NULL,0,NULL);
    return true;
}
static inline void EndingMusic_Update(EndingMusic *m,bool encounter,bool paused) {
    if(!encounter) {
        if(m->opened && m->playing) {
            m->command(L"stop ending_theme",NULL,0,NULL);
            m->command(L"seek ending_theme to start",NULL,0,NULL);
        }
        m->playing=m->paused=false;
        return;
    }
    if(!EndingMusic_Open(m)) return;
    if(paused) {
        if(m->playing && !m->paused) {
            m->command(L"pause ending_theme",NULL,0,NULL);m->paused=true;
        }
    } else if(!m->playing || m->paused) {
        MCIERROR error=m->command(L"play ending_theme repeat",NULL,0,NULL);
        if(error) { SDL_Log("Ending music playback failed (%lu)",(unsigned long)error);return; }
        m->playing=true;m->paused=false;
    }
}
static inline void EndingMusic_Close(EndingMusic *m) {
    if(m->opened) m->command(L"close ending_theme",NULL,0,NULL);
    if(m->library) SDL_UnloadObject(m->library);
    *m=(EndingMusic){0};
}
#else
typedef struct { bool unused; } EndingMusic;
static inline void EndingMusic_Update(EndingMusic *m,bool encounter,bool paused) {
    (void)m;(void)encounter;(void)paused;
}
static inline void EndingMusic_Close(EndingMusic *m) { (void)m; }
#endif
#endif
