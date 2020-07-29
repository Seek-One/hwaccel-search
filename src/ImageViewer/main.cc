#include <iostream>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/PresentationQueue.h>
#include <VdpWrapper/SurfaceYUV.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect parameter" << std::endl;
        std::cerr << "Usage" << std::endl;
        std::cerr << "\t" << std::string(argv[0]) << " IMAGE_FILE" << std::endl;

        return 1;
    }

    vw::SizeU screenSize(1920, 1080);

    vw::Display display(screenSize);
    vw::Device device(display);
    vw::PresentationQueue presentationQueue(display, device);
    vw::SurfaceYUV surface(device, argv[1], screenSize);

    while (display.isOpened()) {
        display.processEvent();
    }

    return 0;
}
