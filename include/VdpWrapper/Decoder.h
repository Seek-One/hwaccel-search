#ifndef VW_DECODER_H
#define VW_DECODER_H

#include <vdpau/vdpau.h>

#include "DecodedPictureBuffer.h"

namespace vw {
    class Device;
    class NalUnit;
    class DecodedSurface;

    class Decoder {
    public:
        Decoder(Device& device);
        ~Decoder();

        Decoder(const Decoder&) = delete;
        Decoder(Decoder&&) = delete;

        Decoder& operator=(const Decoder&) = delete;
        Decoder& operator=(Decoder&&) = delete;

        DecodedSurface& decode(const NalUnit& nal);

    private:
        Device& m_device;
        VdpDecoder m_decoder;
        DecodedPictureBuffer m_decodedPicturesBuffer;
    };
}

#endif // VW_DECODER_H
