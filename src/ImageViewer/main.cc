#include <cassert>
#include <csignal>
#include <thread>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>

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

    vw::Display display(1280, 720);
    vw::Device device(display);

    while (!gMustExit) {
        using namespace std::chrono_literals;
        std::this_thread::sleep_for(100ms);
    }

    return 0;
}
