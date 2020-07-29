#ifndef LOCAL_NAL_UNIT_H
#define LOCAL_NAL_UNIT_H

#include <h264_stream.h>

class NalUnit {
public:

private:
    nal_t m_nalHeader;
    union {
        sps_t sps;
    } rspb;
};

#endif // LOCAL_NAL_UNIT_H
