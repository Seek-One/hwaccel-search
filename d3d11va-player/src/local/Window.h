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

#ifndef LOCAL_WINDOW_H_
#define LOCAL_WINDOW_H_

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <WinSDKVer.h>
#include <windows.h>

#include <d3d11.h>
#include <dxgi1_2.h>

namespace dp {
  class D3D11Device;

  class Window {
  public:
    Window(D3D11Device& d3d11Device);
    ~Window();

    Window(const Window&) = delete;
    Window(Window&&) = delete;

    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    bool isActive() const;
    void procMessage();
    void clear();
    void render();

  private:
    D3D11Device& m_d3d11Device;
    IDXGISwapChain1* m_swapChain;
    ID3D11RenderTargetView* m_renderView;
    D3D11_VIEWPORT m_viewport;

    const LPCWSTR m_szTitle;
    const LPCWSTR m_szWindowClass;
    bool m_isActive;

    ID3D11VertexShader* m_vertexShader;
    ID3D11PixelShader* m_pixelShader;
    ID3D11InputLayout* m_inputLayout;
    ID3D11Buffer* m_vertexBuffer;
    ID3D11Buffer* m_indexBuffer;
  };
}

#endif // LOCAL_WINDOW_H_