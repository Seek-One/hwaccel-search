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

#ifndef VW_VDP_FUNCTIONS_H
#define VW_VDP_FUNCTIONS_H

#include <string>

#include <vdpau/vdpau.h>
#include <vdpau/vdpau_x11.h>

namespace vw {
    /**
     * @brief VdpFunctions initializes all API function pointers
     *
     * This class holds all function pointers provide by VDPAU API.
     */
    class VdpFunctions {
    public:
        /**
         * @brief Construct a new VdpFunctions
         *
         * @param vdpDevice A valid handle of VdpDevice
         * @param pGetProcAddress Function pointer created by vdp_device_create_x11
         */
        VdpFunctions(VdpDevice& vdpDevice, VdpGetProcAddress* pGetProcAddress);
        ~VdpFunctions() = default;

        VdpFunctions(const VdpFunctions&) = delete;
        VdpFunctions(VdpFunctions&&) = delete;

        VdpFunctions& operator=(const VdpFunctions&) = delete;
        VdpFunctions& operator=(VdpFunctions&&) = delete;

        /**
         * @brief Get the error string of specified VdpStatus
         *
         * @param status Status of VDPAU function call
         * @return std::string The error string
         */
        std::string getErrorString(VdpStatus status) const;

        /**
         * @brief Throw an explicit exception on fail of VDPAU function call
         *
         * @param vdpStatus Status of VDPAU function call
         * @param message Contextual error message
         */
        void throwExceptionOnFail(VdpStatus vdpStatus, const std::string& message);

        /*
         * All this fields are function pointers initalized by the constructor
         */
        VdpGetInformationString* getInformationString;
        VdpDeviceDestroy* deviceDestroy;
        VdpDecoderCreate* decoderCreate;
        VdpDecoderDestroy* decoderDestroy;
        VdpDecoderRender* decoderRender;
        VdpOutputSurfaceCreate* outputSurfaceCreate;
        VdpOutputSurfaceDestroy* outputSurfaceDestroy;
        VdpOutputSurfacePutBitsNative* outputSurfacePutBitsNative;
        VdpOutputSurfaceGetBitsNative* outputSurfaceGetBitsNative;
        VdpOutputSurfaceGetParameters* outputSurfaceGetParameters;
        VdpVideoSurfaceCreate* videoSurfaceCreate;
        VdpVideoSurfaceDestroy* videoSurfaceDestroy;
        VdpVideoSurfacePutBitsYCbCr* videoSurfacePutBitsYCbCr;
        VdpVideoSurfaceGetBitsYCbCr* videoSurfaceGetBitsYCbCr;
        VdpVideoSurfaceGetParameters* videoSurfaceGetParameters;
        VdpVideoMixerCreate* videoMixerCreate;
        VdpVideoMixerDestroy* videoMixerDestroy;
        VdpVideoMixerRender* videoMixerRender;
        VdpPresentationQueueTargetCreateX11* presentationQueueTargetCreateX11;
        VdpPresentationQueueTargetDestroy* presentationQueueTargetDestroy;
        VdpPresentationQueueCreate* presentationQueueCreate;
        VdpPresentationQueueDestroy* presentationQueueDestroy;
        VdpPresentationQueueSetBackgroundColor* presentationQueueSetBackgroundColor;
        VdpPresentationQueueGetTime* presentationQueueGetTime;
        VdpPresentationQueueDisplay* presentationQueueDisplay;
        VdpPresentationQueueQuerySurfaceStatus* presentationQueueQuerySurfaceStatus;

    private:
        void storeFunction(VdpDevice& vdpDevice, VdpFuncId functionID);

    private:
        VdpGetProcAddress* m_pGetProcAddress;
        VdpGetErrorString* m_pGetErrorString;
    };

    /**
     * @brief VdpFunctionsInstance is a singleton of VdpFunctions
     *
     * This class aims to provide access at the VDPAU fonctions for all classes.
     * The gVdpFunctionsInstance is created during the creation of a Device object et
     * disposed when it's destroyed.
     */
    class VdpFunctionsInstance {
    public:
        /**
         * @brief Initialize the sigleton with a null pointer
         */
        VdpFunctionsInstance();

        /**
         * @brief Release the singleton
         */
        ~VdpFunctionsInstance();

        VdpFunctionsInstance(const VdpFunctionsInstance&) = delete;
        VdpFunctionsInstance(VdpFunctionsInstance&&) = delete;

        VdpFunctionsInstance& operator=(const VdpFunctionsInstance&) = delete;
        VdpFunctionsInstance& operator=(VdpFunctionsInstance&&) = delete;

        /**
         * @brief Effective instantiation of VdpFunctions object
         *
         * @param vdpDevice A valid handle of VdpDevice
         * @param pGetProcAddress Function pointer created by vdp_device_create_x11
         */
        void create(VdpDevice& vdpDevice, VdpGetProcAddress* pGetProcAddress);
        /**
         * @brief Destroy the instance of VdpFunctions
         */
        void dispose();

        /**
         * @brief Accessor to the instance of VdpFunctions
         *
         * @return VdpFunctions* The pointer to VdpFunctions instance
         */
        VdpFunctions* operator()();

    private:
        VdpFunctions *m_pVdpFunction;
    };

    extern VdpFunctionsInstance gVdpFunctionsInstance; ///< Global VdpFunctionsInstance singleton access
}

#endif // VW_VDP_FUNCTIONS_H
