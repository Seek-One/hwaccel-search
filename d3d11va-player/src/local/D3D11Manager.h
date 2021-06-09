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

  /**
   * @brief D3D11 objects manager
   *
   * The ID3D11Device, ID3D11DeviceContext, ID3D11VideoDevice and ID3D11VideoContext
   * must be shared between all class which use D3D11 API. So the main aim of this class
   * is ensure that this object are not recreated or destroyed before the end of program.
   *
   * Moreover, this class offers a way to create all other objects needed to decode a
   * video stream.
   */
  class D3D11Manager {
  public:
    D3D11Manager() = default;
    ~D3D11Manager() = default;

    D3D11Manager(const D3D11Manager&) = delete;
    D3D11Manager(D3D11Manager&&) = delete;

    D3D11Manager& operator=(const D3D11Manager&) = delete;
    D3D11Manager& operator=(D3D11Manager&&) = delete;

    /**
     * @brief Get the Device object
     *
     * @return ComPtr<ID3D11Device> to the main D3D11 device
     */
    ComPtr<ID3D11Device> getDevice();

    /**
     * @brief Get the Device Context object
     *
     * @return ComPtr<ID3D11DeviceContext> to the immediate context
     */
    ComPtr<ID3D11DeviceContext> getDeviceContext();

    /**
     * @brief Get the Video Device object
     *
     * @return ComPtr<ID3D11VideoDevice> to the video device
     */
    ComPtr<ID3D11VideoDevice> getVideoDevice();

    /**
     * @brief Get the Video Context object
     *
     * @return ComPtr<ID3D11VideoContext> to the video device context
     */
    ComPtr<ID3D11VideoContext> getVideoContext();

    /**
     * @brief Create a Video Decoder object
     *
     * @param rawPictureSize
     * @return ComPtr<ID3D11VideoDecoder> to newly create video decoder
     */
    ComPtr<ID3D11VideoDecoder> createVideoDecoder(const SizeI& rawPictureSize);

    /**
     * @brief Create a Video Texture object using to store NV12 surface
     *
     * @param decoder The decoder on which the texture will be binded
     * @param nbSurface The number of different surface
     * @return VideoTexture to the newly create nv12 surface
     */
    VideoTexture createVideoTexture(ComPtr<ID3D11VideoDecoder> decoder, UINT nbSurface);

    /**
     * @brief Create a Swap Chain object
     *
     * @param hWnd The window handle on which the swap will be attached
     * @return ComPtr<IDXGISwapChain1> to the newly swap chain
     */
    ComPtr<IDXGISwapChain1> createSwapChain(HWND hWnd);

    /**
     * @brief Create a Render Target object
     *
     * @param backBuffer The back buffer pointed by the render target
     * @return ComPtr<ID3D11RenderTargetView> to the newly render target
     */
    ComPtr<ID3D11RenderTargetView> createRenderTarget(ComPtr<ID3D11Texture2D> backBuffer);


    /**
     * @brief Create a Video Processor Enumerator object
     *
     * @param inputSize Video input size (uncropped size and aligned to 16 pixels)
     * @param outputSize Video output size
     * @return ComPtr<ID3D11VideoProcessorEnumerator> to the newly video processor enumerator
     */
    ComPtr<ID3D11VideoProcessorEnumerator> createVideoProcessorEnumerator(const SizeI& inputSize, const SizeI& outputSize);

    /**
     * @brief Create a Video Processor object
     *
     * @param enumerator Video processor enumerator
     * @return ComPtr<ID3D11VideoProcessor> to the newly video processor
     */
    ComPtr<ID3D11VideoProcessor> createVideoProcessor(ComPtr<ID3D11VideoProcessorEnumerator> enumerator);

    /**
     * @brief Create a Video Processor Input View object
     *
     * @param enumerator Video processor enumerator
     * @param texture Texture reference from the decoder (aka. VideoTexture)
     * @param surfaceIndex The index of surface in surface array
     * @return ComPtr<ID3D11VideoProcessorInputView> to the input view
     *
     * @sa VideoTexture
     */
    ComPtr<ID3D11VideoProcessorInputView> createVideoProcessorInputView(
      ComPtr<ID3D11VideoProcessorEnumerator> enumerator,
      ComPtr<ID3D11Texture2D> texture,
      UINT surfaceIndex
    );

    /**
     * @brief Create a Video Processor Output View object
     *
     * @param enumerator Video processor enumerator
     * @param texture Texture reference form the swap chain (see Window)
     * @return ComPtr<ID3D11VideoProcessorOutputView> to the output view
     */
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
