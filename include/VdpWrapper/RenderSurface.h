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
        RenderSurface(RenderSurface&& other);

        RenderSurface& operator=(const RenderSurface&) = delete;
        RenderSurface& operator=(RenderSurface&& other);

        VdpOutputSurface getVdpHandle() const;
        void setPictureOrderCount(int iPictureOrderCount);
        int getPictureOrderCount() const;

    private:
        void allocateVdpSurface(Device& device, const SizeU& size);

    private:
        VdpOutputSurface m_vdpOutputSurface;
        SizeU m_size;
        int m_iPictureOrderCount;
    };
}

#endif // VW_RENDER_SURFACE_H
