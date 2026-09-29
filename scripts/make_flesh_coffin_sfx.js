// Build the Flesh Coffin's dark, organic cues from the two user-supplied MP3s.
// Usage: node scripts/make_flesh_coffin_sfx.js [path-to-ffmpeg]
const fs = require('fs');
const path = require('path');
const { spawnSync } = require('child_process');

const root = path.resolve(__dirname, '..');
const sourceDir = path.join(root, 'assets', 'sfx');
const outDir = path.join(sourceDir, 'coffin');
const ffmpeg = process.argv[2] || 'ffmpeg';
const rate = 44100;
const bossSfxGain = .5; // About 6 dB below the original boss effects.
fs.mkdirSync(outDir, { recursive: true });

function decode(filename) {
    const result = spawnSync(ffmpeg, [
        '-v', 'error', '-i', path.join(sourceDir, filename), '-ac', '1',
        '-ar', String(rate), '-f', 'f32le', 'pipe:1'
    ], { maxBuffer: 8 * 1024 * 1024 });
    if (result.status !== 0) throw new Error(result.stderr.toString());
    const samples = new Float32Array(result.stdout.length / 4);
    for (let i = 0; i < samples.length; i++)
        samples[i] = result.stdout.readFloatLE(i * 4);
    return samples;
}

const bones = decode('freesound_community-bones-and-flesh-movement-98677.mp3');
const blood = decode('ragecore29-htf-blood-splatter-explode-478992.mp3');
const B = (start, end, options = {}) => ({ source: bones, start, end, ...options });
const W = (start, end, options = {}) => ({ source: blood, start, end, ...options });

