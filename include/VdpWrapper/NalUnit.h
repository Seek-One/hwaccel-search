#ifndef VW_NAL_UNIT_H
#define VW_NAL_UNIT_H

#include <vector>
#include <stdexcept>
#include <string>

#include <vdpau/vdpau.h>

#include <VdpWrapper/Size.h>

namespace vw {
    /**
     * @brief Enumeration of NAL type
     */
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

    /**
     * @brief Type of picture reference
     *
     * @todo This enumeration must be rename into PictureStructureType
     * and don't include the reference information just the type of field is used
     */
    enum class PictureReferenceType {
        NoReference                   = 0x0,                                ///< The picture it's not a reference
        TopReference                  = 0x1,                                ///< The picture is a TopField
        BottomReference               = 0x2,                                ///< The picture is a BottomField
        FrameReference                = TopReference | BottomReference,     ///< The picture is a Frame
    };

    /**
     * @brief Enumeration of coded slice type
     */
    enum class SliceType {
        P,      ///< Past predictive frame (only from previous frames)
        B,      ///< Bidirectional predictive frame (form past and future frames)
        I,      ///< Intra frame (prediction only on current frame)
        SP,     ///< ???
        SI,     ///< ???
    };

    /**
     * @brief Convert the bitstream slice type value to a SliceType enumeration
     *
     * @param value Bitstream slice type value
     * @return SliceType The SliceType value
     */
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

    /**
     * @brief Convert a SliceType to string
     *
     * @param sliceType Slice type
     * @return std::string The slice type string
     */
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

    /**
     * @brief H264Infos inherits of attribute of VdpPictureInfoH264 and add
     * other inforamtions for the Decoder
     */
    struct H264Infos : public VdpPictureInfoH264 {
        /**
         * @brief Construct a new H264Infos
         *
         * Each field is initialized with default values
         */
        H264Infos();

        // For decoder creation
        int iProfile; ///< Profile number from bitstream (used by the Decoder to create an instance of VdpDecoder)

        // For surfaces creation
        vw::SizeU pictureSize; ///< Picture size (used to create DecodedSurface)

        // For decoding process
        bool bFirstSPSReceived; ///< Indicate if a SPS has been received
        bool bFirstPPSReceived; ///< Indicate if a PPS has been received
        PictureReferenceType referenceType; ///< The type of picture structure
    };

    /**
     * @brief NalUnit represents a NAL unit
     *
     * This class encapsulate a NAL unit data. A NalUnit object must be
     * CodedSlice (IDR or not).
     */
    class NalUnit {
    public:
        /**
         * @brief Construct a new empty NalUnit
         */
        NalUnit();

        /**
         * @brief Construct a new NalUnit and initialize its informations
         *
         * @param h264Infos Inforamations about H264 bitstream
         * @param type NAL type
         * @param data Coded data
         */
        NalUnit(const H264Infos& h264Infos, NalType type, const std::vector<uint8_t>& data);

        /**
         * @brief Get the NAL type
         *
         * @return NalType The nal type
         */
        NalType getType() const;
        /**
         * @brief Get the H264 inforamtions
         *
         * @return const H264Infos&
         */
        const H264Infos& getH264Infos() const;
        /**
         * @brief Get the coded data read from bitstream
         *
         * @return const std::vector<uint8_t>& The coded data
         */
        const std::vector<uint8_t>& getBitstream() const;

    private:
        H264Infos m_h264Infos;
        NalType m_type;
        std::vector<uint8_t> m_data;
    };
}


#endif // VW_NAL_UNIT_H
