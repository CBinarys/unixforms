#include <X11/Xlib.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

Display *xiDisplay;
int xiScreen;
Window xiRoot;
XEvent xiEvent;

typedef struct {
    unsigned char r; // 0-255
    unsigned char g; // 0-255
    unsigned char b; // 0-255
} Color;

#define RED (Color){255,0,0}
#define BLACK (Color){0,0,0}
#define GREEN (Color){0,255,0}
#define BLUE (Color){0,0,255}
#define GRAY (Color){128,128,128}

static unsigned long convert_color_to_pixel(Color color) {
    return (color.r << 16) | (color.g << 8) | color.b;
}

GC create_graphics_context_for_coloring(Window win, Color color) {
    XGCValues values;
    values.foreground = convert_color_to_pixel(color);
    GC gc = XCreateGC(xiDisplay, win, GCForeground, &values);
    return gc;
}

typedef enum { FILLED, OUTLINE } DrawMode;

static void DrawRectangle(Window win, int x, int y, int w, int h, Color color, DrawMode mode) {
    GC gc = create_graphics_context_for_coloring(win, color);
    if (mode == FILLED) {
        XFillRectangle(xiDisplay, win, gc, x, y, w, h);
    } else {
        XDrawRectangle(xiDisplay, win, gc, x, y, w, h);
    }
    XFreeGC(xiDisplay, gc);
}

void DrawText(Window win, int x, int y, const char *text, Color color) {
    GC gc = create_graphics_context_for_coloring(win, color);
    XFontStruct *font = XLoadQueryFont(xiDisplay, "fixed");
    if (font) {
        XSetFont(xiDisplay, gc, font->fid);
    }
    XDrawString(xiDisplay, win, gc, x, y, text, strlen(text));
    if (font) {
        XFreeFont(xiDisplay, font);
    }
    XFreeGC(xiDisplay, gc);
}

typedef struct {
    Window containerWin;
    int x, y;
    int width, height;
    Color backgroundColor;
    bool fixed;
    bool resizable;
    char *title;
    bool dragging;
    int dragStartX, dragStartY;
} Container;

Container xiCreateContainer(Window parentWin, int x, int y, int width, int height, Color bgColor, bool fixed, bool resizable, char *title) {
    XSetWindowAttributes xva;
    xva.background_pixel = convert_color_to_pixel(bgColor);
    xva.border_pixel = BlackPixel(xiDisplay, xiScreen);
    xva.event_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask;

    Window containerWin = XCreateWindow(
        xiDisplay, parentWin,
        x, y, width, height,
        0, DefaultDepth(xiDisplay, xiScreen),
        InputOutput, DefaultVisual(xiDisplay, xiScreen),
        CWBackPixel | CWBorderPixel | CWEventMask, &xva);

    XMapWindow(xiDisplay, containerWin);

    Container container = {containerWin, x, y, width, height, bgColor, fixed, resizable, title, false, 0, 0};
    return container;
}

void xiRenderContainer(Container *container) {
    GC gc = create_graphics_context_for_coloring(container->containerWin, container->backgroundColor);
    XFillRectangle(xiDisplay, container->containerWin, gc, 0, 0, container->width, container->height);
    if (container->title) {
        DrawRectangle(container->containerWin, 0, 0, container->width, 20, BLACK, FILLED);
        DrawText(container->containerWin, 5, 15, container->title, GRAY);
    }
    XFreeGC(xiDisplay, gc);
}

void xiHandleContainerEvents(Container *container, XEvent *event) {
    if (!container->fixed && event->type == ButtonPress && event->xbutton.y < 20) {
        container->dragging = true;
        container->dragStartX = event->xbutton.x_root;
        container->dragStartY = event->xbutton.y_root;
    } else if (event->type == ButtonRelease) {
        container->dragging = false;
    } else if (container->dragging && event->type == MotionNotify) {
        int dx = event->xmotion.x_root - container->dragStartX;
        int dy = event->xmotion.y_root - container->dragStartY;
        container->x += dx;
        container->y += dy;
        XMoveWindow(xiDisplay, container->containerWin, container->x, container->y);
        container->dragStartX = event->xmotion.x_root;
        container->dragStartY = event->xmotion.y_root;
    }
}

int main() {
    if ((xiDisplay = XOpenDisplay(NULL)) == NULL) {
        fprintf(stderr, "Unable to open X display\n");
        exit(1);
    }

    xiScreen = DefaultScreen(xiDisplay);
    xiRoot = RootWindow(xiDisplay, xiScreen);

    Window win = XCreateSimpleWindow(xiDisplay, xiRoot, 0, 0, 640, 480, 1, BlackPixel(xiDisplay, xiScreen), WhitePixel(xiDisplay, xiScreen));
    XMapWindow(xiDisplay, win);

    Container con = xiCreateContainer(win, 200, 200, 320, 240, GRAY, false, true, "My Container");

    while (1) {
        XNextEvent(xiDisplay, &xiEvent);
        switch (xiEvent.type) {
            case Expose:
                xiRenderContainer(&con);
                break;
            case ButtonPress:
            case ButtonRelease:
            case MotionNotify:
                xiHandleContainerEvents(&con, &xiEvent);
                break;
            default:
                break;
        }
    }

    XCloseDisplay(xiDisplay);
    return 0;
}
