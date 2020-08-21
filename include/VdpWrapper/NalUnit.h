#ifndef VW_NAL_UNIT_H
#define VW_NAL_UNIT_H

#include <vector>

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

    enum class NalReferenceType {
        NoReference                   = 0x0,
        TopReference                  = 0x1,
        BottomReference               = 0x2,
        FrameReference                = TopReference | BottomReference,
    };

    struct PictureOrderCount {
        PictureOrderCount();

        // Context part
        struct {
            int lsb;
            int msb;
        } prevPicOrderCnt;
        unsigned prevFrameNum;
        unsigned prevFrameNumOffset;
        int prevRefPictureTFOC;
        int prevRefPictureIsBottomField;

        // Compute part
        int iPictureOrderCount;
        int iTopFieldOrderCount;
        int iBottomFieldOrderCount;
    };

    struct H264Infos : public VdpPictureInfoH264 {
        H264Infos();

        PictureOrderCount poc;

        bool bFirstSPSReceived;
        bool bFirstPPSReceived;

        vw::SizeU videoSize;
        int iProfile;
        NalReferenceType referenceType;
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
