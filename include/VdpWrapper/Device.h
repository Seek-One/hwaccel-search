#ifndef VW_DEVICE_H
#define VW_DEVICE_H

#include <vdpau/vdpau.h>

namespace vw {
    class Display;

    /**
     * @brief Device encapsules a VdpDevice
     *
     * This class aims to wrap the call to VDPAU API to handle a VdpDevice. The class
     * respect the RAII programming idiom hence when a new object is created is owning
     * a VdpVideoSurface and when it is destroyed the VdpDevice is freed.
     *
     * Moreover, the class will instantiate the VdpFunctions singleton for the other
     * classes which use VDPAU API. When Device is destroyed, the VdpFunctions singelton
     * come back to an invalid state.
     *
     * @todo Change the VdpFunctions singleton by an overriding operator->.
     */
    class Device {
    public:
        /**
         * @brief Construct a new Device object
         *
         * @param display The reference of current Display
         */
        Device(Display& display);

        /**
         * @brief Destroy the Device object
         */
        ~Device();

        Device(const Device&) = delete;
        Device(Device&&) = delete;

        Device& operator=(const Device&) = delete;
        Device& operator=(Device&&) = delete;

        /**
         * @brief Get the opaque VDPAU handle value
         *
         * @return VdpDevice The opaque VDPAU handle value
         */
        VdpDevice getVdpHandle() const;

    private:
        VdpDevice m_VdpDevice;
    };
}

#endif // VW_DEVICE_H
