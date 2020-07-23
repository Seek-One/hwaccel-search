#include <cassert>
#include <csignal>
#include <thread>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/SurfaceRGBA.h>

namespace {
    std::sig_atomic_t gMustExit = 0;
}

void trap_sig(int sig) {
    assert(sig == SIGINT || sig == SIGTERM);

    gMustExit = 1;
}

int main() {
    std::signal(SIGINT, trap_sig);
    std::signal(SIGTERM, trap_sig);

    vw::SizeU screenSize(1280, 720);

    vw::Display display(screenSize.width, screenSize.height);
    vw::Device device(display);
    vw::SurfaceRGBA surface(device, screenSize);

    while (!gMustExit) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }

    return 0;
}
