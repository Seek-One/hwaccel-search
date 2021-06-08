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

#ifndef LOCAL_VIDEO_TEXTURE_H_
#define LOCAL_VIDEO_TEXTURE_H_

#include "WindowsHeaders.h"

#include <filesystem>
#include <vector>

#include <wrl/client.h>

#include "D3D11Manager.h"
#include "Size.h"

struct ID3D11Texture2D;

namespace dp {
  class VideoTexture {
  public:
    VideoTexture(D3D11Manager& d3d11Manager, D3D11_VIDEO_DECODER_DESC decoderDesc, UINT nbSurface);
    ~VideoTexture() = default;

    VideoTexture(const VideoTexture&) = delete;
    VideoTexture(VideoTexture&&) = delete;

    VideoTexture& operator=(const VideoTexture&) = delete;
    VideoTexture& operator=(VideoTexture&&) = delete;

    ComPtr<ID3D11Texture2D> getTexture() const;
    ComPtr<ID3D11VideoDecoderOutputView> getCurrentOutputView() const;
    UINT getCurrentSurfaceIndex() const;
    void nextSurfaceIndex();

    void copyToFile(const SizeI& rawPictureSize, const std::filesystem::path& filename);

  private:
    D3D11Manager& m_d3d11Manager;
    ComPtr<ID3D11Texture2D> m_texture;
    std::vector<ComPtr<ID3D11VideoDecoderOutputView>> m_outputViews;
    UINT m_currentSurface;
  };
}

#endif // LOCAL_VIDEO_TEXTURE_H_
