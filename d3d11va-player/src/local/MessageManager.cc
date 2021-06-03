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

#include "MessageManager.h"

#include <iostream>
#include <stdexcept>

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
  MessageManager::MessageManager() {
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
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = nullptr;

    // Register the class
    ATOM atom = RegisterClassEx(&wcex);
    if (atom == 0) {
      throw std::runtime_error("[MessageManager] Unable to register Windows class");
    }
  }

  bool MessageManager::poolMessage(Message& message) {
    MSG msg;
    bool hasMessage = false;
    if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
      switch (msg.message) {
      case WM_QUIT:
        hasMessage = true;
        message.type = MessageType::Quit;
        break;

      case WM_SIZE: {
        UINT width = LOWORD(msg.lParam);
        UINT height = HIWORD(msg.lParam);
        std::cout << "[MessageManager] Resize message: " << width << "x" << height << std::endl;

        message.type = MessageType::Resize;
        message.resize.windowSize = SizeI(width, height);
        break;
      }

      default:
        if (GetAsyncKeyState(VK_ESCAPE)) {
          message.type = MessageType::Quit;
          hasMessage = true;
        }
        break;
      }

      if (!TranslateAccelerator(msg.hwnd, nullptr, &msg)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }

    return hasMessage;
  }
}
