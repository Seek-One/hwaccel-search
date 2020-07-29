#include "H264Parser.h"

#include <fstream>
#include <iterator>
#include <stdexcept>

H264Parser::H264Parser(const std::string& filename)
: m_h264Stream(h264_new())
, m_pDataCursor(nullptr)
, m_unprocessedDataSize(0) {
    std::ifstream bitstreamFile(filename, std::ios::binary);
    if (!bitstreamFile.good()) {
        throw std::runtime_error("[H264Parser] Couldn't open the bitstream file");
    }
    bitstreamFile.unsetf(std::ios::skipws);

    // Load all data
    m_bitstream.insert(m_bitstream.begin(), std::istream_iterator<uint8_t>(bitstreamFile), std::istream_iterator<uint8_t>());
    m_pDataCursor = m_bitstream.data();
    m_unprocessedDataSize = m_bitstream.size();
}

H264Parser::~H264Parser() {
    h264_free(m_h264Stream);
}

bool H264Parser::readNextNAL() {
    int iNalStart = 0;
    int iNalEnd = 0;

    if (find_nal_unit(m_pDataCursor, m_unprocessedDataSize, &iNalStart, &iNalEnd) <= 0) {
        return false;
    }

    // Print debug
    m_pDataCursor += iNalStart;
    read_debug_nal_unit(m_h264Stream, m_pDataCursor, iNalEnd - iNalStart);

    // Skip to next NAL
    m_pDataCursor += (iNalEnd - iNalStart);
    m_unprocessedDataSize -= iNalEnd;

    return true;
}
