/* Copyright (c) 2020 Jet1oeil
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

#ifndef LOCAL_H264_PARSER_H
#define LOCAL_H264_PARSER_H

#include <string>
#include <vector>

#include <h264_stream.h>

#include <VdpWrapper/NalUnit.h>

/**
 * @brief H264Parser is wrapper around h264bitstream library
 */
class H264Parser {
public:
    /**
     * @brief Construct a new H264Parser
     *
     * @param filename Filename of bitstream
     */
    H264Parser(const std::string& filename);
    ~H264Parser();

    H264Parser(const H264Parser&) = delete;
    H264Parser(H264Parser&&) = delete;

    H264Parser& operator=(const H264Parser&) = delete;
    H264Parser& operator=(H264Parser&&) = delete;

    /**
     * @brief Read the next NAL unit and update H264 informations
     *
     * The parser keeps the H264 informations to be able to
     * update the vw::NalUnit object.
     *
     * @param nalUnit A NAL unit filled by the bitstream informations
     * @return true If a NAL unit has been parsed
     * @return false Otherwise
     */
    bool readNextNAL(vw::NalUnit &nalUnit);

private:
    void updateH264Infos();
    int computeSubWidthC() const;
    int computeSubHeightC() const;
    void computePicutreSize();

    void computePoc();
    void computePocType0();
    void computePocType1();
    void computePocType2();

private:
    h264_stream_t* m_h264Stream;
    vw::H264Infos m_h264Infos;
    std::vector<uint8_t> m_bitstream;
    uint8_t* m_pDataCursor;
    int m_unprocessedDataSize;

    // Picture Order Count
    int m_prevPicOrderCntMsb;
    int m_prevPicOrderCntLsb;
    int m_prevFrameNumOffset;
    int m_prevFrameNum;
    int m_iPrevMMCO;
};

#endif // LOCAL_H264_PARSER_H
