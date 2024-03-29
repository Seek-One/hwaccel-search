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

#ifndef LOCAL_FILTER_H_
#define LOCAL_FILTER_H_

#include "D3D11Manager.h"

namespace dp {
  /**
   * @brief Resize the video frame and do the color space conversion
   */
  class Filter {
  public:
    /**
     * @brief Construct a new Filter object
     *
     * @param d3d11Manager Reference to D3D11Manager
     */
    Filter(D3D11Manager& d3d11Manager);
    ~Filter() = default;

    Filter(const Filter&) = delete;
    Filter(Filter&&) = delete;

    Filter& operator=(const Filter&) = delete;
    Filter& operator=(Filter&&) = delete;

    /**
     * @brief Resize the decodedTexture
     *
     * @param decodedTexture VideoTexture form the Decoder
     * @param renderTexture Render texture from the window swap chain
     */
    void process(const VideoTexture& decodedTexture, ComPtr<ID3D11Texture2D> renderTexture);

  private:
    void createVideoProcessor(const SizeI& inputSize, const SizeI& outputSize);

  private:
    D3D11Manager& m_d3d11Manager;

    ComPtr<ID3D11VideoProcessorEnumerator> m_videoProcessorEnumerator;
    ComPtr<ID3D11VideoProcessor> m_videoProcessor;

    SizeI m_currentOutputSize;
  };
}

#endif // LOCAL_FILTER_H_
