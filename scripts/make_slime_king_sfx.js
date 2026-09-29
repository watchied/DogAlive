// Build wet Slime King cues from the five recordings supplied by the user.
// Usage: node scripts/make_slime_king_sfx.js [path-to-ffmpeg]
const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const root = path.resolve(__dirname, '..');
const sourceDir = path.join(root, 'assets', 'sfx');
const outDir = path.join(sourceDir, 'slime_king');
const ffmpeg = process.argv[2] || 'ffmpeg';
const rate = 44100;
const bossSfxGain = .8; // About 2 dB below the original boss effects.
fs.mkdirSync(outDir, { recursive: true });

function decode(filename) {
    const result = spawnSync(ffmpeg, [
        '-v', 'error', '-i', path.join(sourceDir, filename),
        '-ac', '1', '-ar', String(rate), '-f', 'f32le', 'pipe:1'
    ], { maxBuffer: 8 * 1024 * 1024 });
    if (result.status !== 0) throw new Error(result.stderr.toString());
    const samples = new Float32Array(result.stdout.length / 4);
    for (let i = 0; i < samples.length; i++)
        samples[i] = result.stdout.readFloatLE(i * 4);
    return samples;
}

const goop = decode('floraphonic-goopy-slime-4-219777.mp3');
const impact = decode('universfield-slime-impact-352473.mp3');
const splatter = decode('floraphonic-slime-splatter-1-220262.mp3');
const pop = decode('soundreality-pop-sound-423716.mp3');
const squish = decode('floraphonic-slime-squish-5-218569.mp3');
const G = (start, end, options = {}) => ({ source: goop, start, end, ...options });
const I = (start, end, options = {}) => ({ source: impact, start, end, ...options });
const S = (start, end, options = {}) => ({ source: splatter, start, end, ...options });
const P = (start, end, options = {}) => ({ source: pop, start, end, ...options });
const Q = (start, end, options = {}) => ({ source: squish, start, end, ...options });

// Short cues stay short during projectile volleys. Long cues match the intro,
// phase change, beam charge and death without replacing the game's music.
const cues = {
    intro:          { layers: [G(.04, .59, { speed: .47, gain: .9, lowpass: 2500 }), I(.06, .48, { speed: .83, gain: .45, at: .57 }), S(.03, .39, { speed: .95, gain: .38, at: 1.09 })], sub: .09, room: .25, peak: .49 },
    hurt:           { layers: [Q(.055, .38, { speed: 1.16, gain: .9 }), I(.07, .29, { speed: 1.35, gain: .35, at: .035 })], room: .14, peak: .35 },
    bubble_launch:  { layers: [G(.07, .51, { speed: 1.27, gain: .8 }), P(.105, .27, { speed: 1.14, gain: .32, at: .15 })], room: .19, peak: .35 },
    bubble_bounce:  { layers: [Q(.07, .26, { speed: 1.56, gain: .78 }), P(.11, .21, { speed: 1.45, gain: .16, at: .02 })], room: .1, peak: .27 },
    bubble_pop:     { layers: [P(.104, .285, { speed: 1.13, gain: .69 }), S(.035, .25, { speed: 1.48, gain: .47, at: .025 })], room: .12, peak: .34 },
    dash_warning:   { layers: [G(.08, .55, { speed: .77, reverse: true, gain: .83, lowpass: 1900 }), Q(.07, .27, { speed: .91, reverse: true, gain: .36, at: .19 })], room: .2, peak: .34 },
    dash:           { layers: [G(.08, .61, { speed: 1.04, gain: .83 }), S(.03, .37, { speed: 1.27, gain: .56, at: .14 })], room: .19, peak: .47 },
    burrow:         { layers: [G(.07, .58, { speed: .65, gain: .9, lowpass: 1600 }), Q(.06, .33, { speed: 1.06, gain: .4, at: .29 })], room: .24, peak: .39 },
    slam_warning:   { layers: [I(.075, .48, { speed: .72, reverse: true, gain: .73, lowpass: 2200 }), G(.08, .45, { speed: 1.08, reverse: true, gain: .42, at: .12 })], room: .2, peak: .36 },
    slam:           { layers: [I(.065, .63, { speed: .89, gain: .86 }), S(.03, .50, { speed: .98, gain: .75, at: .045 }), G(.08, .36, { speed: 1.09, gain: .3, at: .2 })], sub: .16, room: .27, peak: .54 },
    phase:          { layers: [G(.04, .60, { speed: .42, gain: .86, lowpass: 2300 }), S(.03, .49, { speed: .78, gain: .68, at: .49 }), I(.07, .58, { speed: .78, gain: .7, at: 1.38 })], sub: .13, room: .29, peak: .53 },
    laser_warning:  { layers: [Q(.06, .35, { speed: .61, reverse: true, gain: .76, lowpass: 2500 }), G(.09, .37, { speed: 1.03, reverse: true, gain: .31, at: .22 })], room: .16, peak: .31 },
    laser:          { layers: [P(.105, .26, { speed: 1.32, gain: .66 }), S(.05, .22, { speed: 1.72, gain: .39, at: .025 })], room: .12, peak: .28 },
    laser_bounce:   { layers: [P(.112, .225, { speed: 1.61, gain: .61 }), Q(.075, .20, { speed: 1.59, gain: .32, at: .013 })], room: .1, peak: .23 },
    beam_charge:    { layers: [G(.04, .60, { speed: .31, reverse: true, gain: .81, lowpass: 2400 }), Q(.07, .34, { speed: .79, reverse: true, gain: .37, at: 1.04 }), I(.08, .43, { speed: 1.48, reverse: true, gain: .28, at: 1.57 })], room: .27, peak: .46 },
    beam_fire:      { layers: [I(.07, .64, { speed: .76, gain: .83 }), S(.025, .53, { speed: .85, gain: .78, at: .055 })], sub: .11, room: .25, peak: .54 },
    beam_interrupt: { layers: [Q(.055, .43, { speed: .87, gain: .88 }), G(.09, .47, { speed: 1.16, gain: .49, at: .09 })], room: .2, peak: .42 },
    death:          { layers: [G(.04, .60, { speed: .46, gain: .8, lowpass: 1700 }), S(.03, .53, { speed: .63, gain: .84, at: .25 }), I(.07, .64, { speed: .86, gain: .71, at: 1.07 })], sub: .2, room: .32, peak: .56 },
    npc:            { layers: [Q(.06, .34, { speed: .91, gain: .76 }), G(.09, .40, { speed: .96, gain: .4, at: .06 })], room: .17, peak: .31 }
};

