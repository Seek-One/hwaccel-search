#include <VdpWrapper/Device.h>

#include <array>
#include <iostream>
#include <stdexcept>

#include <vdpau/vdpau_x11.h>

#include <VdpWrapper/Display.h>

namespace vw {
    Device::Device(Display& display)
    : m_funcGetProcAddress(nullptr)
    , m_funcGetErrorString(nullptr)
    , m_funcGetInformationString(nullptr)
    , m_funcDeviceDestroy(nullptr) {
        // Create the device
        VdpStatus vdpStatus = vdp_device_create_x11(
            display.getXDisplay(),
            display.getXScreen(),
            &m_VdpDevice,
            &m_funcGetProcAddress
        );

        if (vdpStatus != VDP_STATUS_OK) {
            throw std::runtime_error("[Device] VDPAU device creation failed");
        }

        // Store callback
        storeFunction(VDP_FUNC_ID_GET_INFORMATION_STRING);
        storeFunction(VDP_FUNC_ID_GET_INFORMATION_STRING);
        storeFunction(VDP_FUNC_ID_DEVICE_DESTROY);

        const char *szVdpInfos = nullptr;
        vdpStatus = m_funcGetInformationString(&szVdpInfos);
        if (vdpStatus != VDP_STATUS_OK) {
            throw std::runtime_error("[Device] Couldn't retrive VDPAU device inforamtions");
        }

        std::cout << "[Device] VDPAU device created: " << std::string(szVdpInfos) << std::endl;
    }

    Device::~Device() {
        if (m_funcDeviceDestroy) {
            m_funcDeviceDestroy(m_VdpDevice);
        }
    }

    void Device::storeFunction(VdpFuncId functionID) {
        // Get the callback
        void* func = nullptr;
        VdpStatus ret = m_funcGetProcAddress(m_VdpDevice, functionID, &func);
        if (ret != VDP_STATUS_OK) {
            throw std::runtime_error("[Device] Error getting the #" + std::to_string(functionID) + " callback");
        }

        // Store the callback
        switch (functionID) {
            case VDP_FUNC_ID_GET_ERROR_STRING:
                m_funcGetErrorString = reinterpret_cast<VdpGetErrorString*>(func);
                break;

            case VDP_FUNC_ID_GET_INFORMATION_STRING:
                m_funcGetInformationString = reinterpret_cast<VdpGetInformationString*>(func);
                break;

            case VDP_FUNC_ID_DEVICE_DESTROY:
                m_funcDeviceDestroy = reinterpret_cast<VdpDeviceDestroy*>(func);
                break;

            default:
                break;
        }
    }
}
