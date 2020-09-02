/* Copyright (c) 2020 Jet1oeil
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef VW_PRESENTATION_QUEUE_H
#define VW_PRESENTATION_QUEUE_H

#include <deque>

#include <vdpau/vdpau.h>

#include "RenderSurface.h"

namespace vw {
    class Device;
    class Display;

    /**
     * @brief QueuedSurface represents a surface waiting to be displayed
     */
    struct QueuedSurface {
        /**
         * @brief Construct a new QueuedSurface object
         *
         * When the object is constructed, it get the ownership of
         * the RenderSurcace in parameter.
         *
         * @param surface Surface will be displayed
         */
        QueuedSurface(RenderSurface surface);

        RenderSurface surface; ///< Surface will be displayed
        int iPOC; ///< Picture Order Count
        VdpTime iPresentationTimeStamp; ///< Presentation time in nanoseconds
        bool bIsEnqueued; ///< Indicate if the surface is currently in VdpPresentationQueue
    };

    /**
     * @brief PresentationQueue class encapsules a VdpPresentationQueue and a VdpPresentationQueueTarget
     *
     * This class aims to wrap the call to VDPAU API to handle a VdpPresentationQueue or VdpPresentationQueueTarget.
     * The class respect the RAII programming idiom hence when a new object is created is owning
     * a VdpPresentationQueue and a VdpPresentationQueueTarget and when they are destroyed the PresentationQueue is freed.
     */
    class PresentationQueue {
    public:
        /**
         * @brief Construct a new PresentationQueue
         *
         * @param display Reference to the current Display
         * @param device A reference to a valid Device
         */
        PresentationQueue(Display& display, Device &device);
        /**
         * @brief Destroy the Presentation Queue object
         *
         */
        ~PresentationQueue();

        PresentationQueue(const PresentationQueue&) = delete;
        PresentationQueue(PresentationQueue&&) = delete;

        PresentationQueue& operator=(const PresentationQueue&) = delete;
        PresentationQueue& operator=(PresentationQueue&&) = delete;

        /**
         * @brief Set the video framerate
         *
         * @param iFPS Number of Frame Per Second
         */
        void setFramerate(int iFPS);

        /**
         * @brief Set the order of displayed images
         *
         * @param bEnabled If true, the display is in presentation order otherwise in decode order
         */
        void enablePresentationOrderDisplay(bool bEnabled);

        /**
         * @brief Enable the direct ouput
         *
         * @param bDirectOutput If true, the surface will be displayed immediately otherwise the image respect the framerate
         */
        void enableDirectOutput(bool bDirectOutput);

        /**
         * @brief Enqueue a surface will be displayed
         *
         * This method create a new QueuedSurface and add this
         * object to its intern queue.
         *
         * When a surface is added, the queue is sorted by surface
         * presentation times. This time is computed form the Picture
         * Order Count if it's specified otherwise the presentation time
         * is set to the current time.
         *
         * The sort is mandatory by VDPAU to keep the right display order.
         *
         * @param surface
         * @return true
         * @return false
         */
        bool enqueue(RenderSurface surface);

    private:
        VdpTime getCurrentTime();

    private:
        VdpPresentationQueueTarget m_vdpQueueTarget;
        VdpPresentationQueue m_vdpQueue;
        std::deque<QueuedSurface> m_queuedSurfaces;

        bool m_bEnablePTS;
        bool m_bDirectOutput;
        VdpTime m_beginTime;
        VdpTime m_endTime;
        VdpTime m_framerateStep;

        int m_iNextPOC;
    };
}

#endif // VW_PRESENTATION_QUEUE_H
