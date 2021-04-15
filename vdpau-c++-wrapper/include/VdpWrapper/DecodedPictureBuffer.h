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

#ifndef VW_DECODED_PICTURE_BUFFER_H
#define VW_DECODED_PICTURE_BUFFER_H

#include <deque>
#include <vector>

#include "NalUnit.h"
#include "Size.h"
#include "DecodedSurface.h"

namespace vw {
    class Device;

    /**
     * @brief DecodedPicture is a structure that encapsulates a decoded surface form Decoder
     *
     * The structure get the ownership of DecodedSurface and store all usefull informations
     * about the surface received form the docoder. This structure is stored into a
     * DecodedPictureBuffer.
     *
     * @sa Decoder, DecodedPictureBuffer
     */
    struct DecodedPicture {
        /**
         * @brief Construct a new Decoded Picture object
         *
         * A new DecodedSurface is created and it will be destroyed when the
         * implicit destructor is called.
         *
         * @param device A reference to a valid Device
         * @param surfaceSize The size of new surface
         */
        DecodedPicture(Device& device, SizeI surfaceSize);

        PictureReferenceType referenceType;     ///< The type of picture reference
        DecodedSurface surface;                 ///< The VDPAU decoded surface
        int iFrameNum;                          ///< The current frame_num
        int iTopFieldOrderCount;                ///< The value of top field picture order count
        int iBottomFieldOrderCount;             ///< The value of bottom field picture order count
        int iPictureOrderCount;                 ///< The picture order count has specified in H264 reference (pseudo-code 8-1)
    };

    /**
     * @brief DecodedPictureBuffer aims to handle a Decoded Picture Buffer (DPB)
     *
     * The class allocate a pool of DecodedPicture and reuse the oldest one. Hence
     * we have only one surface allocation and we avoid some artifacts during the
     * decoding process.
     * Currently the class doesn't handle the long term reference and the Memory
     * Management Control Operation (MMCO).
     *
     * @sa  DecodedPicture
     */
    class DecodedPictureBuffer {
    public:
        /**
         * @brief Construct a new DecodedPictureBuffer object
         *
         * @warning The constructor do not allocate any DecodedPicture. The allocation
         * is do by initializeSurfacePool() method.
         */
        DecodedPictureBuffer();

        /**
         * @brief Destroy the DecodedPictureBuffer object
         */
        ~DecodedPictureBuffer();

        DecodedPictureBuffer(const DecodedPictureBuffer&) = delete;
        DecodedPictureBuffer(DecodedPictureBuffer&&) = delete;

        DecodedPictureBuffer& operator=(const DecodedPictureBuffer&) = delete;
        DecodedPictureBuffer& operator=(DecodedPictureBuffer&&) = delete;

        /**
         * @brief Create the next available decoded surface
         *
         * The buffer acts like a ring buffer, we get here the next available surface.
         *
         * @param device A reference to a valid Device
         * @param infos The usefull H264 bitstream informations
         * @return DecodedPicture& A reference to a DecodedPicture
         */
        DecodedPicture& getNextDecodedPicture(Device& device, const H264Infos &infos);

        /**
         * @brief Update the H264Infos passed in parameter
         *
         * Set correctly the VdpPictureInfoH264::referenceFrames field from VDPAU API.
         *
         * @param infos A reference to H264Infos that will be updated
         */
        void updateReferenceList(H264Infos &infos);

        /**
         * @brief Clear all DecodedSurface in DPB
         *
         * This function must be call at each new IDR frame the clear all
         * old reference pictures stored in DPB.
         */
        void clear();

    private:
        /**
         * @brief Initialize the surface pool
         *
         * To avoid some black screen blinking, we allocate a pool of DecodedPicture.
         * This is called on the first call of getNextDecodedPicture and when all
         * DecodedPicture arse used, when override the oldest one. Hence, we avoid
         * to allocate "on the fly" the VdpVideoSurface.
         *
         * @param device A reference to a valid Device
         * @param surfaceSize The size of DecodedPicture
         */
        void initializeSurfacePool(Device& device, const SizeU& surfaceSize, int poolSize);

    private:
        std::vector<DecodedPicture> m_listDecodedPictures;
        std::deque<int> m_listIndexReferencePictures;
        int m_currentIndex;
    };
}

#endif // VW_DECODED_PICTURE_BUFFER_H
