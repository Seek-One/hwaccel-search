# VDPAU C++ Wrapper

**VDPAU C++ Wrapper (VdpWrapper)** is a library to encapsulate the [VDPAU API](https://vdpau.pages.freedesktop.org/libvdpau/) in c++.

VDPAU C++ Wrapper is licensed under the terms and conditions of [MIT licence](https://spdx.org/licenses/MIT.html).

## Dependencies
You have to install the following dependencies:

- [CMake](https://cmake.org/) >= 3.13
- A VDPAU driver provides by your distribution

For `image-viewer`:

- [OpenCV](https://opencv.org/) >= 3.2

For `h264-player`

- [h264bitstream](https://github.com/aizvorski/h264bitstream/tree/master) (defined as submodule)

## VdpWrapper

**VdpWrapper** is the short name of library. The sources are located here: [src/VdpWrapper](src/VdpWrapper) and the include
are located here: [include/VdpWrapper](include/VdpWrapper).

This library is more an illustration of VDPAU API than a robust project but we hope that it's enough to understand how VDPAU works.
Hence, all parts of API aren't wrapped. You can contact us or propose some contributions to improve the project.

We provide two exemples to show how our library works.

## ImageViewer

**ImageViewer** is a first example of library usage. It's a simple YUV image viewer which take an raw image and
display it via VDPAU. The source are located here : [src/ImageViewer](src/ImageViewer)

Currently, the software only take image on [NV12 format](https://wiki.videolan.org/YUV#NV12). This following
command convert a image to a NV12 format via FFMPEG:

```
ffmpeg -i <input_image> -vf format=nv12 -qscale:v 2 <output_image.yuv>

# You can check your YUV image with this command:
ffplay -video_size <width>x<height> -pixel_format nv12 <output_image.yuv>
```

To run the program:
```
./image-viewer --image-size <width>x<height> image.yuv
```

Some options are available:
- `--image-size <width>x<height>`       Set the source image size (this option is mandatory and must be specified)
- `--initial-size <width>x<height>`     Set the initial screen size (default: 1280x720)

**NOTE:** if `--image-size` is incorrect an assertion will be throw.

## h264Player

**h264Player** is a simple h264 bitstream player. It read the bitstream and send NAL unit to the
decoder for display it in presentation order.

To generate a H264 bitstream file, you can use FFMPEG:
```
ffmpeg -i <input_video> [-ss HH:MM:SS] [-t HH:MM:SS] -c:v copy -vbsf h264_mp4toannexb <output_file.h264>
```
The options `-ss` and `-t` are optional and their purpose, respectively, is to start the copy at
a specific time of input video and to produce a output video with specified duration.

To run the program:
```
./h264-player <output_file.h264>
```

Some options are available:
- `--initial-size <width>x<height>`     Set the initial screen size (default: 1280x720)
- `--disable-pts`                       Display images in decode order (default: false)
- `--enable-pts`                        Display images in presentation order (default: true)
- `--fps <FPS>`                         Set the video FPS (default: 25)

**NOTE:** The computation of Presentation Time Stamp (PTS) is a tricky part and it's not the main purpose of this
project, so it works for video whose POCs increase by 2 every each reference frame but we have some
difficulties to reading videos whose POCs increase by 1 every each reference frame. If you are
in this case, try to disable this feature with option `--disable-pts`
