#include <VdpWrapper/VideoMixer.h>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/VdpFunctions.h>

namespace vw {
    VideoMixer::VideoMixer(Device &device, SizeU size) {
        VdpVideoMixerParameter parameterList[2] = {
            VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH,
            VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT
        };

        void* parameterValues[2] = {
            &size.width,
            &size.height
        };

        VdpStatus vdpStatus = gVdpFunctionsInstance()->videoMixerCreate(
            device.m_VdpDevice,
            0, // No features
            nullptr,
            2,
            parameterList,
            parameterValues,
            &m_mixer
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[VideoMixer] Couldn't create the video mixer");
    }

    VideoMixer::~VideoMixer() {
        gVdpFunctionsInstance()->videoMixerDestroy(m_mixer);
    }
}
