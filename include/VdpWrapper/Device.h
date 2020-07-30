#ifndef VW_DEVICE_H
#define VW_DEVICE_H

#include <vdpau/vdpau.h>

namespace vw {
    class Display;

    class Device {
    public:
        Device(Display& display);
        ~Device();

        Device(const Device&) = delete;
        Device(Device&&) = delete;

        Device& operator=(const Device&) = delete;
        Device& operator=(Device&&) = delete;

    private:
        VdpDevice m_VdpDevice;

    private:
        friend class SurfaceRGBA;
        friend class SurfaceYUV;
        friend class PresentationQueue;
        friend class VideoMixer;
        friend class Decoder;
    };
}

#endif // VW_DEVICE_H
