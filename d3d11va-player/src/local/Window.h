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

#include "D3D11Manager.h"

namespace dp {
  class VideoTexture;

  /**
   * @brief Display the window
   */
  class Window {
  public:
    /**
     * @brief Construct a new Window object
     *
     * @param d3d11Manager Reference to D3D11Manager
     */
    Window(D3D11Manager& d3d11Manager);
    ~Window() = default;

    Window(const Window&) = delete;
    Window(Window&&) = delete;

    Window& operator=(const Window&) = delete;
    Window& operator=(Window&&) = delete;

    /**
     * @brief Get the current back buffer
     *
     * @return ComPtr<ID3D11Texture2D>
     */
    ComPtr<ID3D11Texture2D> getCurrentBackbuffer();

    /**
     * @brief Resize the window
     *
     * This method must be call to recreate the swap chain when on resize event arrive
     *
     * @param newSize New window size
     */
    void resize(const SizeI& newSize);

    /**
     * @brief Indicate if the window is active
     *
     * @return true if the window is active otherwise false
     */
    bool isActive() const;

    /**
     * @brief Process WIN32 message
     */
    void procMessage();

    /**
     * @brief Clear the render
     */
    void clear();

    /**
     * @brief Display the filtered image
     *
     * We do not need to pass the buffer here since the Filter take directly the back buffer
     */
    void render();

  private:
    void setCurrentRenderTargetView();
    void setViewport();

  private:
    const LPCWSTR m_szTitle;
    const LPCWSTR m_szWindowClass;
    HWND m_hwnd;
    bool m_isActive;
    SizeI m_rendererSize;

    D3D11Manager& m_d3d11Manager;

    ComPtr<IDXGISwapChain1> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderView;
  };
}

#endif // LOCAL_WINDOW_H_
