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

#include "Window.h"

#include <chrono>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <vector>

#include <d3dcompiler.h>
#include <directxmath.h>

#include "D3D11Decoder.h"
#include "D3D11Device.h"

namespace details {
  LRESULT CALLBACK procMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    switch (message) {
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    default:
      return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
  }
}

namespace dp {
  Window::Window(D3D11Device& d3d11Device, D3D11Decoder& decoder)
  : m_d3d11Device(d3d11Device)
  , m_d3d11Decoder(decoder)
  , m_swapChain(nullptr)
  , m_renderView(nullptr)
  , m_szTitle(L"D3D11VA Player")
  , m_szWindowClass(L"D3D11VAPLAYER")
  , m_isActive(true) {
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = details::procMessage;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = nullptr;
    wcex.hIcon          = nullptr;
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = nullptr;
    wcex.lpszClassName  = m_szWindowClass;
    wcex.hIconSm        = nullptr;

    // Register the class
    ATOM atom = RegisterClassEx(&wcex);
    if (atom == 0) {
      throw std::runtime_error("[Window] Unable to register Windows class");
    }

    // Create the effective window
    HWND hWnd = CreateWindow(
      m_szWindowClass,
      m_szTitle,
      WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT,
      0,
      CW_USEDEFAULT,
      0,
      nullptr,
      nullptr,
      nullptr,
      nullptr
    );

    if (hWnd == nullptr) {
      throw std::runtime_error("[Window] Unable to create the window");
    }

    ShowWindow(hWnd, SW_NORMAL);

    if (!UpdateWindow(hWnd)) {
      throw std::runtime_error("[Window] Unable to update the window");
    }

    // Get DXGI instances
    IDXGIDevice2* dxgiDevice = nullptr;
    HRESULT hRes = m_d3d11Device.getDevice().QueryInterface(&dxgiDevice);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get IDXGIDevice2");
    }

