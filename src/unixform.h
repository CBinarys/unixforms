/// HEADERS AND LIBS CONTAINED IN UNIXFORM.H ARE:
#include <X11/Xlib.h> /// X11/XLIB
#include<stdio.h> /// stdio.h 
#include<stdlib.h> /// stdlib.h
#include<stdbool.h> /// stdbool.h
#include<string.h> /// string.h
/// in your example program you may choose to include the above and it will work fine, but they are already done for you, and it may slow down your program due to multiple re-including

///------------------------- GLOBAL VAR -------------------------
Display *xiDisplay;
int xiScreen;
Window xiRoot;
XEvent xiEvent;

///------------------------- FUNCTIONS -------------------------
//------------------------- window --------------------------

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

void xiDestroyWindow(Window *win) {
    // Clean up and close
    XUnmapWindow(xiDisplay, *win);
    XDestroyWindow(xiDisplay,* win);
    XCloseDisplay(xiDisplay);
}



//------------------------- colors, rendering, shapes and graphics context --------------------------
typedef struct {
    unsigned char r; // 0-255
    unsigned char g; // 0-255
    unsigned char b; // 0-255
} Color;

// Converts 8-bit color to 16-bit for Xlib, in alignment to 0-255
static unsigned long convert_color_to_pixel(Color color) {
    return (color.r << 16) | (color.g << 8) | color.b;
}

// created few color constants
#define RED (Color){255,0,0}
#define BLACK (Color){0,0,0}
#define GREEN (Color){0,255,0}
#define BLUE (Color){0,0,255}
#define GRAY (Color){128,128,128}

GC create_graphics_context_for_coloring(Window win,Color color) {
    XGCValues values;
    values.foreground = convert_color_to_pixel(color);

    GC gc = XCreateGC(xiDisplay, win, GCForeground, &values);
    return gc;
}

typedef enum {
    FILLED,
    OUTLINE
} DrawMode;

static void DrawRectangle(Window win, int x, int y, int w, int h,Color color, DrawMode mode) {
	// first creates a new graphics context for rendering
    GC gc = create_graphics_context_for_coloring( win, color);

    if (mode == FILLED) {

        XFillRectangle(xiDisplay, win, gc, x, y, w, h); // Draw a filled rectangle

    } else if (mode == OUTLINE) {
        // Draw an outlined rectangle
        XDrawRectangle(xiDisplay, win, gc, x, y, w, h);
        
    } else {
        // TODO: Handle invalid mode (e.g., throw an error or log it)
        fprintf(stderr, "Invalid DrawMode\n");
    }

        XFreeGC(xiDisplay, gc); // Free the GC after use

}
// Function to draw text
static void DrawText(Window win, int x, int y, const char *text, Color color) {
    // Create a graphics context for the given color
    GC gc = create_graphics_context_for_coloring(win, color);

    // Set font (optional, default font will be used if this is omitted)
    XFontStruct *font = XLoadQueryFont(xiDisplay, "fixed");
    if (font) {
        XSetFont(xiDisplay, gc, font->fid);
    } else {
        fprintf(stderr, "Failed to load font. Using default font.\n");
    }

    // Draw the text
    XDrawString(xiDisplay, win, gc, x, y, text, strlen(text));

    // Free the font and graphics context
    if (font) {
        XFreeFont(xiDisplay, font);
    }
    XFreeGC(xiDisplay, gc);
}


//-------
typedef struct {
    Window containerWin;
    int x, y;
    int width, height;
    Color backgroundColor;
    bool fixed;
    char *title;
    bool dragging;
    int dragStartX, dragStartY;
} Container;

Container xiCreateContainer(Window *parentWin, int x, int y, int width, int height, Color bgColor, bool fixed, char *title) {
    XSetWindowAttributes xva;
    xva.background_pixel = convert_color_to_pixel(bgColor);
    xva.border_pixel = BlackPixel(xiDisplay, xiScreen);
    xva.event_mask = ButtonPressMask | ButtonReleaseMask | PointerMotionMask | ExposureMask;

    Window containerWin = XCreateWindow(
        xiDisplay, *parentWin,
        x, y, width, height,
        0, DefaultDepth(xiDisplay, xiScreen),
        InputOutput, DefaultVisual(xiDisplay, xiScreen),
        CWBackPixel | CWBorderPixel | CWEventMask, &xva);

    XMapWindow(xiDisplay, containerWin);

    Container container = {containerWin, x, y, width, height, bgColor, fixed, title, false, 0, 0};
    return container;
}

void xiRenderContainer(Container *container) {
    GC gc = create_graphics_context_for_coloring(container->containerWin, container->backgroundColor);
    XFillRectangle(xiDisplay, container->containerWin, gc, 0, 0, container->width, container->height);
    // if a string title was passed in instead of null, it should draw the title bar and render the title string else ignore
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

//-----
bool getWindowAndAdjustCoords(void *winOrContainer, Window *outWindow) {
    Container *container = (Container *)winOrContainer;

    if (container->containerWin) { // Check if it's a Container
        *outWindow = container->containerWin;
        return true; // It's a Container
    }

    // Otherwise, it's a Window
    *outWindow = *(Window *)winOrContainer;
    return false; // It's a Window
}

//----------------------------------- WIDGETS --------------------------------
void xiDrawRectangle(void *winOrContainer, int x, int y, int w, int h, Color color, DrawMode mode) {
    Window actualWin;
    bool isContainer = getWindowAndAdjustCoords(winOrContainer, &actualWin);

    // If it's a Container, adjust the coordinates using container's position
    if (isContainer) {
        Container *container = (Container *)winOrContainer;
        x += container->x;
        y += container->y;
    }

    // Draw the rectangle with the resolved coordinates
    DrawRectangle(actualWin, x, y, w, h, color, mode);
}

void xiDrawText(Window *win, int x, int y, const char *text, Color color){
	DrawText(*win, x, y, text, color);
}

void xiUpdate() {
    // Event loop

}
