#!/bin/bash

# Copyright (c) 2020 Jet1oeil

# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:

# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.

# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

# Codecs
VP8=true
VP9=true
H263=false
H264=true
OPENH264=false
H265=true
MJPEG=true
AV1=false

# Specific encoder
VAAPI=false
NVENC=false
CPU_SLOW=true
CPU_FAST=true

# Misc
FFMPEG_CMD=ffmpeg
NEED_CLEAN=false

print_usage() {
    echo -e "\
Usage:
\t$1 [OPTIONS...]

Codec options:
\t--help | -h print this help
\t--vp8 --no-vp8 enable or disable VP8 codec
\t--vp9 --no-vp9 enable or disable VP9 codec
\t--h263 --no-h263 enable or disable H263 codec
\t--h264 --no-h264 enable or disable H264 codec
\t--h265 --no-h265 enable or disable H265 codec
\t--openh264 --no-openh264 enable or disable openH264 codec
\t--mjpeg --no-mjpeg enable or disable MJPEG codec
\t--av1 --no-av1 enable or disable AV1 codec
\t--only-<codec> enable only the mentioned codec

Device options:
\t--cpu-slow --no-cpu-slow enable or disable the usage of slower CPU encoder
\t--cpu-fast --no-cpu-fast enable or disable the usage of faster CPU encoder
\t--vaapi --no-vaapi enable or disable the usage of VA-API for compatible codecs
\t--nvenc --no-nvenc enable or disable the usage of NVDEC/NVENC for compatible codecs

Misc:
\t--ffmpeg-cmd 'command' define custom ffmpeg command (like ~/ffmpeg-release/ffmpeg)
\t--clean clean previous results
\t--only-clean only clean previous results, no encoding
" 1>&2
    exit 1
}

# Trap interrupt to quit script instead cancel the current encode
trap 'echo "Signal caught." 1>&2; exit 1' SIGINT SIGQUIT SIGTERM

# Parse option
while [ $# -ne 0 ]; do
    ARG=$1

    case $ARG in
	"--help" | "-h")
		print_usage $0
		;;

    "--vp8")
        VP8=true
        ;;

    "--no-vp8")
        VP8=false
        ;;

    "--vp9")
        VP9=true
        ;;

    "--no-vp9")
        VP9=false
        ;;

    "--h263")
        H263=true
        ;;

    "--no-h263")
        H263=false
        ;;

    "--h264")
        H264=true
        ;;

    "--no-h264")
        H264=false
        ;;

    "--h265")
        H265=true
        ;;

    "--no-h265")
        H265=false
        ;;

    "--openh264")
        OPENH264=true
        ;;

    "--no-openh264")
        OPENH264=false
        ;;

    "--mjpeg")
        MJPEG=true
        ;;

    "--no-mjpeg")
        MJPEG=false
        ;;

    "--av1")
        AV1=true
        ;;

    "--no-av1")
        AV1=false
        ;;

    "--only-vp8")
        VP8=true
        VP9=false
        H263=false
        H264=false
        OPENH264=false
        H265=false
        MJPEG=false
        AV1=false
        ;;

    "--only-vp9")
        VP8=false
        VP9=true
        H263=false
        H264=false
        OPENH264=false
        H265=false
        MJPEG=false
        AV1=false
        ;;

    "--only-h263")
        VP8=false
        VP9=false
        H263=true
        H264=false
        OPENH264=false
        H265=false
        MJPEG=false
        AV1=false
        ;;

    "--only-h264")
        VP8=false
        VP9=false
        H263=false
        H264=true
        OPENH264=false
        H265=false
        MJPEG=false
        AV1=false
        ;;

    "--only-h265")
        VP8=false
        VP9=false
        H263=false
        H264=false
        OPENH264=false
        H265=true
        MJPEG=false
        AV1=false
        ;;

    "--only-openh264")
        VP8=false
        VP9=false
        H263=false
        H264=false
        OPENH264=true
        H265=false
        MJPEG=false
        AV1=false
        ;;

    "--only-mjpeg")
        VP8=false
        VP9=false
        H263=false
        H264=false
        OPENH264=false
        H265=false
        MJPEG=true
        AV1=false
        ;;

    "--only-av1")
        VP8=false
        VP9=false
        H263=false
        H264=false
        OPENH264=false
        H265=false
        MJPEG=false
        AV1=true
        ;;

    "--cpu-slow")
        CPU_SLOW=true
        ;;

    "--no-cpu-slow")
        CPU_SLOW=false
        ;;

    "--cpu-fast")
        CPU_FAST=true
        ;;

    "--no-cpu-fast")
        CPU_FAST=false
        ;;

    "--vaapi")
        VAAPI=true
        ;;

    "--no-vaapi")
        VAAPI=false
        ;;

    "--nvenc")
        NVENC=true
        ;;

    "--no-nvenc")
        NVENC=false
        ;;

    "--ffmpeg-cmd")
        if [ -z "$2" ]; then
            echo "After '--ffmpeg-cmd' a command must be defined" 1>&2
            print_usage $0
        fi

        FFMPEG_CMD=$2
        shift # We have already handle the next parameter
        ;;

    "--clean")
        NEED_CLEAN=true
        ;;

    "--only-clean")
        NEED_CLEAN=true
        VP8=false
        VP9=false
        H264=false
        OPENH264=false
        H265=false
        MJPEG=false
        AV1=false
        VAAPI=false
        CPU_SLOW=false
        CPU_FAST=false
        ;;

	*)
		echo "$ARG unrecognized parameter" 1>&2
        print_usage $0
		;;
  esac

  shift
