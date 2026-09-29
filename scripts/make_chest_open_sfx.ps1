param(
    [string]$Ffmpeg = 'ffmpeg'
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$source = Join-Path $root 'assets/sfx/floraphonic-wooden-trunk-latch-3-183946.mp3'
$target = Join-Path $root 'assets/sfx/chest_open.wav'

if (-not (Test-Path -LiteralPath $source)) {
    throw "Missing source recording: $source"
}

# Keep the complete latch sequence, with a quieter peak and a smooth tail.
& $Ffmpeg -y -i $source -af 'volume=-7dB,afade=t=in:st=0:d=0.005,afade=t=out:st=0.736:d=0.08' `
    -ac 1 -ar 44100 -c:a pcm_s16le $target -hide_banner -loglevel error
if ($LASTEXITCODE -ne 0) { throw "Failed to create $target" }
