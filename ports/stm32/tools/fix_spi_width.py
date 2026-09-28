from pathlib import Path
import datetime, shutil
target=Path('D:/watt/cube/gameboy')
backup=target/'Backup'/('spi-width-'+datetime.datetime.now().strftime('%Y%m%d-%H%M%S'))
backup.mkdir(parents=True)
for rel in ['Core/Src/spi.c','gameboy.ioc']:
    p=target/rel
    shutil.copy2(p,backup/p.name)
    s=p.read_text(encoding='utf-8-sig')
    if rel.endswith('.c'):
        assert 'hspi1.Init.DataSize = SPI_DATASIZE_4BIT;' in s
        s=s.replace('hspi1.Init.DataSize = SPI_DATASIZE_4BIT;', 'hspi1.Init.DataSize = SPI_DATASIZE_8BIT;')
    else:
        lines=s.splitlines()
        lines=[line for line in lines if not line.startswith('SPI1.DataSize=')]
        for i,line in enumerate(lines):
            if line.startswith('SPI1.IPParameters=') and 'DataSize' not in line.split('=',1)[1].split(','):
                lines[i]=line+',DataSize'
        lines.append('SPI1.DataSize=SPI_DATASIZE_8BIT')
        s='\n'.join(lines)+'\n'
    p.write_text(s,encoding='utf-8')
print('SPI1 changed to 8-bit in source and CubeMX. Backup:',backup)
