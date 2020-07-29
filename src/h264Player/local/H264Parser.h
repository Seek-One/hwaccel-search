#ifndef LOCAL_H264_PARSER_H
#define LOCAL_H264_PARSER_H

#include <string>
#include <vector>

#include <h264_stream.h>

class H264Parser {
public:
    H264Parser(const std::string& filename);
    ~H264Parser();

    H264Parser(const H264Parser&) = delete;
    H264Parser(H264Parser&&) = delete;

    H264Parser& operator=(const H264Parser&) = delete;
    H264Parser& operator=(H264Parser&&) = delete;

    bool readNextNAL();

private:
    h264_stream_t* m_h264Stream;
    std::vector<uint8_t> m_bitstream;
    uint8_t* m_pDataCursor;
    int m_unprocessedDataSize;

};

#endif // LOCAL_H264_PARSER_H
