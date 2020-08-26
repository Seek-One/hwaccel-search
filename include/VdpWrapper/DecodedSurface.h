#ifndef VW_DECODED_SURFACE_H
#define VW_DECODED_SURFACE_H

#include <string>

#include <vdpau/vdpau.h>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Size.h>

namespace vw {
    class DecodedSurface {
    public:
        DecodedSurface(Device& device, SizeU size);
        DecodedSurface(Device& device, const std::string& filename, SizeU size);
        ~DecodedSurface();

        DecodedSurface(const DecodedSurface&) = delete;
        DecodedSurface(DecodedSurface&& other);

        DecodedSurface& operator=(const DecodedSurface&) = delete;
        DecodedSurface& operator=(DecodedSurface&& other);

        SizeU getSize() const;
        VdpVideoSurface getVdpHandle() const;
        void setPictureOrderCount(int iPictureOrderCount);
        int getPictureOrderCount() const;

    private:
        void allocateVdpSurface(Device& device, const SizeU& size);

    private:
        VdpVideoSurface m_vdpVideoSurface;
        SizeU m_size;
        int m_iPictureOrderCount;
    };
}

#endif // VW_DECODED_SURFACE_H
