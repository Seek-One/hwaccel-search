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

#include "D3D11Manager.h"

#include <iostream>

// For some unkown reasons, the D3D11_DECODER_PROFILE_H264_VLD_NOFGT its not found,
// so I need to redeclare the constant and add initguid.h header file
#include <initguid.h>
DEFINE_GUID(D3D11_DECODER_PROFILE_H264_VLD_NOFGT,    0x1b81be68, 0xa0c7,0x11d3,0xb9,0x84,0x00,0xc0,0x4f,0x2e,0x73,0xc5);

#include "VideoTexture.h"

namespace {
  constexpr int alignedSize(int size, int nbPixels) {
    return (size + nbPixels - 1) & ~(nbPixels - 1);
  }
}

namespace dp {
  ComPtr<ID3D11Device> D3D11Manager::getDevice() {
    if (m_device.Get() == nullptr || m_deviceContext.Get() == nullptr) {
      initializeDevice();
    }

    return m_device;
  }

  ComPtr<ID3D11DeviceContext> D3D11Manager::getDeviceContext() {
    if (m_device.Get() == nullptr || m_deviceContext.Get() == nullptr) {
      initializeDevice();
    }

    return m_deviceContext;
  }

  ComPtr<ID3D11VideoDevice> D3D11Manager::getVideoDevice() {
    if (m_videoDevice.Get() == nullptr) {
      auto device = getDevice();
      device.As(&m_videoDevice);

      if (m_videoDevice.Get() == nullptr) {
        throw std::runtime_error("[D3D11Manager] Unable to get D3D11 video device");
      }
    }

    return m_videoDevice;
  }

  ComPtr<ID3D11VideoContext> D3D11Manager::getVideoContext() {
    if (m_videoContext.Get() == nullptr) {
      auto deviceContext = getDeviceContext();
      deviceContext.As(&m_videoContext);

      if (m_videoContext.Get() == nullptr) {
        throw std::runtime_error("[D3D11Manager] Unable to get D3D11 video context");
      }
    }

    return m_videoContext;
  }

  ComPtr<ID3D11VideoDecoder> D3D11Manager::createVideoDecoder(const SizeI& rawPictureSize) {
    ComPtr<ID3D11VideoDecoder> videoDecoder;
    auto videoDevice = getVideoDevice();

    // Try to get a H264 decoder profile
    const GUID h264DecoderProfile = D3D11_DECODER_PROFILE_H264_VLD_NOFGT;

    bool profileFound = false;
    UINT profileCount = videoDevice->GetVideoDecoderProfileCount();
    for (UINT i = 0; i < profileCount; ++i) {
      GUID selectedProfileGUID;
      HRESULT hRes = videoDevice->GetVideoDecoderProfile(i, &selectedProfileGUID);
      if (FAILED(hRes)) {
        throw std::runtime_error("[D3D11Manager] Invalid profile selected");
      }

      if (h264DecoderProfile == selectedProfileGUID) {
        profileFound = true;
        break;
      }
    }

    if (!profileFound) {
      throw std::runtime_error("[D3D11Manager] No hardware profile found");
    }

    // Check support for NV12
    BOOL supportedFormat;
    HRESULT hRes = videoDevice->CheckVideoDecoderFormat(&h264DecoderProfile, DXGI_FORMAT_NV12, &supportedFormat);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unsupported output format");
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

    hRes = videoDevice->GetVideoDecoderConfigCount(&decoderDesc, &iDecoderConfigCount);
    if (FAILED(hRes) || iDecoderConfigCount == 0) {
      throw std::runtime_error("[D3D11Manager] Unable to get the decoder configurations");
    }

    // Select a decoder profile with ConfigBitstreamRaw != 0
    // TODO: we need to prefer no crypt buffer and ConfigBitstreamRaw == 2
    D3D11_VIDEO_DECODER_CONFIG decoderConfig;
    bool configFound = false;
    for (UINT i = 0; i < iDecoderConfigCount; ++i) {
      hRes = videoDevice->GetVideoDecoderConfig(&decoderDesc, i, &decoderConfig);
      if (FAILED(hRes)) {
        throw std::runtime_error("[D3D11Manager] Invalid configuration index provided");
      }

      if (/*conf.ConfigBitstreamRaw == 1 || */decoderConfig.ConfigBitstreamRaw == 2) {
        configFound = true;
        break;
      }
    }

    hRes = videoDevice->CreateVideoDecoder(&decoderDesc, &decoderConfig, videoDecoder.GetAddressOf());
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unable to create ID3D11VideoDecoder");
    }

