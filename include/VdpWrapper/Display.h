#ifndef VW_DISPLAY_H
#define VW_DISPLAY_H

#include <X11/Xlib.h>

#include <VdpWrapper/Size.h>

namespace vw {
    class Display {
    public:
        Display(SizeI size);
        ~Display();

        Display(const Display&) = delete;
        Display(Display&&) = delete;

        Display& operator=(const Display&) = delete;
        Display& operator=(Display&&) = delete;

        ::Display* getXDisplay();
        int getXScreen() const;
        ::Window getXWindow() const;

        bool isOpened() const;

        SizeI getScreenSize() const;

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
