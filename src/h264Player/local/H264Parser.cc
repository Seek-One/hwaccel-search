#include "H264Parser.h"

#include <cmath>
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>

H264Parser::H264Parser(const std::string& filename)
: m_h264Stream(h264_new())
, m_pDataCursor(nullptr)
, m_unprocessedDataSize(0)
, m_prevPicOrderCntMsb(0)
, m_prevPicOrderCntLsb(0) {
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
        std::cout << "[H264Parser] New IDR" << std::endl;
        [[fallthrough]];
    case vw::NalType::CodedSliceNonIDR:
        // if (nalType == vw::NalType::CodedSliceIDR) {
        //     std::cout << "[H264Parser] New IDR" << std::endl;
        // }
        // if (m_h264Stream->nal->nal_ref_idc) {
        //     std::cout << "[H264Parser] is a reference" << std::endl;
        // } else {
        //     std::cout << "[H264Parser] is not a reference" << std::endl;
        // }
        // if (m_h264Stream->sh->drpm.long_term_reference_flag) {
        //     std::cout << "[H264Parser] is a long term reference" << std::endl;
        // } else {
        //     std::cout << "[H264Parser] is a short term reference" << std::endl;
        // }
        // std::cout << "[H264Parser] LongTermFrameIdx = " << m_h264Stream->sh->drpm.long_term_frame_idx[0] << std::endl;
        // std::cout << "[H264Parser] adaptive_ref_pic_marking_mode_flag = " << m_h264Stream->sh->drpm.adaptive_ref_pic_marking_mode_flag << std::endl;
        // std::cout << "[H264Parser] field_pic_flag = " << m_h264Stream->sh->field_pic_flag << std::endl;
        // std::cout << "[H264Parser] bottom_field_flag = " << m_h264Stream->sh->bottom_field_flag << std::endl;
        // std::cout << "[H264Parser] memory_management_control_operation = " << m_h264Stream->sh->drpm.memory_management_control_operation[0] << std::endl;
        // m_h264Infos.is_reference = (nalType == vw::NalType::CodedSliceIDR) ? VDP_TRUE : VDP_FALSE;
        if (m_h264Stream->nal->nal_ref_idc) {
            if (m_h264Stream->sh->field_pic_flag && !m_h264Stream->sh->bottom_field_flag) {
                m_h264Infos.referenceType = vw::NalReferenceType::TopReference;
            } else if (m_h264Stream->sh->field_pic_flag && m_h264Stream->sh->bottom_field_flag) {
                m_h264Infos.referenceType = vw::NalReferenceType::BottomReference;
            } else {
                m_h264Infos.referenceType = vw::NalReferenceType::FrameReference;
            }
        } else {
            m_h264Infos.referenceType = vw::NalReferenceType::NoReference;
        }

        m_h264Infos.is_reference = (m_h264Stream->nal->nal_ref_idc ? VDP_TRUE : VDP_FALSE);
        m_h264Infos.frame_num = m_h264Stream->sh->frame_num;
        m_h264Infos.field_pic_flag = m_h264Stream->sh->field_pic_flag;
        m_h264Infos.bottom_field_flag = m_h264Stream->sh->bottom_field_flag;
        m_h264Infos.slice_count = 1; // We send always 1 slice
        computePoc();
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

    // std::cout << "[H264Parser] Video size: " << m_h264Infos.videoSize.width << "x" << m_h264Infos.videoSize.height << std::endl;
}

