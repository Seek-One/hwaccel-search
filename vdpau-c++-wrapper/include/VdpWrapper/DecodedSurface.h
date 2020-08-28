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

#ifndef VW_DECODED_SURFACE_H
#define VW_DECODED_SURFACE_H

#include <string>

#include <vdpau/vdpau.h>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Size.h>

namespace vw {
    /**
     * @brief DecodedSurface encapsules a VdpVideoSurface
     *
     * This class aims to wrap the call to VDPAU API to handle a VdpVideoSurface. The class
     * respect the RAII programming idiom hence when a new object is created is owning
     * a VdpVideoSurface and when it is destroyed the VdpVideoSurface is freed.
     */
    class DecodedSurface {
    public:
        /**
         * @brief Construct a empty DecodedSurface object
         *
         * @param device A reference to a valid Device
         * @param size The new surface size
         */
        DecodedSurface(Device& device, SizeU size);
        /**
         * @brief Construct a DecodedSurface object form raw YUV data
         *
         * @param device A reference to a valid Device
         * @param filename The filename were raw YUV data are stored (must be in NV12 format)
         * @param size The new surface size
         */
        DecodedSurface(Device& device, const std::string& filename, SizeU size);

        /**
         * @brief Destroy the DecodedSurface object
         */
        ~DecodedSurface();

        DecodedSurface(const DecodedSurface&) = delete;
        /**
         * @brief Move constructor of DecodedSurface
         *
         * @param other The old DecodedSurface
         */
        DecodedSurface(DecodedSurface&& other);

        DecodedSurface& operator=(const DecodedSurface&) = delete;
        /**
         * @brief Move assignment operator of DecodedSurface
         *
         * @param other The old DecodedSurface
         * @return DecodedSurface& The new DecodedSurface
         */
        DecodedSurface& operator=(DecodedSurface&& other);

        /**
         * @brief Get the sufrace size
         *
         * @return SizeU The surface size
         */
        SizeU getSize() const;
        /**
         * @brief Get the opaque VDPAU handle value
         *
         * @return VdpVideoSurface The opaque VDPAU handle value
         */
        VdpVideoSurface getVdpHandle() const;
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

    private:
        void allocateVdpSurface(Device& device, const SizeU& size);

    private:
        VdpVideoSurface m_vdpVideoSurface;
        SizeU m_size;
        int m_iPictureOrderCount;
    };
}

#endif // VW_DECODED_SURFACE_H
