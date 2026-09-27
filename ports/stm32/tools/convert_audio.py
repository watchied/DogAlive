from pathlib import Path
import sys
sys.path.insert(0,str(Path(__file__).parent/'vendor'))
import miniaudio
root=Path(__file__).resolve().parents[3]
out=root/'ports/stm32/sd-card/DOGALIVE';out.mkdir(parents=True,exist_ok=True)
for source,name in [('boss_phase1.mp3','BOSS.PCM'),('Ending song.mp3','ENDING.PCM')]:
    sound=miniaudio.decode_file(str(root/'assets/music'/source),output_format=miniaudio.SampleFormat.SIGNED16,nchannels=2,sample_rate=43831)
    # Conservative software volume; preserves the original tracks.
    for i in range(len(sound.samples)):sound.samples[i]=int(sound.samples[i]*0.35)
    (out/name).write_bytes(sound.samples.tobytes())
    print(name,len(sound.samples)*2,'bytes, 43831 Hz stereo signed 16-bit LE')
