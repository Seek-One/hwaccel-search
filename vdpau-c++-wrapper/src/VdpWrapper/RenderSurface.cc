#include <VdpWrapper/RenderSurface.h>

#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <opencv2/imgcodecs.hpp>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/ImageBuffer.h>
#include <VdpWrapper/VdpFunctions.h>

namespace vw {
    RenderSurface::RenderSurface(Device& device, const SizeU& size)
    : m_vdpOutputSurface(VDP_INVALID_HANDLE)
    , m_size(size)
    , m_iPictureOrderCount(-1) {
        if (m_size != SizeU(0u, 0u)) {
            allocateVdpSurface(device, size);
        }
    }

    RenderSurface::RenderSurface(Device& device, const std::string& filename)
    : RenderSurface(device, SizeU(0u, 0u)) {
        // Load the image with openCV
        cv::Mat decompressedImage = cv::imread(filename);

        // Create planes
        ImageBuffer buffer(decompressedImage);

        // Create VdpSurface
        SizeU imageSize(decompressedImage.size().width, decompressedImage.size().height);
        allocateVdpSurface(device, imageSize);

        // Upload bytes to the surface
        const void* planes[1] = { buffer.getPlane(0) };
        const uint32_t lineSize[1] = { buffer.getLineSize(0) };
        auto vdpStatus = gVdpFunctionsInstance()->outputSurfacePutBitsNative(
            m_vdpOutputSurface,
            planes,
            lineSize,
            nullptr // Update all the surface
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[RenderSurface] Couldn't upload bytes from source image");
    }

    RenderSurface::~RenderSurface() {
        if (m_vdpOutputSurface != VDP_INVALID_HANDLE) {
            gVdpFunctionsInstance()->outputSurfaceDestroy(m_vdpOutputSurface);
        }
    }

    RenderSurface::RenderSurface(RenderSurface&& other)
    : m_vdpOutputSurface(std::exchange(other.m_vdpOutputSurface, VDP_INVALID_HANDLE))
    , m_size(std::exchange(other.m_size, 0))
    , m_iPictureOrderCount(std::exchange(other.m_iPictureOrderCount, -1)) {

    }

    RenderSurface& RenderSurface::operator=(RenderSurface&& other) {
        std::swap(m_vdpOutputSurface, other.m_vdpOutputSurface);
        std::swap(m_size, other.m_size);
        std::swap(m_iPictureOrderCount, other.m_iPictureOrderCount);

        return *this;
    }

    VdpOutputSurface RenderSurface::getVdpHandle() const {
        return m_vdpOutputSurface;
    }

    void RenderSurface::setPictureOrderCount(int iPictureOrderCount) {
        m_iPictureOrderCount = iPictureOrderCount;
    }

    int RenderSurface::getPictureOrderCount() const {
        return m_iPictureOrderCount;
    }

    void RenderSurface::allocateVdpSurface(Device& device, const SizeU& size) {
        // Update the surface size
        m_size = size;

        auto vdpStatus = gVdpFunctionsInstance()->outputSurfaceCreate(
            device.getVdpHandle(),
            VDP_RGBA_FORMAT_B8G8R8A8,
            size.width,
            size.height,
            &m_vdpOutputSurface
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[RenderSurface] Couldn't create an output surface");

        SizeU realSize;
        VdpRGBAFormat format;
        vdpStatus = gVdpFunctionsInstance()->outputSurfaceGetParameters(
            m_vdpOutputSurface,
            &format,
            &realSize.width,
            &realSize.height
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[RenderSurface] Couldn't retreive surface informations");

        assert(format == VDP_RGBA_FORMAT_B8G8R8A8);
        assert(realSize == m_size); // TODO: VDPAU API says the size may be different form call to align data
                                    //       So we need to handle this case
    }
}
