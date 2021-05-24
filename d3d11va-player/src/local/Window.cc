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

#include <stdexcept>

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
  Window::Window(D3D11Device& d3d11Device)
  : m_d3d11Device(d3d11Device)
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

    // Get RenderRect
    RECT rect;
    GetClientRect(hWnd, &rect);

    // Get IDXGIFactory
    auto& device = d3d11Device.getDevice();
    IDXGIDevice* dxgiDevice = nullptr;
    HRESULT hRes = device.QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<void **>(&dxgiDevice));
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get the IDXGIDevice");
    }

    IDXGIAdapter * dxgiAdapter = nullptr;
    hRes = dxgiDevice->GetAdapter(&dxgiAdapter);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get the IDXGIAdapter");
    }

    IDXGIFactory * dxgiFactory = nullptr;
    hRes = dxgiAdapter->GetParent(__uuidof(IDXGIFactory), reinterpret_cast<void **>(&dxgiFactory));
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get the IDXGIFactory");
    }

    // Create SwapChain
    DXGI_SWAP_CHAIN_DESC swapDesc;
    ZeroMemory(&swapDesc, sizeof(DXGI_SWAP_CHAIN_DESC));
    swapDesc.BufferCount = 2;
    swapDesc.OutputWindow = hWnd;
    swapDesc.Windowed = true;
    swapDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapDesc.BufferDesc.Width = rect.right - rect.left;
    swapDesc.BufferDesc.Height = rect.bottom - rect.top;
    swapDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapDesc.SampleDesc.Count = 1;
    hRes = dxgiFactory->CreateSwapChain(&device, &swapDesc, &m_swapChain);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create a swapchain");
    }

    // Release dxgi interfaces
    dxgiFactory->Release();
    dxgiAdapter->Release();
    dxgiDevice->Release();

    // Get the render target view
    ID3D11Resource* backbuffer = nullptr;
    hRes = m_swapChain->GetBuffer(0, __uuidof(ID3D11Resource), reinterpret_cast<void **>(&backbuffer));
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to get backbuffer");
    }

    hRes = device.CreateRenderTargetView(backbuffer, nullptr, &m_renderView);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create a render target view");
    }

    // Release the backbuffer
    backbuffer->Release();

    // Compute viewport
    m_viewport.Width = static_cast<float>(swapDesc.BufferDesc.Width);
    m_viewport.Height = static_cast<float>(swapDesc.BufferDesc.Height);
    m_viewport.TopLeftY = 0;
    m_viewport.TopLeftX = 0;
    m_viewport.MinDepth = 0;
    m_viewport.MaxDepth = 1;
  }

  Window::~Window() {
    m_swapChain->Release();
    m_renderView->Release();
  }

  bool Window::isActive() const {
    return m_isActive;
  }

  void Window::procMessage() {
    MSG msg;
    PeekMessage(&msg, NULL, 0, 0, PM_REMOVE);
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
    float color[4] = { 1, 0, 0, 1 };
    m_d3d11Device.getDeviceContext().ClearRenderTargetView(m_renderView, color);
  }

  void Window::render() {
    auto& deviceContext = m_d3d11Device.getDeviceContext();

    ID3D11RenderTargetView* tempRTV[] = { m_renderView };
    deviceContext.OMSetRenderTargets(1, tempRTV, nullptr);

    deviceContext.RSSetViewports(1, &m_viewport);

    m_swapChain->Present(0, 0);
  }
}