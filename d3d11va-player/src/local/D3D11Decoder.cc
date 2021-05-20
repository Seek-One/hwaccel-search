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

#ifdef _MSC_VER
#pragma once
#pragma comment(lib, "d3d11")
#endif // _MSC_VER

#include "D3D11Decoder.h"

#include <fstream>
#include <iostream>
#include <cmath>
#include <stdexcept>

// For some unkown reasons, the D3D11_DECODER_PROFILE_H264_VLD_NOFGT its not found,
// so I need to redeclare the constant and add initguid.h header file
#include <initguid.h>
DEFINE_GUID(D3D11_DECODER_PROFILE_H264_VLD_NOFGT,    0x1b81be68, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);

#include "FileParser.h"

namespace {
  constexpr int alignedSize(int size, int nbPixels) {
    return (size + nbPixels - 1) & ~(nbPixels - 1);
  }
}

namespace dp {
  D3D11Decoder::D3D11Decoder(const SizeI rawPictureSize)
  : m_device(nullptr)
  , m_deviceContext(nullptr)
  , m_videoDevice(nullptr)
  , m_videoContext(nullptr)
  , m_videoDecoder(nullptr)
  , m_texture(nullptr)
  , m_currentReportID(1) {
    // Create the D3D11VA device
    UINT deviceFlags = D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
    std::cout << "[D3D11Decoder] D3D11 debug layer enabled" << std::endl;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0
    };

