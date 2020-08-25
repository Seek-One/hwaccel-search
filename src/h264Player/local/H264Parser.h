#ifndef LOCAL_H264_PARSER_H
#define LOCAL_H264_PARSER_H

#include <string>
#include <vector>

#include <h264_stream.h>

#include <VdpWrapper/NalUnit.h>

class H264Parser {
public:
    H264Parser(const std::string& filename);
    ~H264Parser();

    H264Parser(const H264Parser&) = delete;
    H264Parser(H264Parser&&) = delete;

    H264Parser& operator=(const H264Parser&) = delete;
    H264Parser& operator=(H264Parser&&) = delete;

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
