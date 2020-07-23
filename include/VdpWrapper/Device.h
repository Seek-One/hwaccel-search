#ifndef VW_DEVICE_H
#define VW_DEVICE_H

#include <vdpau/vdpau.h>

namespace vw {
    class Display;

    class Device {
    public:
        Device(Display& display);
        ~Device();

        Device(const Device&) = delete;
        Device(Device&&) = delete;

        Device& operator=(const Device&) = delete;
        Device& operator=(Device&&) = delete;

    private:
        void storeFunction(VdpFuncId functionID);

    private:
        // VDPAU objects
        VdpDevice m_VdpDevice;

        // VDPAU function pointors
        VdpGetProcAddress* m_funcGetProcAddress;
        VdpGetErrorString* m_funcGetErrorString;
        VdpGetInformationString* m_funcGetInformationString;
        VdpDeviceDestroy* m_funcDeviceDestroy;
    };
}

#endif // VW_DEVICE_H
