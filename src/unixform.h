/// HEADERS AND LIBS CONTAINED IN UNIXFORM.H ARE:
#include <X11/Xlib.h> /// X11/XLIB
#include<stdio.h> /// stdio.h 
#include<stdlib.h> /// stdlib.h
/// in your example program you may choose to include the above and it will work fine, but they are already done for you, and it may slow down your program due to multiple re-including

///------------------------- GLOBAL VAR -------------------------
///------------------------- FUNCTIONS -------------------------
int xiCreateWindow(){
    // Step 1: Open a connection to the X server
    Display *display = XOpenDisplay(NULL);
    if (!display) {
        fprintf(stderr, "Unable to open X display\n");
        exit(1);
    }

    // Step 2: Create a window
    int screen = DefaultScreen(display);
    Window win = XCreateSimpleWindow(display, RootWindow(display, screen),10, 10, 500, 300, 1,BlackPixel(display, screen),WhitePixel(display, screen));
	
}

void xiDestroyWindow(){
//  Clean up and close
    XCloseDisplay(display);
}
int main() {

    // Step 3: Select events to handle
    XSelectInput(display, win, ExposureMask | KeyPressMask);

    // Step 4: Make the window visible
    XMapWindow(display, win);

    // Step 5: Event loop
    XEvent event;
    while (1) {
        XNextEvent(display, &event);

        if (event.type == Expose) {
            // Draw a rectangle when the window is exposed
            GC gc = XCreateGC(display, win, 0, NULL);
            XSetForeground(display, gc, BlackPixel(display, screen));
            XFillRectangle(display, win, gc, 50, 50, 200, 100);
            XFreeGC(display, gc);
        } else if (event.type == KeyPress) {
            // Exit on key press
            break;
        }
    }

    return 0;
}
