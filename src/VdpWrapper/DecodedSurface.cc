#include <VdpWrapper/DecodedSurface.h>

#include <fstream>
#include <iostream>
#include <iterator>

#include <VdpWrapper/ImageBuffer.h>
#include <VdpWrapper/VdpFunctions.h>

namespace vw {
    DecodedSurface::DecodedSurface(Device& device, SizeU size)
    : m_vdpVideoSurface(VDP_INVALID_HANDLE)
    , m_size(size)
    , m_iPictureOrderCount(-1) {
        allocateVdpSurface(device, size);
    }

    DecodedSurface::DecodedSurface(Device& device, const std::string& filename, SizeU size)
    : DecodedSurface(device, size) {
        // Load raw bytes
        std::vector<uint8_t> rawBytes;
        std::ifstream file(filename, std::ios::binary);
        file.unsetf(std::ios::skipws);
        rawBytes.insert(rawBytes.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

        // Create planes
        ImageBuffer buffer(size, rawBytes);

        // Upload bytes to the surface
        const void* planes[2] = { buffer.getPlane(0), buffer.getPlane(1) };
        const uint32_t lineSize[2] = { buffer.getLineSize(0), buffer.getLineSize(1) };
        auto vdpStatus = gVdpFunctionsInstance()->videoSurfacePutBitsYCbCr(
            m_vdpVideoSurface,
            VDP_YCBCR_FORMAT_NV12,
            planes,
            lineSize
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[DecodedSurface] Couldn't upload bytes from source image");
    }

    DecodedSurface::~DecodedSurface() {
        if (m_vdpVideoSurface != VDP_INVALID_HANDLE) {
            gVdpFunctionsInstance()->videoSurfaceDestroy(m_vdpVideoSurface);
        }
    }

    DecodedSurface::DecodedSurface(DecodedSurface&& other)
    : m_vdpVideoSurface(std::exchange(other.m_vdpVideoSurface, VDP_INVALID_HANDLE))
    , m_size(std::exchange(other.m_size, 0))
    , m_iPictureOrderCount(std::exchange(other.m_iPictureOrderCount, -1)) {

    }

    DecodedSurface& DecodedSurface::operator=(DecodedSurface&& other) {
        std::swap(m_vdpVideoSurface, other.m_vdpVideoSurface);
        std::swap(m_size, other.m_size);
        std::swap(m_iPictureOrderCount, other.m_iPictureOrderCount);

        return *this;
    }

    SizeU DecodedSurface::getSize() const {
        return m_size;
    }

    VdpVideoSurface DecodedSurface::getVdpHandle() const {
        return m_vdpVideoSurface;
    }

    void DecodedSurface::setPictureOrderCount(int iPictureOrderCount) {
        m_iPictureOrderCount = iPictureOrderCount;
    }

    int DecodedSurface::getPictureOrderCount() const {
        return m_iPictureOrderCount;
    }

    void DecodedSurface::allocateVdpSurface(Device& device, const SizeU& size) {
        // Update the surface size
        m_size = size;

        auto vdpStatus = gVdpFunctionsInstance()->videoSurfaceCreate(
            device.getVdpHandle(),
            VDP_YCBCR_FORMAT_NV12,
            size.width,
            size.height,
            &m_vdpVideoSurface
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[DecodedSurface] Couldn't create an video surface");

        SizeU realSize;
        VdpYCbCrFormat format;
        vdpStatus = gVdpFunctionsInstance()->videoSurfaceGetParameters(
            m_vdpVideoSurface,
            &format,
            &realSize.width,
            &realSize.height
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[DecodedSurface] Couldn't retreive surface informations");

        assert(format == VDP_YCBCR_FORMAT_NV12);
        assert(realSize == m_size); // TODO: VDPAU API says the size may be different form call to align data
                                    //       So we need to handle this case
    }
}
