#include <VdpWrapper/Device.h>

#include <array>
#include <iostream>
#include <stdexcept>

#include <vdpau/vdpau_x11.h>

#include <VdpWrapper/Display.h>
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
            auto szError = getVdpFunctions()->getErrorString(vdpStatus);
            throw std::runtime_error("[Device] VDPAU device creation failed: " + szError);
        }

        // Initialize the VdpFunctionsInstance
        initializeVdpFunctions(m_VdpDevice, pGetProcAddress);

        const char *szVdpInfos = nullptr;
        vdpStatus = getVdpFunctions()->getInformationString(&szVdpInfos);
        if (vdpStatus != VDP_STATUS_OK) {
            auto szError = getVdpFunctions()->getErrorString(vdpStatus);
            throw std::runtime_error("[Device] Couldn't retrive VDPAU device inforamtions:  " + szError);
        }

        std::cout << "[Device] VDPAU device created" << std::endl;
        std::cout << "[Device] VDPAU version: " << std::string(szVdpInfos) << std::endl;
    }

    Device::~Device() {
        getVdpFunctions()->deviceDestroy(m_VdpDevice);
    }
}
