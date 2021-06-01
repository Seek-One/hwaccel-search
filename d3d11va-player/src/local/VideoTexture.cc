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

#include "VideoTexture.h"

#include <stdexcept>

namespace dp {
  VideoTexture::VideoTexture(D3D11Manager& d3d11Manager, D3D11_VIDEO_DECODER_DESC decoderDesc, UINT nbSurface)
  : m_currentSurface(0) {
    auto device = d3d11Manager.getDevice();
    auto videoDevice = d3d11Manager.getVideoDevice();

    // Create video surfaces
    D3D11_TEXTURE2D_DESC textureDesc;
    textureDesc.Width = decoderDesc.SampleWidth;
    textureDesc.Height = decoderDesc.SampleHeight;
    textureDesc.Format = decoderDesc.OutputFormat;
    textureDesc.ArraySize = nbSurface;
    textureDesc.MipLevels = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_DECODER;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;

    HRESULT hRes = device->CreateTexture2D(&textureDesc, nullptr, &m_texture);
    if (FAILED(hRes)) {
      throw std::runtime_error("[VideoTexture] Unable to create ID3D11Texture2D");
    }

    for (UINT i = 0; i < textureDesc.ArraySize; ++i) {
      ComPtr<ID3D11VideoDecoderOutputView> outputView;
      D3D11_VIDEO_DECODER_OUTPUT_VIEW_DESC viewDesc;
      viewDesc.DecodeProfile = decoderDesc.Guid;
      viewDesc.Texture2D.ArraySlice = i;
      viewDesc.ViewDimension = D3D11_VDOV_DIMENSION_TEXTURE2D;

      hRes = videoDevice->CreateVideoDecoderOutputView(
        static_cast<ID3D11Resource*>(m_texture.Get()),
        &viewDesc,
        static_cast<ID3D11VideoDecoderOutputView**>(outputView.GetAddressOf())
      );
      if (FAILED(hRes)) {
        throw std::runtime_error("[VideoTexture] Unable to create ID3D11VideoDecoderOutputView");
      }

      m_outputViews.push_back(outputView);
    }
  }

  ComPtr<ID3D11Texture2D> VideoTexture::getTexture() const {
    return m_texture;
  }

  ComPtr<ID3D11VideoDecoderOutputView> VideoTexture::getCurrentOutputView() const {
    return m_outputViews[m_currentSurface];
  }

  UINT VideoTexture::getCurrentSurfaceIndex() const {
    return m_currentSurface;
  }

  void VideoTexture::nextSurfaceIndex() {
    m_currentSurface = (m_currentSurface + 1) % m_outputViews.size();
  }
}