done

if $NEED_CLEAN; then
    rm test-* time-*
fi

#######
# VP8 #
#######

if $VP8; then
    # CPU
    if $CPU_SLOW; then
        /usr/bin/time -o time-vp8.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test VP8" -t 00:00:05 -crf 15 -b:v 12000K -minrate 6000K -maxrate 17400K -c:v libvpx test-vp8.mkv
    fi

    if $CPU_FAST; then
        /usr/bin/time -o time-vp8-rt.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test VP8 RT" -t 00:00:05 -quality realtime -speed 8 -threads 2 -crf 15 -b:v 12000K -minrate 6000K -maxrate 17400K -qmin 4 -qmax 48 -c:v libvpx test-vp8-rt.mkv
    fi

    # GPU
    if $VAAPI; then
        /usr/bin/time -o time-vp8-vaapi.txt $FFMPEG_CMD -y -vaapi_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test VP8 VA-API" -t 00:00:05 -vf 'format=nv12,hwupload' -c:v vp8_vaapi test-vp8-vaapi.mkv
        /usr/bin/time -o time-vp8-vaapi-full.txt $FFMPEG_CMD -y -hwaccel vaapi -hwaccel_output_format vaapi -hwaccel_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test vp8 VA-API full" -t 00:00:05 -c:v vp8_vaapi test-vp8-vaapi-full.mkv
    fi
fi

#######
# VP9 #
#######

if $VP9; then
    # CPU
    if $CPU_SLOW; then
        /usr/bin/time -o time-vp9.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test VP9" -t 00:00:05 -crf 15 -b:v 12000K -minrate 6000K -maxrate 17400K -c:v libvpx-vp9 test-vp9.mkv
    fi
    if $CPU_FAST; then
        /usr/bin/time -o time-vp9-rt.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test VP9 RT" -t 00:00:05 -quality realtime -speed 8 -threads 4 -tile-columns 2 -crf 15 -b:v 12000K -minrate 6000K -maxrate 17400K -frame-parallel 1 -qmin 4 -qmax 48 -row-mt 1 -c:v libvpx-vp9 test-vp9-rt.mkv
    fi

    # GPU
    if $VAAPI; then
        /usr/bin/time -o time-vp9-vaapi.txt $FFMPEG_CMD -y -vaapi_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test VP9 VA-API" -t 00:00:05 -vf 'format=nv12,hwupload' -c:v vp9_vaapi test-vp9-vaapi.mkv
        /usr/bin/time -o time-vp9-vaapi-full.txt $FFMPEG_CMD -y -hwaccel vaapi -hwaccel_output_format vaapi -hwaccel_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test vp9 VA-API full" -t 00:00:05 -c:v vp9_vaapi test-vp9-vaapi-full.mkv
    fi
