#ifndef VW_VDP_FUNCTIONS_H
#define VW_VDP_FUNCTIONS_H

#include <string>

#include <vdpau/vdpau.h>
#include <vdpau/vdpau_x11.h>

namespace vw {
    class VdpFunctions {
    public:
        VdpFunctions(VdpDevice& vdpDevice, VdpGetProcAddress* pGetProcAddress);
        ~VdpFunctions() = default;

        VdpFunctions(const VdpFunctions&) = delete;
        VdpFunctions(VdpFunctions&&) = delete;

        VdpFunctions& operator=(const VdpFunctions&) = delete;
        VdpFunctions& operator=(VdpFunctions&&) = delete;

        std::string getErrorString(VdpStatus status) const;
        void throwExceptionOnFail(VdpStatus vdpStatus, const std::string& message);

        VdpGetInformationString* getInformationString;
        VdpDeviceDestroy* deviceDestroy;
        VdpDecoderCreate* decoderCreate;
        VdpDecoderDestroy* decoderDestroy;
        VdpOutputSurfaceCreate* outputSurfaceCreate;
        VdpOutputSurfaceDestroy* outputSurfaceDestroy;
        VdpOutputSurfacePutBitsNative* outputSurfacePutBitsNative;
        VdpOutputSurfaceGetParameters* outputSurfaceGetParameters;
        VdpVideoSurfaceCreate* videoSurfaceCreate;
        VdpVideoSurfaceDestroy* videoSurfaceDestroy;
        VdpVideoSurfacePutBitsYCbCr* videoSurfacePutBitsYCbCr;
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
