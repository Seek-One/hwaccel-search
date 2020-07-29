#include <VdpWrapper/SurfaceRGBA.h>

#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <opencv2/imgcodecs.hpp>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/ImageBuffer.h>
#include <VdpWrapper/VdpFunctions.h>

namespace vw {
    SurfaceRGBA::SurfaceRGBA(Device& device, const SizeU& size)
    : m_vdpOutputSurface(VDP_INVALID_HANDLE)
    , m_size(size) {
        allocateVdpSurface(device, size);
    }

    SurfaceRGBA::SurfaceRGBA(Device& device, const std::string& filename)
    : m_vdpOutputSurface(VDP_INVALID_HANDLE)
    , m_size({ 0u, 0u}) {
        // Load the image with openCV
        cv::Mat decompressedImage = cv::imread(filename);

        // Create planes
        ImageBuffer buffer(decompressedImage);

        // Create VdpSurface
        SizeU imageSize(decompressedImage.size().width, decompressedImage.size().height);
        std::cout << "[SurfaceRGBA] Image size: " << imageSize.width << " x " << imageSize.height << std::endl;
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
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[SurfaceRGBA] Couldn't upload bytes from source image");
    }

    SurfaceRGBA::~SurfaceRGBA() {
        gVdpFunctionsInstance()->outputSurfaceDestroy(m_vdpOutputSurface);
    }

    void SurfaceRGBA::allocateVdpSurface(Device& device, const SizeU& size) {
        // Update the surface size
        m_size = size;

        auto vdpStatus = gVdpFunctionsInstance()->outputSurfaceCreate(
            device.m_VdpDevice,
            VDP_RGBA_FORMAT_B8G8R8A8,
            size.width,
            size.height,
            &m_vdpOutputSurface
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[SurfaceRGBA] Couldn't create an output surface");

        SizeU realSize;
        VdpRGBAFormat format;
        vdpStatus = gVdpFunctionsInstance()->outputSurfaceGetParameters(
            m_vdpOutputSurface,
            &format,
            &realSize.width,
            &realSize.height
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[SurfaceRGBA] Couldn't retreive surface informations");

        assert(format == VDP_RGBA_FORMAT_B8G8R8A8);
        assert(realSize == m_size); // TODO: VDPAU API says the size may be different form call to align data
                                    //       So we need to handle this case
        std::cout << "[SurfaceRGBA] Surface size: " << realSize.width << " x " << realSize.height << std::endl;
    }
}
