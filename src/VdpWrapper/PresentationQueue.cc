#include <VdpWrapper/PresentationQueue.h>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/VdpFunctions.h>
#include <VdpWrapper/SurfaceRGBA.h>

namespace vw {
    PresentationQueue::PresentationQueue(Display& display, Device &device) {
        VdpStatus vdpStatus = gVdpFunctionsInstance()->presentationQueueTargetCreateX11(
            device.getVdpHandle(),
            display.getXWindow(),
            &m_vdpQueueTarget
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't create the queue target");

        vdpStatus = gVdpFunctionsInstance()->presentationQueueCreate(
            device.getVdpHandle(),
            m_vdpQueueTarget,
            &m_vdpQueue
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't create the queue");

        // Set background color
        VdpColor backgroundColor;
        backgroundColor.blue = 0.0f;
        backgroundColor.green = 0.0f;
        backgroundColor.red = 0.0f;
        backgroundColor.alpha = 1.0f;

        vdpStatus = gVdpFunctionsInstance()->presentationQueueSetBackgroundColor(
            m_vdpQueue,
            &backgroundColor
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't set the background color");
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
            surface.getVdpHandle(),
            0,
            0,
            currentTime
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't display the sufrace");

        return true;
    }
}
