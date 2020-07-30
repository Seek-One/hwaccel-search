#include <iostream>

#include <VdpWrapper/Display.h>
#include <VdpWrapper/Decoder.h>
#include <VdpWrapper/Device.h>
#include <VdpWrapper/NalUnit.h>
#include <VdpWrapper/Size.h>

#include "local/H264Parser.h"

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Missing parameter" << std::endl;
        std::cerr << "Usage:" << std::endl;
        std::cerr << "\t" << std::string(argv[0]) << " BITSTREAM_FILE" << std::endl;

        return 1;
    }

    vw::SizeU screenSize(1280, 720);
    vw::Display display(screenSize);
    vw::Device device(display);
    vw::Decoder decoder(device);

    H264Parser parser(argv[1]);
    vw::NalUnit nalUnit;

    while (parser.readNextNAL(nalUnit)) {
        switch (nalUnit.getType()) {
        case vw::NalType::SPS:
        case vw::NalType::PPS:
        case vw::NalType::CodedSliceIDR:
            decoder.decode(nalUnit);
            break;

        case vw::NalType::Unspecified:
            std::cerr << "[main] Unknown NAL type" << std::endl;
            break;

        default:
            std::cout << "[main] Unhandled NAL type: " << static_cast<int>(nalUnit.getType()) << std::endl;
        }
    }

    return 0;
}
