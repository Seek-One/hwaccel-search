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

#ifndef VW_RENDER_SURFACE_H
#define VW_RENDER_SURFACE_H

#include <string>

#include <vdpau/vdpau.h>

#include "ImageBuffer.h"
#include "Size.h"

namespace vw {
    class Device;

    /**
     * @brief DecodedSurface encapsules a VdpOutputSurface
     *
     * This class aims to wrap the call to VDPAU API to handle a VdpOutputSurface. The class
     * respect the RAII programming idiom hence when a new object is created is owning
     * a VdpOutputSurface and when it is destroyed the VdpOutputSurface is freed.
     */
    class RenderSurface {
    public:
        /**
         * @brief Construct a empty RenderSurface
         *
         * @param device A reference to a valid Device
         * @param size The new surface size
         */
        RenderSurface(Device& device, const SizeU& size);
        /**
         * @brief Construct a new RenderSurface form an image
         *
         * The specified image is decompressed by OpenCV and the
         * new surface will be filled with this values
         *
         * @param device A reference to a valid Device
         * @param filename Filname of the source image
         */
        RenderSurface(Device& device, const std::string& filename);
        /**
         * @brief Destroy the Render Surface object
         */
        ~RenderSurface();

        RenderSurface(const RenderSurface&) = delete;
        /**
         * @brief Move constructor of RenderSurface
         *
         * @param other The old RenderSurface
         */
        RenderSurface(RenderSurface&& other);

        RenderSurface& operator=(const RenderSurface&) = delete;
        /**
         * @brief Move assignment operator of RenderSurface
         *
         * @param other The old RenderSurface
         * @return RenderSurface& The new RenderSurface
         */
        RenderSurface& operator=(RenderSurface&& other);

        /**
         * @brief Get the opaque VDPAU handle value
         *
         * @return VdpVideoSurface The opaque VDPAU handle value
         */
        VdpOutputSurface getVdpHandle() const;
        /**
         * @brief Set the Picture Order Count (POC)
         *
         * @param iPictureOrderCount New POC value
         */
        void setPictureOrderCount(int iPictureOrderCount);
        /**
         * @brief Get the Picture Order Count (POC)
         *
         * @return int The current POC value
         */
        int getPictureOrderCount() const;

        /**
         * @brief Copy the GPU data to an ImageBuffer
         *
         * @return ImageBuffer The image buffer filled with GPU data
         */
        ImageBuffer copyHardwareMemory();

    private:
        void allocateVdpSurface(Device& device, const SizeU& size);

    private:
        VdpOutputSurface m_vdpOutputSurface;
        SizeU m_size;
        int m_iPictureOrderCount;
    };
}

#endif // VW_RENDER_SURFACE_H
