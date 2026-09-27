"""Install the validated staged port, with a complete pre-install ZIP backup."""
from pathlib import Path
import datetime,hashlib,json,shutil,subprocess,zipfile
root=Path(__file__).resolve().parents[3];port=root/'ports/stm32';stage=port/'build-project'
target=Path('D:/watt/cube/gameboy').resolve()
assert str(target).lower().replace('\\','/')=='d:/watt/cube/gameboy'
assert (target/'gameboy.ioc').is_file()
assert (stage/'Debug/gameboy.elf').is_file()
backup=target/'Backup'/('before-dogalive-'+datetime.datetime.now().strftime('%Y%m%d-%H%M%S')+'.zip')
backup.parent.mkdir(exist_ok=True)
files=[p for p in target.rglob('*') if p.is_file() and 'Backup' not in p.relative_to(target).parts]
with zipfile.ZipFile(backup,'w',zipfile.ZIP_DEFLATED) as archive:
    for p in files:archive.write(p,p.relative_to(target))
with zipfile.ZipFile(backup) as archive:
    assert archive.testzip() is None
changed=[
 '.cproject','gameboy.ioc','STM32F767ZITX_FLASH.ld','STM32F767ZITX_RAM.ld',
 'Core/Src/main.c','Core/Src/stm32f7xx_it.c','Core/Src/game.c','Core/Src/enemy.c',
 'Core/Src/ili9341.c','Core/Src/dogalive_platform.c','Core/Src/dogalive_audio.c',
 'Core/Inc/ILI9341_STM32_Driver.h','FATFS/Target/sd_diskio.c']
for p in (stage/'Core/Inc/dogalive').rglob('*'):
    if p.is_file():changed.append(p.relative_to(stage).as_posix())
manifest={}
for rel in changed:
    source=stage/rel;dest=target/rel;dest.parent.mkdir(parents=True,exist_ok=True)
    if dest.exists():dest.chmod(0o666)
    shutil.copyfile(source,dest)
    a=hashlib.sha256(source.read_bytes()).hexdigest();b=hashlib.sha256(dest.read_bytes()).hexdigest()
    assert a==b;manifest[rel]=a
for name in ['README.md','VALIDATION.md']:
    shutil.copyfile(port/name,target/('DOGALIVE_'+name))
shutil.copytree(port/'sd-card',target/'SDCard',dirs_exist_ok=True)
firmware=target/'Firmware';firmware.mkdir(exist_ok=True)
shutil.copyfile(stage/'Debug/gameboy.elf',firmware/'gameboy.elf')
shutil.copyfile(stage/'Debug/gameboy.map',firmware/'gameboy.map')
objcopy=next(Path('C:/ST').rglob('arm-none-eabi-objcopy.exe'))
subprocess.run([str(objcopy),'-O','binary',str(firmware/'gameboy.elf'),str(firmware/'gameboy.bin')],check=True)
(target/'DOGALIVE_INSTALL.json').write_text(json.dumps({'backup':str(backup),'sha256':manifest},indent=2))
print('Installed',len(changed),'files into',target)
print('Verified backup:',backup)
print('Firmware:',firmware/'gameboy.elf')
print('SD card files:',target/'SDCard/DOGALIVE')