function sampleAt(data, point) {
    const index = Math.floor(point);
    return index >= 0 && index + 1 < data.length
        ? data[index] + (data[index + 1] - data[index]) * (point - index) : 0;
}

function render(config) {
    let seconds = 0;
    for (const layer of config.layers)
        seconds = Math.max(seconds, (layer.at || 0) + (layer.end - layer.start) / (layer.speed || 1));
    const output = new Float32Array(Math.ceil((seconds + .22) * rate));
    for (const layer of config.layers) {
        const speed = layer.speed || 1;
        const duration = (layer.end - layer.start) / speed;
        const count = Math.floor(duration * rate);
        const start = Math.floor((layer.at || 0) * rate);
        const alpha = 1 - Math.exp(-2 * Math.PI * (layer.lowpass || 7500) / rate);
        let smoothed = 0, envelope = 0;
        for (let i = 0; i < count && start + i < output.length; i++) {
            const point = (layer.reverse ? layer.end : layer.start) * rate +
                (layer.reverse ? -1 : 1) * i * speed;
            smoothed += alpha * (sampleAt(layer.source, point) - smoothed);
            envelope = Math.max(Math.abs(smoothed), envelope * .998);
            const body = Math.min(2.8, Math.pow(.18 / Math.max(envelope, .018), .4));
            const fade = Math.min(1, i / (rate * .004),
                (count - i) / (rate * Math.min(.07, duration * .35)));
            output[start + i] += smoothed * body * fade * layer.gain;
        }
    }
    if (config.sub) {
        let phase = 0;
        for (let i = 0; i < output.length; i++) {
            const t = i / rate;
            phase += 2 * Math.PI * (75 * Math.exp(-t * 5) + 35) / rate;
            output[i] += Math.sin(phase) * Math.exp(-t * 6) * config.sub;
        }
    }
    const dry = output.slice();
    for (const [delay, gain] of [[.047, .41], [.108, .26], [.179, .16]]) {
        const shift = Math.floor(delay * rate);
        for (let i = shift; i < output.length; i++)
            output[i] += dry[i - shift] * gain * config.room;
    }
    let peak = 0;
    for (const sample of output) peak = Math.max(peak, Math.abs(sample));
    if (!peak) throw new Error('Silent Slime King cue');
    const pcm = Buffer.alloc(output.length * 2);
    for (let i = 0; i < output.length; i++) {
        const shaped = Math.tanh(output[i] / peak * 1.25) / Math.tanh(1.25);
        pcm.writeInt16LE(Math.round(shaped * config.peak * bossSfxGain * 32767), i * 2);
    }
    return pcm;
}

function wav(pcm) {
    const header = Buffer.alloc(44);
    header.write('RIFF', 0); header.writeUInt32LE(36 + pcm.length, 4);
    header.write('WAVEfmt ', 8); header.writeUInt32LE(16, 16);
    header.writeUInt16LE(1, 20); header.writeUInt16LE(1, 22);
    header.writeUInt32LE(rate, 24); header.writeUInt32LE(rate * 2, 28);
    header.writeUInt16LE(2, 32); header.writeUInt16LE(16, 34);
    header.write('data', 36); header.writeUInt32LE(pcm.length, 40);
    return Buffer.concat([header, pcm]);
}

const rendered = {};
for (const [name, config] of Object.entries(cues)) {
    rendered[name] = render(config);
    fs.writeFileSync(path.join(outDir, `${name}.wav`), wav(rendered[name]));
    process.stdout.write(`${name}: ${(rendered[name].length / (rate * 2)).toFixed(2)} s\n`);
}

const previewNames = [
    'intro', 'bubble_launch', 'bubble_bounce', 'bubble_pop', 'dash_warning',
    'dash', 'burrow', 'slam_warning', 'slam', 'phase', 'laser_warning',
    'laser', 'beam_charge', 'beam_fire', 'hurt', 'death'
];
const silence = Buffer.alloc(Math.round(rate * .24) * 2);
const preview = Buffer.concat(previewNames.flatMap(name => [rendered[name], silence]));
fs.writeFileSync(path.join(sourceDir, 'slime_king_preview.wav'), wav(preview));
