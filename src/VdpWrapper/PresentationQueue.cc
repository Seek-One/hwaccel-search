#include <VdpWrapper/PresentationQueue.h>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/VdpFunctions.h>

namespace vw {
    PresentationQueue::PresentationQueue(Display& display, Device &device) {
        VdpStatus vdpStatus;

        vdpStatus = gVdpFunctionsInstance()->presentationQueueTargetCreateX11(
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
}
