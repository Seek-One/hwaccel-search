#ifndef VW_VIDEO_MIXER_H
#define VW_VIDEO_MIXER_H

#include <vdpau/vdpau.h>

#include <VdpWrapper/Size.h>

namespace vw {
    class Device;
    class SurfaceRGBA;
    class SurfaceYUV;

    class VideoMixer {
    public:
        VideoMixer(Device& device);
        ~VideoMixer();

        VideoMixer(const VideoMixer&) = delete;
        VideoMixer(VideoMixer&&) = delete;

        VideoMixer& operator=(const VideoMixer&) = delete;
        VideoMixer& operator=(VideoMixer&&) = delete;

        void process(SurfaceYUV &inputSurface, SurfaceRGBA &outputSurface);

    private:
        void createMixer(SizeU size);

    private:
        Device& m_device;
        VdpVideoMixer m_mixer;
    };
}

#endif // VW_VIDEO_MIXER_H