// Each move has a separate cut, speed and layering; phases keep their source
// recordings recognizable while attacks share the same bone-and-blood palette.
const cues = {
    awakening:       { layers: [B(.26, 2.42, { speed: .55, gain: 1.0 }), B(1.0, 1.65, { speed: .55, gain: .48, at: 3.6 })], sub: .18, room: .17, density: 2.2 },
    eye_warning:     { layers: [B(.63, 1.42, { reverse: true, speed: .88, gain: .86 })], room: .2 },
    sword_release:   { layers: [B(.46, 1.38, { speed: 1.28, gain: 1.0 })], room: .11 },
    sword_return:    { layers: [B(.64, 1.52, { reverse: true, speed: 1.19, gain: .9 })], room: .12 },
    ground_wave:     { layers: [W(.04, .61, { speed: 1.12, gain: .82 }), B(.69, 1.17, { speed: 1.15, gain: .3 })], sub: .14, room: .13 },
    stab_warning:    { layers: [B(1.09, 1.71, { reverse: true, speed: .91, gain: .9 })], room: .16 },
    stab:            { layers: [W(.035, .43, { speed: 1.4, gain: .86 }), B(.70, 1.10, { speed: 1.48, gain: .35 })], sub: .12, room: .08 },
    wall_blades:     { layers: [B(.47, 1.53, { speed: 1.2, gain: .48 }), W(.05, .63, { speed: 1.31, gain: .67, at: .1 })], room: .14 },
    orbit:           { layers: [B(.46, 1.84, { reverse: true, speed: .72, gain: .95 })], room: .22, density: 1.5 },
    teleport:        { layers: [B(.70, 1.83, { reverse: true, speed: .95, gain: .72 }), W(.29, .64, { speed: .92, gain: .52, at: .56 })], room: .23 },
    slam:            { layers: [W(.01, 1.16, { speed: .87, gain: 1.0 }), B(1.16, 1.85, { speed: .9, gain: .45, at: .07 })], sub: .32, room: .22 },
    dash:            { layers: [B(.43, 1.32, { speed: 1.48, gain: .82 }), W(.07, .36, { speed: 1.24, gain: .37, at: .16 })], room: .14 },
    trail_warning:   { layers: [B(1.09, 1.86, { reverse: true, speed: .43, gain: .85 })], room: .18, density: 1.4 },
    trail_strike:    { layers: [W(.015, .86, { speed: 1.13, gain: 1.0 })], sub: .22, room: .14 },
    slash:           { layers: [B(.52, 1.24, { speed: 1.34, gain: .83 }), W(.08, .55, { speed: 1.3, gain: .58, at: .13 })], room: .12 },
    charge:          { layers: [B(.42, 1.98, { reverse: true, speed: .5, gain: .9 })], sub: .12, room: .2, density: 1.55 },
    charged_slash:   { layers: [B(.42, 1.44, { speed: 1.08, gain: .84 }), W(.01, .87, { speed: .85, gain: .91, at: .16 })], sub: .23, room: .18 },
    sword_block:     { layers: [B(.74, 1.27, { speed: 1.27, gain: .88 }), W(.09, .37, { speed: 1.32, gain: .55 })], room: .1 },
    sword_break:     { layers: [B(.42, 1.82, { speed: .95, gain: .87 }), W(.01, 1.2, { speed: .82, gain: .9, at: .1 })], sub: .27, room: .2 },
    phase_two:       { layers: [W(0, 1.52, { speed: .82, gain: 1.0 }), B(.56, 1.86, { speed: .33, gain: .53, at: 1.4 }), W(.17, .86, { speed: .9, gain: .52, at: 5.35 })], sub: .35, room: .27 },
    stun:            { layers: [B(1.13, 2.04, { speed: .81, gain: .85 }), W(.21, .68, { speed: .95, gain: .32, at: .16 })], room: .16 },
    death:           { layers: [W(.01, 1.55, { speed: .71, gain: .93 }), B(.47, 2.42, { speed: .52, gain: .88, at: .16 })], sub: .4, room: .3 }
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
    seconds += .18;
    const output = new Float32Array(Math.ceil(seconds * rate));
    for (const layer of config.layers) {
        const length = Math.floor((layer.end - layer.start) * rate / (layer.speed || 1));
        const at = Math.floor((layer.at || 0) * rate);
        let envelope = 0;
        for (let i = 0; i < length && at + i < output.length; i++) {
            const time = i / length;
            const original = layer.reverse
                ? layer.end * rate - i * (layer.speed || 1)
                : layer.start * rate + i * (layer.speed || 1);
            const raw = sampleAt(layer.source, original);
            envelope = Math.max(Math.abs(raw), envelope * .9985);
            const gain = Math.min(5.5, Math.pow(.16 / Math.max(envelope, .012), .47));
            const fade = Math.min(1, i / (rate * .012), (length - i) / (rate * .09));
            output[at + i] += Math.tanh(raw * gain * 1.4) * fade * (layer.gain || 1);
        }
    }
    if (config.sub) {
        let phase = 0;
        for (let i = 0; i < output.length; i++) {
            const t = i / rate;
            phase += 2 * Math.PI * (72 * Math.exp(-t * 5) + 31) / rate;
            output[i] += Math.sin(phase) * Math.exp(-t * 7) * config.sub;
        }
    }
    const dry = output.slice();
    const room = config.room || 0;
    const taps = [[.047, .38], [.091, .23], [.149, .13]];
    for (const [delay, amount] of taps) {
        const shift = Math.round(delay * rate);
        for (let i = shift; i < output.length; i++) output[i] += dry[i - shift] * amount * room;
    }
    let peak = 0;
    for (const value of output) peak = Math.max(peak, Math.abs(value));
    const density = config.density || 1;
    const pcm = Buffer.alloc(output.length * 2);
    for (let i = 0; i < output.length; i++) {
        const normalized = peak ? output[i] / peak : 0;
        const shaped = Math.tanh(normalized * density) / Math.tanh(density) * .68 * bossSfxGain;
        pcm.writeInt16LE(Math.round(Math.max(-1, Math.min(1, shaped)) * 32767), i * 2);
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

for (const [name, config] of Object.entries(cues)) {
    const destination = path.join(outDir, `${name}.wav`);
    const data = wav(render(config));
    fs.writeFileSync(destination, data);
    process.stdout.write(`${name}: ${(data.length / (rate * 2)).toFixed(2)} s\n`);
}
