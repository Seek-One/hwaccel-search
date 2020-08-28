#ifndef VW_DECODER_H
#define VW_DECODER_H

#include <vdpau/vdpau.h>

#include "DecodedPictureBuffer.h"

namespace vw {
    class Device;
    class NalUnit;
    class DecodedSurface;

    /**
     * @brief Decoder class encapsules a VdpDecoder
     *
     * This class aims to wrap the call to VDPAU API to handle a VdpDecoder. The class
     * respect the RAII programming idiom hence when a new object is created is owning
     * a VdpVideoSurface and when it is destroyed the Decoder is freed.
     */
    class Decoder {
    public:
        /**
         * @brief Construct a new Decoder object
         *
         * @param device A reference to a valid Device
         */
        Decoder(Device& device);
        /**
         * @brief Destroy the Decoder object
         */
        ~Decoder();

        Decoder(const Decoder&) = delete;
        Decoder(Decoder&&) = delete;

        Decoder& operator=(const Decoder&) = delete;
        Decoder& operator=(Decoder&&) = delete;

        /**
         * @brief Send a nal unit to be decoded
         *
         * The function expects that the nal unit is a coded slice (not a SPS, PPS or SEI nal)
         *
         * @param nal A coded slice nal
         * @return DecodedSurface& A reference on the new decoded surface
         */
        DecodedSurface& decode(const NalUnit& nal);

    private:
        Device& m_device;
        VdpDecoder m_decoder;
        DecodedPictureBuffer m_decodedPicturesBuffer;
    };
}

#endif // VW_DECODER_H
