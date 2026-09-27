from pathlib import Path
import re, itertools

root = Path(__file__).resolve().parents[3]
raw = packed = 0
for path in (root / 'assets/sprites').rglob('*.h'):
    size = compressed = 0
    for match in re.finditer(r'static const uint16_t\s+(\w+)\s*\[\s*\]\s*=\s*\{([^}]+)\}', path.read_text(encoding='utf-8-sig'), re.S):
        values = [int(x, 16) for x in re.findall(r'0x[0-9a-fA-F]+', match[2])]
        size += len(values)*2
        compressed += sum(4*((sum(1 for _ in group)+65534)//65535) for _,group in itertools.groupby(values))
    raw += size
    packed += compressed
    if size > 100000:
        print(path.name, size, compressed)
print('TOTAL raw bytes:', raw, 'RLE bytes:', packed)
