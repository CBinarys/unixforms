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

void xiDestroyWindow(Window win) {
    // Clean up and close
    XUnmapWindow(xiDisplay, win);
    XDestroyWindow(xiDisplay, win);
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
#define BLACK (Color){255,255,255}
#define GREEN (Color){0,255,0}
#define BLUE (Color){0,0,255}

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
void DrawText(Window win, int x, int y, const char *text, Color color) {
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


//----------------------------------- WIDGETS --------------------------------
void xiDrawRectangle(Window win, int x, int y, int w, int h,Color color, DrawMode mode){
	DrawRectangle(win, x, y, w, h,color, mode);
	// the two draw rect functions are different, this present rect func is more a widgets, because it registers
}
typedef struct {
    Window containerWin;  // The container widget (a child window)
    int x, y;             // Position of the container widget
    int width, height;    // Size of the container widget
    Color backgroundColor; // Background color of the container
    bool fixed;
    bool resizable;
    char * title;
} Container;

// Create a container window inside the main window
Container xiCreateContainer(Window parentWin, int x, int y, int width, int height, Color bgColor, bool fixed,bool resizable, char * title) {
    // Create a container (child window) inside the main window (parentWin)
    XSetWindowAttributes xva;
    xva.background_pixel = convert_color_to_pixel(bgColor);
    xva.border_pixel = BlackPixel(xiDisplay, xiScreen);
    xva.event_mask = ButtonPressMask | ExposureMask | KeyPressMask;

    Window containerWin = XCreateWindow(
        xiDisplay, parentWin,
        x, y, width, height,
        0,                // Border width (0 for no border)
        DefaultDepth(xiDisplay, xiScreen),
        InputOutput,
        DefaultVisual(xiDisplay, xiScreen),
        CWBackPixel | CWBorderPixel | CWEventMask,
        &xva
    );

    // Map (show) the container window
    XMapWindow(xiDisplay, containerWin);

    Container container = {containerWin, x, y, width, height, bgColor,fixed, resizable, title};
    return container;
}

void xiRenderContainer(Container *container) {
    // Create a graphics context for the container
    GC gc = create_graphics_context_for_coloring(container->containerWin, container->backgroundColor);

        XFillRectangle(xiDisplay, container->containerWin, gc, container->x, container->y, container->width, container->height);

	// logic to add title bar and render the title text, if user pass in null instead of a title then it will skip
	if(container->title !=NULL){
			DrawRectangle(container->containerWin,container->x, container->y, container->width, 20,BLACK, FILLED);
			DrawText(container->containerWin,,container->title, BLACK);
	}
    XFreeGC(xiDisplay, gc); // Free the GC after use
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
