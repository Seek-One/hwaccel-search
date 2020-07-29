#ifndef VW_SURFACE_RGBA_H
#define VW_SURFACE_RGBA_H

#include <string>

#include <vdpau/vdpau.h>

#include <VdpWrapper/Size.h>

namespace vw {
    class Device;

    class SurfaceRGBA {
    public:
        SurfaceRGBA(Device& device, const SizeU& size);
        SurfaceRGBA(Device& device, const std::string& filename);
        ~SurfaceRGBA();

        SurfaceRGBA(const SurfaceRGBA&) = delete;
        SurfaceRGBA(SurfaceRGBA&&) = delete;

        SurfaceRGBA& operator=(const SurfaceRGBA&) = delete;
        SurfaceRGBA& operator=(SurfaceRGBA&&) = delete;

    private:
        void allocateVdpSurface(Device& device, const SizeU& size);

    private:
        VdpOutputSurface m_vdpOutputSurface;
        SizeU m_size;

    private:
        friend class PresentationQueue;
        friend class VideoMixer;
    };
}

#endif // VW_SURFACE_RGBA_H
