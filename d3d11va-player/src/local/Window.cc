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

#include <iostream>
#include <stdexcept>

#include <d3dcompiler.h>
#include <directxmath.h>

#include "D3D11Device.h"

namespace details {
  const std::string vertexShaderCode =
R"vs(
struct VertexShaderInput
{
  float2 pos : POSITION;
};

struct PixelShaderInput
{
  float4 pos : SV_POSITION;
};

PixelShaderInput SimpleVertexShader(VertexShaderInput input)
{
  PixelShaderInput vertexShaderOutput;

  // For this lesson, set the vertex depth value to 0.5, so it is guaranteed to be drawn.
  vertexShaderOutput.pos = float4(input.pos, 0.5f, 1.0f);

  return vertexShaderOutput;
}
)vs";

  const std::string pixelShaderCode =
R"ps(
struct PixelShaderInput
{
  float4 pos : SV_POSITION;
};

float4 SimplePixelShader(PixelShaderInput input) : SV_TARGET
{
  // Draw the entire triangle yellow.
  return float4(1.0f, 1.0f, 0.0f, 1.0f);
}
)ps";

  void logShaderErrors(ID3D10Blob* errorMessage) {
    std::string errorString(reinterpret_cast<const char*>(errorMessage->GetBufferPointer()), errorMessage->GetBufferSize());

    std::cerr << "[Window] Shader compilation errors: " << errorString << std::endl;

    errorMessage->Release();
  }

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

    // Compile vertex shader
    ID3D10Blob* vertexShaderBuffer = nullptr;
    ID3D10Blob* errorMessage = nullptr;
    hRes = D3DCompile(
      details::vertexShaderCode.data(),
      details::vertexShaderCode.size(),
      "SimpleVertexShader",
      nullptr,
      D3D_COMPILE_STANDARD_FILE_INCLUDE,
      "SimpleVertexShader",
      "vs_5_0",
      0,
      0,
      &vertexShaderBuffer,
      &errorMessage
    );
    if (FAILED(hRes)) {
      details::logShaderErrors(errorMessage);
      throw std::runtime_error("[Window] Unable to compile vertex shader");
    }

    // Create vertex shader
    m_vertexShader = nullptr;
    hRes = device.CreateVertexShader(
      vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(),
      nullptr,
      &m_vertexShader
    );
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create vertex shader");
    }

    // Create an input layout
    const D3D11_INPUT_ELEMENT_DESC basicVertexLayoutDesc[] = {
      { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    m_inputLayout = nullptr;
    hRes = device.CreateInputLayout(
      basicVertexLayoutDesc,
      ARRAYSIZE(basicVertexLayoutDesc),
      vertexShaderBuffer->GetBufferPointer(),
      vertexShaderBuffer->GetBufferSize(),
      &m_inputLayout
    );
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create a input layout for vertex shader");
    }

    vertexShaderBuffer->Release();

    // Compile pixel shader
    ID3D10Blob* pixelShaderBuffer = nullptr;
    hRes = D3DCompile(
      details::pixelShaderCode.data(),
      details::pixelShaderCode.size(),
      "SimplePixelShader",
      nullptr,
      nullptr,
      "SimplePixelShader",
      "ps_5_0",
      0,
      0,
      &pixelShaderBuffer,
      &errorMessage
    );
    if (FAILED(hRes)) {
      details::logShaderErrors(errorMessage);
      throw std::runtime_error("[Window] Unable to compile pixel shader");
    }

    // Create pixel shader
    m_pixelShader = nullptr;
    hRes = device.CreatePixelShader(
      pixelShaderBuffer->GetBufferPointer(),
      pixelShaderBuffer->GetBufferSize(),
      nullptr,
      &m_pixelShader
    );
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create pixel shader");
    }

    pixelShaderBuffer->Release();

    // Create primitive
    DirectX::XMFLOAT2 triangleVertices[] = {
      DirectX::XMFLOAT2(-0.5f, -0.5f),
      DirectX::XMFLOAT2( 0.0f,  0.5f),
      DirectX::XMFLOAT2( 0.5f, -0.5f),
    };

    unsigned short triangleIndices[] = {
        0, 1, 2,
    };

    D3D11_BUFFER_DESC vertexBufferDesc = {0};
    vertexBufferDesc.ByteWidth = sizeof(DirectX::XMFLOAT2) * ARRAYSIZE(triangleVertices);
    vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    vertexBufferDesc.CPUAccessFlags = 0;
    vertexBufferDesc.MiscFlags = 0;
    vertexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA vertexBufferData;
    vertexBufferData.pSysMem = triangleVertices;
    vertexBufferData.SysMemPitch = 0;
    vertexBufferData.SysMemSlicePitch = 0;

    m_vertexBuffer = nullptr;
    hRes = device.CreateBuffer(
      &vertexBufferDesc,
      &vertexBufferData,
      &m_vertexBuffer
    );
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create a buffer");
    }

    D3D11_BUFFER_DESC indexBufferDesc;
    indexBufferDesc.ByteWidth = sizeof(unsigned short) * ARRAYSIZE(triangleIndices);
    indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
    indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    indexBufferDesc.CPUAccessFlags = 0;
    indexBufferDesc.MiscFlags = 0;
    indexBufferDesc.StructureByteStride = 0;

    D3D11_SUBRESOURCE_DATA indexBufferData;
    indexBufferData.pSysMem = triangleIndices;
    indexBufferData.SysMemPitch = 0;
    indexBufferData.SysMemSlicePitch = 0;

    m_indexBuffer = nullptr;
    hRes = device.CreateBuffer(
      &indexBufferDesc,
      &indexBufferData,
      &m_indexBuffer
    );
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to create a buffer");
    }
  }

  Window::~Window() {
    m_swapChain->Release();
    m_renderView->Release();
    m_vertexShader->Release();
    m_pixelShader->Release();
    m_inputLayout->Release();
    m_indexBuffer->Release();
    m_vertexBuffer->Release();
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
    auto& deviceContext = m_d3d11Device.getDeviceContext();
    deviceContext.OMSetRenderTargets(1, &m_renderView, nullptr);

    float color[4] = { 0.071f, 0.04f, 0.561f, 1.0f };
    deviceContext.ClearRenderTargetView(m_renderView, color);
  }

  void Window::render() {
    auto& deviceContext = m_d3d11Device.getDeviceContext();
    deviceContext.IASetInputLayout(m_inputLayout);

    UINT stride = sizeof(DirectX::XMFLOAT2);
    UINT offset = 0;
    deviceContext.IASetVertexBuffers(
      0,
      1,
      &m_vertexBuffer,
      &stride,
      &offset
    );

    deviceContext.IASetIndexBuffer(
      m_indexBuffer,
      DXGI_FORMAT_R16_UINT,
      0
    );

    deviceContext.IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Set the vertex and pixel shader stage state
    deviceContext.VSSetShader(
      m_vertexShader,
      nullptr,
      0
    );

    deviceContext.PSSetShader(
      m_pixelShader,
      nullptr,
      0
    );

    // Draw the cube.
    deviceContext.DrawIndexed(
      3,
      0,
      0
    );

    HRESULT hRes = m_swapChain->Present(1, 0);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Window] Unable to present the swapchain");
    }
  }
}