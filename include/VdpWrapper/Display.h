#ifndef VW_DISPLAY_H
#define VW_DISPLAY_H

#include <X11/Xlib.h>

namespace vw {
    class Display {
    public:
        Display(int iWidth, int iHeight);
        ~Display();

        Display(const Display&) = delete;
        Display(Display&&) = delete;

        Display& operator=(const Display&) = delete;
        Display& operator=(Display&&) = delete;

        ::Display* getXDisplay();
        int getXScreen() const;

    private:
        ::Display* m_pXDisplay;
        int m_iXScreen;
        ::Window m_XWindow;
        GC m_gc;
        int m_iWidth;
        int m_iHeight;
    };
}

#endif // VW_DISPLAY_H
