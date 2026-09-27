from pathlib import Path
import shutil,re
root=Path(__file__).resolve().parents[3];port=root/'ports/stm32'
target=Path('D:/watt/cube/gameboy');stage=port/'build-project'
shutil.copytree(target,stage,dirs_exist_ok=True,ignore=shutil.ignore_patterns('Debug','.settings'))
for p in stage.rglob('*'):
    if p.is_file():p.chmod(0o666)
dest=stage/'Core/Inc/dogalive'
shutil.copytree(port/'prepared',dest,dirs_exist_ok=True)
shutil.copytree(port/'SDL3',dest/'SDL3',dirs_exist_ok=True)
shutil.copyfile(port/'platform.c',stage/'Core/Src/dogalive_platform.c')
shutil.copyfile(port/'audio.c',stage/'Core/Src/dogalive_audio.c')
shutil.copyfile(port/'audio.h',dest/'audio.h')
(stage/'Core/Src/game.c').write_text('#include "src/core/game_core.c"\n#include "src/main.c"\n',encoding='utf-8')
lab=Path('D:/watt/cube/lab8.2')
s=(lab/'Core/Inc/ILI9341_STM32_Driver.h').read_text()
s=re.sub(r'(#define HSPI_INSTANCE)\s+&hspi5',r'\1 (&hspi1)',s)
s=re.sub(r'(#define LCD_CS_PORT)\s+GPIOC',r'\1 CS_GPIO_Port',s)
s=re.sub(r'(#define LCD_DC_PORT)\s+GPIOC',r'\1 D_C_GPIO_Port',s)
s=re.sub(r'(#define LCD_DC_PIN)\s+DC_Pin',r'\1 D_C_Pin',s)
(stage/'Core/Inc/ILI9341_STM32_Driver.h').write_text(s)
s=(lab/'Core/Src/ILI9341_STM32_Driver.c').read_text()
s=re.sub(r'HAL_GPIO_WritePin\(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET\);','/* RESET is held high externally. */',s)
s=re.sub(r'MX_SPI5_Init\(\);[^\n]*','/* SPI1 initialized by CubeMX main. */',s)
s=re.sub(r'MX_GPIO_Init\(\);[^\n]*','/* Preserve button and chip-select states. */',s)
a=s.index('void ILI9341_Reset(void)');b=s.index('/*Ser rotation',a)
s=s[:a]+'''void ILI9341_Reset(void)
{
    /* No dedicated reset GPIO in gameboy.ioc. LCD RESET must be held high. */
    HAL_Delay(150);
    ILI9341_Write_Command(0x01);
    HAL_Delay(150);
}

'''+s[b:]
(stage/'Core/Src/ili9341.c').write_text(s)
p=stage/'Core/Src/main.c';s=p.read_text(encoding='utf-8-sig')
s=s.replace('#include "game.h"','int DogAlive_Run(void);')
s=s.replace('  Game_Init();','  DogAlive_Run();')
s=re.sub(r'\s*Game_Update\(\);[^\n]*','',s);s=re.sub(r'\s*Game_Render\(\);[^\n]*','',s)
p.write_text(s,encoding='utf-8')
p=stage/'Core/Src/stm32f7xx_it.c';s=p.read_text(encoding='utf-8')
s=s.replace('/* USER CODE BEGIN SysTick_IRQn 1 */','/* USER CODE BEGIN SysTick_IRQn 1 */\n  extern void DA_InputTick(void);\n  DA_InputTick();')
p.write_text(s,encoding='utf-8')
# Reserve a realistic minimum heap for the small floor atlas and textures.
for name in ['STM32F767ZITX_FLASH.ld','STM32F767ZITX_RAM.ld']:
 p=stage/name;s=p.read_text();s=re.sub(r'_Min_Heap_Size = 0x[0-9a-fA-F]+;', '_Min_Heap_Size = 0x10000;',s);s=re.sub(r'_Min_Stack_Size = 0x[0-9a-fA-F]+;', '_Min_Stack_Size = 0x2000;',s);p.write_text(s)
p=stage/'gameboy.ioc';s=p.read_text();s=s.replace('ProjectManager.HeapSize=0x200','ProjectManager.HeapSize=0x10000').replace('ProjectManager.StackSize=0x1000','ProjectManager.StackSize=0x2000');p.write_text(s)
p=stage/'FATFS/Target/sd_diskio.c';s=p.read_text()
s=s.replace('while(BSP_SD_GetCardState()!= MSD_OK)', 'uint32_t started=HAL_GetTick();\n    while(BSP_SD_GetCardState()!= MSD_OK)')
s=s.replace('while(BSP_SD_GetCardState() != MSD_OK)', 'uint32_t started=HAL_GetTick();\n    while(BSP_SD_GetCardState() != MSD_OK)')
s=re.sub(r'(while\(BSP_SD_GetCardState\(\)\s*!= MSD_OK\)\s*\{)',r'\1\n      if((uint32_t)(HAL_GetTick()-started)>1000) return RES_ERROR;',s)
p.write_text(s)
# The old game/renderer is retained in the backup, not linked into this port.
(stage/'Core/Src/enemy.c').write_text('/* Superseded by DogAlive enemy simulation in game.c. */\n')
p=stage/'.cproject';s=p.read_text()
s=re.sub(r'(<option[^>]+valueType="includePath">)',r'\1\n<listOptionValue builtIn="false" value="../Core/Inc/dogalive"/>',s)
s=re.sub(r'value="[^"\n]*STM32F767ZITX_FLASH.ld"', 'value="${workspace_loc:/${ProjName}/STM32F767ZITX_FLASH.ld}"',s)
s=s.replace('kind="sourcePath" name="Core"','excluding="Inc/dogalive/**" kind="sourcePath" name="Core"')
def optimize(m):
    tag=m[0]
    tag=re.sub(r' value="[^"]*"','',tag)
    tag=re.sub(r' valueType="[^"]*"','',tag)
    return tag[:-2]+' value="com.st.stm32cube.ide.mcu.gnu.managedbuild.tool.c.compiler.option.optimization.level.value.os" valueType="enumerated"/>'
s=re.sub(r'<option[^>]*superClass="com.st.stm32cube.ide.mcu.gnu.managedbuild.tool.c.compiler.option.optimization.level"[^>]*/>',optimize,s)
p.write_text(s)
print(stage)
