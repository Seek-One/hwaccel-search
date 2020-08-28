#include <iostream>

#include <VdpWrapper/Display.h>
#include <VdpWrapper/Decoder.h>
#include <VdpWrapper/Device.h>
#include <VdpWrapper/NalUnit.h>
#include <VdpWrapper/PresentationQueue.h>
#include <VdpWrapper/Size.h>
#include <VdpWrapper/RenderSurface.h>
#include <VdpWrapper/DecodedSurface.h>
#include <VdpWrapper/VideoMixer.h>

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
    vw::PresentationQueue presentationQueue(display, device);
    presentationQueue.setFramerate(24);
    vw::VideoMixer mixer(device, screenSize);

    H264Parser parser(argv[1]);
    vw::NalUnit nalUnit;

    while (parser.readNextNAL(nalUnit) && display.isOpened()) {
        // Handle XEvent
        display.processEvent();
        mixer.setOutputSize(display.getScreenSize());

        switch (nalUnit.getType()) {
        case vw::NalType::SPS:
        case vw::NalType::PPS:
        case vw::NalType::SEI:
            // Nothing to do
            break;

        case vw::NalType::CodedSliceNonIDR:
        case vw::NalType::CodedSliceIDR: {
            vw::DecodedSurface& decodedSurface = decoder.decode(nalUnit);
            vw::RenderSurface outputSurface = mixer.process(decodedSurface);
            presentationQueue.enqueue(std::move(outputSurface));
            break;
        }

        case vw::NalType::Unspecified:
            std::cerr << "[main] Unknown NAL type" << std::endl;
            break;

        default:
            std::cout << "[main] Unhandled NAL type: " << static_cast<int>(nalUnit.getType()) << std::endl;
        }
    }

    std::cout << "[main] End of parsing" << std::endl;

    while (display.isOpened()) {
        display.waitEvent();
    }

    return 0;
}
