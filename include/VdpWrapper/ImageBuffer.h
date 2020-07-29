#ifndef VW_IMAGE_BUFFER_H
#define VW_IMAGE_BUFFER_H

#include <opencv2/imgcodecs.hpp>

#include <VdpWrapper/Size.h>

namespace vw {
    using Plane = std::vector<uint8_t>;

    class ImageBuffer {
    public:
        // Constructor for BGR images
        ImageBuffer(cv::Mat &decodedImage);

        // Constructor for NV12 YUV images
        ImageBuffer(SizeU imageSize, const std::vector<uint8_t> &rawBytes);

        uint32_t getLineSize(uint32_t index = 0) const;
        const uint8_t* getPlane(uint32_t index = 0) const;

    private:
        void storeBGRAImage(cv::Mat &decodedImage);
        void storeNV12Image(SizeU imageSize, const std::vector<uint8_t> &rawBytes);

    private:
        std::vector<Plane> m_planes;
        std::vector<uint32_t> m_lineSizes;
    };
}

#endif // VW_IMAGE_BUFFER_H
