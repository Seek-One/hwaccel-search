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

#include "Filter.h"

#include <iostream>
#include <stdexcept>

#include "Clock.h"
#include "VideoTexture.h"

namespace dp {
  Filter::Filter(D3D11Manager& d3d11Manager, const SizeI& filteredPictureSize)
  : m_d3d11Manager(d3d11Manager) {
    // Create a video processor enumerator
    m_videoProcessorEnumerator = m_d3d11Manager.createVideoProcessorEnumerator(filteredPictureSize);

    // Check supported format
    UINT uiFlags;
    HRESULT hRes = m_videoProcessorEnumerator->CheckVideoProcessorFormat(DXGI_FORMAT_NV12, &uiFlags);
    if (FAILED(hRes) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_INPUT)) {
      throw std::runtime_error("[Window] NV12 is not supported as input format");
    }

    hRes = m_videoProcessorEnumerator->CheckVideoProcessorFormat(DXGI_FORMAT_B8G8R8A8_UNORM, &uiFlags);
    if (FAILED(hRes) || 0 == (uiFlags & D3D11_VIDEO_PROCESSOR_FORMAT_SUPPORT_OUTPUT)) {
      throw std::runtime_error("[Window] BGRA is not supported as output format");
    }

    // TODO: which framerate conversion check? For now, just take the first
    UINT rateConversionIndex = 0;

    // Create the video processor
    m_videoProcessor = m_d3d11Manager.createVideoProcessor(m_videoProcessorEnumerator, rateConversionIndex);
  }

  void Filter::process(const VideoTexture& decodedTexture, ComPtr<ID3D11Texture2D> renderTexture) {
    Clock filterClock;

    // Create video processor input view
    auto inputView = m_d3d11Manager.createVideoProcessorInputView(
      m_videoProcessorEnumerator,
      decodedTexture.getTexture(),
      decodedTexture.getCurrentSurfaceIndex()
    );

    // Create video processor output view
    auto outputView = m_d3d11Manager.createVideoProcessorOutputView(m_videoProcessorEnumerator, renderTexture);

    D3D11_VIDEO_PROCESSOR_STREAM streamData;
    ZeroMemory(&streamData, sizeof(D3D11_VIDEO_PROCESSOR_STREAM));
    streamData.Enable = TRUE;
    streamData.OutputIndex = 0;
    streamData.InputFrameOrField = 0;
    streamData.PastFrames = 0;
    streamData.FutureFrames = 0;
    streamData.ppPastSurfaces = nullptr;
    streamData.ppFutureSurfaces = nullptr;
    streamData.pInputSurface = inputView.Get();
    streamData.ppPastSurfacesRight = nullptr;
    streamData.ppFutureSurfacesRight = nullptr;


    // Process the Decoder input
    auto videoContext = m_d3d11Manager.getVideoContext();
    HRESULT hRes = videoContext->VideoProcessorBlt(m_videoProcessor.Get(), outputView.Get(), 0, 1, &streamData);
    if (FAILED(hRes)) {
      throw std::runtime_error("[Filter] Unable to process the frame");
    }

    auto elapsedTime = filterClock.elapsed();
    std::cout << "[Filter] Frame resized in " << elapsedTime.count() << "ms" << std::endl;
  }

}
