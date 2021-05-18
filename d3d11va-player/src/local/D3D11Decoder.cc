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

#include <iostream>
#include <stdexcept>

// For some unkown reasons, the D3D11_DECODER_PROFILE_H264_VLD_NOFGT its not found,
// so I need to redeclare the constant and add initguid.h header file
#include <initguid.h>
DEFINE_GUID(D3D11_DECODER_PROFILE_H264_VLD_NOFGT,    0x1b81be68, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);

namespace {
  constexpr int alignedSize(int size, int nbPixels) {
    return (size + nbPixels - 1) & ~(nbPixels - 1);
  }
}

namespace dp {
  D3D11Decoder::D3D11Decoder(const SizeI rawPictureSize) {
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

    hRes = m_device->CreateTexture2D(&textureDesc, NULL, &m_texture);
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
}