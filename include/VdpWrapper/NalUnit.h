#ifndef VW_NAL_UNIT_H
#define VW_NAL_UNIT_H

#include <vector>
#include <stdexcept>
#include <string>

#include <vdpau/vdpau.h>

#include <VdpWrapper/Size.h>

namespace vw {
    enum class NalType {
        Unspecified                   =  0,
        CodedSliceNonIDR              =  1,
        CodedSliceDataPartitionA      =  2,
        CodedSliceDataPartitionB      =  3,
        CodedSliceDataPartitionC      =  4,
        CodedSliceIDR                 =  5,
        SEI                           =  6,
        SPS                           =  7,
        PPS                           =  8,
        AUD                           =  9,
        EndOfSequence                 = 10,
        EndOfStream                   = 11,
        Filler                        = 12,
        SPSExtension                  = 13,
        PrefixNAL                     = 14,
        SubsetSPS                     = 15,
        DPS                           = 16,
        CodedSliceAUX                 = 19,
        CodedSliceSVCExtension        = 20,
    };

    enum class PictureReferenceType {
        NoReference                   = 0x0,
        TopReference                  = 0x1,
        BottomReference               = 0x2,
        FrameReference                = TopReference | BottomReference,
    };

    enum class SliceType {
        P,
        B,
        I,
        SP,
        SI,
    };

    inline constexpr SliceType getSliceType(int value) {
        switch(value) {
        case 0:
        case 5:
            return SliceType::P;

        case 1:
        case 6:
            return SliceType::B;

        case 2:
        case 7:
            return SliceType::I;

        case 3:
        case 8:
            return SliceType::SP;

        case 4:
        case 9:
            return SliceType::SI;

        default:
            throw std::runtime_error("[SliceType] Unkown slice type");
        }
    }

    inline std::string sliceToString(SliceType sliceType) {
        switch (sliceType) {
        case SliceType::P:
            return "P slice";

        case SliceType::B:
            return "B slice";

        case SliceType::I:
            return "I slice";

        case SliceType::SP:
            return "SP slice";

        case SliceType::SI:
            return "SI slice";
        }

        return "";
    }

    struct H264Infos : public VdpPictureInfoH264 {
        H264Infos();

        // For decoder creation
        int iProfile;

        // For surfaces creation
        vw::SizeU pictureSize;

        // For decoding process
        bool bFirstSPSReceived;
        bool bFirstPPSReceived;
        PictureReferenceType referenceType;
    };

    class NalUnit {
    public:
        NalUnit();
        NalUnit(const H264Infos& h264Infos, NalType type, const std::vector<uint8_t>& data);

        NalType getType() const;
        const H264Infos& getH264Infos() const;
        const std::vector<uint8_t>& getBitstream() const;

    private:
        H264Infos m_h264Infos;
        NalType m_type;
        std::vector<uint8_t> m_data;
    };
}


#endif // VW_NAL_UNIT_H
