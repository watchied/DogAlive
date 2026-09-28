from pathlib import Path
from datetime import datetime
import shutil

root=Path('D:/watt/cube/gameboy')
backup=root/'Backup'/('any-button-start-'+datetime.now().strftime('%Y%m%d-%H%M%S'))
changes={}
p=root/'Core/Src/dogalive_platform.c'
s=p.read_text(encoding='utf-8-sig')
a=s.index('/* Temporary startup diagnostic:')
b=s.index('SDL_Window *SDL_CreateWindow',a)
s=s[:a]+'''bool SDL_Init(int f){
 /* Retain the working slow initialization, then restore gameplay SPI speed. */
 __HAL_SPI_DISABLE(HSPI_INSTANCE);
 MODIFY_REG(HSPI_INSTANCE->Instance->CR1, SPI_CR1_BR, SPI_BAUDRATEPRESCALER_128);
 __HAL_SPI_ENABLE(HSPI_INSTANCE);
 ILI9341_Init();ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
 __HAL_SPI_DISABLE(HSPI_INSTANCE);
 MODIFY_REG(HSPI_INSTANCE->Instance->CR1, SPI_CR1_BR, HSPI_INSTANCE->Init.BaudRatePrescaler);
 __HAL_SPI_ENABLE(HSPI_INSTANCE);
 inputReady=true;return true;
}
'''+s[b:]
changes[p]=s
p=root/'Core/Inc/dogalive/src/main.c'
s=p.read_text(encoding='utf-8-sig')
assert '(gameState == GAME_MENU && key == SDL_SCANCODE_RETURN)' in s
s=s.replace('(gameState == GAME_MENU && key == SDL_SCANCODE_RETURN)','(gameState == GAME_MENU)')
s=s.replace('"Start: Start"','"Press any button to start"')
anchor='                    Reset_Game(&stage, &player, &enemies, projectiles, slimeShots, &runningEffect);'
assert anchor in s
s=s.replace(anchor,anchor+'\n                    player.attackWasDown = SDL_GetKeyboardState(NULL)[SDL_SCANCODE_SPACE];')
changes[p]=s
for p,s in changes.items():
 b=backup/p.relative_to(root);b.parent.mkdir(parents=True,exist_ok=True)
 shutil.copy2(p,b);p.write_text(s,encoding='utf-8')
print('Removed color test; any menu button starts. Backup:',backup)
