#include "H264Parser.h"

#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>

H264Parser::H264Parser(const std::string& filename)
: m_h264Stream(h264_new())
, m_pDataCursor(nullptr)
, m_unprocessedDataSize(0) {
    std::ifstream bitstreamFile(filename, std::ios::binary);
    if (!bitstreamFile.good()) {
        throw std::runtime_error("[H264Parser] Couldn't open the bitstream file");
    }
    bitstreamFile.unsetf(std::ios::skipws);

    // Load all data
    m_bitstream.insert(m_bitstream.begin(), std::istream_iterator<uint8_t>(bitstreamFile), std::istream_iterator<uint8_t>());
    m_pDataCursor = m_bitstream.data();
    m_unprocessedDataSize = m_bitstream.size();
}

H264Parser::~H264Parser() {
    h264_free(m_h264Stream);
}

bool H264Parser::readNextNAL(vw::NalUnit &nalUnit) {
    int iNalStart = 0;
    int iNalEnd = 0;

    if (find_nal_unit(m_pDataCursor, m_unprocessedDataSize, &iNalStart, &iNalEnd) <= 0) {
        return false;
    }

    // Keep the begin of NAL
    // We need to keep the start code for VDPAU API.
    // Without the start code, the VDPAU decoder cannot
    // decode properly the bitstream and the surface is empty (filled in black)
    uint8_t* nalUnitStartData = m_pDataCursor;

    // Process the NAL unit and update the h264 context
    m_pDataCursor += iNalStart;
    read_nal_unit(m_h264Stream, m_pDataCursor, iNalEnd - iNalStart);
    updateH264Infos();

    // Copy NAL data
    std::vector<uint8_t> nalData(nalUnitStartData, nalUnitStartData + iNalEnd);
    nalUnit = vw::NalUnit(m_h264Infos, static_cast<vw::NalType>(m_h264Stream->nal->nal_unit_type), nalData);

    // Skip to next NAL
    m_pDataCursor += (iNalEnd - iNalStart);
    m_unprocessedDataSize -= iNalEnd;

    return true;
}

void H264Parser::updateH264Infos() {
    vw::NalType nalType = static_cast<vw::NalType>(m_h264Stream->nal->nal_unit_type);
    switch (nalType) {
    case vw::NalType::SPS:
        m_h264Infos.num_ref_frames = m_h264Stream->sps->num_ref_frames;
        m_h264Infos.mb_adaptive_frame_field_flag = m_h264Stream->sps->mb_adaptive_frame_field_flag;
        m_h264Infos.frame_mbs_only_flag = m_h264Stream->sps->frame_mbs_only_flag;
        m_h264Infos.log2_max_frame_num_minus4 = m_h264Stream->sps->log2_max_frame_num_minus4;
        m_h264Infos.pic_order_cnt_type = m_h264Stream->sps->pic_order_cnt_type;
        m_h264Infos.log2_max_pic_order_cnt_lsb_minus4 = m_h264Stream->sps->log2_max_pic_order_cnt_lsb_minus4;
        m_h264Infos.delta_pic_order_always_zero_flag = m_h264Stream->sps->delta_pic_order_always_zero_flag;
        m_h264Infos.direct_8x8_inference_flag = m_h264Stream->sps->direct_8x8_inference_flag;

        // FIXME: h264bitstream do not compute correctly the scaling list
        //        they are always at zero. So I set manually the default scaling list
        for (int i = 0; i < 6; i++) {
            for (int j = 0; j < 16; j++) {
                // m_h264Infos.scaling_lists_4x4[i][j] = m_h264Stream->sps->ScalingList4x4[i][j];
                m_h264Infos.scaling_lists_4x4[i][j] = 16;
            }
        }

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 64; j++) {
                // m_h264Infos.scaling_lists_8x8[i][j] = m_h264Stream->sps->ScalingList8x8[i][j];
                m_h264Infos.scaling_lists_8x8[i][j] = 16;
            }
        }

        m_h264Infos.bFirstSPSReceived = true;
        computeVideoSize();
        m_h264Infos.iProfile = m_h264Stream->sps->profile_idc;
        break;

    case vw::NalType::PPS:
        m_h264Infos.constrained_intra_pred_flag = m_h264Stream->pps->constrained_intra_pred_flag;
        m_h264Infos.weighted_pred_flag = m_h264Stream->pps->weighted_pred_flag;
        m_h264Infos.weighted_bipred_idc = m_h264Stream->pps->weighted_bipred_idc;
        m_h264Infos.transform_8x8_mode_flag = m_h264Stream->pps->transform_8x8_mode_flag;
        m_h264Infos.chroma_qp_index_offset = m_h264Stream->pps->chroma_qp_index_offset;
        m_h264Infos.second_chroma_qp_index_offset = m_h264Stream->pps->second_chroma_qp_index_offset;
        m_h264Infos.pic_init_qp_minus26 = m_h264Stream->pps->pic_init_qp_minus26;
        m_h264Infos.num_ref_idx_l0_active_minus1 = m_h264Stream->pps->num_ref_idx_l0_active_minus1;
        m_h264Infos.num_ref_idx_l1_active_minus1 = m_h264Stream->pps->num_ref_idx_l1_active_minus1;
        m_h264Infos.entropy_coding_mode_flag = m_h264Stream->pps->entropy_coding_mode_flag;
        m_h264Infos.pic_order_present_flag = m_h264Stream->pps->pic_order_present_flag;
        m_h264Infos.deblocking_filter_control_present_flag = m_h264Stream->pps->deblocking_filter_control_present_flag;
        m_h264Infos.redundant_pic_cnt_present_flag = m_h264Stream->pps->redundant_pic_cnt_present_flag;

        // FIXME: h264bitstream do not compute correctly the scaling list
        //        they are always at zero. So I set manually the default scaling list
        // for(int i=0; i<6; i++) {
        //     for(int j=0; j<16; j++) {
        //         m_h264Infos.scaling_lists_4x4[i][j] = m_h264Stream->pps->ScalingList4x4[i][j];
        //     }
        // }

        // for(int i=0; i<2; i++) {
        //     for(int j=0; j<64; j++) {
        //         m_h264Infos.scaling_lists_8x8[i][j] = m_h264Stream->pps->ScalingList8x8[i][j];
        //     }
        // }

        m_h264Infos.bFirstPPSReceived = true;
        break;

    case vw::NalType::CodedSliceIDR:
        m_h264Infos.is_reference = VDP_TRUE;
        m_h264Infos.frame_num = m_h264Stream->sh->frame_num;
        m_h264Infos.field_pic_flag = m_h264Stream->sh->field_pic_flag;
        m_h264Infos.bottom_field_flag = m_h264Stream->sh->bottom_field_flag;
        m_h264Infos.slice_count = 1; // We send always 1 slice
        break;

    default:
        // Nothing to do
        break;
    }
}

