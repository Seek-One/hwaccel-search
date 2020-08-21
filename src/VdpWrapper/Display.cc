#include <VdpWrapper/Display.h>

#include <iostream>
#include <stdexcept>

namespace vw {
    Display::Display(SizeI size)
    : m_pXDisplay(nullptr)
    , m_iXScreen(0)
    , m_screenSize(size)
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
            m_screenSize.width,
            m_screenSize.height,
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

    ::Window Display::getXWindow() const {
        return m_XWindow;
    }

    bool Display::isOpened() const {
        return m_bIsOpened;
    }

    SizeI Display::getScreenSize() const {
        return m_screenSize;
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

        case ConfigureNotify: {
            XConfigureEvent xce = event.xconfigure;
            SizeI newScreenSize(xce.width, xce.height);

            if (newScreenSize != m_screenSize) {
                std::cout << "[Display] New screen size: " << newScreenSize.width << " x " << newScreenSize.height << std::endl;
                m_screenSize = newScreenSize;
            }

            break;
        }

        default:
            break;
        }
    }
}
