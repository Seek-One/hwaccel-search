#include <VdpWrapper/Decoder.h>

#include <stdexcept>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/NalUnit.h>
#include <VdpWrapper/VdpFunctions.h>

namespace {
    VdpDecoderProfile convertBitstreamProfileToVdpProfile(int profile) {
        switch (profile) {
        case 66:
            return VDP_DECODER_PROFILE_H264_BASELINE;

        case 77:
            return VDP_DECODER_PROFILE_H264_MAIN;

        case 88:
            return VDP_DECODER_PROFILE_H264_EXTENDED;

        case 100:
            return VDP_DECODER_PROFILE_H264_HIGH;

        default:
            throw std::runtime_error("[Decoder] Profile unsupported");
        }

        return VDP_INVALID_HANDLE;
    }
}

namespace vw {
    Decoder::Decoder(Device& device)
    : m_device(device)
    , m_decoder(VDP_INVALID_HANDLE) {

    }

    Decoder::~Decoder() {
        if (m_decoder != VDP_INVALID_HANDLE) {
            gVdpFunctionsInstance()->decoderDestroy(m_decoder);
        }
    }

    void Decoder::decode(const NalUnit &nal) {
        if (nal.getType() != NalType::CodedSliceIDR) {
            return;
        }

        auto& infos = nal.getH264Infos();
        if (!infos.bFirstSPSReceived) {
            throw std::runtime_error("[Decoder] Couldn't decode picture since no SPS has been received");
        }

        if (!infos.bFirstPPSReceived) {
            throw std::runtime_error("[Decoder] Couldn't decode picture since no PPS has been received");
        }

        if (m_decoder == VDP_INVALID_HANDLE) {
            VdpDecoderProfile profile = convertBitstreamProfileToVdpProfile(infos.iProfile);
            auto vdpStatus = gVdpFunctionsInstance()->decoderCreate(
                m_device.m_VdpDevice,
                profile,
                infos.videoSize.width,
                infos.videoSize.width,
                16, // Required by the standard
                &m_decoder
            );
            gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[Decoder] Couldn't create the decoder");
        }
    }
}
