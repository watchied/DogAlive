from pathlib import Path
import datetime,shutil,re
target=Path('D:/watt/cube/gameboy')
backup=target/'Backup'/('lcd-pd13-'+datetime.datetime.now().strftime('%Y%m%d-%H%M%S'))
files=['gameboy.ioc','Core/Inc/main.h','Core/Src/gpio.c','Core/Inc/ILI9341_STM32_Driver.h','Core/Src/ili9341.c']
for rel in files:
    p=target/rel;b=backup/rel;b.parent.mkdir(parents=True,exist_ok=True);shutil.copy2(p,b)
    s=p.read_text(encoding='utf-8-sig')
    if rel=='gameboy.ioc':
        lines=s.splitlines();count=int(next(x.split('=')[1] for x in lines if x.startswith('Mcu.PinsNb=')))
        assert not any(x.startswith('PD13.') for x in lines),'PD13 already configured; inspect before changing'
        lines=[f'Mcu.PinsNb={count+1}' if x.startswith('Mcu.PinsNb=') else x for x in lines]
        lines += [f'Mcu.Pin{count}=PD13','PD13.GPIOParameters=PinState,GPIO_Label','PD13.GPIO_Label=LCD_RST','PD13.PinState=GPIO_PIN_SET','PD13.Locked=true','PD13.Signal=GPIO_Output']
        s='\n'.join(lines)+'\n'
    elif rel=='Core/Inc/main.h':
        s=s.replace('/* USER CODE BEGIN Private defines */','/* USER CODE BEGIN Private defines */\n#ifndef LCD_RST_Pin\n#define LCD_RST_Pin GPIO_PIN_13\n#define LCD_RST_GPIO_Port GPIOD\n#endif')
    elif rel=='Core/Src/gpio.c':
        # GPIO init USER CODE survives CubeMX regeneration, including a later
        # generated initialization of the same pin from the updated .ioc.
        anchor='/* USER CODE END MX_GPIO_Init_2 */'
        if anchor not in s:
            anchor='\n}\n\n/* USER CODE BEGIN 2 */'
            replacement='''
  /* USER CODE BEGIN MX_GPIO_Init_2 */
  HAL_GPIO_WritePin(LCD_RST_GPIO_Port, LCD_RST_Pin, GPIO_PIN_SET);
  GPIO_InitStruct.Pin = LCD_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LCD_RST_GPIO_Port, &GPIO_InitStruct);
  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 2 */'''
            assert anchor in s;s=s.replace(anchor,replacement,1)
        else:raise RuntimeError('Existing GPIO user block: inspect first')
    elif rel.endswith('ILI9341_STM32_Driver.h'):
        s=re.sub(r'(#define\s+LCD_RST_PORT)\s+GPIOC',r'\1 LCD_RST_GPIO_Port',s)
        s=re.sub(r'(#define\s+LCD_RST_PIN)\s+RST_Pin',r'\1 LCD_RST_Pin',s)
    else:
        a=s.index('void ILI9341_Reset(void)');b=s.index('/*Ser rotation',a)
        s=s[:a]+'''void ILI9341_Reset(void)
{
    HAL_GPIO_WritePin(LCD_CS_PORT, LCD_CS_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(10);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_RESET);
    HAL_Delay(20);
    HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);
    HAL_Delay(150);
}

'''+s[b:]
        s=s.replace('/* RESET is held high externally. */','HAL_GPIO_WritePin(LCD_RST_PORT, LCD_RST_PIN, GPIO_PIN_SET);')
    p.write_text(s,encoding='utf-8')
print('Hardware reset configured on PD13. Backup:',backup)
