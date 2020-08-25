#ifndef VW_DECODED_PICTURE_BUFFER_H
#define VW_DECODED_PICTURE_BUFFER_H

#include <deque>

#include "NalUnit.h"
#include "Size.h"
#include "DecodedSurface.h"


namespace vw {
    class Device;

    struct DecodedPicture {
        DecodedPicture(Device& device, SizeI surfaceSize);

        PictureReferenceType referenceType;
        DecodedSurface surface;
        int iFrameNum;
        int iTopFieldOrderCount;
        int iBottomFieldOrderCount;
    };

    class DecodedPictureBuffer {
    public:
        DecodedPictureBuffer() = default;
        ~DecodedPictureBuffer() = default;

        DecodedPictureBuffer(const DecodedPictureBuffer&) = delete;
        DecodedPictureBuffer(DecodedPictureBuffer&&) = delete;

        DecodedPictureBuffer& operator=(const DecodedPictureBuffer&) = delete;
        DecodedPictureBuffer& operator=(DecodedPictureBuffer&&) = delete;

        DecodedPicture& createDecodedPicture(Device& device, const H264Infos &infos);

        void updateReferenceList(H264Infos &infos);

        void clear();

    private:
        std::deque<DecodedPicture> m_vecDecodedPictures;
    };
}

#endif // VW_DECODED_PICTURE_BUFFER_H
