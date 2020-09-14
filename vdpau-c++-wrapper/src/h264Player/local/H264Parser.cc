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

#include "H264Parser.h"

#include <cmath>
#include <cstring>
#include <fstream>
#include <iterator>
#include <iostream>
#include <stdexcept>

namespace {
    // Default scaling_lists according to Table 7-2
    constexpr uint8_t default_4x4_intra[16] = {
         6,13,20,28,
        13,20,28,32,
        20,28,32,37,
        28,32,37,42
    };

    constexpr uint8_t default_4x4_inter[16] = {
        10,14,20,24,
        14,20,24,27,
        20,24,27,30,
        24,27,30,34
    };

    constexpr uint8_t default_8x8_intra[64] = {
         6,10,13,16,18,23,25,27,
        10,11,16,18,23,25,27,29,
        13,16,18,23,25,27,29,31,
        16,18,23,25,27,29,31,33,
        18,23,25,27,29,31,33,36,
        23,25,27,29,31,33,36,38,
        25,27,29,31,33,36,38,40,
        27,29,31,33,36,38,40,42
    };

    constexpr uint8_t default_8x8_inter[64] = {
        9,13,15,17,19,21,22,24,
        13,13,17,19,21,22,24,25,
        15,17,19,21,22,24,25,27,
        17,19,21,22,24,25,27,28,
        19,21,22,24,25,27,28,30,
        21,22,24,25,27,28,30,32,
        22,24,25,27,28,30,32,33,
        24,25,27,28,30,32,33,35
    };
}

