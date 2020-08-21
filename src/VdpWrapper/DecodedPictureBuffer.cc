#include <VdpWrapper/DecodedPictureBuffer.h>

#include <algorithm>
#include <cassert>
#include <iostream>

#include <VdpWrapper/Device.h>

namespace vw {
    DecodedPicture::DecodedPicture(Device& device, SizeI surfaceSize, NalReferenceType referenceType, int iFrameNum)
    : referenceType(referenceType)
    , surface(device, surfaceSize)
    , iFrameNum(iFrameNum) {

    }

    DecodedPicture& DecodedPictureBuffer::createDecodedPicture(Device& device, SizeI surfaceSize, NalReferenceType referenceType, int iFrameNum) {
        m_vecDecodedPictures.emplace_front(device, surfaceSize, referenceType, iFrameNum);

        while (m_vecDecodedPictures.size() > 16) {
            m_vecDecodedPictures.pop_back();
        }

        return m_vecDecodedPictures.front();
    }

    void DecodedPictureBuffer::updateReferenceList(H264Infos &infos) {
        int iCurrentReferenceIndex = 0;
        for (auto it = m_vecDecodedPictures.begin() + 1; it != m_vecDecodedPictures.end(); ++it) {
            const auto& decodedPicture = *it;

            if (decodedPicture.referenceType == NalReferenceType::NoReference) {
                continue;
            }

            assert(iCurrentReferenceIndex < 16);

            auto& referenceFrame = infos.referenceFrames[iCurrentReferenceIndex++];
            referenceFrame.is_long_term = VDP_FALSE;
            referenceFrame.top_is_reference = (static_cast<uint8_t>(decodedPicture.referenceType) & static_cast<uint8_t>(NalReferenceType::TopReference) ? VDP_TRUE : VDP_FALSE);
            referenceFrame.bottom_is_reference = (static_cast<uint8_t>(decodedPicture.referenceType) & static_cast<uint8_t>(NalReferenceType::BottomReference) ? VDP_TRUE : VDP_FALSE);
            referenceFrame.field_order_cnt[0] = 0;
            referenceFrame.field_order_cnt[1] = 0;
            referenceFrame.frame_idx = decodedPicture.iFrameNum;
            referenceFrame.surface = decodedPicture.surface.getVdpHandle();
        }
    }

    void DecodedPictureBuffer::clear() {
        m_vecDecodedPictures.clear();
    }
}
