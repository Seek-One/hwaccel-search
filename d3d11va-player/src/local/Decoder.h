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

#ifndef LOCAL_DECODER_H
#define LOCAL_DECODER_H

#include "D3D11Manager.h" // Must be included in first

#include <dxva.h>

#include "DecodedPictureBuffer.h"
#include "Size.h"
#include "VideoTexture.h"

namespace dp {
  class FileParser;

  class Decoder {
  public:
    Decoder(D3D11Manager& d3d11Manager, const SizeI& rawPictureSize, const SizeI& realPictureSize);
    ~Decoder() = default;

    Decoder(const Decoder&) = delete;
    Decoder(Decoder&&) = delete;

    Decoder& operator=(const Decoder&) = delete;
    Decoder& operator=(Decoder&&) = delete;

    const VideoTexture& decodeSlice(FileParser& parser);

  private:
    void fillPictureParams(DXVA_PicParams_H264& picParams, FileParser& parser);
    void fillScalingLists(DXVA_Qmatrix_H264& scalingLists, const FileParser& parser);
    void Decoder::sendBistreamAndSliceControl(
      const std::vector<uint8_t>& bitstream,
      D3D11_VIDEO_DECODER_BUFFER_DESC& bitstreamBufferDesc,
      D3D11_VIDEO_DECODER_BUFFER_DESC& sliceControlBufferDesc,
      std::vector<DXVA_Slice_H264_Short>& listSliceControl,
      const FileParser& parser
    );

    template<typename Buffer>
    void sendBuffer(D3D11_VIDEO_DECODER_BUFFER_TYPE bufferType, const Buffer* buffer, int bufferSize, D3D11_VIDEO_DECODER_BUFFER_DESC& bufferDesc, int MBCount = 0) {
      HRESULT hRes = 0;
      void *D3D11VABuffer = NULL;
      UINT D3D11VABufferSize = 0;

      hRes = m_d3d11Manager.getVideoContext()->GetDecoderBuffer(m_videoDecoder.Get(), bufferType, &D3D11VABufferSize, &D3D11VABuffer);
      if (FAILED(hRes)) {
        throw std::runtime_error("[Decoder] Unable to get a decoder buffer");
      }

      if (D3D11VABufferSize < (UINT)bufferSize) {
        throw std::runtime_error("[Decoder] The D3D11 VA buffer is too small");
      }

      std::memcpy(D3D11VABuffer, buffer, bufferSize);
      std::memset(&bufferDesc, 0, sizeof(D3D11_VIDEO_DECODER_BUFFER_DESC));
      bufferDesc.BufferType = bufferType;
      bufferDesc.DataSize = bufferSize;
      bufferDesc.NumMBsInBuffer = MBCount;

      hRes = m_d3d11Manager.getVideoContext()->ReleaseDecoderBuffer(m_videoDecoder.Get(), bufferType);
      if (FAILED(hRes)) {
        throw std::runtime_error("[Decoder] Unable to release a decoder buffer");
      }
    }

    unsigned getNextAvailableSurfaceIndex() const;
    void startNewIDRFrame();

  private:
    D3D11Manager& m_d3d11Manager;
    SizeI m_realPictureSize;

    ComPtr<ID3D11VideoDecoder> m_videoDecoder;
    VideoTexture m_videoTexture;
    unsigned m_currentReportID;

    DecodedPictureBuffer m_dpb;
  };
}

#endif // LOCAL_DECODER_H