fi

########
# h263 #
########

if $H263; then
    echo "Not support 4K resolution" 1>&2
    # # CPU
    # if $CPU_SLOW; then
    #     /usr/bin/time -o time-x264.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test x264" -t 00:00:05 -crf 23 -c:v libx264 test-x264.mkv
    # fi
    # if $CPU_FAST; then
    #     /usr/bin/time -o time-x264-rt.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test x264 RT" -t 00:00:05 -crf 23 -preset ultrafast -c:v libx264 test-x264-rt.mkv
    # fi
fi

########
# h264 #
########

if $H264; then
    # CPU
    if $CPU_SLOW; then
        /usr/bin/time -o time-x264.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test x264" -t 00:00:05 -crf 23 -c:v libx264 test-x264.mkv
    fi
    if $CPU_FAST; then
        /usr/bin/time -o time-x264-rt.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test x264 RT" -t 00:00:05 -crf 23 -preset ultrafast -c:v libx264 test-x264-rt.mkv
    fi

    # GPU
    if $VAAPI; then
        /usr/bin/time -o time-h264-vaapi-baseline.txt $FFMPEG_CMD -y -vaapi_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test h264 VA-API baseline" -t 00:00:05 -vf 'format=nv12,hwupload' -profile:v constrained_baseline -level:v 5.1 -c:v h264_vaapi test-h264-vaapi-baseline.mkv
        /usr/bin/time -o time-h264-vaapi-baseline-full.txt $FFMPEG_CMD -y -hwaccel vaapi -hwaccel_output_format vaapi -hwaccel_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test h264 VA-API baseline full" -t 00:00:05 -profile:v constrained_baseline -level:v 5.1 -c:v h264_vaapi test-h264-vaapi-baseline-full.mkv
        /usr/bin/time -o time-h264-vaapi-high.txt $FFMPEG_CMD -y -vaapi_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test h264 VA-API high" -t 00:00:05 -vf 'format=nv12,hwupload' -profile:v high -level:v 5.1 -c:v h264_vaapi test-h264-vaapi-high.mkv
        /usr/bin/time -o time-h264-vaapi-high-full.txt $FFMPEG_CMD -y -hwaccel vaapi -hwaccel_output_format vaapi -hwaccel_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test h264 VA-API high full" -t 00:00:05 -profile:v high -level:v 5.1 -c:v h264_vaapi test-h264-vaapi-high-full.mkv
    fi
    if $NVENC; then
        /usr/bin/time -o time-h264-nvenc.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test h264 NVENC" -t 00:00:05 -b:v 0 -c:v h264_nvenc test-h264-nvenc.mkv
        /usr/bin/time -o time-h264-nvenc-full.txt $FFMPEG_CMD -y -hwaccel cuda -hwaccel_output_format cuda -i source.mp4 -metadata title="Test h264 NVENC full" -t 00:00:05 -b:v 0 -c:v h264_nvenc test-h264-nvenc-full.mkv
    fi
fi

if $OPENH264; then
    if $CPU_SLOW; then
        /usr/bin/time -o time-openh264.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test openh264" -t 00:00:05 -profile:v high -b:v 18M -c:v libopenh264 test-openh264.mkv
    fi
    if $CPU_FAST; then
        /usr/bin/time -o time-openh264-rt.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test open264 RT" -t 00:00:05 -profile:v constrained_baseline -b:v 18M -c:v libopenh264 test-openh264-rt.mkv
    fi
fi

########
# h265 #
########

