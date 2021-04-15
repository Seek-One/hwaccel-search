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

    DecodedPictureBuffer::DecodedPictureBuffer()
    : m_currentIndex(0) {
    }

    DecodedPictureBuffer::~DecodedPictureBuffer() {
        m_listDecodedPictures.clear();
        m_listIndexReferencePictures.clear();
    }

    DecodedPicture& DecodedPictureBuffer::getNextDecodedPicture(Device& device, const H264Infos &infos) {
        // Initialize the ring buffer
        if (m_listDecodedPictures.size() == 0) {
            initializeSurfacePool(device, infos.pictureSize);
        }

        assert(m_currentIndex >= 0 && m_currentIndex < PoolSize);
        auto& decodedPicture = m_listDecodedPictures[m_currentIndex];

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

        // Add the image to reference picture list
        if (decodedPicture.referenceType != PictureReferenceType::NoReference) {
            // Sliding window
            m_listIndexReferencePictures.push_front(m_currentIndex);
            while (m_listIndexReferencePictures.size() > 16) {
                m_listIndexReferencePictures.pop_back();
            }
        }

        m_currentIndex = (m_currentIndex + 1) % PoolSize;

        return decodedPicture;
    }

    void DecodedPictureBuffer::updateReferenceList(H264Infos &infos) {
        int h264InfoRefIndex = 0;

        // Skip the first ref frame
        for (std::size_t i = 1; i < m_listIndexReferencePictures.size(); ++i) {
            int refDBPIndex = m_listIndexReferencePictures[i];

            assert(refDBPIndex >= 0 && refDBPIndex < PoolSize);
            assert(h264InfoRefIndex < 16);

            const auto& decodedPicture = m_listDecodedPictures[refDBPIndex];

            auto& referenceFrame = infos.referenceFrames[h264InfoRefIndex++];
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
        m_listIndexReferencePictures.clear();
    }

    void DecodedPictureBuffer::initializeSurfacePool(Device& device, const SizeU& surfaceSize) {
        for (int i = 0; i < PoolSize; ++i) {
            m_listDecodedPictures.emplace_back(device, surfaceSize);
        }
    }
}
