#include <VdpWrapper/VideoMixer.h>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/SurfaceRGBA.h>
#include <VdpWrapper/SurfaceYUV.h>
#include <VdpWrapper/VdpFunctions.h>

namespace vw {
    VideoMixer::VideoMixer(Device& device)
    : m_device(device)
    , m_mixer(VDP_INVALID_HANDLE) {

    }

    VideoMixer::~VideoMixer() {
        if (m_mixer != VDP_INVALID_HANDLE) {
            gVdpFunctionsInstance()->videoMixerDestroy(m_mixer);
        }
    }

    void VideoMixer::process(SurfaceYUV &inputSurface, SurfaceRGBA &outputSurface) {
        if (m_mixer == VDP_INVALID_HANDLE) {
            createMixer(inputSurface.getSize());
        }

        VdpStatus vdpStatus = gVdpFunctionsInstance()->videoMixerRender(
            m_mixer,
            // Background
            VDP_INVALID_HANDLE, // No background surface
            nullptr,

            VDP_VIDEO_MIXER_PICTURE_STRUCTURE_FRAME, // Entire frame

            // Past surfaces
            0, // No past surface
            nullptr,

            // Current frame
            inputSurface.m_vdpVideoSurface,

            // Future surfaces
            0, // No future surfaces
            nullptr,

            nullptr, // Work on the entire surface
            outputSurface.m_vdpOutputSurface,

            // Render on the entire sufrace
            nullptr,
            nullptr,

            // Layer
            0, // No layer
            nullptr
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[VideoMixer] Couldn't render the surface");
    }

    void VideoMixer::createMixer(SizeU size) {
        VdpVideoMixerParameter parameterList[2] = {
            VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_WIDTH,
            VDP_VIDEO_MIXER_PARAMETER_VIDEO_SURFACE_HEIGHT
        };

        void* parameterValues[2] = {
            &size.width,
            &size.height
        };

        VdpStatus vdpStatus = gVdpFunctionsInstance()->videoMixerCreate(
            m_device.m_VdpDevice,
            0, // No features
            nullptr,
            2,
            parameterList,
            parameterValues,
            &m_mixer
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[VideoMixer] Couldn't create the video mixer");
    }
}
