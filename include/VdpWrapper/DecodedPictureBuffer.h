#ifndef VW_DECODED_PICTURE_BUFFER_H
#define VW_DECODED_PICTURE_BUFFER_H

#include <deque>

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
     * The class keep a collection of DecodedPicture ordered by desc decoding order.
     * The limit size for the DPB is 16 pictures, when the max capacity is reached
     * the class remove the oldest picture (sliding window).
     * Currently the class doesn't handle the long term reference and the Memory
     * Management Control Operation (MMCO).
     *
     * @sa  DecodedPicture
     */
    class DecodedPictureBuffer {
    public:
        DecodedPictureBuffer() = default;
        ~DecodedPictureBuffer() = default;

        DecodedPictureBuffer(const DecodedPictureBuffer&) = delete;
        DecodedPictureBuffer(DecodedPictureBuffer&&) = delete;

        DecodedPictureBuffer& operator=(const DecodedPictureBuffer&) = delete;
        DecodedPictureBuffer& operator=(DecodedPictureBuffer&&) = delete;

        /**
         * @brief Create a new DecodedPicture
         *
         * The new created DecodedPicture is automatically added to the DPB even if it's not
         * a reference picture.
         *
         * @todo Don't add the decoded picture here. Juste return the structure by moving and
         * after, if the picture is a reference add it to the DPB.
         *
         * @param device A reference to a valid Device
         * @param infos The usefull H264 bitstream informations
         * @return DecodedPicture& A reference to the new DecodedPicture
         */
        DecodedPicture& createDecodedPicture(Device& device, const H264Infos &infos);

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
        std::deque<DecodedPicture> m_vecDecodedPictures;
    };
}

#endif // VW_DECODED_PICTURE_BUFFER_H
