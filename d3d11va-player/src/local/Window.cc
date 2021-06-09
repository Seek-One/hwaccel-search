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

#include "Clock.h"
#include "Decoder.h"
#include "VideoTexture.h"

namespace details {
  LRESULT CALLBACK procMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    dp::Window *windowPtr = nullptr;
    if (message == WM_CREATE) {
      // Store the pointer to the main window
      CREATESTRUCT *pCreate = reinterpret_cast<CREATESTRUCT*>(lParam);
      windowPtr = reinterpret_cast<dp::Window*>(pCreate->lpCreateParams);
      SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)windowPtr);
    } else {
      // Get the window pointer form local userdata
      LONG_PTR ptr = GetWindowLongPtr(hWnd, GWLP_USERDATA);
      windowPtr = reinterpret_cast<dp::Window*>(ptr);
    }

    switch (message) {
    case WM_SIZE: {
      UINT width = LOWORD(lParam);
      UINT height = HIWORD(lParam);
      windowPtr->resize(dp::SizeI(width, height));
      break;
    }

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
  Window::Window(D3D11Manager& d3d11Manager)
  : m_d3d11Manager(d3d11Manager)
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
      GetModuleHandle(nullptr),
      this
    );

    if (hWnd == nullptr) {
      throw std::runtime_error("[Window] Unable to create the window");
    }

    ShowWindow(hWnd, SW_NORMAL);

    if (!UpdateWindow(hWnd)) {
      throw std::runtime_error("[Window] Unable to update the window");
    }

    m_swapChain = m_d3d11Manager.createSwapChain(hWnd);

    // Set viewport
    setViewport();
  }

  ComPtr<ID3D11Texture2D> Window::getCurrentBackbuffer() {
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hRes = m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get backbuffer");
    }

    return backBuffer;
  }

  void Window::resize(const SizeI& newSize) {
    std::cout << "[Window] resize event: " << newSize.width << "x" << newSize.height << std::endl;

    // If the swap chain is created
    if (m_swapChain.Get() != nullptr) {
      // We need to release all backbuffer in use
      m_renderView.Reset();

      HRESULT hRes = m_swapChain->ResizeBuffers(2, newSize.width, newSize.height, DXGI_FORMAT_B8G8R8A8_UNORM, 0);
      if (FAILED(hRes)) {
        throw std::runtime_error("[Window] Unable to resize rhe back buffers");
      }

      setViewport();
    }
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
    setCurrentRenderTargetView();

    auto deviceContext = m_d3d11Manager.getDeviceContext();
    deviceContext->OMSetRenderTargets(1, m_renderView.GetAddressOf(), nullptr);

    float color[4] = { 0.071f, 0.04f, 0.561f, 1.0f };
    deviceContext->ClearRenderTargetView(m_renderView.Get(), color);
  }

  void Window::render() {
    Clock renderClock;

    HRESULT hRes = m_swapChain->Present(0, 0);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to present the swapchain");
    }

    auto elapsedTime = renderClock.elapsed();
    std::cout << "[Filter] Frame rendered in " << elapsedTime.count() << "ms" << std::endl;
  }

  void Window::setCurrentRenderTargetView() {
    // Get current back buffer
    auto backBuffer = getCurrentBackbuffer();

    // Create the render target view
    m_renderView = m_d3d11Manager.createRenderTarget(backBuffer);
  }

  void Window::setViewport() {
    // Get the back buffer
    auto backBuffer = getCurrentBackbuffer();

    // Set the view port
    D3D11_TEXTURE2D_DESC backBufferDesc;
    backBuffer->GetDesc(&backBufferDesc);
    m_rendererSize = SizeI(backBufferDesc.Width, backBufferDesc.Height);

    D3D11_VIEWPORT viewport;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    viewport.Width = static_cast<FLOAT>(backBufferDesc.Width);
    viewport.Height = static_cast<FLOAT>(backBufferDesc.Height);
    viewport.MinDepth = D3D11_MIN_DEPTH;
    viewport.MaxDepth = D3D11_MAX_DEPTH;

    m_d3d11Manager.getDeviceContext()->RSSetViewports(1, &viewport);
  }
}
