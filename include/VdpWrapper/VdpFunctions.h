#ifndef VW_VDP_FUNCTIONS_H
#define VW_VDP_FUNCTIONS_H

#include <string>

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

        std::string&& getErrorString(VdpStatus status) const;

        VdpGetInformationString* getInformationString;
        VdpDeviceDestroy* deviceDestroy;
        VdpOutputSurfaceCreate* outputSurfaceCreate;
        VdpOutputSurfaceDestroy* outputSurfaceDestroy;

    private:
        void storeFunction(VdpDevice& vdpDevice, VdpFuncId functionID);

    private:
        VdpGetProcAddress* m_pGetProcAddress;
        VdpGetErrorString* m_pGetErrorString;
    };

    class VdpFunctionsInstance {
    public:
        VdpFunctionsInstance();
        ~VdpFunctionsInstance();

        VdpFunctionsInstance(const VdpFunctionsInstance&) = delete;
        VdpFunctionsInstance(VdpFunctionsInstance&&) = delete;

        VdpFunctionsInstance& operator=(const VdpFunctionsInstance&) = delete;
        VdpFunctionsInstance& operator=(VdpFunctionsInstance&&) = delete;

        void create(VdpDevice& vdpDevice, VdpGetProcAddress* pGetProcAddress);
        void dispose();

        VdpFunctions* operator()();

    private:
        VdpFunctions *m_pVdpFunction;
    };

    extern VdpFunctionsInstance gVdpFunctionsInstance;
}

#endif // VW_VDP_FUNCTIONS_H
