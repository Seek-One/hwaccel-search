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

#ifndef VW_DISPLAY_H
#define VW_DISPLAY_H

#include <X11/Xlib.h>

#include <VdpWrapper/Size.h>

namespace vw {
    /**
     * @brief Display encapsules a X11 display (including the ::Display and ::Window from XLib)
     *
     * This class aims to wrap the ::Display and ::Window classes of XLib. The class while
     * handle all the window interactions and will be used by PresentationQueue to create
     * the VdpPresentationQueue.
     */
    class Display {
    public:
        /**
         * @brief Construct a new Display object
         *
         * @param size The initial window size
         */
        Display(SizeI size);
        /**
         * @brief Destroy the Display object
         */
        ~Display();

        Display(const Display&) = delete;
        Display(Display&&) = delete;

        Display& operator=(const Display&) = delete;
        Display& operator=(Display&&) = delete;

        /**
         * @brief Get the pointer to ::Display
         *
         * @return ::Display*
         */
        ::Display* getXDisplay();
        /**
         * @brief Get the screen number
         *
         * @return int The screen number
         */
        int getXScreen() const;
        /**
         * @brief Get the ::Window handle
         *
         * @return ::Window The ::Window handle
         */
        ::Window getXWindow() const;

        /**
         * @brief Check if the window is currently opened
         *
         * @return true If the window is opened
         * @return false If the window is closed
         */
        bool isOpened() const;

        /**
         * @brief Get the screen size
         *
         * @return SizeI
         */
        SizeI getScreenSize() const;

        /**
         * @brief Wait for the next X11 event
         *
         * This function blocks until the next event as opposed to processEvent().
         */
        void waitEvent();
        /**
         * @brief Process the next X11 event
         *
         * This function returns immediately if there has no pending X11 event as opposed to processEvent().
         */
        void processEvent();

    private:
        ::Display* m_pXDisplay;
        int m_iXScreen;
        ::Window m_XWindow;
        GC m_gc;
        SizeI m_screenSize;
        bool m_bIsOpened;
        Atom m_windowDeleteMessage;
    };
}

#endif // VW_DISPLAY_H
