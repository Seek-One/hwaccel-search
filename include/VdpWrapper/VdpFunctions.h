#ifndef VW_VDP_FUNCTIONS_H
#define VW_VDP_FUNCTIONS_H

#include <memory>

#include <vdpau/vdpau.h>

namespace vw {
    class VdpFunctions {
    public:
        VdpFunctions(VdpDevice& vdpDevice, VdpGetProcAddress* pGetProcAddress);
        ~VdpFunctions() = default;

        VdpFunctions(const VdpFunctions&) = delete;
        VdpFunctions(VdpFunctions&&) = delete;

        VdpFunctions& operator=(const VdpFunctions&) = delete;
        VdpFunctions& operator=(VdpFunctions&&) = delete;

        VdpGetErrorString* getErrorString;
        VdpGetInformationString* getInformationString;
        VdpDeviceDestroy* deviceDestroy;

    private:
        void storeFunction(VdpDevice& vdpDevice, VdpFuncId functionID);

    private:
        VdpGetProcAddress* m_pGetProcAddress;
    };

    static std::unique_ptr<vw::VdpFunctions> gVdpFunctionsInstance;

    static inline void initializeVdpFunctions(VdpDevice& vdpDevice, VdpGetProcAddress* pGetProcAddress) {
        if (gVdpFunctionsInstance) {
            throw std::runtime_error("The VDPAU functions are already initialized");
        }

        gVdpFunctionsInstance = std::make_unique<VdpFunctions>(vdpDevice, pGetProcAddress);
    }

    static inline void disposeVdpFunctions() {
        gVdpFunctionsInstance.reset();
    }

    static inline const VdpFunctions* getVdpFunctions() {
        if (!gVdpFunctionsInstance) {
            throw std::runtime_error("The VDPAU functions are not initialized. You must first create a vw::Device.");
        }

        return gVdpFunctionsInstance.get();
    }
}

#endif // VW_VDP_FUNCTIONS_H
