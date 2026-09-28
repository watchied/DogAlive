from pathlib import Path
from datetime import datetime
import shutil

root = Path('D:/watt/cube/gameboy')
backup = root / 'Backup' / ('lcd-diagnostic-' + datetime.now().strftime('%Y%m%d-%H%M%S'))
changes = {}
p = root / 'Core/Src/ili9341.c'
s = p.read_text(encoding='utf-8-sig')
assert 'lcd_spi_failures' not in s
anchor = '/* Global Variables'
i = s.index(anchor)
s = s[:i] + '''/* Debugger-visible transport status; HAL_OK does not confirm LCD reception. */
volatile uint32_t lcd_spi_failures;
volatile uint32_t lcd_spi_last_status;
volatile uint32_t lcd_spi_last_error;
static HAL_StatusTypeDef LCD_Transmit(SPI_HandleTypeDef *spi, uint8_t *data,
                                    uint16_t size, uint32_t timeout)
{
    HAL_StatusTypeDef status = HAL_SPI_Transmit(spi, data, size, timeout);
    if (status != HAL_OK) {
        lcd_spi_failures++;
        lcd_spi_last_status = status;
        lcd_spi_last_error = HAL_SPI_GetError(spi);
    }
    return status;
}

''' + s[i:].replace('HAL_SPI_Transmit(', 'LCD_Transmit(')
changes[p] = s
p = root / 'Core/Src/dogalive_platform.c'
s = p.read_text(encoding='utf-8-sig')
old = 'bool SDL_Init(int f){ILI9341_Init();ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);inputReady=true;return true;}'
assert old in s
s = s.replace(old, '''/* Temporary startup diagnostic: direct driver fills bypass game rendering. */
volatile uint32_t lcd_test_stage;
bool SDL_Init(int f){
 lcd_test_stage=1;
 /* Slow SPI during initialization/test to help diagnose wiring signal quality. */
 __HAL_SPI_DISABLE(HSPI_INSTANCE);
 MODIFY_REG(HSPI_INSTANCE->Instance->CR1, SPI_CR1_BR, SPI_BAUDRATEPRESCALER_128);
 __HAL_SPI_ENABLE(HSPI_INSTANCE);
 ILI9341_Init();ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);
 lcd_test_stage=2;ILI9341_Fill_Screen(RED);HAL_Delay(1500);
 lcd_test_stage=3;ILI9341_Fill_Screen(GREEN);HAL_Delay(1500);
 lcd_test_stage=4;ILI9341_Fill_Screen(BLUE);HAL_Delay(1500);
 lcd_test_stage=5;ILI9341_Fill_Screen(BLACK);HAL_Delay(500);
 __HAL_SPI_DISABLE(HSPI_INSTANCE);
 MODIFY_REG(HSPI_INSTANCE->Instance->CR1, SPI_CR1_BR, HSPI_INSTANCE->Init.BaudRatePrescaler);
 __HAL_SPI_ENABLE(HSPI_INSTANCE);
 lcd_test_stage=6;inputReady=true;return true;
}''')
s = s.replace('#include "fonts.h"', '#include "fonts.h"\n#include "spi.h"')
changes[p] = s
for p, s in changes.items():
 b = backup / p.relative_to(root)
 b.parent.mkdir(parents=True, exist_ok=True)
 shutil.copy2(p, b)
 p.write_text(s, encoding='utf-8')
print('Startup LCD diagnostic installed. Backup:', backup)
