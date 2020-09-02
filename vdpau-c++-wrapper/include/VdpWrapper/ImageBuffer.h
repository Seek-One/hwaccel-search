/* Copyright (c) 2020 Jet1oeil
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef VW_IMAGE_BUFFER_H
#define VW_IMAGE_BUFFER_H

#include <opencv2/imgcodecs.hpp>

#include "Size.h"

namespace vw {
    using Plane = std::vector<uint8_t>;

    /**
     * @brief ImageBuffer is an utilty class to encapsule a raw image
     *
     * This class represents a raw image of any type (BGRA or YUV). Depending of
     * the real format, the number of planes must be different. Moreover, this
     * class can be read raw YUV file or image file.
     */
    class ImageBuffer {
    public:
        /**
         * @brief Construct a new ImageBuffer form a OpenCV matrix (aka. from a image file)
         *
         * @param decodedImage Raw data
         */
        ImageBuffer(cv::Mat &decodedImage);

        /**
         * @brief Construct a new ImageBuffer form a raw YUV file
         *
         * @param imageSize Image size
         * @param rawBytes  Raw data must be in NV12 format
         */
        ImageBuffer(SizeU imageSize, const std::vector<uint8_t> &rawBytes);

        ImageBuffer(std::vector<Plane>&& planes, std::vector<uint32_t>&& linesizes);

        /**
         * @brief Get the line size for the specified plane
         *
         * @param index Index of plane (default is 0)
         * @return uint32_t The plane size
         */
        uint32_t getLineSize(uint32_t index = 0) const;

        /**
         * @brief Get the specified plane data
         *
         * @param index Index of plane (default is 0)
         * @return const uint8_t* The pointer to the plane data
         */
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
