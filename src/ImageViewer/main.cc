#include <iostream>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/PresentationQueue.h>
#include <VdpWrapper/SurfaceRGBA.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect parameter" << std::endl;
        std::cerr << "Usage" << std::endl;
        std::cerr << "\t" << std::string(argv[0]) << " IMAGE_FILE" << std::endl;

        return 1;
    }

    vw::SizeU screenSize(1280, 720);

    vw::Display display(screenSize.width, screenSize.height);
    vw::Device device(display);
    vw::PresentationQueue presentationQueue(display, device);
    vw::SurfaceRGBA surface(device, argv[1]);

    presentationQueue.enqueue(surface);

    while (display.isOpened()) {
        display.processEvent();
    }

    return 0;
}
