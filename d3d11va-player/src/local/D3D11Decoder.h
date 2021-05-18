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

#ifndef LOCAL_D3D11_DECODER_H
#define LOCAL_D3D11_DECODER_H

#include <windows.h>
#include <WinSDKVer.h>

#include <d3d11.h>

#include <vector>

#include "Size.h"

namespace dp {
  class D3D11Decoder {
  public:
    D3D11Decoder(const SizeI rawPictureSize);
    ~D3D11Decoder();

    D3D11Decoder(const D3D11Decoder&) = delete;
    D3D11Decoder(D3D11Decoder&&) = delete;

    D3D11Decoder& operator=(const D3D11Decoder&) = delete;
    D3D11Decoder& operator=(D3D11Decoder&&) = delete;

  private:
    ID3D11Device* m_device;
    ID3D11DeviceContext* m_deviceContext;
    ID3D11VideoDevice* m_videoDevice;
    ID3D11VideoContext* m_videoContext;
    ID3D11VideoDecoder* m_videoDecoder;
    ID3D11Texture2D* m_texture;
    std::vector<ID3D11VideoDecoderOutputView*> m_outputViews;
  };
}

#endif // LOCAL_D3D11_DECODER_H