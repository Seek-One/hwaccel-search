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

#ifndef LOCAL_D3D11_DEVICE_H_
#define LOCAL_D3D11_DEVICE_H_

#define NOMINMAX
#include <WinSDKVer.h>
#include <windows.h>

#include <d3d11.h>

namespace dp {
  class D3D11Device {
  public:
    D3D11Device();
    ~D3D11Device();

    D3D11Device(const D3D11Device&) = delete;
    D3D11Device(D3D11Device&&) = delete;

    D3D11Device& operator=(const D3D11Device&) = delete;
    D3D11Device& operator=(D3D11Device&&) = delete;

    ID3D11Device& getDevice();
    const ID3D11Device& getDevice() const;

    ID3D11DeviceContext& getDeviceContext();
    const ID3D11DeviceContext& getDeviceContext() const;

  private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_deviceContext;
  };
}

#endif // LOCAL_D3D11_DEVICE_H_