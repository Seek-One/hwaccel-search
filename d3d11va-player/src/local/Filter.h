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
  class Filter {
  public:
    Filter(D3D11Manager& d3d11Manager, const SizeI& filteredPictureSize);
    ~Filter() = default;

    Filter(const Filter&) = delete;
    Filter(Filter&&) = delete;

    Filter& operator=(const Filter&) = delete;
    Filter& operator=(Filter&&) = delete;

    ComPtr<ID3D11Texture2D> process(const VideoTexture& decodedTexture);

  private:
    D3D11Manager& m_d3d11Manager;

    ComPtr<ID3D11VideoProcessorEnumerator> m_videoProcessorEnumerator;
    ComPtr<ID3D11VideoProcessor> m_videoProcessor;
    ComPtr<ID3D11Texture2D> m_textureBGRA;
    ComPtr<ID3D11VideoProcessorOutputView> m_outputView;
  };
}

#endif // LOCAL_FILTER_H_
