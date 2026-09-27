#include "audio.h"
#include "fatfs.h"
#include "i2s.h"
#include <string.h>
#define HALF_SAMPLES 4096
static int16_t buffer[HALF_SAMPLES*2] __attribute__((aligned(32)));
static uint8_t staging[HALF_SAMPLES*2];
static FIL music;
static bool mounted,opened,playing,paused;
static int selected;
static volatile unsigned refill;
volatile unsigned dogalive_audio_underruns;
volatile int dogalive_audio_error;
static bool fill(unsigned half){
 UINT got=0;unsigned bytes=HALF_SAMPLES*2;uint8_t *dst=staging;
 FRESULT result=f_read(&music,dst,bytes,&got);
 if(result!=FR_OK){dogalive_audio_error=result;memset(dst,0,bytes);return false;}
 if(got<bytes){
  unsigned remain=bytes-got;UINT more=0;
  if(f_lseek(&music,0)!=FR_OK||f_read(&music,dst+got,remain,&more)!=FR_OK){memset(dst+got,0,remain);return false;}
  if(more<remain)memset(dst+got+more,0,remain-more);
 }
 if(playing){uint32_t remaining=__HAL_DMA_GET_COUNTER(hi2s2.hdmatx);unsigned active=remaining>HALF_SAMPLES?0:1;
  if(active==half||(remaining%HALF_SAMPLES)<256){dogalive_audio_underruns++;return true;}}
 memcpy(buffer+half*HALF_SAMPLES,staging,bytes);return true;
}
static void stop(void){if(playing)HAL_I2S_DMAStop(&hi2s2);playing=false;if(opened)f_close(&music);opened=false;refill=0;}
void DA_AudioSelect(int track,bool pause){
 if(track!=selected){stop();selected=track;paused=false;
  if(track){
   if(!mounted){FRESULT r=f_mount(&SDFatFS,SDPath,1);dogalive_audio_error=r;mounted=r==FR_OK;}
   if(mounted){FRESULT r=f_open(&music,track==1?"0:/DOGALIVE/BOSS.PCM":"0:/DOGALIVE/ENDING.PCM",FA_READ);dogalive_audio_error=r;opened=r==FR_OK;}
   if(opened&&fill(0)&&fill(1)){refill=0;playing=HAL_I2S_Transmit_DMA(&hi2s2,(uint16_t*)buffer,HALF_SAMPLES*2)==HAL_OK;}
  }
 }
 if(playing&&pause!=paused){if(pause)HAL_I2S_DMAPause(&hi2s2);else HAL_I2S_DMAResume(&hi2s2);paused=pause;}
 DA_AudioPump();
}
void DA_AudioPump(void){
 if(!playing||paused)return;
 for(unsigned half=0;half<2;half++){
  uint32_t saved=__get_PRIMASK();__disable_irq();bool needed=(refill&(1u<<half))!=0;refill&=~(1u<<half);__set_PRIMASK(saved);
  if(needed&&!fill(half)){stop();return;}
 }
}
static void completed(unsigned half){if(refill)dogalive_audio_underruns++;refill=1u<<half;}
void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef*h){if(h==&hi2s2)completed(0);}
void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef*h){if(h==&hi2s2)completed(1);}
