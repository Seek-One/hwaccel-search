#include <iostream>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/PresentationQueue.h>
#include <VdpWrapper/SurfaceRGBA.h>
#include <VdpWrapper/SurfaceYUV.h>
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
    vw::SurfaceYUV inputSurface(device, argv[1], sourceSize);
    vw::SurfaceRGBA outputSurface(device, screenSize);
    vw::VideoMixer mixer(device, sourceSize);

    // Render the first image
    mixer.process(inputSurface, outputSurface);
    presentationQueue.enqueue(outputSurface);

    while (display.isOpened()) {
        display.processEvent();

        // Update the output surface
        outputSurface.resize(display.getScreenSize());
        mixer.process(inputSurface, outputSurface);
        presentationQueue.enqueue(outputSurface);
    }

    return 0;
}
