#include <VdpWrapper/Device.h>

#include <array>
#include <iostream>
#include <stdexcept>

#include <vdpau/vdpau_x11.h>

#include <VdpWrapper/Display.h>
#include <VdpWrapper/RenderSurface.h>
#include <VdpWrapper/VdpFunctions.h>

namespace vw {
    Device::Device(Display& display) {
        // Create the device
        VdpGetProcAddress* pGetProcAddress = nullptr;
        VdpStatus vdpStatus = vdp_device_create_x11(
            display.getXDisplay(),
            display.getXScreen(),
            &m_VdpDevice,
            &pGetProcAddress
        );
        if (vdpStatus != VDP_STATUS_OK) {
            throw std::runtime_error("[Device] VDPAU device creation failed\n\tError code: " + std::to_string(vdpStatus));
        }

        // Initialize the VdpFunctionsInstance
        gVdpFunctionsInstance.create(m_VdpDevice, pGetProcAddress);

        const char *szVdpInfos = nullptr;
        vdpStatus = gVdpFunctionsInstance()->getInformationString(&szVdpInfos);
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[Device] Couldn't retrive VDPAU device inforamtions");

        std::cout << "[Device] VDPAU device created" << std::endl;
        std::cout << "[Device] VDPAU version: " << std::string(szVdpInfos) << std::endl;
    }

    Device::~Device() {
        gVdpFunctionsInstance()->deviceDestroy(m_VdpDevice);
        gVdpFunctionsInstance.dispose();
    }

    VdpDevice Device::getVdpHandle() const {
        return m_VdpDevice;
    }
}
