#include <VdpWrapper/PresentationQueue.h>

#include <algorithm>
#include <cmath>
#include <iostream>

#include <VdpWrapper/Device.h>
#include <VdpWrapper/Display.h>
#include <VdpWrapper/VdpFunctions.h>
#include <VdpWrapper/RenderSurface.h>

namespace vw {
    QueuedSurface::QueuedSurface(RenderSurface surface)
    : surface(std::move(surface))
    , iPOC(0)
    , iPresentationTimeStamp(0)
    , bIsEnqueued(false) {

    }

    PresentationQueue::PresentationQueue(Display& display, Device &device)
    : m_beginTime(0)
    , m_endTime(0)
    , m_framerateStep(0)
    , m_iNextPOC(0) {
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

    void PresentationQueue::setFramerate(int iFPS) {
        double framerateStepAsSeconds = 1.0 / static_cast<double>(iFPS);
        m_framerateStep = std::round(framerateStepAsSeconds * 1000000000.0);
        std::cout << "[PresentationQueue] Framerate set to: " << m_framerateStep << std::endl;
    }

    bool PresentationQueue::enqueue(RenderSurface surface) {
        // Keep the ownership of the surface
        m_queuedSurfaces.emplace_back(std::move(surface));
        auto& queuedSurface = m_queuedSurfaces.back();

        // Compute the presentation time
        VdpTime presentationTime = 0;
        // I divide the POC by 2 to have continuous values
        // I guess the POC goes up two by two because we're processing picture
        // frames and not picutures fields
        int iPOC = queuedSurface.surface.getPictureOrderCount() / 2;
        queuedSurface.iPOC = iPOC;

        // This parameters are useful to handle presentation time
        // When POC == 0, we start a new sequence. Hence, we set
        // m_beginTime to current time for the first sequence (aka
        // if m_endTime == 0) or we set m_beginTime to m_endTime
        // When POC > 1 we set the presentation time to the  m_beginTime +
        // m_framerateStep * iPOC and we updated the m_endTime if needed

        // If it's a new sequence
        if (iPOC == 0) {
            // If it's the first sequence
            if (m_endTime == 0) {
                m_beginTime = getCurrentTime();
            } else {
                m_beginTime = m_endTime;
            }
            m_endTime = m_beginTime + m_framerateStep;
            presentationTime = m_beginTime;
        } else if (iPOC > 0) {
            presentationTime = m_beginTime + m_framerateStep * iPOC;
            if (presentationTime >= m_endTime) {
                m_endTime = presentationTime + m_framerateStep;
            }
        }
        // If no POC specified
        else {
            presentationTime = getCurrentTime();
        }
        queuedSurface.iPresentationTimeStamp = presentationTime;

        // Sort queue by presentation time
        std::sort(m_queuedSurfaces.begin(), m_queuedSurfaces.end(), [](auto& lhs, auto& rhs) {
            return lhs.iPresentationTimeStamp < rhs.iPresentationTimeStamp;
        });

        // Get the first no queued surface
        auto nextSurface = std::find_if(m_queuedSurfaces.begin(), m_queuedSurfaces.end(), [](auto& queuedSurface) {
            return !queuedSurface.bIsEnqueued;
        });

        // Enqueued all possible surfaces
        while (nextSurface != m_queuedSurfaces.end() && (nextSurface->iPOC == m_iNextPOC || nextSurface->iPOC == 0)) {
            auto vdpStatus = gVdpFunctionsInstance()->presentationQueueDisplay(
                m_vdpQueue,
                nextSurface->surface.getVdpHandle(),
                0,
                0,
                nextSurface->iPresentationTimeStamp
            );
            gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't display the sufrace");
            nextSurface->bIsEnqueued = true;

            // Update the next expected POC
            if (nextSurface->iPOC == 0) {
                m_iNextPOC = 1;
            } else {
                m_iNextPOC++;
            }

            // Get the next no queued surface
            nextSurface = std::find_if(m_queuedSurfaces.begin(), m_queuedSurfaces.end(), [](auto& queuedSurface) {
                return !queuedSurface.bIsEnqueued;
            });
        }

        // Delete unused surfaces
        m_queuedSurfaces.erase(std::remove_if(
            m_queuedSurfaces.begin(), m_queuedSurfaces.end(), [this](auto& queuedSurface) {
                // Remove only already displayed surface
                if (!queuedSurface.bIsEnqueued) {
                    return false;
                }

                VdpPresentationQueueStatus surfaceStatus = VDP_PRESENTATION_QUEUE_STATUS_QUEUED;
                VdpTime unusedTime = 0;
                auto vdpStatus = gVdpFunctionsInstance()->presentationQueueQuerySurfaceStatus(
                    m_vdpQueue,
                    queuedSurface.surface.getVdpHandle(),
                    &surfaceStatus,
                    &unusedTime
                );
                gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't query the sufrace status");

                return surfaceStatus == VDP_PRESENTATION_QUEUE_STATUS_IDLE;
            }
        ), m_queuedSurfaces.end());

        return true;
    }

    VdpTime PresentationQueue::getCurrentTime() {
        VdpTime currentTime = 0;
        auto vdpStatus = gVdpFunctionsInstance()->presentationQueueGetTime(
            m_vdpQueue,
            &currentTime
        );
        gVdpFunctionsInstance()->throwExceptionOnFail(vdpStatus, "[PresentationQueue] Couldn't get the presentation time");

        return currentTime;
    }
}
