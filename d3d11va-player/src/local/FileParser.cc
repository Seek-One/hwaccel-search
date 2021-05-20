/* Copyright (c) 2021 Jet1oeil
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

#include "FileParser.h"

#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

namespace dp {
  FileParser::FileParser(const std::filesystem::path& bitstreamPath)
  : m_h264Stream(h264_new())
  , m_sizeComputed(false)
  , m_mbPictureSize({ 0, 0 })
  , m_rawPictureSize({ 0, 0 })
  , m_realPictureSize({ 0, 0 })
  , m_prevPicOrderCntMsb(0)
  , m_prevPicOrderCntLsb(0)
  , m_prevFrameNumOffset(0)
  , m_prevFrameNum(0)
  , m_prevMMCO(0) {
    if (m_h264Stream == nullptr) {
      throw std::runtime_error("[FileParser] Unable to create h264 parser");
    }

    std::ifstream bitstreamFile(bitstreamPath, std::ios::in | std::ios::binary);
    if (!bitstreamFile.good()) {
      throw std::runtime_error("[FileParser] Unable to open bitstream file '" + bitstreamPath.string() + "'");
    }

    bitstreamFile.unsetf(std::ios::skipws);

    // Load all data
    m_bitstreamData.insert(m_bitstreamData.begin(), std::istream_iterator<uint8_t>(bitstreamFile), std::istream_iterator<uint8_t>());
    m_dataCursor = m_bitstreamData.data();
    m_unprocessedDataSize = static_cast<int>(m_bitstreamData.size());
  }

  FileParser::~FileParser() {
    h264_free(m_h264Stream);
  }

  void FileParser::extractPictureSizes() {
    int nalStart = 0;
    int nalEnd = 0;

    uint8_t* data = m_bitstreamData.data();
    int dataLength = static_cast<int>(m_bitstreamData.size());

    // Seek the first SPS and IDR slice
    bool spsFound = false;
    bool idrFound = false;
    do {
      if (find_nal_unit(data, dataLength, &nalStart, &nalEnd) <= 0) {
        throw std::runtime_error("[FileParser] End of bitstream reached without parsing a SPS NAL");
      }

      data += nalStart;
      read_nal_unit(m_h264Stream, data, nalEnd - nalStart);

      if (m_h264Stream->nal->nal_unit_type != NAL_UNIT_TYPE_SPS) {
        spsFound = true;
      } else if (m_h264Stream->nal->nal_unit_type != NAL_UNIT_TYPE_CODED_SLICE_IDR) {
        idrFound = true;
      }
    } while (!spsFound && !idrFound);

    // Compute size
    computeSizes();
  }

  const SizeI& FileParser::getMbPictureSize() const {
    if (!m_sizeComputed) {
      throw std::runtime_error("[FileParser] Size are not computed. Call extractPictureSize() first");
    }

    return m_mbPictureSize;
  }

  const SizeI& FileParser::getRawPictureSize() const {
    if (!m_sizeComputed) {
      throw std::runtime_error("[FileParser] Size are not computed. Call extractPictureSize() first");
    }

    return m_rawPictureSize;
  }

  const SizeI& FileParser::getRealPictureSize() const {
    if (!m_sizeComputed) {
      throw std::runtime_error("[FileParser] Size are not computed. Call extractPictureSize() first");
    }

    return m_realPictureSize;
  }


  bool FileParser::parseNextNAL() {
    int nalStart = 0;
    int nalEnd = 0;

    if (find_nal_unit(m_dataCursor, m_unprocessedDataSize, &nalStart, &nalEnd) <= 0) {
      return false;
    }

    // Keep the nal with the start code
    m_currentNAL = std::vector<uint8_t>(m_dataCursor, m_dataCursor + nalEnd);

    // Process the NAL unit and update the h264 context
    m_dataCursor += nalStart;
    read_nal_unit(m_h264Stream, m_dataCursor, nalEnd - nalStart);

    // Forward to next NAL
    m_dataCursor += (nalEnd - nalStart) + 1;
    m_unprocessedDataSize -= nalEnd;

    return true;
  }

  const h264_stream_t& FileParser::getStream() const {
    return *m_h264Stream;
  }

  const std::vector<uint8_t>& FileParser::getCurrentNAL() const {
    return m_currentNAL;
  }

  void FileParser::computePoc(int& TopFieldOrderCnt, int& BottomFieldOrderCnt) {
    switch (m_h264Stream->sps->pic_order_cnt_type) {
      case 0:
        computePocType0(TopFieldOrderCnt, BottomFieldOrderCnt);
        break;

      case 1:
        computePocType1(TopFieldOrderCnt, BottomFieldOrderCnt);
        break;

      case 2:
        computePocType2(TopFieldOrderCnt, BottomFieldOrderCnt);
        break;

      default:
        throw std::runtime_error("[FileParser] The picture order count type '" + std::to_string(m_h264Stream->sps->pic_order_cnt_type) + "' is not handled");
    }
  }

  void FileParser::computeSizes() {
    // All of equation reference are taken form Rec. ITU-T H.264 (06/2019)
    // Compute luma width
    uint32_t PicWidthInMbs = m_h264Stream->sps->pic_width_in_mbs_minus1 + 1; // (7-13)

    // Compute luma height
    uint32_t PicHeightInMapUnits = m_h264Stream->sps->pic_height_in_map_units_minus1 + 1; // (7-16)
    uint32_t FrameHeightInMbs = (2 - m_h264Stream->sps->frame_mbs_only_flag) * PicHeightInMapUnits; // (7-18)
    uint32_t PicHeightInMbs = FrameHeightInMbs / (1 + m_h264Stream->sh->field_pic_flag); // (7-26)

    m_mbPictureSize = SizeI(PicWidthInMbs, PicHeightInMbs);

    uint32_t PicWidthInSamplesL = PicWidthInMbs * 16; // (7-14)
    uint32_t PicHeightInSamplesL = PicHeightInMbs * 16; // (7-27)

    // Uncropped size
    m_rawPictureSize = SizeI(PicWidthInSamplesL, PicHeightInSamplesL);

    // If the image was cropped
    if (m_h264Stream->sps->frame_cropping_flag) {
      // Computing ChromaArrayType (separate_colour_plane_flag page 74)
      uint8_t ChromaArrayType = 0;
      if (!m_h264Stream->sps->residual_colour_transform_flag) {
        ChromaArrayType = static_cast<uint8_t>(m_h264Stream->sps->chroma_format_idc);
      }

      uint8_t CropUnitX = 1; // (7-19)
      uint8_t CropUnitY = static_cast<uint8_t>(2 - m_h264Stream->sps->frame_mbs_only_flag); // (7-20)

      if (ChromaArrayType >= 1 && ChromaArrayType <= 3) {
        CropUnitX = static_cast<uint8_t>(computeSubWidthC()); // (7-21)
        CropUnitY = static_cast<uint8_t>(computeSubHeightC() * (2 - m_h264Stream->sps->frame_mbs_only_flag)); // (7-22)
      }

      int width = (PicWidthInSamplesL - (CropUnitX * m_h264Stream->sps->frame_crop_right_offset + 1)) - (CropUnitX * m_h264Stream->sps->frame_crop_left_offset) + 1;
      int height = ((16 * PicHeightInMbs) - (CropUnitY * m_h264Stream->sps->frame_crop_bottom_offset + 1)) - (CropUnitY * m_h264Stream->sps->frame_crop_top_offset) + 1;

      m_realPictureSize = SizeI(width, height);
    }

    m_sizeComputed = true;
  }

  int FileParser::computeSubWidthC() {
    if (m_h264Stream->sps->residual_colour_transform_flag != 0 || m_h264Stream->sps->chroma_format_idc == 0 || m_h264Stream->sps->chroma_format_idc == 3) {
      throw std::runtime_error("[FileParser] Unexpected bitstream values");
    }

    if (m_h264Stream->sps->chroma_format_idc == 1 || m_h264Stream->sps->chroma_format_idc == 2) {
      return 2;
    }

    return 1;
  }

  int FileParser::computeSubHeightC() {
    if (m_h264Stream->sps->residual_colour_transform_flag != 0 || m_h264Stream->sps->chroma_format_idc == 0 || m_h264Stream->sps->chroma_format_idc == 3) {
      throw std::runtime_error("[FileParser] Unexpected bitstream values");
    }

    if (m_h264Stream->sps->chroma_format_idc == 1) {
      return 2;
    }

    return 1;
  }

  void FileParser::computePocType0(int& TopFieldOrderCnt, int& BottomFieldOrderCnt) {
    // Compute POC in according section 8.2.1.1 of H264 reference

    // IDR case
    int prevPicOrderCntMsb = 0;
    int prevPicOrderCntLsb = 0;

    // No IDR case
    if (m_h264Stream->nal->nal_unit_type != NAL_UNIT_TYPE_CODED_SLICE_IDR) {
      if (m_prevMMCO == 5) {
        throw std::runtime_error("[FileParser] Memory management control operation for POC computation not handled");
      } else {
        prevPicOrderCntMsb = m_prevPicOrderCntMsb;
        prevPicOrderCntLsb = m_prevPicOrderCntLsb;
      }
    }

    // h264 reference equation 8-3
    int iPicOrderCntMsb = 0;
    int iMaxPicOrderCntLsb = static_cast<int>(std::exp2(m_h264Stream->sps->log2_max_pic_order_cnt_lsb_minus4 + 4));
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

    TopFieldOrderCnt = iPicOrderCntMsb + m_h264Stream->sh->pic_order_cnt_lsb; // Equation 8-4
    BottomFieldOrderCnt = TopFieldOrderCnt;

    // If it's a frame
    if (!m_h264Stream->sh->field_pic_flag) {
      BottomFieldOrderCnt += TopFieldOrderCnt + m_h264Stream->sh->delta_pic_order_cnt_bottom;
    }

    // Update previous reference infos for next computation
    if (m_h264Stream->nal->nal_ref_idc) {
      m_prevPicOrderCntMsb = iPicOrderCntMsb;
      m_prevPicOrderCntLsb = m_h264Stream->sh->pic_order_cnt_lsb;
      m_prevMMCO = m_h264Stream->sh->drpm.memory_management_control_operation[0];
    }
  }

  void FileParser::computePocType1(int& TopFieldOrderCnt, int& BottomFieldOrderCnt) {
    // Compute POC in according section 8.2.1.2 of H264 reference

    // IDR case
    int prevFrameNumOffset = 0;
    int iFrameNumOffset = 0;

    // No IDR case
    if (m_h264Stream->nal->nal_unit_type != NAL_UNIT_TYPE_CODED_SLICE_IDR) {
      // Set prevFrameNumOffset value
      if (m_prevMMCO != 5) {
        prevFrameNumOffset = m_prevFrameNumOffset;
      }

      // Set FrameNumOffset value
      int iMaxFrameNum = static_cast<int>(std::exp2(m_h264Stream->sps->log2_max_frame_num_minus4 + 4));
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

    TopFieldOrderCnt = expectedPicOrderCnt + m_h264Stream->sh->delta_pic_order_cnt[0];
    BottomFieldOrderCnt = TopFieldOrderCnt + m_h264Stream->sps->offset_for_top_to_bottom_field;
    if (m_h264Stream->sh->bottom_field_flag) {
      BottomFieldOrderCnt += m_h264Stream->sh->delta_pic_order_cnt[0];
    } else {
      BottomFieldOrderCnt += m_h264Stream->sh->delta_pic_order_cnt[1];
    }

    // Update previous reference infos for next computation
    if (m_h264Stream->nal->nal_ref_idc) {
      m_prevFrameNumOffset = iFrameNumOffset;
      m_prevFrameNum = m_h264Stream->sh->frame_num;
      m_prevMMCO = m_h264Stream->sh->drpm.memory_management_control_operation[0];
    }
  }

  void FileParser::computePocType2(int& TopFieldOrderCnt, int& BottomFieldOrderCnt) {
    // Compute POC in according section 8.2.1.3 of H264 reference

    // IDR case
    int prevFrameNumOffset = 0;
    int iFrameNumOffset = 0;
    int tempPicOrderCnt = 0;

    // No IDR case
    if (m_h264Stream->nal->nal_unit_type != NAL_UNIT_TYPE_CODED_SLICE_IDR) {
      // Set prevFrameNumOffset value
      if (m_prevMMCO != 5) {
        prevFrameNumOffset = m_prevFrameNumOffset;
      }

      // Set FrameNumOffset value
      int iMaxFrameNum = static_cast<int>(std::exp2(m_h264Stream->sps->log2_max_frame_num_minus4 + 4));
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

    TopFieldOrderCnt = tempPicOrderCnt;
    BottomFieldOrderCnt = tempPicOrderCnt;

    // Update previous reference infos for next computation
    if (m_h264Stream->nal->nal_ref_idc) {
      m_prevFrameNumOffset = iFrameNumOffset;
      m_prevMMCO = m_h264Stream->sh->drpm.memory_management_control_operation[0];
    }
  }
}