# FFMPEG benchmarks

**FFMPEG benchmarks** is a shell script used to call ffmpeg encoders and to benchmark the time performance.

FFMPEG benchmarks is licensed under the terms and conditions of [MIT licence](https://spdx.org/licenses/MIT.html).

## Dependencies
- [FFMPEG](https://www.ffmpeg.org/) >= 4.1
- [GNU time](https://www.gnu.org/software/time/) >= 1.7

## Script usage

This script will be run some encoding with different codecs and parameters. Each encoding will produce
a output video file with name `test-<codec_name>[-<config_name>].mkv` and the elapsed time stored in
`time-<codec_name>[-<config_name>].txt`.

When all configuration are finished, the script create a csv file with all elapsed time for each
configuration and the size of video file.

All encoding have the *same* quality parameters and use the same input video which must be named `source.mp4`.

By default, the script benchmarks 4 codecs:
- VP8 (libvpx)
- VP9 (libvpx-vp9)
- x264
- x265

And for each codecs have two configuration:
- "": the encoder is used with default parameter
- "rt": the encoder is used with parameters to smallest encoding time

This script handle also other configuration / codecs whose be enabled with options. In
addition, we can provide an other FFMPEG to run the tests (useful to test some feature not
include in distribution package).

### Options
This is the exhaustive option list:
#### Codec options
- `--help | -h` print help
- `--vp8 --no-vp8` enable or disable VP8 codec
- `--vp9 --no-vp9` enable or disable VP9 codec
- `--h263 --no-h263` enable or disable H263 codec
- `--h264 --no-h264` enable or disable H264 codec
- `--h265 --no-h265` enable or disable H265 codec
- `--openh264 --no-openh264` enable or disable openH264 codec
- `--mjpeg --no-mjpeg` enable or disable MJPEG codec
- `--av1 --no-av1` enable or disable AV1 codec
- `--only-<codec>` enable only the mentionned codec

#### Device options
- `--cpu-slow --no-cpu-slow` enable or disable the usage of slower CPU encoder
- `--cpu-fast --no-cpu-fast` enable or disable the usage of faster CPU encoder
- `--vaapi --no-vaapi` enable or disable the usage of VA-API for compatible codecs
- `--nvenc --no-nvenc` enable or disable the usage of NVDEC/NVENC for compatible codecs

#### Misc
- `--ffmpeg-cmd 'command'` define custom ffmpeg command (like ~/ffmpeg-release/ffmpeg)
- `--clean` clean previous results
- `--only-clean` only clean previous results, no encoding
