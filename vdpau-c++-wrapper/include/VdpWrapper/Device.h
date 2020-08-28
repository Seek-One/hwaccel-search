/* Copyright (c) 2020 Jet1oeil
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

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
