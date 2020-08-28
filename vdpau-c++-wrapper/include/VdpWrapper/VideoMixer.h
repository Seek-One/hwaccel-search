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

#ifndef VW_VIDEO_MIXER_H
#define VW_VIDEO_MIXER_H

#include <vdpau/vdpau.h>

#include <VdpWrapper/Size.h>

namespace vw {
    class Device;
    class RenderSurface;
    class DecodedSurface;

    /**
     * @brief VideoMixer class encapsules a VdpVideoMixer
     *
     * This class aims to wrap the call to VDPAU API to handle a VdpVideoMixer. The class
     * respect the RAII programming idiom hence when a new object is created is owning
     * a VdpVideoMixer and when it is destroyed the VdpVideoMixer is freed.
     */
    class VideoMixer {
    public:
        /**
         * @brief Construct a new VideoMixer object
         *
         * @param device A reference to a valid Device
         * @param outputSize The output surface size
         */
        VideoMixer(Device& device, SizeU outputSize);
        /**
         * @brief Destroy the VideoMixer object
         *
         */
        ~VideoMixer();

        VideoMixer(const VideoMixer&) = delete;
        VideoMixer(VideoMixer&&) = delete;

        VideoMixer& operator=(const VideoMixer&) = delete;
        VideoMixer& operator=(VideoMixer&&) = delete;

        /**
         * @brief Set the ouput surface size
         *
         * @param outputSize New output surface size
         */
        void setOutputSize(SizeU outputSize);

        /**
         * @brief Post-process an input surface and return a processed surface
         *
         * This method actually process the image. Currently, it's juste an scaling
         * but VDPAU provides other post-processing (like deinterlacing, noise-reduction,
         * Color space conversion...).
         *
         * @note The VdpVideoMixer object is on the first call of this method since
         * the video mixer depending on input surface size. Hence if the input size
         * changes, it's mandatory to recreate a new VideoMixer object.
         *
         * @param inputSurface Raw input surface
         * @return RenderSurface The post-proccesed surface
         */
        RenderSurface process(DecodedSurface &inputSurface);

    private:
        void createMixer(SizeU size);

    private:
        Device& m_device;
        VdpVideoMixer m_mixer;
        SizeU m_outputSize;
    };
}

#endif // VW_VIDEO_MIXER_H
