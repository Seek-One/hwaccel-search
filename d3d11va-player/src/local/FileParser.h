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

#ifndef LOCAL_FILE_PARSER_H_
#define LOCAL_FILE_PARSER_H_

#include <filesystem>
#include <vector>

#include <h264_stream.h>

#include "Size.h"

namespace dp {
  class FileParser {
  public:
    FileParser(const std::filesystem::path& bitstreamPath);
    ~FileParser();

    FileParser(const FileParser&) = delete;
    FileParser(FileParser&&) = delete;

    FileParser& operator=(const FileParser&) = delete;
    FileParser& operator=(FileParser&&) = delete;

    void extractPictureSizes();
    const SizeI& getMbPictureSize() const;
    const SizeI& getRawPictureSize() const;
    const SizeI& getRealPictureSize() const;

    bool parseNextNAL();
    const h264_stream_t& getStream() const;

  private:
    void computeSizes();
    int computeSubWidthC();
    int computeSubHeightC();

  private:
    h264_stream_t* m_h264Stream;
    std::vector<uint8_t> m_bitstreamData;
    uint8_t* m_pDataCursor;
    int m_unprocessedDataSize;
    bool m_sizeComputed;
    SizeI m_mbPictureSize;
    SizeI m_rawPictureSize;
    SizeI m_realPictureSize;
  };
}

#endif // LOCAL_FILE_PARSER_H_