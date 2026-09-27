from pathlib import Path
import subprocess,concurrent.futures,sys
root=Path(__file__).resolve().parents[3];stage=root/'ports/stm32/build-project'
gcc=next(Path('C:/ST').rglob('arm-none-eabi-gcc.exe'));out=stage/'PortBuild';out.mkdir(exist_ok=True)
inc=['Core/Inc/dogalive','Core/Inc','FATFS/Target','FATFS/App','Drivers/STM32F7xx_HAL_Driver/Inc','Drivers/STM32F7xx_HAL_Driver/Inc/Legacy','Middlewares/Third_Party/FatFs/src','Drivers/CMSIS/Device/ST/STM32F7xx/Include','Drivers/CMSIS/Include']
flags=['-mcpu=cortex-m7','-mthumb','-mfpu=fpv5-d16','-mfloat-abi=hard','-DUSE_HAL_DRIVER','-DSTM32F767xx','-Os','-g','-ffunction-sections','-fdata-sections','-fstack-usage','-Wall','-Wno-unused-parameter','--specs=nano.specs']+['-I'+str(stage/p) for p in inc]
files=list((stage/'Core/Src').glob('*.c'))+list((stage/'FATFS').rglob('*.c'))+list((stage/'Middlewares').rglob('*.c'))+list((stage/'Drivers/STM32F7xx_HAL_Driver/Src').glob('*.c'))+list((stage/'Core/Startup').glob('*.s'))
files=[p for p in files if not p.name.endswith('_template.c')]
def compile(p):
    obj=out/(str(p.relative_to(stage)).replace('/','_').replace('\\','_')+'.o')
    result=subprocess.run([str(gcc),*flags,'-c',str(p),'-o',str(obj)],capture_output=True,text=True)
    return result.returncode,result.stdout+result.stderr,obj
with concurrent.futures.ThreadPoolExecutor(max_workers=4) as pool: results=list(pool.map(compile,files))
for code,msg,obj in results:
    if msg:print(msg)
if any(code for code,_,_ in results):sys.exit(1)
rsp=out/'objects.rsp';rsp.write_text('\n'.join('"'+str(obj).replace('\\','/')+'"' for _,_,obj in results))
cmd=[str(gcc),*flags,'@'+str(rsp),'-T'+str(stage/'STM32F767ZITX_FLASH.ld'),'--specs=nosys.specs','-Wl,--gc-sections','-Wl,-Map='+str(out/'gameboy.map'),'-Wl,--print-memory-usage','-o',str(out/'gameboy.elf'),'-Wl,--start-group','-lc','-lm','-Wl,--end-group']
r=subprocess.run(cmd);sys.exit(r.returncode)
