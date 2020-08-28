#include <VdpWrapper/DecodedPictureBuffer.h>

#include <cassert>
#include <cmath>

#include <VdpWrapper/Device.h>

namespace vw {
    DecodedPicture::DecodedPicture(Device& device, SizeI surfaceSize)
    : referenceType(PictureReferenceType::NoReference)
    , surface(device, surfaceSize)
    , iFrameNum(0)
    , iTopFieldOrderCount(0)
    , iBottomFieldOrderCount(0)
    , iPictureOrderCount(0) {

    }

    DecodedPicture& DecodedPictureBuffer::createDecodedPicture(Device& device, const H264Infos &infos) {
        m_vecDecodedPictures.emplace_front(device, infos.pictureSize);
        auto& decodedPicture = m_vecDecodedPictures.front();

        decodedPicture.referenceType = infos.referenceType;
        decodedPicture.iFrameNum = infos.frame_num;
        decodedPicture.iTopFieldOrderCount = infos.field_order_cnt[0];
        decodedPicture.iBottomFieldOrderCount = infos.field_order_cnt[1];

        if (infos.field_order_cnt && infos.bottom_field_flag) {
            decodedPicture.iPictureOrderCount = decodedPicture.iBottomFieldOrderCount;
        } else if (infos.field_order_cnt) {
            decodedPicture.iPictureOrderCount = decodedPicture.iTopFieldOrderCount;
        } else {
            decodedPicture.iPictureOrderCount = std::min(decodedPicture.iTopFieldOrderCount, decodedPicture.iBottomFieldOrderCount);
        }

        // Sliding window
        // TODO: must be refactored
        while (m_vecDecodedPictures.size() > 16) {
            m_vecDecodedPictures.pop_back();
        }

        return decodedPicture;
    }

    void DecodedPictureBuffer::updateReferenceList(H264Infos &infos) {
        int iCurrentReferenceIndex = 0;
        for (auto it = m_vecDecodedPictures.begin() + 1; it != m_vecDecodedPictures.end(); ++it) {
            const auto& decodedPicture = *it;

            if (decodedPicture.referenceType == PictureReferenceType::NoReference) {
                continue;
            }

            assert(iCurrentReferenceIndex < 16);

            auto& referenceFrame = infos.referenceFrames[iCurrentReferenceIndex++];
            referenceFrame.is_long_term = VDP_FALSE;
            referenceFrame.top_is_reference = (static_cast<uint8_t>(decodedPicture.referenceType) & static_cast<uint8_t>(PictureReferenceType::TopReference) ? VDP_TRUE : VDP_FALSE);
            referenceFrame.bottom_is_reference = (static_cast<uint8_t>(decodedPicture.referenceType) & static_cast<uint8_t>(PictureReferenceType::BottomReference) ? VDP_TRUE : VDP_FALSE);
            referenceFrame.field_order_cnt[0] = decodedPicture.iTopFieldOrderCount;
            referenceFrame.field_order_cnt[1] = decodedPicture.iBottomFieldOrderCount;
            referenceFrame.frame_idx = decodedPicture.iFrameNum;
            referenceFrame.surface = decodedPicture.surface.getVdpHandle();
        }
    }

    void DecodedPictureBuffer::clear() {
        m_vecDecodedPictures.clear();
    }
}
