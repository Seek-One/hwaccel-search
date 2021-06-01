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

#include "Decoder.h"
#include "VideoTexture.h"

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

    m_swapChain = m_d3d11Manager.createSwapChain(hWnd);

    // Get the back buffer
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hRes = m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get backbuffer");
    }

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

    // Create the render target view
    m_renderView = m_d3d11Manager.createRenderTarget(backBuffer);
  }

  SizeI Window::getRendererSize() const {
    return m_rendererSize;
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
    auto deviceContext = m_d3d11Manager.getDeviceContext();
    deviceContext->OMSetRenderTargets(1, m_renderView.GetAddressOf(), nullptr);

    float color[4] = { 0.071f, 0.04f, 0.561f, 1.0f };
    deviceContext->ClearRenderTargetView(m_renderView.Get(), color);
  }

  void Window::render(ComPtr<ID3D11Texture2D> filteredTexture) {
    // Render the VideoProcessor output
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hRes = m_swapChain->GetBuffer(0, IID_PPV_ARGS(backBuffer.GetAddressOf()));
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get backbuffer");
    }

    m_d3d11Manager.getDeviceContext()->CopyResource(backBuffer.Get(), filteredTexture.Get());

    hRes = m_swapChain->Present(1, 0);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to present the swapchain");
    }
  }
}
