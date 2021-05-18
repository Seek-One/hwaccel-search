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
  }

  D3D11Decoder::~D3D11Decoder() {
    m_device->Release();
    m_deviceContext->Release();

    m_videoDevice->Release();
    m_videoContext->Release();
  }
}