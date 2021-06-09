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
  /**
   * @brief H264 bitstream parser
   */
  class FileParser {
  public:
    /**
     * @brief Construct a new File Parser object
     *
     * @param bitstreamPath Path to the bitstream
     */
    FileParser(const std::filesystem::path& bitstreamPath);

    /**
     * @brief Destroy the File Parser object
     */
    ~FileParser();

    FileParser(const FileParser&) = delete;
    FileParser(FileParser&&) = delete;

    FileParser& operator=(const FileParser&) = delete;
    FileParser& operator=(FileParser&&) = delete;

    /**
     * @brief Read the bitstream to compute the picture size
     *
     * This method read just enough NAL to get 3 different sizes:
     * 1. Picture size expressed in macroblocks
     * 2. Picture size without cropping
     * 3. Actually picture size
     */
    void extractPictureSizes();

    /**
     * @brief Get the Picture Size expressed in macroblock
     *
     * Must be call after extractPictureSizes() method.
     *
     * @return const SizeI& Macroblock size
     */
    const SizeI& getMbPictureSize() const;

    /**
     * @brief Get the Picture Size without cropping
     *
     * Must be call after extractPictureSizes() method.
     *
     * @return const SizeI& Uncropped size
     */
    const SizeI& getRawPictureSize() const;

    /**
     * @brief Get the actually Picture Size
     *
     * Must be call after extractPictureSizes() method.
     *
     * @return const SizeI& Real picture size
     */
    const SizeI& getRealPictureSize() const;

    /**
     * @brief Process the next NAL
     *
     * @return true when a NAL was processed otherwise false
     */
    bool parseNextNAL();

    /**
     * @brief Get the h264 nal informations
     *
     * @return const h264_stream_t& Reference to the h264_stream_t structure
     */
    const h264_stream_t& getStream() const;

    /**
     * @brief Get the Current NAL data
     *
     * The NAL is returned whitout start code
     *
     * @return const std::vector<uint8_t>& Bytes vector containing the NAL
     */
    const std::vector<uint8_t>& getCurrentNAL() const;

    /**
     * @brief Compute the Picture Order Count for the current image
     *
     * @param TopFieldOrderCnt Computed Top Field Order Count
     * @param BottomFieldOrderCnt Computed Bottom Field Order Count
     */
    void computePoc(int& TopFieldOrderCnt, int& BottomFieldOrderCnt);

  private:
    void computeSizes();
    int computeSubWidthC();
    int computeSubHeightC();

    void computePocType0(int& TopFieldOrderCnt, int& BottomFieldOrderCnt);
    void computePocType1(int& TopFieldOrderCnt, int& BottomFieldOrderCnt);
    void computePocType2(int& TopFieldOrderCnt, int& BottomFieldOrderCnt);

  private:
    h264_stream_t* m_h264Stream;
    std::vector<uint8_t> m_bitstreamData;
    uint8_t* m_dataCursor;
    int m_unprocessedDataSize;
    bool m_sizeComputed;
    SizeI m_mbPictureSize;
    SizeI m_rawPictureSize;
    SizeI m_realPictureSize;
    std::vector<uint8_t> m_currentNAL;

    // Picture Order Count
    int m_prevPicOrderCntMsb;
    int m_prevPicOrderCntLsb;
    int m_prevFrameNumOffset;
    int m_prevFrameNum;
    int m_prevMMCO;
  };
}

#endif // LOCAL_FILE_PARSER_H_