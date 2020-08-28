#include <iostream>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/PresentationQueue.h>
#include <VdpWrapper/RenderSurface.h>
#include <VdpWrapper/DecodedSurface.h>
#include <VdpWrapper/VideoMixer.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect parameter" << std::endl;
        std::cerr << "Usage" << std::endl;
        std::cerr << "\t" << std::string(argv[0]) << " IMAGE_FILE" << std::endl;

        return 1;
    }

    vw::SizeU screenSize(1280, 720);
    vw::SizeU sourceSize(1920, 1080);

    vw::Display display(screenSize);
    vw::Device device(display);
    vw::PresentationQueue presentationQueue(display, device);
    vw::DecodedSurface inputSurface(device, argv[1], sourceSize);
    vw::VideoMixer mixer(device, screenSize);

    while (display.isOpened()) {
        // Update the output surface
        mixer.setOutputSize(display.getScreenSize());
        vw::RenderSurface outputSurface = mixer.process(inputSurface);
        presentationQueue.enqueue(std::move(outputSurface));

        display.waitEvent();
    }

    return 0;
}