    return videoDecoder;
  }

  VideoTexture D3D11Manager::createVideoTexture(ComPtr<ID3D11VideoDecoder> decoder, UINT nbSurface) {
    D3D11_VIDEO_DECODER_DESC decoderDesc;
    D3D11_VIDEO_DECODER_CONFIG decoderConfig;
    decoder->GetCreationParameters(&decoderDesc, &decoderConfig);

    return VideoTexture(*this, decoderDesc, nbSurface);
  }

  ComPtr<ID3D11Texture2D> D3D11Manager::createOutputTexture(SizeI backBufferSize) {
    ComPtr<ID3D11Texture2D> outputTexture;

    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
    textureDesc.Width = backBufferSize.width;
    textureDesc.Height = backBufferSize.height;
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;

    auto device = getDevice();
    HRESULT hRes = device->CreateTexture2D(
      &textureDesc,
      nullptr,
      outputTexture.GetAddressOf()
    );
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unable to create BGRA output texture");
    }

    return outputTexture;
  }

  ComPtr<IDXGISwapChain1> D3D11Manager::createSwapChain(HWND hWnd) {
    ComPtr<IDXGISwapChain1> swapChain;

    // Create SwapChain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
    swapChainDesc.Stereo = false;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.Flags = 0;
    // Use automatic sizing
    swapChainDesc.Width = 0;
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    auto device = getDevice();
    auto dxgiFactory = getDXGIFactory();

    HRESULT hRes = dxgiFactory->CreateSwapChainForHwnd(device.Get(), hWnd, &swapChainDesc, nullptr, nullptr, swapChain.GetAddressOf());
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unable to get IDXGIFactory2");
    }

    return swapChain;
  }

  ComPtr<ID3D11RenderTargetView> D3D11Manager::createRenderTarget(ComPtr<ID3D11Texture2D> backBuffer) {
    ComPtr<ID3D11RenderTargetView> renderView;

    // Get the render target view
    auto device = getDevice();
    HRESULT hRes = device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderView.GetAddressOf());
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unable to create a render target view");
    }

    return renderView;
  }

  ComPtr<ID3D11VideoProcessorEnumerator> D3D11Manager::createVideoProcessorEnumerator(const SizeI& inputSize, const SizeI& outputSize) {
    ComPtr<ID3D11VideoProcessorEnumerator> videoProcessorEnumerator;

    D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc;
    ZeroMemory(&ContentDesc, sizeof(D3D11_VIDEO_PROCESSOR_CONTENT_DESC));
    ContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_PROGRESSIVE;
    ContentDesc.InputWidth = inputSize.width;
    ContentDesc.InputHeight = inputSize.height;
    ContentDesc.OutputWidth = outputSize.width;
    ContentDesc.OutputHeight = outputSize.height;
    ContentDesc.Usage = D3D11_VIDEO_USAGE_OPTIMAL_SPEED;

    auto videoDevice = getVideoDevice();
    HRESULT hRes = videoDevice->CreateVideoProcessorEnumerator(&ContentDesc, videoProcessorEnumerator.GetAddressOf());
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unable to create a video processor enumerator");
    }

    return videoProcessorEnumerator;
  }

  ComPtr<ID3D11VideoProcessor> D3D11Manager::createVideoProcessor(ComPtr<ID3D11VideoProcessorEnumerator> enumerator, UINT rateConversionIndex) {
    ComPtr<ID3D11VideoProcessor> videoProcessor;

    auto videoDevice = getVideoDevice();

    // Second effective creation
    HRESULT hRes = videoDevice->CreateVideoProcessor(enumerator.Get(), rateConversionIndex, videoProcessor.GetAddressOf());
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unable to create a video processor");
    }

    return videoProcessor;
  }

  ComPtr<ID3D11VideoProcessorInputView> D3D11Manager::createVideoProcessorInputView(
    ComPtr<ID3D11VideoProcessorEnumerator> enumerator,
    ComPtr<ID3D11Texture2D> texture,
    UINT surfaceIndex
  ) {
    ComPtr<ID3D11VideoProcessorInputView> inputView;

    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputViewDesc;
    ZeroMemory(&inputViewDesc, sizeof(D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC));
    inputViewDesc.FourCC = 0;
    inputViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
    inputViewDesc.Texture2D.MipSlice = 0;
    inputViewDesc.Texture2D.ArraySlice = surfaceIndex;

    auto videoDevice = getVideoDevice();
    HRESULT hRes = videoDevice->CreateVideoProcessorInputView(
      texture.Get(),
      enumerator.Get(),
      &inputViewDesc,
      inputView.GetAddressOf()
    );
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unable to create a video processor input view");
    }

    return inputView;
  }

  ComPtr<ID3D11VideoProcessorOutputView> D3D11Manager::createVideoProcessorOutputView(ComPtr<ID3D11VideoProcessorEnumerator> enumerator, ComPtr<ID3D11Texture2D> texture) {
    ComPtr<ID3D11VideoProcessorOutputView> outputView;

    D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC outputViewDesc;
    ZeroMemory(&outputViewDesc, sizeof(D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC));
    outputViewDesc.ViewDimension =  D3D11_VPOV_DIMENSION_TEXTURE2D;
    outputViewDesc.Texture2D.MipSlice = 0;
    outputViewDesc.Texture2DArray.MipSlice = 0;
    outputViewDesc.Texture2DArray.FirstArraySlice = 0;
    outputViewDesc.Texture2DArray.ArraySize = 1;

    auto videoDevice = getVideoDevice();
    HRESULT hRes = videoDevice->CreateVideoProcessorOutputView(texture.Get(), enumerator.Get(), &outputViewDesc, outputView.GetAddressOf());
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unable to create a video processor input view");
    }

    return outputView;
  }

  void D3D11Manager::initializeDevice() {
    UINT deviceFlags = D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
#if defined(_DEBUG) || defined(DEBUG) || !defined(NDEBUG)
    std::cout << "[D3D11Manager] D3D11 debug layer enabled" << std::endl;
    deviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevels[] = {
      D3D_FEATURE_LEVEL_11_1,
      D3D_FEATURE_LEVEL_11_0
    };

    HRESULT hRes = D3D11CreateDevice(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      deviceFlags,
      featureLevels,
      2,
      D3D11_SDK_VERSION,
      m_device.GetAddressOf(),
      nullptr,
      m_deviceContext.GetAddressOf()
    );
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unable to create D3D11 device");
    }
  }

  ComPtr<IDXGIDevice2> D3D11Manager::getDXGIDevice() {
    if (m_dxgiDevice.Get() == nullptr) {
      auto device = getDevice();
      device.As(&m_dxgiDevice);

      if (m_dxgiDevice.Get() == nullptr) {
        throw std::runtime_error("[D3D11Manager] Unable to get DXGI device");
      }
    }

    HRESULT hRes = m_dxgiDevice->SetMaximumFrameLatency(1);
    if (FAILED(hRes)) {
      throw std::runtime_error("[D3D11Manager] Unable to set frame latency");
    }

    return m_dxgiDevice;
  }

  ComPtr<IDXGIAdapter> D3D11Manager::getDXGIAdapter()  {
    if (m_dxgiAdapter.Get() == nullptr) {
      auto dxgiDevice = getDXGIDevice();

      HRESULT hRes = dxgiDevice->GetAdapter(m_dxgiAdapter.GetAddressOf());
      if (FAILED(hRes)) {
        throw std::runtime_error("[D3D11Manager] Unable to get DXGI adapter");
      }
    }

    return m_dxgiAdapter;
  }

  ComPtr<IDXGIFactory2> D3D11Manager::getDXGIFactory()  {
    if (m_dxgiAdapter.Get() == nullptr) {
      auto dxgiAdapter = getDXGIAdapter();

      HRESULT hRes = dxgiAdapter->GetParent(IID_PPV_ARGS(m_dxgiFactory.GetAddressOf()));
      if (FAILED(hRes)) {
        throw std::runtime_error("[D3D11Manager] Unable to get DXGI factory");
      }
    }

    return m_dxgiFactory;
  }
}
