#include <VdpWrapper/SurfaceRGBA.h>

#include <stdexcept>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/VdpFunctions.h>

namespace vw {
    SurfaceRGBA::SurfaceRGBA(Device& device, const SizeU& size)
    : m_vdpOutputSurface(VDP_INVALID_HANDLE)
    , m_size(size) {
        auto vdpStatus = gVdpFunctionsInstance()->outputSurfaceCreate(
            device.m_VdpDevice,
            VDP_RGBA_FORMAT_B8G8R8A8,
            size.width,
            size.height,
            &m_vdpOutputSurface
        );
        if (vdpStatus != VDP_STATUS_OK) {
            auto szError = gVdpFunctionsInstance()->getErrorString(vdpStatus);
            throw std::runtime_error("[SurfaceRGBA] Couldn't create an output surface:  " + szError);
        }
    }

    SurfaceRGBA::~SurfaceRGBA() {
        gVdpFunctionsInstance()->outputSurfaceDestroy(m_vdpOutputSurface);
    }
}
