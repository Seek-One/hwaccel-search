#include <VdpWrapper/VideoMixer.h>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/RenderSurface.h>
#include <VdpWrapper/DecodedSurface.h>
#include <VdpWrapper/VdpFunctions.h>

namespace vw {
    VideoMixer::VideoMixer(Device& device, SizeU outputSize)
    : m_device(device)
    , m_mixer(VDP_INVALID_HANDLE)
    , m_outputSize(outputSize) {

    }

    VideoMixer::~VideoMixer() {
        if (m_mixer != VDP_INVALID_HANDLE) {
            gVdpFunctionsInstance()->videoMixerDestroy(m_mixer);
        }
    }

    void VideoMixer::setOutputSize(SizeU outputSize) {
        m_outputSize = outputSize;
    }

    RenderSurface VideoMixer::process(DecodedSurface &inputSurface) {
        if (m_mixer == VDP_INVALID_HANDLE) {
            createMixer(inputSurface.getSize());
        }

        // Create the output surface
        RenderSurface outputSurface(m_device, m_outputSize);

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
            inputSurface.getVdpHandle(),

            // Future surfaces
            0, // No future surfaces
            nullptr,

            nullptr, // Work on the entire surface
            outputSurface.getVdpHandle(),

            // Render on the entire sufrace
            nullptr,
            nullptr,

            // Layer
            0, // No layer
            nullptr
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[VideoMixer] Couldn't render the surface");

        outputSurface.setPictureOrderCount(inputSurface.getPictureOrderCount());

        return std::move(outputSurface);
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
            m_device.getVdpHandle(),
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
