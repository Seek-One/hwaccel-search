#ifndef VW_DECODER_H
#define VW_DECODER_H

#include <vdpau/vdpau.h>

namespace vw {
    class Device;
    class NalUnit;
    class SurfaceYUV;

    class Decoder {
    public:
        Decoder(Device& device);
        ~Decoder();

        Decoder(const Decoder&) = delete;
        Decoder(Decoder&&) = delete;

        Decoder& operator=(const Decoder&) = delete;
        Decoder& operator=(Decoder&&) = delete;

        SurfaceYUV decode(const NalUnit& nal);

    private:
        Device& m_device;
        VdpDecoder m_decoder;
    };
}

#endif // VW_DECODER_H
