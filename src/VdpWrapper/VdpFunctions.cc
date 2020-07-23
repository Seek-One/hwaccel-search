#include <VdpWrapper/VdpFunctions.h>

#include <stdexcept>
#include <string>

#include <VdpWrapper/Device.h>

namespace vw {
    VdpFunctions::VdpFunctions(VdpDevice& vdpDevice, VdpGetProcAddress* pGetProcAddress)
    : getInformationString(nullptr)
    , deviceDestroy(nullptr)
    , m_pGetProcAddress(pGetProcAddress)
    , m_pGetErrorString(nullptr) {
        storeFunction(vdpDevice, VDP_FUNC_ID_GET_INFORMATION_STRING);
        storeFunction(vdpDevice, VDP_FUNC_ID_GET_INFORMATION_STRING);
        storeFunction(vdpDevice, VDP_FUNC_ID_DEVICE_DESTROY);
    }

    std::string&& VdpFunctions::getErrorString(VdpStatus status) const {
        return std::move(std::string(getErrorString(status)));
    }

    void VdpFunctions::storeFunction(VdpDevice& vdpDevice, VdpFuncId functionID) {
        // Get the callback
        void* func = nullptr;
        VdpStatus ret = m_pGetProcAddress(vdpDevice, functionID, &func);
        if (ret != VDP_STATUS_OK) {
            throw std::runtime_error("[VdpFunctions] Error getting the #" + std::to_string(functionID) + " callback");
        }

        // Store the callback
        switch (functionID) {
            case VDP_FUNC_ID_GET_ERROR_STRING:
                m_pGetErrorString = reinterpret_cast<VdpGetErrorString*>(func);
                break;

            case VDP_FUNC_ID_GET_INFORMATION_STRING:
                getInformationString = reinterpret_cast<VdpGetInformationString*>(func);
                break;

            case VDP_FUNC_ID_DEVICE_DESTROY:
                deviceDestroy = reinterpret_cast<VdpDeviceDestroy*>(func);
                break;

            default:
                break;
        }
    }
}
