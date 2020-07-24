#include <cassert>
#include <csignal>
#include <iostream>
#include <thread>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/PresentationQueue.h>
#include <VdpWrapper/SurfaceRGBA.h>

namespace {
    std::sig_atomic_t gMustExit = 0;
}

void trap_sig(int sig) {
    assert(sig == SIGINT || sig == SIGTERM);

    gMustExit = 1;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Incorrect parameter" << std::endl;
        std::cerr << "Usage" << std::endl;
        std::cerr << "\t" << std::string(argv[0]) << " IMAGE_FILE" << std::endl;

        return 1;
    }

    std::signal(SIGINT, trap_sig);
    std::signal(SIGTERM, trap_sig);

    vw::SizeU screenSize(1280, 720);

    vw::Display display(screenSize.width, screenSize.height);
    vw::Device device(display);
    vw::PresentationQueue presentationQueue(display, device);
    vw::SurfaceRGBA surface(device, screenSize);

    while (!gMustExit) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }

    return 0;
}
