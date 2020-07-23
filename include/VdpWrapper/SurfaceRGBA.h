#ifndef VW_SURFACE_RGBA_H
#define VW_SURFACE_RGBA_H

#include <vdpau/vdpau.h>

#include <VdpWrapper/Size.h>

namespace vw {
    class Device;

    class SurfaceRGBA {
    public:
        SurfaceRGBA(Device& device, const SizeU& size);
        ~SurfaceRGBA();

        SurfaceRGBA(const SurfaceRGBA&) = delete;
        SurfaceRGBA(SurfaceRGBA&&) = delete;

        SurfaceRGBA& operator=(const SurfaceRGBA&) = delete;
        SurfaceRGBA& operator=(SurfaceRGBA&&) = delete;

    private:
        VdpOutputSurface m_vdpOutputSurface;
        SizeU m_size;
    };
}

#endif // VW_SURFACE_RGBA_H
