#ifndef VW_RENDER_SURFACE_H
#define VW_RENDER_SURFACE_H

#include <string>

#include <vdpau/vdpau.h>

#include <VdpWrapper/Size.h>

namespace vw {
    class Device;

    class RenderSurface {
    public:
        RenderSurface(Device& device, const SizeU& size);
        RenderSurface(Device& device, const std::string& filename);
        ~RenderSurface();

        RenderSurface(const RenderSurface&) = delete;
        RenderSurface(RenderSurface&&) = delete;

        RenderSurface& operator=(const RenderSurface&) = delete;
        RenderSurface& operator=(RenderSurface&&) = delete;

        VdpOutputSurface getVdpHandle() const;

        void resize(SizeU newSize);

    private:
        void allocateVdpSurface(Device& device, const SizeU& size);

    private:
        Device& m_device;
        VdpOutputSurface m_vdpOutputSurface;
        SizeU m_size;
    };
}

#endif // VW_RENDER_SURFACE_H
