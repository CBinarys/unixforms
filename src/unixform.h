/// HEADERS AND LIBS CONTAINED IN UNIXFORM.H ARE:
#include <X11/Xlib.h> /// X11/XLIB
#include<stdio.h> /// stdio.h 
#include<stdlib.h> /// stdlib.h
/// in your example program you may choose to include the above and it will work fine, but they are already done for you, and it may slow down your program due to multiple re-including

///------------------------- GLOBAL VAR -------------------------
Display *xiDisplay;
int xiScreen;
Window xiRoot;
XEvent xiEvent;

///------------------------- FUNCTIONS -------------------------
Window xiCreateWindow(int width, int height, char *title, int xpos, int ypos) {
    // Open the display if not already opened
    if ((xiDisplay = XOpenDisplay(NULL)) == NULL) {
        fprintf(stderr, "Unable to open X display\n");
        exit(1);
    }

    xiScreen = DefaultScreen(xiDisplay);
    xiRoot = RootWindow(xiDisplay, xiScreen);

    // Set window attributes
    XSetWindowAttributes xva;
    xva.background_pixel = WhitePixel(xiDisplay, xiScreen);
    xva.border_pixel = BlackPixel(xiDisplay, xiScreen);
    xva.event_mask = ButtonPressMask | ExposureMask | KeyPressMask;

    // Create the window
    Window win = XCreateWindow(
        xiDisplay, xiRoot,
        xpos, ypos, width, height, 
        1,                // Border width
        DefaultDepth(xiDisplay, xiScreen), 
        InputOutput, 
        DefaultVisual(xiDisplay, xiScreen),
        CWBackPixel | CWBorderPixel | CWEventMask, 
        &xva
    );

    // Set the window title
    XStoreName(xiDisplay, win, title);

    // Map (show) the window
    XMapWindow(xiDisplay, win);

    return win;
}

void xiDestroyWindow(Window win) {
    // Clean up and close
    XUnmapWindow(xiDisplay, win);
    XDestroyWindow(xiDisplay, win);
    XCloseDisplay(xiDisplay);
}
typedef enum {
    FILLED,
    OUTLINE
} DrawMode;

void DrawRectangle(Display *display, GC gc, Window win, int x, int y, int w, int h, DrawMode mode) {
    if (mode == FILLED) {
        // Draw a filled rectangle
        XFillRectangle(display, win, gc, x, y, w, h);
    } else if (mode == OUTLINE) {
        // Draw an outlined rectangle
        XDrawRectangle(display, win, gc, x, y, w, h);
    } else {
        // TODO: Handle invalid mode (e.g., throw an error or log it)
        fprintf(stderr, "Invalid DrawMode\n");
    }
}

void xiUpdate() {
    // Event loop
    while (1) {
        XNextEvent(xiDisplay, &xiEvent);

        switch (xiEvent.type) {
            case Expose:
                // Handle window expose (redraw)
                printf("Window Exposed\n");
                break;

            case ButtonPress:
                // Handle button press
                printf("Mouse button pressed at (%d, %d)\n",
                    xiEvent.xbutton.x, xiEvent.xbutton.y);
                break;

            case KeyPress:
                // Handle key press
                printf("Key pressed\n");
                break;

            default:
                break;
        }
    }
}
