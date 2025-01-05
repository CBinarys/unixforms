#include <X11/Xlib.h>
#include<stdio.h>
int main() {
    Display *display = XOpenDisplay(NULL);
    if (display == NULL) return -1;

    int screen = DefaultScreen(display);
    Window win = XCreateSimpleWindow(display, RootWindow(display, screen), 10, 10, 500, 500, 1,
                                     BlackPixel(display, screen), WhitePixel(display, screen));
    XMapWindow(display, win);
    XFlush(display);

    getchar();  // Wait for user input
    XCloseDisplay(display);
    return 0;
}
