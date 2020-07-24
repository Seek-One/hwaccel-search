#ifndef VW_PRESENTATION_QUEUE_H
#define VW_PRESENTATION_QUEUE_H

#include <vdpau/vdpau.h>

namespace vw {
    class Device;
    class Display;

    class PresentationQueue {
    public:
        PresentationQueue(Display& display, Device &device);
        ~PresentationQueue();

        PresentationQueue(const PresentationQueue&) = delete;
        PresentationQueue(PresentationQueue&&) = delete;

        PresentationQueue& operator=(const PresentationQueue&) = delete;
        PresentationQueue& operator=(PresentationQueue&&) = delete;

    private:
        VdpPresentationQueueTarget m_vdpQueueTarget;
        VdpPresentationQueue m_vdpQueue;
    };
}

#endif // VW_PRESENTATION_QUEUE_H
