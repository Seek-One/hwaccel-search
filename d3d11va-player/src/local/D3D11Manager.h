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

#ifndef LOCAL_D3D11_MANAGER_H
#define LOCAL_D3D11_MANAGER_H

#include "WindowsHeaders.h"

#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>

#include "Size.h"

namespace dp {
  class VideoTexture;

  // Simple alias for ComPtr
  template<typename T>
  struct ComPtr : public Microsoft::WRL::ComPtr<T> {};

  class D3D11Manager {
  public:
    D3D11Manager() = default;
    ~D3D11Manager() = default;

    D3D11Manager(const D3D11Manager&) = delete;
    D3D11Manager(D3D11Manager&&) = delete;

    D3D11Manager& operator=(const D3D11Manager&) = delete;
    D3D11Manager& operator=(D3D11Manager&&) = delete;

    ComPtr<ID3D11Device> getDevice();
    ComPtr<ID3D11DeviceContext> getDeviceContext();
    ComPtr<ID3D11VideoDevice> getVideoDevice();
    ComPtr<ID3D11VideoContext> getVideoContext();

    ComPtr<ID3D11VideoDecoder> createVideoDecoder(const SizeI& rawPictureSize);

    VideoTexture createVideoTexture(ComPtr<ID3D11VideoDecoder> decoder, UINT nbSurface);
    ComPtr<ID3D11Texture2D> createOutputTexture(SizeI backBufferSize);

    ComPtr<IDXGISwapChain1> createSwapChain(HWND hWnd);
    ComPtr<ID3D11RenderTargetView> createRenderTarget(ComPtr<ID3D11Texture2D> backBuffer);

    ComPtr<ID3D11VideoProcessorEnumerator> createVideoProcessorEnumerator(const SizeI& inputSize, const SizeI& outputSize);
    ComPtr<ID3D11VideoProcessor> createVideoProcessor(ComPtr<ID3D11VideoProcessorEnumerator> enumerator, UINT rateConversionIndex);

    ComPtr<ID3D11VideoProcessorInputView> createVideoProcessorInputView(
      ComPtr<ID3D11VideoProcessorEnumerator> enumerator,
      ComPtr<ID3D11Texture2D> texture,
      UINT surfaceIndex
    );
    ComPtr<ID3D11VideoProcessorOutputView> createVideoProcessorOutputView(
      ComPtr<ID3D11VideoProcessorEnumerator> enumerator,
      ComPtr<ID3D11Texture2D> texture
    );

  private:
    void initializeDevice();

    ComPtr<IDXGIDevice2> getDXGIDevice();
    ComPtr<IDXGIAdapter> getDXGIAdapter();
    ComPtr<IDXGIFactory2> getDXGIFactory();

  private:
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_deviceContext;
    ComPtr<ID3D11VideoDevice> m_videoDevice;
    ComPtr<ID3D11VideoContext> m_videoContext;
    ComPtr<IDXGIDevice2> m_dxgiDevice;
    ComPtr<IDXGIAdapter> m_dxgiAdapter;
    ComPtr<IDXGIFactory2> m_dxgiFactory;
  };
}

#endif // LOCAL_D3D11_MANAGER_H
