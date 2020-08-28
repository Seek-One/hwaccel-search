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
