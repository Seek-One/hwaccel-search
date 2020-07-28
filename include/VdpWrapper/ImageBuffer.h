#ifndef VW_IMAGE_BUFFER_H
#define VW_IMAGE_BUFFER_H

#include <opencv2/imgcodecs.hpp>

namespace vw {
    enum class ImageFormat {
        BGRA, // packed BGRA 8:8:8:8, 32bpp, BGRABGRA (single plane)
        YUV420p, // planar YUV 4:2:0, 12bpp, (1 Cr & Cb sample per 2x2 Y samples)
    };

    using Plane = std::vector<uint8_t>;

    class ImageBuffer {
    public:
        ImageBuffer(cv::Mat &decodedImage, ImageFormat format);

        uint32_t getLineSize(uint32_t index = 0) const;
        const uint8_t* getPlane(uint32_t index = 0) const;

    private:
        void storeBGRAImage(cv::Mat &decodedImage);

    private:
        ImageFormat m_format;
        std::vector<Plane> m_planes;
        std::vector<uint32_t> m_lineSizes;
    };
}

#endif // VW_IMAGE_BUFFER_H