if $H265; then
    # CPU
    if $CPU_SLOW; then
        /usr/bin/time -o time-x265.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test x265" -t 00:00:05 -crf 28 -c:v libx265 test-x265.mkv
    fi
    if $CPU_FAST; then
        /usr/bin/time -o time-x265-rt.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test x265 RT" -t 00:00:05 -crf 28 -preset ultrafast -c:v libx265 test-x265-rt.mkv
    fi

    # GPU
    if $VAAPI; then
        /usr/bin/time -o time-h265-vaapi.txt $FFMPEG_CMD -y -vaapi_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test h265 VA-API" -t 00:00:05 -vf 'format=nv12,hwupload' -level:v 5.1 -c:v hevc_vaapi test-h265-vaapi.mkv
        /usr/bin/time -o time-h265-vaapi-full.txt $FFMPEG_CMD -y -hwaccel vaapi -hwaccel_output_format vaapi -hwaccel_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test h265 VA-API full" -t 00:00:05 -c:v hevc_vaapi test-h265-vaapi-full.mkv
    fi
    if $NVENC; then
        /usr/bin/time -o time-h265-nvenc.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test h265 NVENC" -t 00:00:05 -b:v 0 -c:v hevc_nvenc test-h265-nvenc.mkv
        /usr/bin/time -o time-h265-nvenc-full.txt $FFMPEG_CMD -y -hwaccel cuda -hwaccel_output_format cuda -i source.mp4 -metadata title="Test h265 NVENC full" -t 00:00:05 -b:v 0 -c:v hevc_nvenc test-h265-nvenc-full.mkv
    fi
fi

#########
# MJPEG #
#########

if $MJPEG; then
    # CPU
    if $CPU_SLOW || $CPU_FAST; then
        /usr/bin/time -o time-mjpeg.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test mjpeg" -t 00:00:05 -c:v mjpeg -q:v 5 -an test-mjpeg.mkv
    fi

    # GPU
    if $VAAPI; then
        /usr/bin/time -o time-mjpeg-vaapi.txt $FFMPEG_CMD -y -vaapi_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test MJPEG VA-API" -t 00:00:05 -vf 'format=nv12,hwupload' -c:v mjpeg_vaapi test-mjpeg-vaapi.mkv
        /usr/bin/time -o time-mjpeg-vaapi-full.txt $FFMPEG_CMD -y -hwaccel vaapi -hwaccel_output_format vaapi -hwaccel_device /dev/dri/renderD128 -i source.mp4 -metadata title="Test mjpeg VA-API full" -t 00:00:05 -c:v mjpeg_vaapi test-mjpeg-vaapi-full.mkv
    fi
fi

#######
# AV1 #
#######

if $AV1; then
    /usr/bin/time -o time-av1.txt $FFMPEG_CMD -y -i source.mp4 -metadata title="Test AV1" -t 00:00:05 -c:v libaom-av1 -strict experimental test-av1.mkv
fi

#######################
# Extract all results #
#######################

echo "codec;time;size" > results.csv
for MKV_FILE in *.mkv; do
    TMP=${MKV_FILE%.mkv}
    CONFIG=${TMP#test-}
    TIME_FILE="time-$CONFIG.txt"

    # Retrieve time result
    if [ ! -f $TIME_FILE ]; then
        echo "The time profile '$TIME_FILE' is not found for '$CONFIG'" 1>&2
        continue;
    fi

    TIME_STRING=$(cat $TIME_FILE | head -n 1 | cut -d " " -f 3)
    TIME_STRING=${TIME_STRING%elapsed}

    MIN=$(echo ${TIME_STRING} | cut -d ":" -f 1)
    SEC=$(echo ${TIME_STRING} | cut -d ":" -f 2)
    TIME=$(bc -l <<< "$MIN * 60 + $SEC")

    # Retrieve video size
    RAW_SIZE=$(stat -c "%s" $MKV_FILE)
    SIZE=$(bc -l <<< "$RAW_SIZE / 1024.0 / 1024.0")

    echo "$CONFIG;$TIME;$SIZE" >> results.csv
done
