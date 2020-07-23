#include <VdpWrapper/VdpFunctions.h>

#include <stdexcept>
#include <string>

#include <VdpWrapper/Device.h>

namespace vw {
    VdpFunctions::VdpFunctions(VdpDevice& vdpDevice, VdpGetProcAddress* pGetProcAddress)
    : getErrorString(nullptr)
    , getInformationString(nullptr)
    , deviceDestroy(nullptr)
    , m_pGetProcAddress(pGetProcAddress) {
        storeFunction(vdpDevice, VDP_FUNC_ID_GET_INFORMATION_STRING);
        storeFunction(vdpDevice, VDP_FUNC_ID_GET_INFORMATION_STRING);
        storeFunction(vdpDevice, VDP_FUNC_ID_DEVICE_DESTROY);
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
                getErrorString = reinterpret_cast<VdpGetErrorString*>(func);
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
