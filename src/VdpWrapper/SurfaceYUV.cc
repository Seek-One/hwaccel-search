#include <VdpWrapper/SurfaceYUV.h>

#include <fstream>
#include <iostream>
#include <iterator>

#include <VdpWrapper/ImageBuffer.h>
#include <VdpWrapper/VdpFunctions.h>

namespace vw {
    SurfaceYUV::SurfaceYUV(Device& device, const std::string& filename, SizeU size)
    : m_vdpVideoSurface(VDP_INVALID_HANDLE)
    , m_size(size) {
        // Load raw bytes
        std::vector<uint8_t> rawBytes;
        std::ifstream file(filename, std::ios::binary);
        file.unsetf(std::ios::skipws);
        rawBytes.insert(rawBytes.begin(), std::istream_iterator<uint8_t>(file), std::istream_iterator<uint8_t>());

        // Create planes
        ImageBuffer buffer(size, rawBytes);

        // Create VdpSurface
        std::cout << "[SurfaceYUV] Surface size: " << size.width << " x " << size.height << std::endl;
        allocateVdpSurface(device, size);

        // Upload bytes to the surface
        const void* planes[2] = { buffer.getPlane(0), buffer.getPlane(1) };
        const uint32_t lineSize[2] = { buffer.getLineSize(0), buffer.getLineSize(1) };
        auto vdpStatus = gVdpFunctionsInstance()->videoSurfacePutBitsYCbCr(
            m_vdpVideoSurface,
            VDP_YCBCR_FORMAT_NV12,
            planes,
            lineSize
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[SurfaceYUV] Couldn't upload bytes from source image");
    }

    SurfaceYUV::~SurfaceYUV() {
        gVdpFunctionsInstance()->videoSurfaceDestroy(m_vdpVideoSurface);
    }

    void SurfaceYUV::allocateVdpSurface(Device& device, const SizeU& size) {
        // Update the surface size
        m_size = size;

        auto vdpStatus = gVdpFunctionsInstance()->videoSurfaceCreate(
            device.m_VdpDevice,
            VDP_YCBCR_FORMAT_NV12,
            size.width,
            size.height,
            &m_vdpVideoSurface
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[SurfaceYUV] Couldn't create an video surface");

        SizeU realSize;
        VdpYCbCrFormat format;
        vdpStatus = gVdpFunctionsInstance()->videoSurfaceGetParameters(
            m_vdpVideoSurface,
            &format,
            &realSize.width,
            &realSize.height
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[SurfaceYUV] Couldn't retreive surface informations");

        assert(format == VDP_YCBCR_FORMAT_NV12);
        assert(realSize == m_size); // TODO: VDPAU API says the size may be different form call to align data
                                    //       So we need to handle this case
        std::cout << "[SurfaceYUV] Surface size: " << realSize.width << " x " << realSize.height << std::endl;
    }
}
