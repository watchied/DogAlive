param(
    [string]$Ffmpeg = 'ffmpeg'
)

$ErrorActionPreference = 'Stop'
$root = Split-Path -Parent $PSScriptRoot
$source = Join-Path $root 'assets/sfx/freesound_community-walking-46245.mp3'
$output = Join-Path $root 'assets/sfx'

if (-not (Test-Path -LiteralPath $source)) {
    throw "Missing source recording: $source"
}

# Pick three clear footfalls and keep their tails short enough for sprinting.
$clips = @(
    @{ Start = '1.02'; Gain = 12; File = 'dungeon_step_1.wav' },
    @{ Start = '2.80'; Gain = 12; File = 'dungeon_step_2.wav' },
    @{ Start = '7.48'; Gain = 14; File = 'dungeon_step_3.wav' }
)

foreach ($clip in $clips) {
    $target = Join-Path $output $clip.File
    $filter = "volume=$($clip.Gain)dB,afade=t=in:st=0:d=0.005,afade=t=out:st=0.16:d=0.05"
    & $Ffmpeg -y -ss $clip.Start -t 0.21 -i $source -af $filter `
        -ac 1 -ar 44100 -c:a pcm_s16le $target -hide_banner -loglevel error
    if ($LASTEXITCODE -ne 0) { throw "Failed to create $target" }
}
