#ifndef VW_PRESENTATION_QUEUE_H
#define VW_PRESENTATION_QUEUE_H

#include <deque>

#include <vdpau/vdpau.h>

#include "RenderSurface.h"

namespace vw {
    class Device;
    class Display;

    struct QueuedSurface {
        QueuedSurface(RenderSurface surface);

        RenderSurface surface;
        int iPOC;
        VdpTime iPresentationTimeStamp;
        bool bIsEnqueued;
    };

    class PresentationQueue {
    public:
        PresentationQueue(Display& display, Device &device);
        ~PresentationQueue();

        PresentationQueue(const PresentationQueue&) = delete;
        PresentationQueue(PresentationQueue&&) = delete;

        PresentationQueue& operator=(const PresentationQueue&) = delete;
        PresentationQueue& operator=(PresentationQueue&&) = delete;

        void setFramerate(int iFPS);

        bool enqueue(RenderSurface surface);

    private:
        VdpTime getCurrentTime();

    private:
        VdpPresentationQueueTarget m_vdpQueueTarget;
        VdpPresentationQueue m_vdpQueue;
        std::deque<QueuedSurface> m_queuedSurfaces;

        // This parameters are useful to handle presentation time
        // When POC == 0, we start a new sequence. Hence, we set
        // m_beginTime to current time for the first sequence (aka
        // if m_endTime == 0) or we set m_beginTime to m_endTime +
        // m_framerateStep
        VdpTime m_beginTime;
        VdpTime m_endTime;
        VdpTime m_framerateStep;

        int m_iNextPOC;
    };
}

#endif // VW_PRESENTATION_QUEUE_H
