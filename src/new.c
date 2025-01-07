#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// Constants
#define MAX_LINES 10
#define MAX_LINE_LENGTH 128
#define MAX_TEXT_LENGTH 256

// Color structure
typedef struct {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} Color;

#define RED (Color){255,0,0}
#define BLACK (Color){0,0,0}
#define GREEN (Color){0,255,0}
#define BLUE (Color){0,0,255}
#define GRAY (Color){128,128,128}
#define WHITE (Color){255,255,255}

// Globals
Display *xiDisplay;
int xiScreen;

unsigned long convert_color_to_pixel(Color color) {
    return (color.r << 16) | (color.g << 8) | color.b;
}

GC create_graphics_context_for_coloring(Window win, Color color) {
    XGCValues values;
    values.foreground = convert_color_to_pixel(color);
    return XCreateGC(xiDisplay, win, GCForeground, &values);
}

typedef enum {
    FILLED,
    OUTLINE
} DrawMode;

void DrawRectangle(Window win, int x, int y, int w, int h, Color color, DrawMode mode) {
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
    int x, y, width, height;
} Rect;

// TextBox structure
typedef struct {
    Rect bounds;
    char text[MAX_LINES][MAX_LINE_LENGTH];
    int line_count;
    Color background_color;
    bool active;
    int current_line;
    int current_pos;
    const char *placeholder;
    Color placeholder_color;
    int font_size;
    Color text_color;
} TextBox;

TextBox create_text_box(int x, int y, int width, int height, int font_size, Color text_color, Color background_color, const char *placeholder) {
    TextBox text_box = {0};
    text_box.bounds = (Rect){x, y, width, height};
    text_box.font_size = font_size;
    text_box.text_color = text_color;
    text_box.background_color = background_color;
    text_box.line_count = 1;
    text_box.current_line = 0;
    text_box.current_pos = 0;
    text_box.active = false;
    text_box.placeholder = placeholder;
    text_box.placeholder_color = GRAY;
    memset(text_box.text, 0, sizeof(text_box.text));
    return text_box;
}

void render_text_box(Window win, TextBox *text_box) {
    DrawRectangle(win, text_box->bounds.x, text_box->bounds.y, text_box->bounds.width, text_box->bounds.height, text_box->background_color, FILLED);
    DrawRectangle(win, text_box->bounds.x, text_box->bounds.y, text_box->bounds.width, text_box->bounds.height, BLACK, OUTLINE);

    bool is_empty = (text_box->line_count == 1 && strlen(text_box->text[0]) == 0);
    if (is_empty && !text_box->active) {
        DrawText(win, text_box->bounds.x + 5, text_box->bounds.y + 20, text_box->placeholder, text_box->placeholder_color);
    } else {
        for (int i = 0; i < text_box->line_count; i++) {
            DrawText(win, text_box->bounds.x + 5, text_box->bounds.y + 20 + (i * (text_box->font_size + 5)), text_box->text[i], text_box->text_color);
        }
    }
}

// Main example
int main() {
    xiDisplay = XOpenDisplay(NULL);
    if (!xiDisplay) {
        fprintf(stderr, "Failed to open X display\n");
        return -1;
    }
    xiScreen = DefaultScreen(xiDisplay);

    Window root = RootWindow(xiDisplay, xiScreen);
    Window win = XCreateSimpleWindow(xiDisplay, root, 10, 10, 800, 600, 1, BlackPixel(xiDisplay, xiScreen), WhitePixel(xiDisplay, xiScreen));

    XSelectInput(xiDisplay, win, ExposureMask | KeyPressMask | ButtonPressMask);
    XMapWindow(xiDisplay, win);

    TextBox text_box = create_text_box(50, 50, 300, 100, 12, BLACK, WHITE, "Enter text here...");

    XEvent event;
    while (1) {
        XNextEvent(xiDisplay, &event);
        if (event.type == Expose) {
            render_text_box(win, &text_box);
        } else if (event.type == KeyPress) {
            // Handle key input for text box
        } else if (event.type == ButtonPress) {
            // Activate text box on click
        }
    }

    XCloseDisplay(xiDisplay);
    return 0;
}
