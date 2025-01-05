#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
    Display *display;
    Window window;
    GC gc;
    XEvent event;
    int screen;

    // Open connection to X server
    display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open X display\n");
        return 1;
    }

    screen = DefaultScreen(display);

    // Create a simple window
    window = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, 500, 500, 1,
                                 BlackPixel(display, screen), WhitePixel(display, screen));

    // Select input events for the window
    XSelectInput(display, window, ExposureMask | KeyPressMask);

    // Map (show) the window
    XMapWindow(display, window);

    // Create a Graphics Context
    gc = XCreateGC(display, window, 0, NULL);
    XSetForeground(display, gc, BlackPixel(display, screen));
    XSetBackground(display, gc, WhitePixel(display, screen));

    // Event loop
    while (1) {
        XNextEvent(display, &event);

        if (event.type == Expose) {
            // Draw shapes on expose
            XDrawRectangle(display, window, gc, 50, 50, 100, 150);  // Outline rectangle
            XFillRectangle(display, window, gc, 200, 50, 100, 150); // Filled rectangle
            XDrawLine(display, window, gc, 50, 250, 150, 300);       // Line
            XDrawArc(display, window, gc, 50, 350, 100, 100, 0, 360*64); // Circle
            XFillArc(display, window, gc, 200, 350, 100, 100, 0, 360*64); // Filled circle

            // Draw a polygon
            XPoint points[] = {{300, 300}, {350, 350}, {250, 350}};
            XFillPolygon(display, window, gc, points, 3, Convex, CoordModeOrigin);
        }

        if (event.type == KeyPress)
            break;
    }

    // Cleanup
    XFreeGC(display, gc);
    XCloseDisplay(display);

    return 0;
}