void H264Parser::computePoc() {
#if 0
    if (m_h264Stream->sps->pic_order_cnt_type != 0) {
        throw std::runtime_error("[H264Parser] Only pic_order_cnt_type == 0 is handled");
    }

    auto nalType = static_cast<vw::NalType>(m_h264Stream->nal->nal_unit_type);
    std::cout << "nal_unit_type: " << m_h264Stream->nal->nal_unit_type << std::endl;
    std::cout << "nal_ref_idc: " << m_h264Stream->nal->nal_ref_idc << std::endl;

    if (nalType == vw::NalType::CodedSliceIDR) {
        m_prevPicOrderCntMsb = 0;
        m_prevPicOrderCntLsb = 0;
    }
    else {
        // FIXME: Missing case for memory_management_control_operation == 5

        // Otherwise (the previous reference picture in decoding order did
        // not include a memory_management_control_operation equal to 5),
        // prevPicOrderCntMsb is set equal to PicOrderCntMsb of the previous
        // reference picture in decoding order and prevPicOrderCntLsb is set
        // equal to the value of pic_order_cnt_lsb of the previous reference
        // picture in decoding order
        // m_prevPicOrderCntMsb = ?; // FIXME
        // m_prevPicOrderCntLsb = ?; // FIXME
    }
    std::cout << "m_prevPicOrderCntMsb: " << m_prevPicOrderCntMsb << std::endl;
    std::cout << "m_prevPicOrderCntLsb: " << m_prevPicOrderCntLsb << std::endl;

    int32_t MaxPicOrderCntLsb = std::exp2(m_h264Stream->sps->log2_max_pic_order_cnt_lsb_minus4 + 4);
    std::cout << "MaxPicOrderCntLsb: " << MaxPicOrderCntLsb << std::endl;
    int32_t PicOrderCntMsb = 0;
    if (
        m_h264Stream->sh->pic_order_cnt_lsb < m_prevPicOrderCntLsb
        &&
        m_prevPicOrderCntLsb - m_h264Stream->sh->pic_order_cnt_lsb >= MaxPicOrderCntLsb / 2
    ) {
        PicOrderCntMsb = m_prevPicOrderCntMsb + MaxPicOrderCntLsb;
    }
    else if (
        m_h264Stream->sh->pic_order_cnt_lsb > m_prevPicOrderCntLsb
        &&
        m_h264Stream->sh->pic_order_cnt_lsb - m_prevPicOrderCntLsb > MaxPicOrderCntLsb / 2
    ) {
        PicOrderCntMsb = m_prevPicOrderCntMsb - MaxPicOrderCntLsb;
    }
    else {
        PicOrderCntMsb = m_prevPicOrderCntMsb;
    }
    std::cout << "PicOrderCntMsb: " << PicOrderCntMsb << std::endl;

    // Save values for next compute
    m_prevPicOrderCntMsb = PicOrderCntMsb;
    m_prevPicOrderCntLsb = m_h264Stream->sh->pic_order_cnt_lsb;

    int32_t TopFieldOrderCnt = PicOrderCntMsb + m_h264Stream->sh->pic_order_cnt_lsb;
    std::cout << "TopFieldOrderCnt: " << TopFieldOrderCnt << std::endl;
    int32_t BottomFieldOrderCnt = TopFieldOrderCnt + m_h264Stream->sh->delta_pic_order_cnt_bottom;
    if (m_h264Stream->sh->field_pic_flag) {
        BottomFieldOrderCnt = PicOrderCntMsb + m_h264Stream->sh->pic_order_cnt_lsb;
    }
    std::cout << "BottomFieldOrderCnt: " << BottomFieldOrderCnt << std::endl;

#else
    // =======================
    // Adapatation of VLC code
    // =======================
    int topFOC = 0;
    int bottomFOC = 0;

    if (m_h264Stream->sps->pic_order_cnt_type == 0) {
        unsigned maxPocLSB = 1U << (m_h264Stream->sps->log2_max_pic_order_cnt_lsb_minus4 + 4);

        /* POC reference */
        if (static_cast<vw::NalType>(m_h264Stream->nal->nal_unit_type) == vw::NalType::CodedSliceIDR) {
            m_h264Infos.poc.prevPicOrderCnt.lsb = 0;
            m_h264Infos.poc.prevPicOrderCnt.msb = 0;
        }
        // Not yet handled
        // else if (m_prevRefPictureHasMMCO5) {
        //     m_h264Infos.poc.prevPicOrderCnt.msb = 0;
        //     if (!m_h264Infos.poc.prevRefPictureIsBottomField) {
        //         m_h264Infos.poc.prevPicOrderCnt.lsb = m_h264Infos.poc.prevRefPictureTFOC;
        //     }
        //     else {
        //         m_h264Infos.poc.prevPicOrderCnt.lsb = 0;
        //     }
        // }

        /* 8.2.1.1 */
        int pocMSB = m_h264Infos.poc.prevPicOrderCnt.msb;
        int64_t orderDiff = m_h264Stream->sh->pic_order_cnt_lsb - m_h264Infos.poc.prevPicOrderCnt.lsb;
        if (orderDiff < 0 && -orderDiff >= maxPocLSB / 2) {
            pocMSB += maxPocLSB;
        }
        else if (orderDiff > maxPocLSB / 2) {
            pocMSB -= maxPocLSB;
        }

        topFOC = bottomFOC = pocMSB + m_h264Stream->sh->pic_order_cnt_lsb;
        if (m_h264Stream->sh->field_pic_flag) {
            bottomFOC += m_h264Stream->sh->delta_pic_order_cnt_bottom;
        }

        /* Save from ref picture */
        if (m_h264Stream->nal->nal_ref_idc /* Is reference */ ) {
            m_h264Infos.poc.prevRefPictureIsBottomField = (m_h264Stream->sh->field_pic_flag &&
                                             m_h264Stream->sh->bottom_field_flag);
            // m_prevRefPictureHasMMCO5 = m_h264Stream->sh->has_mmco5; // not yet handled
            m_h264Infos.poc.prevRefPictureTFOC = topFOC;
            m_h264Infos.poc.prevPicOrderCnt.lsb = m_h264Stream->sh->pic_order_cnt_lsb;
            m_h264Infos.poc.prevPicOrderCnt.msb = pocMSB;
        }
    }
    else
    {
        // throw std::runtime_error("[H264Parser] This order count is not handled");
        // unsigned maxFrameNum = 1 << (m_h264Stream->sps->log2_max_frame_num_minus4 + 4);
        // unsigned frameNumOffset;
        // unsigned expectedPicOrderCnt = 0;

        // if (static_cast<vw::NalType>(m_h264Stream->nal->nal_unit_type) == vw::NalType::CodedSliceIDR) {
        //     frameNumOffset = 0;
        // }
        // else if (m_prevFrameNum > m_h264Stream->sh->frame_num) {
        //     frameNumOffset = m_prevFrameNumOffset + maxFrameNum;
        // }
        // else {
        //     frameNumOffset = m_prevFrameNumOffset;
        // }

        // if (m_h264Stream->sps->pic_order_cnt_type == 1) {
        //     unsigned absFrameNum;

        //     if (m_h264Stream->sps->num_ref_frames_in_pic_order_cnt_cycle > 0) {
        //         absFrameNum = frameNumOffset + m_h264Stream->sh->frame_num;
        //     }
        //     else {
        //         absFrameNum = 0;
        //     }

        //     if (m_h264Stream->nal->nal_ref_idc == 0 && absFrameNum > 0) {
        //         absFrameNum--;
        //     }

        //     if (absFrameNum > 0) {
        //         int32_t expectedDeltaPerPicOrderCntCycle = 0;
        //         for (int i=0; i<m_h264Stream->sps->num_ref_frames_in_pic_order_cnt_cycle; i++) {
        //             expectedDeltaPerPicOrderCntCycle += m_h264Stream->sps->offset_for_ref_frame[i];
        //         }

        //         unsigned picOrderCntCycleCnt = 0;
        //         unsigned frameNumInPicOrderCntCycle = 0;
        //         if (m_h264Stream->sps->num_ref_frames_in_pic_order_cnt_cycle) {
        //             picOrderCntCycleCnt = (absFrameNum - 1) / m_h264Stream->sps->num_ref_frames_in_pic_order_cnt_cycle;
        //             frameNumInPicOrderCntCycle = (absFrameNum - 1) % m_h264Stream->sps->num_ref_frames_in_pic_order_cnt_cycle;
        //         }

        //         expectedPicOrderCnt = picOrderCntCycleCnt * expectedDeltaPerPicOrderCntCycle;
        //         for( unsigned i=0; i <= frameNumInPicOrderCntCycle; i++ ) {
        //             expectedPicOrderCnt = expectedPicOrderCnt + m_h264Stream->sps->offset_for_ref_frame[i];
        //         }
        //     }

        //     if (m_h264Stream->nal->nal_ref_idc == 0) {
        //         expectedPicOrderCnt = expectedPicOrderCnt + m_h264Stream->sps->offset_for_non_ref_pic;
        //     }

        //     *p_tFOC = expectedPicOrderCnt + m_h264Stream->sh->delta_pic_order_cnt;
        //     if (!m_h264Stream->sh->field_pic_flag) {
        //         *p_bFOC = *p_tFOC + m_h264Stream->sps->offset_for_top_to_bottom_field + m_h264Stream->sh->delta_pic_order_cnt_bottom;
        //     }
        //     else if (m_h264Stream->sh->bottom_field_flag) {
        //         *p_bFOC = expectedPicOrderCnt + m_h264Stream->sps->offset_for_top_to_bottom_field + m_h264Stream->sh->delta_pic_order_cnt;
        //     }
        // }
        // else if (m_h264Stream->sps->pic_order_cnt_type == 2) {
        //     unsigned tempPicOrderCnt;

        //     if (static_cast<vw::NalType>(m_h264Stream->nal->nal_unit_type) == vw::NalType::CodedSliceIDR) {
        //         tempPicOrderCnt = 0;
        //     }
        //     else if (m_h264Stream->nal->nal_ref_idc == 0) {
        //         tempPicOrderCnt = 2 * (frameNumOffset + m_h264Stream->sh->frame_num) - 1;
        //     }
        //     else {
        //         tempPicOrderCnt = 2 * (frameNumOffset + m_h264Stream->sh->frame_num);
        //     }

        //     *p_bFOC = *p_tFOC = tempPicOrderCnt;
        // }

        // m_prevFrameNum = m_h264Stream->sh->frame_num;
        // // Not yet handeled
        // // if (p_slice->has_mmco5)
        // //     m_prevFrameNumOffset = 0;
        // // else
        //     m_prevFrameNumOffset = frameNumOffset;
    }

    /* 8.2.1 (8-1) */
    m_h264Infos.poc.iTopFieldOrderCount = topFOC;
    m_h264Infos.poc.iBottomFieldOrderCount = bottomFOC;
    if (!m_h264Stream->sh->field_pic_flag) { /* progressive or contains both fields */
        m_h264Infos.poc.iPictureOrderCount = (bottomFOC < topFOC) ? bottomFOC : topFOC;
    }
    else { /* split top or bottom field */
        if (m_h264Stream->sh->bottom_field_flag) {
            m_h264Infos.poc.iPictureOrderCount = bottomFOC;
        }
        else {
            m_h264Infos.poc.iPictureOrderCount = topFOC;
        }
    }

    // std::cout << "[H264Parser] iTopFieldOrderCount: " << m_h264Infos.poc.iTopFieldOrderCount << std::endl;
    // std::cout << "[H264Parser] iBottomFieldOrderCount: " << m_h264Infos.poc.iBottomFieldOrderCount << std::endl;
    // std::cout << "[H264Parser] iPictureOrderCount: " << m_h264Infos.poc.iPictureOrderCount << std::endl;
    // std::cout << "[H264Parser] frame_num: " << m_h264Stream->sh->frame_num << std::endl;
#endif // 0
}
