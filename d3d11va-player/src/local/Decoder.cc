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

#include "Decoder.h"

#include <fstream>
#include <iostream>
#include <cmath>
#include <stdexcept>

#include "Clock.h"
#include "FileParser.h"

namespace dp {
  Decoder::Decoder(D3D11Manager& d3d11Manager, const SizeI& rawPictureSize)
  : m_d3d11Manager(d3d11Manager)
  , m_videoDecoder(d3d11Manager.createVideoDecoder(rawPictureSize))
  , m_videoTexture(d3d11Manager.createVideoTexture(m_videoDecoder, DecodedBufferLimit))
  , m_currentReportID(1) {

  }

  const VideoTexture& Decoder::decodeSlice(FileParser& parser) {
    Clock decodingClock;
    const auto& nal = *(parser.getStream().nal);

    // Reset the decoded picture buffer when a IDR frame arrives
    if (nal.nal_unit_type == NAL_UNIT_TYPE_CODED_SLICE_IDR) {
      startNewIDRFrame();
    }

    // Get the next output view
    m_videoTexture.nextSurfaceIndex();
    auto outputView = m_videoTexture.getCurrentOutputView();

    // To send a slice to the decoder, we need to fill and send 4 buffers :
    // - D3D11_VIDEO_DECODER_BUFFER_PICTURE_PARAMETERS
    // - D3D11_VIDEO_DECODER_BUFFER_INVERSE_QUANTIZATION_MATRIX
    // - D3D11_VIDEO_DECODER_BUFFER_BITSTREAM
    // - D3D11_VIDEO_DECODER_BUFFER_SLICE_CONTROL
    D3D11_VIDEO_DECODER_BUFFER_DESC listBufferDesc[4];

    // Start the frame (lock directx surface)
    HRESULT hRes = m_d3d11Manager.getVideoContext()->DecoderBeginFrame(m_videoDecoder.Get(), outputView.Get(), 0, nullptr);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Decoder] Unable to begin the frame");
    }

    // Fill and send PicParams
    DXVA_PicParams_H264 picParams;
    fillPictureParams(picParams, parser);
    sendBuffer(D3D11_VIDEO_DECODER_BUFFER_PICTURE_PARAMETERS, &picParams, sizeof(picParams), listBufferDesc[0]);

    // Fill and send ScalingLists
    DXVA_Qmatrix_H264 scalingLists;
    fillScalingLists(scalingLists, parser);
    sendBuffer(D3D11_VIDEO_DECODER_BUFFER_INVERSE_QUANTIZATION_MATRIX, &scalingLists, sizeof(scalingLists), listBufferDesc[1]);

    // Fill and send both slice data and slice control
    std::vector<DXVA_Slice_H264_Short> listSliceControl;
    sendBistreamAndSliceControl(parser.getCurrentNAL(), listBufferDesc[2], listBufferDesc[3], listSliceControl, parser);

    hRes = m_d3d11Manager.getVideoContext()->SubmitDecoderBuffers(m_videoDecoder.Get(), 4, listBufferDesc);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Decoder] Unable to submit decoder buffers");
    }

    // Release the frame to finalize the decode
    hRes = m_d3d11Manager.getVideoContext()->DecoderEndFrame(m_videoDecoder.Get());
    if (FAILED(hRes)) {
      throw std::runtime_error("[Decoder] Unable to end the frame");
    }

    auto elapsedTime = decodingClock.elapsed();
    std::cout << "[Decoder] Slice decoded in " << elapsedTime.count() << "ms" << std::endl;

    // Add the new decoded frame to the DPB
    if (nal.nal_ref_idc > 0) {
      m_dpb.addRefFrame(picParams.CurrPic, picParams.frame_num, picParams.CurrFieldOrderCnt[0], picParams.CurrFieldOrderCnt[1]);
    }

    return m_videoTexture;
  }

  void Decoder::fillPictureParams(DXVA_PicParams_H264& picParams, FileParser& parser) {
    const h264_stream_t& stream = parser.getStream();
    const nal_t& nal = *stream.nal;
    const sps_t& sps = *stream.sps;
    const pps_t& pps = *stream.pps;
    const slice_header_t& slice = *stream.sh;

    std::memset(&picParams, 0, sizeof(DXVA_PicParams_H264));

    // Compute some variables from H264 spec
    int MbaffFrameFlag = (sps.mb_adaptive_frame_field_flag && !slice.field_pic_flag);

    const SizeI& sizeInMbs = parser.getMbPictureSize();
    picParams.wFrameWidthInMbsMinus1 = static_cast<USHORT>(sizeInMbs.width - 1);
    picParams.wFrameHeightInMbsMinus1 = static_cast<USHORT>(sizeInMbs.height - 1);
    picParams.num_ref_frames = static_cast<UCHAR>(sps.num_ref_frames);

    picParams.CurrPic.Index7Bits = static_cast<UCHAR>(m_videoTexture.getCurrentSurfaceIndex());
    picParams.CurrPic.AssociatedFlag = 0; // Allways 0 since we work with progressive picture
    picParams.field_pic_flag = slice.field_pic_flag;
    if (picParams.field_pic_flag) {
      picParams.CurrPic.AssociatedFlag = slice.bottom_field_flag;
    }
    picParams.MbaffFrameFlag = MbaffFrameFlag;
    picParams.residual_colour_transform_flag = 0; // Seems related to the same element in AVC spec but I don't
                                                  // found it. It related to 4:4:4 picture format so it ok to ignore this parameter
    picParams.sp_for_switch_flag = slice.sp_for_switch_flag;
    picParams.chroma_format_idc = sps.chroma_format_idc; // 0 => 4:0:0 Mono-chrome
                                                         // 1 => 4:2:0 sampling
                                                         // 2 => 4:2:2 sampling
                                                         // 3 => 4:4:4 sampling
    picParams.RefPicFlag = (nal.nal_ref_idc > 0);
    picParams.constrained_intra_pred_flag = pps.constrained_intra_pred_flag;
    picParams.weighted_pred_flag = pps.weighted_pred_flag;
    picParams.weighted_bipred_idc = pps.weighted_bipred_idc;
    picParams.MbsConsecutiveFlag = 1; // Hard coded to 1 in ffmpeg
    picParams.frame_mbs_only_flag = sps.frame_mbs_only_flag;
    picParams.transform_8x8_mode_flag = pps.transform_8x8_mode_flag;
    picParams.MinLumaBipredSize8x8Flag = (sps.level_idc >= 31);
    if (nal.nal_unit_type == NAL_UNIT_TYPE_CODED_SLICE_IDR) {
      picParams.IntraPicFlag = 1;
    } else {
      picParams.IntraPicFlag = 0;
    }

    picParams.bit_depth_luma_minus8 = static_cast<UCHAR>(sps.bit_depth_luma_minus8);
    picParams.bit_depth_chroma_minus8 = static_cast<UCHAR>(sps.bit_depth_chroma_minus8);
    picParams.Reserved16Bits = 3; // In accordance of dxva spec (december 2007)
    picParams.StatusReportFeedbackNumber = m_currentReportID;
    ++m_currentReportID;
    if (m_currentReportID == 0) {
      ++m_currentReportID;
    }

    parser.computePoc(picParams.CurrFieldOrderCnt[0], picParams.CurrFieldOrderCnt[1]);

    picParams.pic_init_qp_minus26 = static_cast<CHAR>(pps.pic_init_qp_minus26);
    picParams.chroma_qp_index_offset = static_cast<CHAR>(pps.chroma_qp_index_offset);
    picParams.second_chroma_qp_index_offset = static_cast<CHAR>(pps.second_chroma_qp_index_offset);

    picParams.ContinuationFlag = 1;
    picParams.pic_init_qs_minus26 = static_cast<CHAR>(pps.pic_init_qs_minus26);
    picParams.num_ref_idx_l0_active_minus1 = static_cast<UCHAR>(pps.num_ref_idx_l0_active_minus1);
    picParams.num_ref_idx_l1_active_minus1 = static_cast<UCHAR>(pps.num_ref_idx_l1_active_minus1);
    picParams.frame_num = static_cast<USHORT>(slice.frame_num);
    picParams.log2_max_frame_num_minus4 = static_cast<UCHAR>(sps.log2_max_frame_num_minus4);
    picParams.pic_order_cnt_type = static_cast<UCHAR>(sps.pic_order_cnt_type);
    picParams.log2_max_pic_order_cnt_lsb_minus4 = static_cast<UCHAR>(sps.log2_max_pic_order_cnt_lsb_minus4);
    picParams.delta_pic_order_always_zero_flag = static_cast<UCHAR>(sps.delta_pic_order_always_zero_flag);
    picParams.direct_8x8_inference_flag = static_cast<UCHAR>(sps.direct_8x8_inference_flag);
    picParams.entropy_coding_mode_flag = static_cast<UCHAR>(pps.entropy_coding_mode_flag);
    picParams.pic_order_present_flag = static_cast<UCHAR>(pps.pic_order_present_flag);
    picParams.num_slice_groups_minus1 = static_cast<UCHAR>(pps.num_slice_groups_minus1);
    picParams.slice_group_map_type = static_cast<UCHAR>(pps.slice_group_map_type);
    picParams.deblocking_filter_control_present_flag = static_cast<UCHAR>(pps.deblocking_filter_control_present_flag);
    picParams.redundant_pic_cnt_present_flag = static_cast<UCHAR>(pps.redundant_pic_cnt_present_flag);
    picParams.slice_group_change_rate_minus1 = static_cast<USHORT>(pps.slice_group_change_rate_minus1);
    // picParams.SliceGroupMap?
    picParams.Reserved8BitsA = 0; // 0 means we work with progressive frames
    picParams.Reserved8BitsB = 0;

    picParams.UsedForReferenceFlags  = 0;
    picParams.NonExistingFrameFlags  = 0;

    auto refFrameList = m_dpb.getRefFrameList();

    for (size_t i = 0; i < refFrameList.size(); ++i) {
      const auto& refFrame = refFrameList[i];
      picParams.RefFrameList[i] = refFrame.dxvaEntry;
      picParams.FieldOrderCntList[i][0] = refFrame.TopFieldOrderCnt;
      picParams.FieldOrderCntList[i][1] = refFrame.BottomFieldOrderCnt;
      picParams.FrameNumList[i] = static_cast<USHORT>(refFrame.FrameNum);
      // TODO: For now, we handle only the progressive so the picture is a always a top and
      // bottom reference
      picParams.UsedForReferenceFlags |= 0b11 << (2*i);
    }

    // Set other entry to default values
    for (size_t i = refFrameList.size(); i < 16; ++i) {
      picParams.RefFrameList[i].bPicEntry = 0xff;
      picParams.FieldOrderCntList[i][0] = 0;
      picParams.FieldOrderCntList[i][1] = 0;
      picParams.FrameNumList[i] = 0;
    }
  }

  void Decoder::fillScalingLists(DXVA_Qmatrix_H264& scalingLists, [[maybe_unused]] const FileParser& parser) {
    // const h264_stream_t& stream = parser.getStream();
    // const pps_t& pps = *stream.pps;

    // TODO: handle the scaling lists since h264bitstream doesn't initialize its correctly
    for (int i = 0; i < 6; ++i) {
      for (int j = 0; j < 16; ++j) {
        scalingLists.bScalingLists4x4[i][j] = 16;
      }
    }

    for (int j = 0; j < 64; ++j) {
      scalingLists.bScalingLists8x8[0][j] = 16;
      scalingLists.bScalingLists8x8[1][j] = 16;
    }
  }

  void Decoder::sendBistreamAndSliceControl(
    const std::vector<uint8_t>& bitstream,
    D3D11_VIDEO_DECODER_BUFFER_DESC& bitstreamBufferDesc,
    D3D11_VIDEO_DECODER_BUFFER_DESC& sliceControlBufferDesc,
    std::vector<DXVA_Slice_H264_Short>& listSliceControl,
    const FileParser& parser
  ) {
    HRESULT hRes = 0;
    uint8_t *D3D11VABuffer = nullptr;
    UINT D3D11VABufferSize = 0;

    // Get the decoder buffer
    hRes = m_d3d11Manager.getVideoContext()->GetDecoderBuffer(m_videoDecoder.Get(), D3D11_VIDEO_DECODER_BUFFER_BITSTREAM, &D3D11VABufferSize, (void**)(&D3D11VABuffer));
    if (FAILED(hRes)) {
      throw std::runtime_error("[Decoder] Unable to get a decoder buffer");
    }

    // In fact, a same picture may be splitted into multiple NAL. In this version, we not handle this case and we send
    // only slice by slice
    if (D3D11VABufferSize < bitstream.size()) {
      throw std::runtime_error("[Decoder] the D3D11 VA buffer is too small");
    }

    // Copy bitstream data
    std::memcpy(D3D11VABuffer, bitstream.data(), bitstream.size());

    // Padding to align to 128 bits
    ptrdiff_t paddingLength = std::min(static_cast<int>(128 - (bitstream.size() & 127)), static_cast<int>((D3D11VABuffer + D3D11VABufferSize) - (D3D11VABuffer + bitstream.size())));
    std::memset(D3D11VABuffer + bitstream.size(), 0, paddingLength);

    // Release the bitstream buffer
    hRes = m_d3d11Manager.getVideoContext()->ReleaseDecoderBuffer(m_videoDecoder.Get(), D3D11_VIDEO_DECODER_BUFFER_BITSTREAM);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Decoder] Unable to release a decoder buffer");
    }

    // Count the total of macroblock
    SizeI sizeInMB = parser.getMbPictureSize();
    UINT totalMB = sizeInMB.width * sizeInMB.height;

    // Update buffer bitstream description
    memset(&bitstreamBufferDesc, 0, sizeof(D3D11_VIDEO_DECODER_BUFFER_DESC));
    bitstreamBufferDesc.BufferType = D3D11_VIDEO_DECODER_BUFFER_BITSTREAM;
    bitstreamBufferDesc.DataSize = static_cast<UINT>(bitstream.size());
    bitstreamBufferDesc.NumMBsInBuffer = totalMB;

    // Add the entries to slice control
    DXVA_Slice_H264_Short sliceControl;
    sliceControl.BSNALunitDataLocation = 1;
    sliceControl.SliceBytesInBuffer = static_cast<UINT>(bitstream.size());
    sliceControl.wBadSliceChopping = 0;
    listSliceControl.push_back(sliceControl);

    // Send slice control buffer
    sendBuffer(
      D3D11_VIDEO_DECODER_BUFFER_SLICE_CONTROL,
      &(listSliceControl[0]),
      static_cast<int>(listSliceControl.size() * sizeof(DXVA_Slice_H264_Short)),
      sliceControlBufferDesc,
      totalMB
    );
  }

  void Decoder::startNewIDRFrame() {
    m_dpb.clear();
  }
}
