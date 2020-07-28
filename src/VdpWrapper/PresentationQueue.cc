#include <VdpWrapper/PresentationQueue.h>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/VdpFunctions.h>
#include <VdpWrapper/SurfaceRGBA.h>

namespace vw {
    PresentationQueue::PresentationQueue(Display& display, Device &device) {
        VdpStatus vdpStatus = gVdpFunctionsInstance()->presentationQueueTargetCreateX11(
            device.m_VdpDevice,
            display.m_XWindow,
            &m_vdpQueueTarget
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't create the queue target");

        vdpStatus = gVdpFunctionsInstance()->presentationQueueCreate(
            device.m_VdpDevice,
            m_vdpQueueTarget,
            &m_vdpQueue
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't create the queue");
    }

    PresentationQueue::~PresentationQueue() {
        gVdpFunctionsInstance()->presentationQueueDestroy(m_vdpQueue);
        gVdpFunctionsInstance()->presentationQueueTargetDestroy(m_vdpQueueTarget);
    }


    bool PresentationQueue::enqueue(SurfaceRGBA &surface) {
        VdpTime currentTime = 0;
        VdpStatus vdpStatus = gVdpFunctionsInstance()->presentationQueueGetTime(
            m_vdpQueue,
            &currentTime
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't get the presentation time");

        vdpStatus = gVdpFunctionsInstance()->presentationQueueDisplay(
            m_vdpQueue,
            surface.m_vdpOutputSurface,
            0,
            0,
            currentTime
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't display the sufrace");

        return true;
    }
}
