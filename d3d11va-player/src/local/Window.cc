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

#pragma comment(lib, "d3d11.lib")

#include "Window.h"

#include <stdexcept>

namespace dp {
  Window::Window()
  : m_szTitle(L"D3D11VA Player")
  , m_szWindowClass(L"D3D11VAPLAYER") {
    WNDCLASSEXW wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = Window::procMessage;
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
  }

  LRESULT CALLBACK Window::procMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
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