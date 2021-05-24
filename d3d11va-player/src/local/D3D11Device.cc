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

#include "D3D11Device.h"

#include <iostream>

namespace dp {
  D3D11Device::D3D11Device()
  : m_device(nullptr)
  , m_deviceContext(nullptr) {
    // Create the D3D11VA device
    UINT deviceFlags = D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
    std::cout << "[D3D11Device] D3D11 debug layer enabled" << std::endl;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0
    };

    HRESULT hRes = D3D11CreateDevice(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, deviceFlags, featureLevels, 2, D3D11_SDK_VERSION, &m_device, nullptr, &m_deviceContext);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Device] Unable to create D3D11 device");
    }
  }

  D3D11Device::~D3D11Device() {
    m_deviceContext->Release();
    m_device->Release();
  }

  ID3D11Device& D3D11Device::getDevice() {
    return *m_device;
  }

  const ID3D11Device& D3D11Device::getDevice() const {
    return *m_device;
  }

  ID3D11DeviceContext& D3D11Device::getDeviceContext() {
    return *m_deviceContext;
  }

  const ID3D11DeviceContext& D3D11Device::getDeviceContext() const {
    return *m_deviceContext;
  }
}