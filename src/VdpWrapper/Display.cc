#include <VdpWrapper/Display.h>

#include <stdexcept>

namespace vw {
    Display::Display(int iWidth, int iHeight)
    : m_pXDisplay(nullptr)
    , m_iXScreen(0)
    , m_iWidth(iWidth)
    , m_iHeight(iHeight)
    , m_bIsOpened(false) {
        // Allocate the X display
        m_pXDisplay = XOpenDisplay(nullptr);
        m_iXScreen = DefaultScreen(m_pXDisplay);

        // Get some colors
        int iBlackColor = BlackPixel(m_pXDisplay, m_iXScreen);
        int iWhiteColor = WhitePixel(m_pXDisplay, m_iXScreen);

        m_XWindow = XCreateSimpleWindow(
            m_pXDisplay,
            RootWindow(m_pXDisplay, m_iXScreen),
            0,
            0,
            m_iWidth,
            m_iHeight,
            0,
            iWhiteColor,
            iBlackColor
        );

        // Map the window => set visible the window
        XSelectInput(m_pXDisplay, m_XWindow, StructureNotifyMask);
        XMapWindow(m_pXDisplay, m_XWindow);

        // Wait for the MapNotify event
        for(;;) {
            XEvent e;
            XNextEvent(m_pXDisplay, &e);
            if (e.type == MapNotify)
            break;
        }

        XFlush(m_pXDisplay);

        // Handle close window event
        m_bIsOpened = true;
        m_windowDeleteMessage = XInternAtom(m_pXDisplay, "WM_DELETE_WINDOW", false);
        XSetWMProtocols(m_pXDisplay, m_XWindow, &m_windowDeleteMessage, 1);
    }

    Display::~Display() {
        XDestroyWindow(m_pXDisplay, m_XWindow);
        XCloseDisplay(m_pXDisplay);
    }

    ::Display* Display::getXDisplay() {
        return m_pXDisplay;
    }

    int Display::getXScreen() const {
        return m_iXScreen;
    }

    bool Display::isOpened() const {
        return m_bIsOpened;
    }

    void Display::processEvent() {
        XEvent event;
        XNextEvent(m_pXDisplay, &event);

        switch (event.type) {
        case ClientMessage:
            if (static_cast<Atom>(event.xclient.data.l[0]) == m_windowDeleteMessage) {
                m_bIsOpened = false;
            }
            break;

        default:
            break;
        }
    }
}
