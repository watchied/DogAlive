# Dog Alive

เกมภาษา C ใช้ SDL3 สำหรับแสดงผลบน PC

## โครงสร้างโปรเจกต์

```text
src/
  main.c                 หน้าต่างเกม ลูปหลัก เมนู และการวาดฉาก
  core/                  การตั้งค่าและอัปเดตเกม
  player/                ข้อมูลและค่าตั้งต้นของผู้เล่น
  enemies/               พฤติกรรม Ghoul, Flesh Slime และ Eye Parasite
  combat/                การโจมตี ลูกธนู และการสะท้อนกระสุน
  effects/               เอฟเฟกต์วิ่ง
  ui/                    หลอดเลือด สตามินา หลอดธนู และหัวใจ
assets/sprites/
  player/                ภาพผู้เล่น
  enemies/               ภาพศัตรู
  projectiles/           ภาพลูกธนูและกระสุน Slime
  effects/               ภาพเอฟเฟกต์
  ui/                    ภาพหลอดและหัวใจที่ส่งออกจาก Aseprite
tests/                   ชุดทดสอบการต่อสู้
bin/                     game.exe และ SDL3.dll
include/SDL3/            Header ของ SDL3
lib/                     ไลบรารี SDL3 สำหรับบิลด์
.vscode/                 คำสั่งบิลด์ของ VS Code
```

## บิลด์และเล่น (PowerShell)

วิธีง่าย: บันทึกไฟล์ที่แก้ แล้วดับเบิลคลิก `run_game.cmd` ที่โฟลเดอร์หลัก สคริปต์จะบิลด์ค่าล่าสุดก่อนเปิด `bin/game.exe` ทุกครั้ง ถ้าบิลด์ไม่ผ่านจะไม่เปิดเกมเก่า เมนูเกมแสดงจำนวนศัตรูที่บิลด์ไว้ด้วย

รันจากโฟลเดอร์หลักของโปรเจกต์ หรือกด Ctrl+Shift+B ใน VS Code:

```powershell
gcc -std=c11 -Wall -Wextra -Werror src/main.c src/core/game_core.c -o bin/game.exe -I. -Iinclude -Llib -lmingw32 -lSDL3
.\bin\game.exe
```

ให้ `SDL3.dll` อยู่ใน `bin` ข้าง `game.exe`

## ทดสอบ

```powershell
gcc -std=c11 -Wall -Wextra -Werror tests/enemy_combat_test.c -o bin/enemy_combat_test.exe -I. -Iinclude -Llib -lSDL3
.\bin\enemy_combat_test.exe
```

## จุดที่ปรับแต่งได้

- จำนวนศัตรู: `GHOUL_COUNT`, `FLESH_SLIME_COUNT`, `EYE_PARASITE_COUNT` ใน `src/core/game_config.h` (0–10 ตัวต่อชนิด, 0 คือปิดชนิดนั้น) จากนั้นบิลด์และเปิดเกมใหม่
- จุดเกิด: `EnemyGroup_Init` ใน `src/combat/enemy_group.h`

ทดสอบระบบหลายตัว:

```powershell
gcc -std=c11 -Wall -Wextra -Werror tests/enemy_group_test.c -o bin/enemy_group_test.exe -I. -Iinclude -Llib -lSDL3
.\bin\enemy_group_test.exe
```

- ค่าผู้เล่น: `src/player/player.h`
- เวลาเลือกที่จะระเบิดและระยะระเบิด: `src/enemies/eye_parasite.h`
- ความเร็วกระสุนและจังหวะยิง Slime: `src/enemies/flesh_slime.h`
- ขนาดและการจัดวางหลอด: `src/ui/health_ui.h`
- หน้าเริ่มเกม / Pause / Game Over: `Draw_GameUI` ใน `src/main.c`

Header ของเกมใช้พาธจากโฟลเดอร์หลัก จึงต้องมี `-I.` ในคำสั่งบิลด์