    HRESULT hRes = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, featureLevels, 2, D3D11_SDK_VERSION, &m_device, nullptr, &m_deviceContext);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to create D3D11 device");
    }

    // Get the D3D11VA video device
    hRes = m_device->QueryInterface(&m_videoDevice);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to get ID3D11VideoDevice");
    }

    // Get the D3D11VA video context
    hRes = m_deviceContext->QueryInterface(&m_videoContext);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to get ID3D11VideoContext");
    }

    // Try to get a H264 decoder profile
    const GUID h264DecoderProfile = D3D11_DECODER_PROFILE_H264_VLD_NOFGT;

    bool profileFound = false;
    UINT profileCount = m_videoDevice->GetVideoDecoderProfileCount();
    for (UINT i = 0; i < profileCount; ++i) {
      GUID selectedProfileGUID;
      hRes = m_videoDevice->GetVideoDecoderProfile(i, &selectedProfileGUID);
      if (FAILED(hRes)) {
        throw std::runtime_error("[D3D11Decoder] Invalid profile selected");
      }

      if (h264DecoderProfile == selectedProfileGUID) {
        profileFound = true;
        break;
      }
    }

    if (!profileFound) {
      throw std::runtime_error("[D3D11Decoder] No hardware profile found");
    }

    // Check support for NV12
    BOOL supportedFormat;
    hRes = m_videoDevice->CheckVideoDecoderFormat(&h264DecoderProfile, DXGI_FORMAT_NV12, &supportedFormat);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unsupported output format");
    }

    // Align size to 16 bytes
    SizeI alignedPictureSize;
    alignedPictureSize.width = alignedSize(rawPictureSize.width, 16);
    alignedPictureSize.height = alignedSize(rawPictureSize.height, 16);

    // Select decoder configuration
    UINT iDecoderConfigCount = 0;

    D3D11_VIDEO_DECODER_DESC decoderDesc;
    decoderDesc.Guid = h264DecoderProfile;
    decoderDesc.SampleWidth = alignedPictureSize.width;
    decoderDesc.SampleHeight = alignedPictureSize.height;
    decoderDesc.OutputFormat = DXGI_FORMAT_NV12;

    hRes = m_videoDevice->GetVideoDecoderConfigCount(&decoderDesc, &iDecoderConfigCount);
    if (FAILED(hRes) || iDecoderConfigCount == 0) {
      throw std::runtime_error("[D3D11Decoder] Unable to get the decoder configurations");
    }

    // Select a decoder profile with ConfigBitstreamRaw != 0
    // TODO: we need to prefer no crypt buffer and ConfigBitstreamRaw == 2
    D3D11_VIDEO_DECODER_CONFIG decoderConfig;
    bool configFound = false;
    for (UINT i = 0; i < iDecoderConfigCount; ++i) {
      hRes = m_videoDevice->GetVideoDecoderConfig(&decoderDesc, i, &decoderConfig);
      if (FAILED(hRes)) {
        throw std::runtime_error("[D3D11Decoder] Invalid configuration index provided");
      }

      if (/*conf.ConfigBitstreamRaw == 1 || */decoderConfig.ConfigBitstreamRaw == 2) {
        configFound = true;
        break;
      }
    }

    hRes = m_videoDevice->CreateVideoDecoder(&decoderDesc, &decoderConfig, &m_videoDecoder);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to create ID3D11VideoDecoder");
    }

    // Create output surfaces
    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = decoderDesc.SampleWidth;
    textureDesc.Height = decoderDesc.SampleHeight;
    textureDesc.Format = DXGI_FORMAT_NV12;
    textureDesc.ArraySize = 25; // TODO: which value?
    textureDesc.MipLevels = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DECODER;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    hRes = m_device->CreateTexture2D(&textureDesc, nullptr, &m_texture);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to create ID3D11Texture2D");
    }

    for (UINT i = 0; i < textureDesc.ArraySize; ++i) {
      ID3D11VideoDecoderOutputView* outputView = nullptr;
      D3D11_VIDEO_DECODER_OUTPUT_VIEW_DESC viewDesc;
      viewDesc.DecodeProfile = h264DecoderProfile;
      viewDesc.Texture2D.ArraySlice = i;
      viewDesc.ViewDimension = D3D11_VDOV_DIMENSION_TEXTURE2D;

      hRes = m_videoDevice->CreateVideoDecoderOutputView(
        static_cast<ID3D11Resource*>(m_texture),
        &viewDesc,
        static_cast<ID3D11VideoDecoderOutputView**>(&outputView)
      );
      if (FAILED(hRes)) {
        throw std::runtime_error("[D3D11Decoder] Unable to create ID3D11VideoDecoderOutputView");
      }

      m_outputViews.push_back(outputView);
      m_texture->AddRef();
    }
  }

  D3D11Decoder::~D3D11Decoder() {
    for (auto outputView: m_outputViews) {
      outputView->Release();
      m_texture->Release();
    }
    m_texture->Release();

    m_videoDecoder->Release();

    m_videoContext->Release();
    m_videoDevice->Release();

    m_deviceContext->Release();
    m_device->Release();
  }

  void D3D11Decoder::decodeSlice(FileParser& parser) {
    // To send a slice to the decoder, we need to fill and send 4 buffers :
    // - D3D11_VIDEO_DECODER_BUFFER_PICTURE_PARAMETERS
    // - D3D11_VIDEO_DECODER_BUFFER_INVERSE_QUANTIZATION_MATRIX
    // - D3D11_VIDEO_DECODER_BUFFER_BITSTREAM
    // - D3D11_VIDEO_DECODER_BUFFER_SLICE_CONTROL

    D3D11_VIDEO_DECODER_BUFFER_DESC listBufferDesc[4];
    int surfaceIndex = 0; // TODO: handle the surface index computation

    // Start the frame (lock directx surface)
    HRESULT hRes = m_videoContext->DecoderBeginFrame(m_videoDecoder, m_outputViews[surfaceIndex], 0, nullptr);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to begin the frame");
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

    hRes = m_videoContext->SubmitDecoderBuffers(m_videoDecoder, 4, listBufferDesc);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to submit decoder buffers");
    }

    // Release the frame to finalize the decode
    hRes = m_videoContext->DecoderEndFrame(m_videoDecoder);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to end the frame");
    }

    // Create a temporary texture -- move to member variable?
    D3D11_TEXTURE2D_DESC gpuTextureDesc;
    m_texture->GetDesc(&gpuTextureDesc);

    D3D11_TEXTURE2D_DESC cpuTextureDesc;
    cpuTextureDesc.Width = gpuTextureDesc.Width;
    cpuTextureDesc.Height = gpuTextureDesc.Height;
    cpuTextureDesc.MipLevels = gpuTextureDesc.MipLevels;
    cpuTextureDesc.ArraySize = 1; // We copy only the current texture
    cpuTextureDesc.Format = gpuTextureDesc.Format;
    cpuTextureDesc.SampleDesc = gpuTextureDesc.SampleDesc;
    cpuTextureDesc.Usage = D3D11_USAGE_STAGING;
    cpuTextureDesc.BindFlags = 0;
    cpuTextureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    cpuTextureDesc.MiscFlags = 0;

    ID3D11Texture2D* cpuTexture = nullptr;
    hRes = m_device->CreateTexture2D(&cpuTextureDesc, nullptr, &cpuTexture);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to create CPU ID3D11Texture2D");
    }

    // copy the texture to a staging resource
    m_deviceContext->CopySubresourceRegion(
      cpuTexture,
      0,
      0,
      0,
      0,
      m_texture,
      0,
      nullptr
    );

    // Map the staging resource
    D3D11_MAPPED_SUBRESOURCE mapInfo;
    hRes = m_deviceContext->Map(
      cpuTexture,
      0,
      D3D11_MAP_READ,
      0,
      &mapInfo
    );
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to map cpu texture");
    }

    uint8_t* yuvData = static_cast<uint8_t*>(mapInfo.pData);

    auto file = std::fstream("dump.yuv", std::ios::out | std::ios::app | std::ios::binary);

    // Copy luma
    SizeI pictureSize = parser.getRealPictureSize();
    for (int h = 0; h < pictureSize.height; ++h) {
      assert(yuvData < ((static_cast<uint8_t*>(mapInfo.pData) + mapInfo.DepthPitch) - mapInfo.RowPitch));
      file.write(reinterpret_cast<const char*>(yuvData), pictureSize.width);
      yuvData += mapInfo.RowPitch;
    }

    // Skip cropped rows
    SizeI rawPictureSize = parser.getRawPictureSize();
    int skipCount = rawPictureSize.height - pictureSize.height;
    yuvData += mapInfo.RowPitch * skipCount;

    // Copy color
    for (int h = 0; h < (pictureSize.height / 2); ++h) {
      assert(yuvData < (((uint8_t*)mapInfo.pData + mapInfo.DepthPitch) - mapInfo.RowPitch));
      file.write((char*)yuvData, pictureSize.width);
      yuvData += mapInfo.RowPitch;
    }

    file.close();

    std::cout << "[D3D11Decoder] Dump YUV" << std::endl;
  }

  void D3D11Decoder::fillPictureParams(DXVA_PicParams_H264& picParams, FileParser& parser) {
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

    picParams.CurrPic.bPicEntry = 0; // TODO: iterate over surfaces
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
    for (int i = 0/*, j = 0*/; i < 16; i++) {
      // TODO: get last ref frame
      // if (r) {
      //   fill_picture_entry(&picParams.RefFrameList[i],
      //           ff_dxva2_get_surface_index(avctx, ctx, r->f),
      //           r->long_ref != 0);

      //   if ((r->reference & PICT_TOP_FIELD) && r->field_poc[0] != INT_MAX)
      //     picParams.FieldOrderCntList[i][0] = r->field_poc[0];
      //   if ((r->reference & PICT_BOTTOM_FIELD) && r->field_poc[1] != INT_MAX)
      //     picParams.FieldOrderCntList[i][1] = r->field_poc[1];

      //   picParams.FrameNumList[i] = r->long_ref ? r->pic_id : r->frame_num;
      //   if (r->reference & PICT_TOP_FIELD)
      //     picParams.UsedForReferenceFlags |= 1 << (2*i + 0);
      //   if (r->reference & PICT_BOTTOM_FIELD)
      //     picParams.UsedForReferenceFlags |= 1 << (2*i + 1);
      // } else {
        picParams.RefFrameList[i].bPicEntry = 0xff;
        picParams.FieldOrderCntList[i][0]   = 0;
        picParams.FieldOrderCntList[i][1]   = 0;
        picParams.FrameNumList[i]           = 0;
      // }
    }
  }

  void D3D11Decoder::fillScalingLists(DXVA_Qmatrix_H264& scalingLists, [[maybe_unused]] const FileParser& parser) {
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

  void D3D11Decoder::sendBistreamAndSliceControl(
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
    hRes = m_videoContext->GetDecoderBuffer(m_videoDecoder, D3D11_VIDEO_DECODER_BUFFER_BITSTREAM, &D3D11VABufferSize, (void**)(&D3D11VABuffer));
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to get a decoder buffer");
    }

    // In fact, a same picture may be splitted into multiple NAL. In this version, we not handle this case and we send
    // only slice by slice
    if (D3D11VABufferSize < bitstream.size()) {
      throw std::runtime_error("[D3D11Decoder] the D3D11 VA buffer is too small");
    }

    // Copy bitstream data
    std::memcpy(D3D11VABuffer, bitstream.data(), bitstream.size());

    // Padding to align to 128 bits
    ptrdiff_t paddingLength = std::min(static_cast<int>(128 - (bitstream.size() & 127)), static_cast<int>((D3D11VABuffer + D3D11VABufferSize) - (D3D11VABuffer + bitstream.size())));
    std::memset(D3D11VABuffer + bitstream.size(), 0, paddingLength);

    // Release the bitstream buffer
    hRes = m_videoContext->ReleaseDecoderBuffer(m_videoDecoder, D3D11_VIDEO_DECODER_BUFFER_BITSTREAM);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Decoder] Unable to release a decoder buffer");
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
}