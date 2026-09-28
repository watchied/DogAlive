from pathlib import Path
from datetime import datetime
import shutil

p=Path('D:/watt/cube/gameboy/Core/Inc/dogalive/src/main.c')
s=p.read_text(encoding='utf-8-sig')
old='''        if (deltaTime > 0.05f)
            deltaTime = 0.05f;'''
assert old in s
s=s.replace(old,'''        // Ignore long debugger stops; normal slow frames retain elapsed time.
        if (deltaTime > 1.0f)
            deltaTime = 1.0f;''',1)
a=s.index('        if(gameState==GAME_PLAYING) {',s.index('const bool *keyboardState = SDL_GetKeyboardState(NULL);'))
b=s.index('        bool coffinEncounter=',a)
body=s[a:b]
s=s[:a]+'''        // Run bounded simulation steps, then render only once per frame.
        // Equal steps preserve the elapsed time without a large collision jump.
        int simulationSteps = (int)ceilf(deltaTime / 0.02f);
        if (simulationSteps < 1) simulationSteps = 1;
        deltaTime /= (float)simulationSteps;
        for (int simulationStep = 0; simulationStep < simulationSteps; ++simulationStep) {
'''+body+'''            // A release event may fire an arrow/use a potion only once.
            shootRequested = false;
            if (gameState != GAME_PLAYING) break;
        }
'''+s[b:]
assert '        SDL_Delay(16);' in s
s=s.replace('        SDL_Delay(16);','''        uint64_t frameElapsed = SDL_GetTicks() - currentTime;
        if (frameElapsed < 16) SDL_Delay((uint32_t)(16 - frameElapsed));''',1)
backup=p.parents[4]/'Backup'/('frame-timing-'+datetime.now().strftime('%Y%m%d-%H%M%S'))
backup.mkdir(parents=True,exist_ok=True)
shutil.copy2(p,backup/'main.c')
p.write_text(s,encoding='utf-8')
print('Updated frame timing. Backup:',backup)