H264Parser::H264Parser(const std::string& filename)
: m_h264Stream(h264_new())
, m_pDataCursor(nullptr)
, m_unprocessedDataSize(0)
, m_prevPicOrderCntMsb(0)
, m_prevPicOrderCntLsb(0)
, m_prevFrameNumOffset(0)
, m_prevFrameNum(0)
, m_iPrevMMCO(0) {
    // Set the scaling lists to Flat_4x4_16 and Flat_8x8_16
    for (int i = 0; i < 6; ++i) {
        for (int j = 0; j < 16; ++j) {
            m_h264Stream->sps->ScalingList4x4[i][j] = 16;
            m_h264Stream->pps->ScalingList4x4[i][j] = 16;
        }
        for (int j = 0; j < 64; ++j) {
            m_h264Stream->sps->ScalingList8x8[i][j] = 16;
            if (i < 2) {
                m_h264Stream->pps->ScalingList8x8[i][j] = 16;
            }
        }
    }

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
    case vw::NalType::SPS: {
        m_h264Infos.num_ref_frames = m_h264Stream->sps->num_ref_frames;
        m_h264Infos.mb_adaptive_frame_field_flag = m_h264Stream->sps->mb_adaptive_frame_field_flag;
        m_h264Infos.frame_mbs_only_flag = m_h264Stream->sps->frame_mbs_only_flag;
        m_h264Infos.log2_max_frame_num_minus4 = m_h264Stream->sps->log2_max_frame_num_minus4;
        m_h264Infos.pic_order_cnt_type = m_h264Stream->sps->pic_order_cnt_type;
        m_h264Infos.log2_max_pic_order_cnt_lsb_minus4 = m_h264Stream->sps->log2_max_pic_order_cnt_lsb_minus4;
        m_h264Infos.delta_pic_order_always_zero_flag = m_h264Stream->sps->delta_pic_order_always_zero_flag;
        m_h264Infos.direct_8x8_inference_flag = m_h264Stream->sps->direct_8x8_inference_flag;

        // Set fallback rules set A - cf table 7-2 from ref H264
        const uint8_t* fallbackRules[12] = {
            default_4x4_intra,
            m_h264Infos.scaling_lists_4x4[0],
            m_h264Infos.scaling_lists_4x4[1],
            default_4x4_inter,
            m_h264Infos.scaling_lists_4x4[3],
            m_h264Infos.scaling_lists_4x4[4],
            default_8x8_intra,
            default_8x8_inter,
            m_h264Infos.scaling_lists_8x8[0],
            m_h264Infos.scaling_lists_8x8[1],
            m_h264Infos.scaling_lists_8x8[2],
            m_h264Infos.scaling_lists_8x8[3]
        };

        // Set the ScalingLists
        if (m_h264Stream->sps->seq_scaling_matrix_present_flag) {
            for (int i = 0; i < 8; ++i) {
                if (!m_h264Stream->sps->seq_scaling_list_present_flag[i]) {
                    if (i < 6) {
                        std::memcpy(m_h264Infos.scaling_lists_4x4[i], fallbackRules[i], sizeof(uint8_t) * 16);
                    } else {
                        std::memcpy(m_h264Infos.scaling_lists_8x8[i - 6], fallbackRules[i], sizeof(uint8_t) * 64);
                    }
                }
                else {
                    if (i < 6) {
                        for (int j = 0; j < 16; ++j) {
                            m_h264Infos.scaling_lists_4x4[i][j] = m_h264Stream->sps->ScalingList4x4[i][j];
                        }
                    } else {
                        for (int j = 0; j < 64; ++j) {
                            m_h264Infos.scaling_lists_8x8[i - 6][j] = m_h264Stream->sps->ScalingList8x8[i - 6][j];
                        }
                    }
                }
            }
        } else {
            for (int i = 0; i < 8; ++i) {
                if (i < 6) {
                    for (int j = 0; j < 16; ++j) {
                        m_h264Infos.scaling_lists_4x4[i][j] = 16;
                    }
                } else {
                    for (int j = 0; j < 64; ++j) {
                        m_h264Infos.scaling_lists_8x8[i - 6][j] = 16;
                    }
                }
            }
        }

        m_h264Infos.bFirstSPSReceived = true;
        m_h264Infos.iProfile = m_h264Stream->sps->profile_idc;
        computePicutreSize();
        break;
    }

    case vw::NalType::PPS: {
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

        // Set fallback rules set A - cf table 7-2 from ref H264
        const uint8_t* fallbackRules[12] = {
            default_4x4_intra,
            m_h264Infos.scaling_lists_4x4[0],
            m_h264Infos.scaling_lists_4x4[1],
            default_4x4_inter,
            m_h264Infos.scaling_lists_4x4[3],
            m_h264Infos.scaling_lists_4x4[4],
            default_8x8_intra,
            default_8x8_inter,
            m_h264Infos.scaling_lists_8x8[0],
            m_h264Infos.scaling_lists_8x8[1],
            m_h264Infos.scaling_lists_8x8[2],
            m_h264Infos.scaling_lists_8x8[3]
        };

        // Set the ScalingLists
        if (m_h264Stream->pps->pic_scaling_matrix_present_flag) {
            for (int i = 0; i < 8; ++i) {
                if (!m_h264Stream->pps->pic_scaling_list_present_flag[i]) {
                    if (i < 6) {
                        std::memcpy(m_h264Infos.scaling_lists_4x4[i], fallbackRules[i], sizeof(uint8_t) * 16);
                    } else {
                        std::memcpy(m_h264Infos.scaling_lists_8x8[i - 6], fallbackRules[i], sizeof(uint8_t) * 64);
                    }
                }
                else {
                    if (i < 6) {
                        for (int j = 0; j < 16; ++j) {
                            m_h264Infos.scaling_lists_4x4[i][j] = m_h264Stream->pps->ScalingList4x4[i][j];
                        }
                    } else {
                        for (int j = 0; j < 64; ++j) {
                            m_h264Infos.scaling_lists_8x8[i - 6][j] = m_h264Stream->pps->ScalingList8x8[i - 6][j];
                        }
                    }
                }
            }
        }

        m_h264Infos.bFirstPPSReceived = true;
        break;
    }

    case vw::NalType::CodedSliceIDR:
    case vw::NalType::CodedSliceNonIDR: {

        auto sliceType = vw::getSliceType(m_h264Stream->sh->slice_type);
        switch (sliceType) {
        case vw::SliceType::SP:
        case vw::SliceType::SI:
            throw std::runtime_error("[H264Parser] Slice type '" + vw::sliceToString(sliceType) + "' no handled");

        default:
            break;
        }

        if (m_h264Stream->sh->drpm.long_term_reference_flag) {
            throw std::runtime_error("[H264Parser] Long term reference no handled");
        }

        if (m_h264Stream->sh->drpm.adaptive_ref_pic_marking_mode_flag) {
            if (m_h264Stream->sh->drpm.memory_management_control_operation[0] == 4 && m_h264Stream->sh->drpm.max_long_term_frame_idx_plus1[0] == 1) {
                std::cout << "[H264Parser] Delete all long term references but they are not handled so ignore the command" << std::endl;
            } else if (m_h264Stream->sh->drpm.memory_management_control_operation[0] == 5) {
                std::cout << "[H264Parser] Delete all references, no yet implemented" << std::endl;
            }else {
                throw std::runtime_error("[H264Parser] adaptive_ref_pic_marking_mode_flag mode no handled");
            }
        }

        if (m_h264Stream->nal->nal_ref_idc) {
            if (m_h264Stream->sh->field_pic_flag && !m_h264Stream->sh->bottom_field_flag) {
                m_h264Infos.referenceType = vw::PictureReferenceType::TopReference;
            } else if (m_h264Stream->sh->field_pic_flag && m_h264Stream->sh->bottom_field_flag) {
                m_h264Infos.referenceType = vw::PictureReferenceType::BottomReference;
            } else {
                m_h264Infos.referenceType = vw::PictureReferenceType::FrameReference;
            }
        } else {
            m_h264Infos.referenceType = vw::PictureReferenceType::NoReference;
        }

        m_h264Infos.is_reference = (m_h264Stream->nal->nal_ref_idc ? VDP_TRUE : VDP_FALSE);
        m_h264Infos.frame_num = m_h264Stream->sh->frame_num;
        m_h264Infos.field_pic_flag = m_h264Stream->sh->field_pic_flag;
        m_h264Infos.bottom_field_flag = m_h264Stream->sh->bottom_field_flag;
        m_h264Infos.slice_count = 1; // We send always 1 slice
        computePoc();
        break;
    }

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

void H264Parser::computePicutreSize() {
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

    m_h264Infos.pictureSize = vw::SizeU(iWidthCrop, iHeightCrop);

    // std::cout << "[H264Parser] Video size: " << m_h264Infos.videoSize.width << "x" << m_h264Infos.videoSize.height << std::endl;
}

void H264Parser::computePoc() {
    switch (m_h264Infos.pic_order_cnt_type) {
    case 0:
        computePocType0();
        break;

    case 1:
        computePocType1();
        break;

    case 2:
        computePocType2();
        break;

    default:
        throw std::runtime_error("[H264Parser] The picture order count type '" + std::to_string(m_h264Infos.pic_order_cnt_type) + "' is not handled");
    }
}

void H264Parser::computePocType0() {
    // Compute POC in according section 8.2.1.1 of H264 reference

    // IDR case
    int prevPicOrderCntMsb = 0;
    int prevPicOrderCntLsb = 0;

    // No IDR case
    if (m_h264Stream->nal->nal_unit_type != static_cast<int>(vw::NalType::CodedSliceIDR)) {
        if (m_iPrevMMCO == 5) {
            throw std::runtime_error("[H264Parser] Memory management control operation for POC computation not handled");
        } else {
            prevPicOrderCntMsb = m_prevPicOrderCntMsb;
            prevPicOrderCntLsb = m_prevPicOrderCntLsb;
        }
    }

    // h264 reference equation 8-3
    int iPicOrderCntMsb = 0;
    int iMaxPicOrderCntLsb = std::exp2(m_h264Infos.log2_max_pic_order_cnt_lsb_minus4 + 4);
    if ((m_h264Stream->sh->pic_order_cnt_lsb < prevPicOrderCntLsb) &&
        ((prevPicOrderCntLsb - m_h264Stream->sh->pic_order_cnt_lsb) >= (iMaxPicOrderCntLsb / 2)))
    {
        iPicOrderCntMsb = prevPicOrderCntMsb + iMaxPicOrderCntLsb;
    } else if ((m_h264Stream->sh->pic_order_cnt_lsb > prevPicOrderCntLsb) &&
        ((m_h264Stream->sh->pic_order_cnt_lsb - prevPicOrderCntLsb) > (iMaxPicOrderCntLsb / 2))) {
        iPicOrderCntMsb = prevPicOrderCntMsb - iMaxPicOrderCntLsb;
    } else {
        iPicOrderCntMsb = prevPicOrderCntMsb;
    }

    int iTopFieldOrderCnt = iPicOrderCntMsb + m_h264Stream->sh->pic_order_cnt_lsb; // Equation 8-4
    int iBottomFieldOrderCnt = iTopFieldOrderCnt;

    // If it's a frame
    if (!m_h264Infos.field_pic_flag) {
        iBottomFieldOrderCnt += iTopFieldOrderCnt + m_h264Stream->sh->delta_pic_order_cnt_bottom;
    }

    // Update previous reference infos for next computation
    if (m_h264Stream->nal->nal_ref_idc) {
        m_prevPicOrderCntMsb = iPicOrderCntMsb;
        m_prevPicOrderCntLsb = m_h264Stream->sh->pic_order_cnt_lsb;
        m_iPrevMMCO = m_h264Stream->sh->drpm.memory_management_control_operation[0];
    }

    // Store POC informations for the decoder
    m_h264Infos.field_order_cnt[0] = iTopFieldOrderCnt;
    m_h264Infos.field_order_cnt[1] = iBottomFieldOrderCnt;
}

void H264Parser::computePocType1() {
    // Compute POC in according section 8.2.1.2 of H264 reference

    // IDR case
    int prevFrameNumOffset = 0;
    int iFrameNumOffset = 0;

    // No IDR case
    if (m_h264Stream->nal->nal_unit_type != static_cast<int>(vw::NalType::CodedSliceIDR)) {
        // Set prevFrameNumOffset value
        if (m_iPrevMMCO != 5) {
            prevFrameNumOffset = m_prevFrameNumOffset;
        }

        // Set FrameNumOffset value
        int iMaxFrameNum = std::exp2(m_h264Stream->sps->log2_max_frame_num_minus4 + 4);
        iFrameNumOffset = prevFrameNumOffset;
        if (m_prevFrameNum > m_h264Stream->sh->frame_num) {
            iFrameNumOffset += iMaxFrameNum;
        }
    }

    int absFrameNum = 0;
    if (!m_h264Stream->sps->num_ref_frames_in_pic_order_cnt_cycle) {
        absFrameNum = iFrameNumOffset + m_h264Stream->sh->frame_num;
    }

    if (!m_h264Stream->nal->nal_ref_idc && absFrameNum > 0) {
        --absFrameNum;
    }

    int expectedPicOrderCnt = 0;
    if (absFrameNum > 0) {
        int iExpectedDeltaPerPicOrderCntCycle = 0;
        for (int i = 0; i < m_h264Stream->sps->num_ref_frames_in_pic_order_cnt_cycle; ++i) {
            iExpectedDeltaPerPicOrderCntCycle += m_h264Stream->sps->offset_for_ref_frame[i];
        }

        int picOrderCntCycleCnt = (absFrameNum - 1) / m_h264Stream->sps->num_ref_frames_in_pic_order_cnt_cycle;
        int frameNumInPicOrderCntCycle = (absFrameNum - 1) % m_h264Stream->sps->num_ref_frames_in_pic_order_cnt_cycle;

        expectedPicOrderCnt = picOrderCntCycleCnt * iExpectedDeltaPerPicOrderCntCycle;
        for (int i = 0; i <= frameNumInPicOrderCntCycle; ++i) {
            expectedPicOrderCnt += m_h264Stream->sps->offset_for_ref_frame[i];
        }

        if (!m_h264Stream->nal->nal_ref_idc) {
            expectedPicOrderCnt += m_h264Stream->sps->offset_for_non_ref_pic;
        }
    }

    int iTopFieldOrderCnt = expectedPicOrderCnt + m_h264Stream->sh->delta_pic_order_cnt[0];
    int iBottomFieldOrderCnt = iTopFieldOrderCnt + m_h264Stream->sps->offset_for_top_to_bottom_field;
    if (m_h264Stream->sh->bottom_field_flag) {
        iBottomFieldOrderCnt += m_h264Stream->sh->delta_pic_order_cnt[0];
    } else {
        iBottomFieldOrderCnt += m_h264Stream->sh->delta_pic_order_cnt[1];
    }

    // Update previous reference infos for next computation
    if (m_h264Stream->nal->nal_ref_idc) {
        m_prevFrameNumOffset = iFrameNumOffset;
        m_prevFrameNum = m_h264Stream->sh->frame_num;
        m_iPrevMMCO = m_h264Stream->sh->drpm.memory_management_control_operation[0];
    }

    // Store POC informations for the decoder
    m_h264Infos.field_order_cnt[0] = iTopFieldOrderCnt;
    m_h264Infos.field_order_cnt[1] = iBottomFieldOrderCnt;
}

void H264Parser::computePocType2() {
    // Compute POC in according section 8.2.1.3 of H264 reference

    // IDR case
    int prevFrameNumOffset = 0;
    int iFrameNumOffset = 0;
    int tempPicOrderCnt = 0;

    // No IDR case
    if (m_h264Stream->nal->nal_unit_type != static_cast<int>(vw::NalType::CodedSliceIDR)) {
        // Set prevFrameNumOffset value
        if (m_iPrevMMCO != 5) {
            prevFrameNumOffset = m_prevFrameNumOffset;
        }

        // Set FrameNumOffset value
        int iMaxFrameNum = std::exp2(m_h264Stream->sps->log2_max_frame_num_minus4 + 4);
        iFrameNumOffset = prevFrameNumOffset;
        if (m_prevFrameNum > m_h264Stream->sh->frame_num) {
            iFrameNumOffset += iMaxFrameNum;
        }

        // Set tempPicOrderCnt value
        tempPicOrderCnt = 2 * (iFrameNumOffset + m_h264Stream->sh->frame_num);
        if (!m_h264Stream->nal->nal_ref_idc) {
            --tempPicOrderCnt;
        }
    }

    int iTopFieldOrderCnt = tempPicOrderCnt;
    int iBottomFieldOrderCnt = tempPicOrderCnt;

    // Update previous reference infos for next computation
    if (m_h264Stream->nal->nal_ref_idc) {
        m_prevFrameNumOffset = iFrameNumOffset;
        m_iPrevMMCO = m_h264Stream->sh->drpm.memory_management_control_operation[0];
    }

    // Store POC informations for the decoder
    m_h264Infos.field_order_cnt[0] = iTopFieldOrderCnt;
    m_h264Infos.field_order_cnt[1] = iBottomFieldOrderCnt;
}
