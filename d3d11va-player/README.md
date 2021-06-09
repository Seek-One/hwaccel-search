# D3D11VA-PLAYER

**D3D11 Video Acceleration Player** is a software based on D3D11 API to playback a h264 bitstream using hardware acceleration.

**D3D11VA-PLAYER** is licensed under the terms and conditions of [MIT licence](https://spdx.org/licenses/MIT.html).

## Dependencies
You have to install the following dependencies:

- [CMake](https://cmake.org/) >= 3.13
- [h264bitstream](https://github.com/aizvorski/h264bitstream/tree/master) (defined as submodule)
- A MSVC compiler and a Windows SDK (installed with Visual Studio Community)

## D3D11VA-PLAYER

**D3D11VA-PLAYER** is a simple h264 bitstream player. It read the bitstream and send NAL unit to the
decoder for display it in presentation order.

To generate a H264 bitstream file, you can use FFMPEG:
```
ffmpeg.exe -i <input_video> [-ss HH:MM:SS] [-t HH:MM:SS] -c:v copy -vbsf h264_mp4toannexb <output_file.h264>
```
The options `-ss` and `-t` are optional and their purpose, respectively, is to start the copy at
a specific time of input video and to produce a output video with specified duration.

To run the program:
```
./d3d11va_player.exe <output_file.h264>
```

**NOTE:** This project is more a concept proof than a really efficient video player. So we
do not handle all H.264 specification (like B-Frame, MMCO, reordering DPB...).
