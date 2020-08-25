#include <VdpWrapper/NalUnit.h>

namespace vw {
    H264Infos::H264Infos() {
        // VdpPictureInfoH264 fields
        slice_count = 0;
        field_order_cnt[0] = 0;
        field_order_cnt[1] = 0;
        is_reference = VDP_FALSE;

        frame_num = 0;
        field_pic_flag = 0;
        bottom_field_flag = 0;
        num_ref_frames = 0;
        mb_adaptive_frame_field_flag = 0;
        constrained_intra_pred_flag = 0;
        weighted_pred_flag = 0;
        weighted_bipred_idc = 0;
        frame_mbs_only_flag = 0;
        transform_8x8_mode_flag = 0;
        chroma_qp_index_offset = 0;
        second_chroma_qp_index_offset = 0;
        pic_init_qp_minus26 = 0;
        num_ref_idx_l0_active_minus1 = 0;
        num_ref_idx_l1_active_minus1 = 0;
        log2_max_frame_num_minus4 = 0;
        pic_order_cnt_type = 0;
        log2_max_pic_order_cnt_lsb_minus4 = 0;
        delta_pic_order_always_zero_flag = 0;
        direct_8x8_inference_flag = 0;
        entropy_coding_mode_flag = 0;
        pic_order_present_flag = 0;
        deblocking_filter_control_present_flag = 0;
        redundant_pic_cnt_present_flag = 0;

        for (int i = 0; i < 6; ++i) {
            for (int j = 0; j < 16; ++j) {
                scaling_lists_4x4[i][j] = 0;
            }
        }

        for (int i = 0; i < 2; ++i) {
            for (int j = 0; j < 64; ++j) {
                scaling_lists_8x8[i][j] = 0;
            }
        }

        for (int i = 0; i < 16; ++i) {
            referenceFrames[i].surface = VDP_INVALID_HANDLE;
            referenceFrames[i].is_long_term = VDP_FALSE;
            referenceFrames[i].top_is_reference = VDP_FALSE;
            referenceFrames[i].bottom_is_reference = VDP_FALSE;
            referenceFrames[i].field_order_cnt[0] = 0;
            referenceFrames[i].field_order_cnt[1] = 0;
            referenceFrames[i].frame_idx = 0;
        }

        // Extra fields
        iProfile = 0;

        bFirstSPSReceived = false;
        bFirstPPSReceived = false;
        referenceType = PictureReferenceType::NoReference;
    }

    NalUnit::NalUnit()
    : m_type(NalType::Unspecified) {

    }

    NalUnit::NalUnit(const H264Infos& h264Infos, NalType type, const std::vector<uint8_t>& data)
    : m_h264Infos(h264Infos)
    , m_type(type)
    , m_data(data) {

    }

    NalType NalUnit::getType() const {
        return m_type;
    }

    const H264Infos& NalUnit::getH264Infos() const {
        return m_h264Infos;
    }

    const std::vector<uint8_t>& NalUnit::getBitstream() const {
        return m_data;
    }
}
