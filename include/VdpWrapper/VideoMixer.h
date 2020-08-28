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
