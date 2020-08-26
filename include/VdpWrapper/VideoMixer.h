#ifndef VW_VIDEO_MIXER_H
#define VW_VIDEO_MIXER_H

#include <vdpau/vdpau.h>

#include <VdpWrapper/Size.h>

namespace vw {
    class Device;
    class RenderSurface;
    class DecodedSurface;

    class VideoMixer {
    public:
        VideoMixer(Device& device, SizeU outputSize);
        ~VideoMixer();

        VideoMixer(const VideoMixer&) = delete;
        VideoMixer(VideoMixer&&) = delete;

        VideoMixer& operator=(const VideoMixer&) = delete;
        VideoMixer& operator=(VideoMixer&&) = delete;

        void setOutputSize(SizeU outputSize);

        RenderSurface process(DecodedSurface &inputSurface);

    private:
        void createMixer(SizeU size);

    private:
        Device& m_device;
        VdpVideoMixer m_mixer;
        SizeU m_outputSize;
    };
}

#endif // VW_VIDEO_MIXER_H