    hRes = dxgiDevice->SetMaximumFrameLatency(1);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to set frame latency");
    }

    IDXGIAdapter* dxgiAdapter = nullptr;
    hRes = dxgiDevice->GetAdapter(&dxgiAdapter);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get IDXGIAdapter");
    }

    IDXGIFactory2* dxgiFactory = nullptr;
    hRes = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get IDXGIFactory2");
    }

    // Create SwapChain
    DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {0};
    swapChainDesc.Stereo = false;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.Scaling = DXGI_SCALING_NONE;
    swapChainDesc.Flags = 0;
    // Use automatic sizing.
    swapChainDesc.Width = 0;
    swapChainDesc.Height = 0;
    swapChainDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    // Don't use multi-sampling.
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.BufferCount = 2;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;

    auto& device = m_d3d11Device.getDevice();
    hRes = dxgiFactory->CreateSwapChainForHwnd(&device, hWnd, &swapChainDesc, nullptr, nullptr, &m_swapChain);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get IDXGIFactory2");
    }

    // Release DXGI interfaces
    dxgiDevice->Release();
    dxgiAdapter->Release();
    dxgiFactory->Release();

    ID3D11Texture2D* backBuffer = nullptr;
    hRes = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get backbuffer");
    }

    // Get the render target view
    hRes = device.CreateRenderTargetView(backBuffer, nullptr, &m_renderView);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create a render target view");
    }

    D3D11_TEXTURE2D_DESC backBufferDesc = {0};
    backBuffer->GetDesc(&backBufferDesc);

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<FLOAT>(backBufferDesc.Width);
    viewport.Height = static_cast<FLOAT>(backBufferDesc.Height);
    viewport.MinDepth = D3D11_MIN_DEPTH;
    viewport.MaxDepth = D3D11_MAX_DEPTH;

    m_d3d11Device.getDeviceContext().RSSetViewports(1, &viewport);

    backBuffer->Release();

    // Create a video processor
    // First, create a video processor enumerator
    D3D11_VIDEO_PROCESSOR_CONTENT_DESC ContentDesc;
    ZeroMemory( &ContentDesc, sizeof( ContentDesc ) );
    ContentDesc.InputFrameFormat = D3D11_VIDEO_FRAME_FORMAT_INTERLACED_TOP_FIELD_FIRST;
    ContentDesc.InputWidth = 1920;
    ContentDesc.InputHeight = 1080;
    ContentDesc.OutputWidth = backBufferDesc.Width;
    ContentDesc.OutputHeight = backBufferDesc.Height;
    ContentDesc.Usage = D3D11_VIDEO_USAGE_PLAYBACK_NORMAL;

    auto& videoDevice = decoder.getVideoDevice();
    hRes = videoDevice.CreateVideoProcessorEnumerator(&ContentDesc, &m_videoProcessorEnumerator);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create a video processor enumerator");
    }

    UINT uiFlags;

    hRes = m_videoProcessorEnumerator->CheckVideoProcessorFormat(DXGI_FORMAT_NV12, &uiFlags);
    if (FAILED(hRes) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT)) {
      throw std::runtime_error("[Window] NV12 is not supported as input format");
    }

    hRes = m_videoProcessorEnumerator->CheckVideoProcessorFormat(DXGI_FORMAT_B8G8R8A8_UNORM, &uiFlags);
    if (FAILED(hRes) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT)) {
      throw std::runtime_error("[Window] BGRA is not supported as output format");
    }

    // TODO: which caps check? index == 0

    // Second effective creation
    hRes = videoDevice.CreateVideoProcessor(m_videoProcessorEnumerator, 0, &m_videoProcessor);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create a video processor");
    }

    // Create the output BGRA texture
    D3D11_TEXTURE2D_DESC textureDesc;
    ZeroMemory(&textureDesc, sizeof(D3D11_TEXTURE2D_DESC));
    textureDesc.Width = backBufferDesc.Width;
    textureDesc.Height = backBufferDesc.Height;
    textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.CPUAccessFlags = 0;
    textureDesc.MiscFlags = 0;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.SampleDesc.Quality = 0;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET;

    m_textureBGRA = nullptr;
    hRes = device.CreateTexture2D(
      &textureDesc,
      nullptr,
      &m_textureBGRA
    );
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create BGRA output texture");
    }

    // Create video processor output view
    D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC outputViewDesc;
    ZeroMemory(&outputViewDesc, sizeof(D3D11_VIDEO_PROCESSOR_OUTPUT_VIEW_DESC));
    outputViewDesc.ViewDimension =  D3D11_VPOV_DIMENSION_TEXTURE2D;
    outputViewDesc.Texture2D.MipSlice = 0;
    outputViewDesc.Texture2DArray.MipSlice = 0;
    outputViewDesc.Texture2DArray.FirstArraySlice = 0;
    outputViewDesc.Texture2DArray.ArraySize = 1;

    m_outputView = nullptr;
    hRes = videoDevice.CreateVideoProcessorOutputView(m_textureBGRA, m_videoProcessorEnumerator, &outputViewDesc, &m_outputView);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create a video processor input view");
    }
  }

  Window::~Window() {
    m_swapChain->Release();
    m_renderView->Release();
    m_videoProcessorEnumerator->Release();
    m_videoProcessor->Release();
    m_textureBGRA->Release();
    m_outputView->Release();
  }

  bool Window::isActive() const {
    return m_isActive;
  }

  void Window::procMessage() {
    MSG msg;
    PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
    if (!TranslateAccelerator(msg.hwnd, nullptr, &msg)) {
      TranslateMessage(&msg);
      DispatchMessage(&msg);
    }

    if (msg.message == WM_QUIT) {
      m_isActive = false;
    }
    // Break if user presses escape key
    else if (GetAsyncKeyState(VK_ESCAPE)) {
      m_isActive = false;
    }
  }

  void Window::clear() {
    auto& deviceContext = m_d3d11Device.getDeviceContext();
    deviceContext.OMSetRenderTargets(1, &m_renderView, nullptr);

    float color[4] = { 0.071f, 0.04f, 0.561f, 1.0f };
    deviceContext.ClearRenderTargetView(m_renderView, color);
  }

  void Window::render(const DecodedTexture& decodedTexture) {
    // Create video processor input view
    D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC inputViewDesc;
    ZeroMemory(&inputViewDesc, sizeof(D3D11_VIDEO_PROCESSOR_INPUT_VIEW_DESC));
    inputViewDesc.FourCC = 0;
    inputViewDesc.ViewDimension = D3D11_VPIV_DIMENSION_TEXTURE2D;
    inputViewDesc.Texture2D.MipSlice = 0;
    inputViewDesc.Texture2D.ArraySlice = decodedTexture.index;

    ID3D11VideoProcessorInputView* inputView = nullptr;
    auto& videoDevice = m_d3d11Decoder.getVideoDevice();
    HRESULT hRes = videoDevice.CreateVideoProcessorInputView(decodedTexture.texture, m_videoProcessorEnumerator, &inputViewDesc, &inputView);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create a video processor input view");
    }

    D3D11_VIDEO_PROCESSOR_STREAM streamData;
    ZeroMemory(&streamData, sizeof(D3D11_VIDEO_PROCESSOR_STREAM));
    streamData.Enable = TRUE;
    streamData.OutputIndex = 0;
    streamData.InputFrameOrField = 0;
    streamData.PastFrames = 0;
    streamData.FutureFrames = 0;
    streamData.ppPastSurfaces = nullptr;
    streamData.ppFutureSurfaces = nullptr;
    streamData.pInputSurface = inputView;
    streamData.ppPastSurfacesRight = nullptr;
    streamData.ppFutureSurfacesRight = nullptr;

    // Process the Decoder input
    auto& videoContext = m_d3d11Decoder.getVideoContext();
    hRes = videoContext.VideoProcessorBlt(m_videoProcessor, m_outputView, 0, 1, &streamData);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to process the frame");
    }

    inputView->Release();

    // Render the VideoProcessor output
    ID3D11Texture2D* backBuffer = nullptr;
    hRes = m_swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get backbuffer");
    }

    m_d3d11Device.getDeviceContext().CopyResource(backBuffer, m_textureBGRA);
    backBuffer->Release();

    hRes = m_swapChain->Present(1, 0);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to present the swapchain");
    }
  }
}