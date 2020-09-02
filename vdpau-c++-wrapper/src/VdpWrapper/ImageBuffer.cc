#include <VdpWrapper/ImageBuffer.h>

namespace vw {
    ImageBuffer::ImageBuffer(cv::Mat &decodedImage) {
        storeBGRAImage(decodedImage);
    }

    ImageBuffer::ImageBuffer(SizeU imageSize, const std::vector<uint8_t> &rawBytes) {
        storeNV12Image(imageSize, rawBytes);
    }

    ImageBuffer::ImageBuffer(std::vector<Plane>&& planes, std::vector<uint32_t>&& linesizes)
    : m_planes(planes)
    , m_lineSizes(linesizes) {

    }

    uint32_t ImageBuffer::getLineSize(uint32_t index) const {
        if (index >= m_lineSizes.size()) {
            throw std::runtime_error("[ImageBuffer] Line size index out of bounds");
        }

        return m_lineSizes[index];
    }

    const uint8_t* ImageBuffer::getPlane(uint32_t index) const {
        if (index >= m_planes.size()) {
            throw std::runtime_error("[ImageBuffer] Plane index out of bounds");
        }

        return m_planes[index].data();
    }

    void ImageBuffer::storeBGRAImage(cv::Mat &decodedImage) {
        // Ensure we have a BGR 8bits format (without alpha)
        assert(decodedImage.channels() == 3); // No alpha channel
        assert(decodedImage.depth() == CV_8U); // Only 8 bits color
        assert(decodedImage.isContinuous()); // Data must be continuous

        // Set planes / sizes vectors
        m_lineSizes.resize(1);
        m_planes.resize(1);

        // Set line size
        m_lineSizes[0] = decodedImage.size().width * 4; // one line is 4 unit8_t color
        m_planes[0].resize(decodedImage.size().width * decodedImage.size().height * 4);

        // Fill the buffers
        cv::MatIterator_<cv::Vec3b> it, end;
        int index = 0;
        for (it = decodedImage.begin<cv::Vec3b>(), end = decodedImage.end<cv::Vec3b>(); it != end; ++it, index += 4) {
            m_planes[0][index + 0] = (*it)[0]; // Blue
            m_planes[0][index + 1] = (*it)[1]; // Green
            m_planes[0][index + 2] = (*it)[2]; // Red
            m_planes[0][index + 3] = std::numeric_limits<uint8_t>::max(); // Alpha channel
        }
    }

    void ImageBuffer::storeNV12Image(SizeU imageSize, const std::vector<uint8_t> &rawBytes) {
        // Check if there are enough bytes
        assert(rawBytes.size() == imageSize.width * imageSize.height * 12 / 8);

        // Set planes / sizes vectors
        m_lineSizes.resize(2);
        m_planes.resize(2);

        // Set line size
        m_lineSizes[0] = imageSize.width;
        m_lineSizes[1] = imageSize.width;

        // Fill Y values
        unsigned i = 0;
        unsigned bounds = imageSize.width * imageSize.height;
        for (; i < bounds; ++i) {
            m_planes[0].push_back(rawBytes[i]);
        }

        // Fill UV
        bounds += imageSize.width * imageSize.height / 2;
        for (; i < bounds; ++i) {
            m_planes[1].push_back(rawBytes[i]);
        }

        assert(i == rawBytes.size());
        assert(m_planes[0].size() == imageSize.width * imageSize.height);
        assert(m_planes[1].size() == imageSize.width * imageSize.height / 2);
    }
}
