// Generate an original, short sword swoosh for the player's normal attack.
// Run from any directory: node scripts/generate_normal_slash.js
const fs = require('node:fs');
const path = require('node:path');

const rate = 44100;
const duration = 0.42;
const count = Math.round(rate * duration);
const samples = new Float64Array(count);
let seed = 0x5a17c0de;
let lowFast = 0;
let lowSlow = 0;

function noise() {
  seed ^= seed << 13;
  seed ^= seed >>> 17;
  seed ^= seed << 5;
  return (seed >>> 0) / 0x80000000 - 1;
}

function smoothstep(start, end, t) {
  const x = Math.max(0, Math.min(1, (t - start) / (end - start)));
  return x * x * (3 - 2 * x);
}

for (let i = 0; i < count; i++) {
  const t = i / rate;
  const n = noise();
  const sweep = smoothstep(0, 0.14, t);
  const fastCutoff = 1300 + 4300 * sweep;
  const slowCutoff = 170 + 470 * sweep;
  lowFast += (1 - Math.exp(-2 * Math.PI * fastCutoff / rate)) * (n - lowFast);
  lowSlow += (1 - Math.exp(-2 * Math.PI * slowCutoff / rate)) * (n - lowSlow);

  const rise = t < 0.095 ? Math.sin(Math.PI * 0.5 * t / 0.095) ** 2 : 1;
  const fall = t < 0.095 ? 1 : Math.exp(-13 * (t - 0.095));
  const airEnvelope = rise * fall * (1 - smoothstep(0.32, duration, t));
  const band = lowFast - lowSlow;
  const hiss = n - lowFast;
  const whoosh = airEnvelope * (0.92 * band + 0.22 * hiss + 0.18 * lowSlow);

  // A light blade edge at the animation's 0.4 s hit frame; no heavy impact on a miss.
  const edgeTime = t - 0.10;
  let edge = 0;
  if (edgeTime >= 0) {
    const shortDecay = Math.exp(-64 * edgeTime);
    const ringDecay = Math.exp(-26 * edgeTime);
    edge = shortDecay * (0.20 * n + 0.14 * Math.sin(2 * Math.PI * 3100 * edgeTime))
      + ringDecay * (0.16 * Math.sin(2 * Math.PI * 1070 * edgeTime)
        + 0.07 * Math.sin(2 * Math.PI * 1660 * edgeTime));
  }
  samples[i] = whoosh + edge;
}

let peak = 0;
for (const value of samples) peak = Math.max(peak, Math.abs(value));
const gain = 0.82 / peak;
const pcmBytes = count * 2;
const wav = Buffer.alloc(44 + pcmBytes);
wav.write('RIFF', 0);
wav.writeUInt32LE(36 + pcmBytes, 4);
wav.write('WAVEfmt ', 8);
wav.writeUInt32LE(16, 16);
wav.writeUInt16LE(1, 20);
wav.writeUInt16LE(1, 22);
wav.writeUInt32LE(rate, 24);
wav.writeUInt32LE(rate * 2, 28);
wav.writeUInt16LE(2, 32);
wav.writeUInt16LE(16, 34);
wav.write('data', 36);
wav.writeUInt32LE(pcmBytes, 40);
let power = 0;
for (let i = 0; i < count; i++) {
  const value = Math.max(-1, Math.min(1, samples[i] * gain));
  wav.writeInt16LE(Math.round(value * 32767), 44 + i * 2);
  power += value * value;
}
const output = path.join(__dirname, '..', 'assets', 'sfx', 'custom_normal_slash.wav');
fs.mkdirSync(path.dirname(output), { recursive: true });
fs.writeFileSync(output, wav);
console.log(`${output} (${duration}s, peak ${(20 * Math.log10(0.82)).toFixed(1)} dBFS, RMS ${(10 * Math.log10(power / count)).toFixed(1)} dBFS)`);
