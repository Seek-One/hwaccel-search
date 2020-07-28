#include <VdpWrapper/SurfaceRGBA.h>

#include <array>
#include <fstream>
#include <iostream>
#include <stdexcept>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/VdpFunctions.h>

namespace {
    // The VDP_RGBA_FORMAT_B8G8R8A8 is a one plane format :
    // https://vdpau.pages.freedesktop.org/libvdpau/group__misc__types.html#ga2659adf5d019acade5516ea35e4eb5ad
    class ImageBuffer {
    public:
        ImageBuffer(cv::Mat &decodedImage) {
            assert(decodedImage.channels() == 3); // No alpha channel
            assert(decodedImage.depth() == CV_8U); // Only 8 bits color
            assert(decodedImage.isContinuous()); // Data must be continuous

            // Set line size
            m_lineSize = decodedImage.size().width * 4; // one line is 4 unit8_t color
            m_data.resize(decodedImage.size().width * decodedImage.size().height * 4);

            // Fill the buffers
            cv::MatIterator_<cv::Vec3b> it, end;
            int index = 0;
            for (it = decodedImage.begin<cv::Vec3b>(), end = decodedImage.end<cv::Vec3b>(); it != end; ++it, index += 4) {
                m_data[index + 0] = (*it)[0]; // Blue
                m_data[index + 1] = (*it)[1]; // Green
                m_data[index + 2] = (*it)[2]; // Red
                m_data[index + 3] = std::numeric_limits<uint8_t>::max(); // Alpha channel
            }
        }

        const uint32_t* getLineSize() const {
            return &m_lineSize;
        }

        const uint8_t* getPlane() const {
            return m_data.data();
        }

    private:
        std::vector<uint8_t> m_data;
        uint32_t m_lineSize;
    };
}

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
        const void* planes[1] = { buffer.getPlane() };
        auto vdpStatus = gVdpFunctionsInstance()->outputSurfacePutBitsNative(
            m_vdpOutputSurface,
            planes,
            buffer.getLineSize(),
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
        assert(realSize == m_size); // TODO: VDPAU API says the size must be different form call to align data
                                    //       So we need to handle this case
        std::cout << "[SurfaceRGBA] Surface size: " << realSize.width << " x " << realSize.height << std::endl;
    }
}