int H264Parser::computeSubWidthC() const {
    switch (m_h264Stream->sps->chroma_format_idc) {
    case 0: // monochrome
        break;

    case 1: // 4:2:0
    case 2: // 4:2:2
        return 2;

    case 3: // 4:4:4
        if (!m_h264Stream->sps->residual_colour_transform_flag) {
            return 1;
        }
    }

    return -1;
}

int H264Parser::computeSubHeightC() const {
    switch (m_h264Stream->sps->chroma_format_idc) {
    case 0: // monochrome
        break;

    case 1: // 4:2:0
        return 2;

    case 2: // 4:2:2
        return 1;

    case 3: // 4:4:4
        if (!m_h264Stream->sps->residual_colour_transform_flag) {
            return 1;
        }
        break;
    }

    return -1;
}

void H264Parser::computeVideoSize() {
    // Compute width
    uint32_t iCropUnitX = (computeSubWidthC() == -1 ? 1 : computeSubWidthC());

    // cf. Rec. ITU-T H.264 (06/2019) equation (7-14)
    uint32_t iWidthTotalSize = (m_h264Stream->sps->pic_width_in_mbs_minus1 + 1) * 16;

    // cf. Rec. ITU-T H.264 (06/2019) equation (7-21 and 7-22)
    uint32_t iWidthCrop = iWidthTotalSize - iCropUnitX * (m_h264Stream->sps->frame_crop_right_offset + m_h264Stream->sps->frame_crop_left_offset);

    // Compute height
    int iCropUnitY = computeSubHeightC();
    if (iCropUnitY == -1) {
        iCropUnitY = 2 - m_h264Stream->sps->frame_mbs_only_flag;
    } else {
        iCropUnitY = computeSubHeightC() * (2 - m_h264Stream->sps->frame_mbs_only_flag);
    }

    // Size without cropping
    uint32_t iHeightTotalSize =
            (2 - m_h264Stream->sps->frame_mbs_only_flag)
            * (m_h264Stream->sps->pic_height_in_map_units_minus1 + 1)
            * 16; // cf. Rec. ITU-T H.264 (06/2019) equation (7-18)

    uint32_t iHeightCrop = iHeightTotalSize - iCropUnitY * (m_h264Stream->sps->frame_crop_top_offset + m_h264Stream->sps->frame_crop_bottom_offset);

    m_h264Infos.videoSize = vw::SizeU(iWidthCrop, iHeightCrop);

    std::cout << "[H264Parser] Video size: " << m_h264Infos.videoSize.width << "x" << m_h264Infos.videoSize.height << std::endl;
}
