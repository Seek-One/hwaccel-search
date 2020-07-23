#include <VdpWrapper/SurfaceRGBA.h>

#include <array>
#include <iostream>
#include <stdexcept>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include <VdpWrapper/Device.h>
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
        cv::Mat compressedImage = cv::imread(filename);
        cv::Mat decompressedImage;

        // Add a alpha channel if needed
        if (compressedImage.channels() == 3) {
            // FIXME: On buster 10, this call provoke valgrind errors:
            // Warning: noted but unhandled ioctl 0x30000001 with no size/direction hints.
            //    This could cause spurious value errors to appear.
            //    See README_MISSING_SYSCALL_OR_IOCTL for guidance on writing a proper wrapper.
            // Warning: noted but unhandled ioctl 0x27 with no size/direction hints.
            //    This could cause spurious value errors to appear.
            //    See README_MISSING_SYSCALL_OR_IOCTL for guidance on writing a proper wrapper.
            // Conditional jump or move depends on uninitialised value(s)
            //    at 0x1EFBB2D6: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-opencl.so.440.100)
            //    by 0x1EFBF9ED: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-opencl.so.440.100)
            //    by 0x1EE433B7: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-opencl.so.440.100)
            //    by 0x1EFC206C: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-opencl.so.440.100)
            //    by 0x1EFC22B9: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-opencl.so.440.100)
            //    by 0x1EE4512F: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-opencl.so.440.100)
            //    by 0x1EE07E3C: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-opencl.so.440.100)
            //    by 0x1EE08631: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-opencl.so.440.100)
            //    by 0x1ECE128C: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-opencl.so.440.100)
            //    by 0x1ECE1187: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libnvidia-opencl.so.440.100)
            //    by 0x4854A23: ??? (in /usr/lib/x86_64-linux-gnu/libOpenCL.so.1.0.0)
            //    by 0x48554E2: clGetPlatformIDs (in /usr/lib/x86_64-linux-gnu/libOpenCL.so.1.0.0)
            cv::cvtColor(compressedImage, decompressedImage, CV_BGR2BGRA);
        } else {
            decompressedImage = compressedImage;
        }

        // Split contiguous image data into different planes
        std::vector<cv::Mat> matPlanes(4);
        cv::split(decompressedImage, matPlanes);

        SizeU imageSize(decompressedImage.cols, decompressedImage.rows);
        std::cout << "[SurfaceRGBA] Image size: " << imageSize.width << " x " << imageSize.height << std::endl;
        allocateVdpSurface(device, imageSize);

        // Initialize the surface with image bytes
        std::array<uint32_t, 4> pitches;
        for (auto& pitche: pitches) {
            pitche = imageSize.width;
        }

        std::array<const uint8_t*, 4> planes;
        for (unsigned i = 0; i < 4; ++i) {
            planes[i] = matPlanes[i].data;
        }

        // FIXME: On buster 10, the previous valgrind error have a side effect:
        // Invalid read of size 8
        //    at 0x483A03F: memcpy@GLIBC_2.2.5 (vg_replace_strmem.c:1033)
        //    by 0x1E1A69A4: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libvdpau_nvidia.so.440.100)
        //    by 0x1E1A8E22: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libvdpau_nvidia.so.440.100)
        //    by 0x1E1A90EF: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libvdpau_nvidia.so.440.100)
        //    by 0x1E1A9C09: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libvdpau_nvidia.so.440.100)
        //    by 0x1E187DE3: ??? (in /usr/lib/x86_64-linux-gnu/nvidia/current/libvdpau_nvidia.so.440.100)
        //    by 0x10C782: vw::SurfaceRGBA::SurfaceRGBA(vw::Device&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) (SurfaceRGBA.cc:59)
        //    by 0x10B747: main (main.cc:36)
        //  Address 0x2c6e50a8 is 16 bytes after a block of size 921,624 alloc'd
        //    at 0x483577F: malloc (vg_replace_malloc.c:299)
        //    by 0x5F6EB11: cv::fastMalloc(unsigned long) (in /usr/lib/x86_64-linux-gnu/libopencv_core.so.3.2.0)
        //    by 0x6061D86: cv::Mat::create(int, int const*, int) (in /usr/lib/x86_64-linux-gnu/libopencv_core.so.3.2.0)
        //    by 0x6079065: cv::_OutputArray::create(int, int const*, int, int, bool, int) const (in /usr/lib/x86_64-linux-gnu/libopencv_core.so.3.2.0)
        //    by 0x5FB2653: cv::split(cv::_InputArray const&, cv::_OutputArray const&) (in /usr/lib/x86_64-linux-gnu/libopencv_core.so.3.2.0)
        //    by 0x10C5B3: vw::SurfaceRGBA::SurfaceRGBA(vw::Device&, std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> > const&) (SurfaceRGBA.cc:36)
        //    by 0x10B747: main (main.cc:36)
        auto vdpStatus = gVdpFunctionsInstance()->outputSurfacePutBitsNative(
            m_vdpOutputSurface,
            reinterpret_cast<const void* const*>(planes.data()),
            pitches.data(),
            nullptr // Update all the surface
        );
        if (vdpStatus != VDP_STATUS_OK) {
            auto szError = gVdpFunctionsInstance()->getErrorString(vdpStatus);
            throw std::runtime_error("[SurfaceRGBA] Couldn't upload bytes from source image: " + szError);
        }
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
        if (vdpStatus != VDP_STATUS_OK) {
            auto szError = gVdpFunctionsInstance()->getErrorString(vdpStatus);
            throw std::runtime_error("[SurfaceRGBA] Couldn't create an output surface: " + szError);
        }

        SizeU realSize;
        VdpRGBAFormat format;
        vdpStatus = gVdpFunctionsInstance()->outputSurfaceGetParameters(
            m_vdpOutputSurface,
            &format,
            &realSize.width,
            &realSize.height
        );
        if (vdpStatus != VDP_STATUS_OK) {
            auto szError = gVdpFunctionsInstance()->getErrorString(vdpStatus);
            throw std::runtime_error("[SurfaceRGBA] Couldn't retreive surface informations: " + szError);
        }

        assert(format == VDP_RGBA_FORMAT_B8G8R8A8);
        assert(realSize == m_size); // TODO: VDPAU API says the size must be different form call to align data
                                    //       So we need to handle this case
        std::cout << "[SurfaceRGBA] Surface size: " << realSize.width << " x " << realSize.height << std::endl;
    }
}
