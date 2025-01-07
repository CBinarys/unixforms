#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>

#define MAX_LINES 10
#define MAX_LINE_LENGTH 100
#define MAX_TEXT_LENGTH 256

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
#define WHITE (Color){255,255,255}
// Globals
Display *xiDisplay;
int xiScreen;

GC create_graphics_context_for_coloring(Display *display, Window win, Color color) {
    XGCValues values;
    values.foreground = convert_color_to_pixel(color);

    GC gc = XCreateGC(display, win, GCForeground, &values);
    return gc;
}

typedef enum {
    FILLED,
    OUTLINE
} DrawMode;

static void DrawRectangle(Display *display, Window win, int x, int y, int w, int h, Color color, DrawMode mode) {
    GC gc = create_graphics_context_for_coloring(display, win, color);

    if (mode == FILLED) {
        XFillRectangle(display, win, gc, x, y, w, h);
    } else if (mode == OUTLINE) {
        XDrawRectangle(display, win, gc, x, y, w, h);
    } else {
        fprintf(stderr, "Invalid DrawMode\n");
    }

    XFreeGC(display, gc);
}

static void DrawText(Display *display, Window win, int x, int y, const char *text, Color color) {
    GC gc = create_graphics_context_for_coloring(display, win, color);

    XFontStruct *font = XLoadQueryFont(display, "fixed");
    if (font) {
        XSetFont(display, gc, font->fid);
    } else {
        fprintf(stderr, "Failed to load font. Using default font.\n");
    }

    XDrawString(display, win, gc, x, y, text, strlen(text));

    if (font) {
        XFreeFont(display, font);
    }
    XFreeGC(display, gc);
}

//--------------------------- Text Box Struct ---------------------------
typedef struct {
    int x, y, width, height;
} Rect;

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

void render_text_box(Display *display, Window win, TextBox *text_box) {
    DrawRectangle(display, win, text_box->bounds.x, text_box->bounds.y, text_box->bounds.width, text_box->bounds.height, text_box->background_color, FILLED);
    DrawRectangle(display, win, text_box->bounds.x, text_box->bounds.y, text_box->bounds.width, text_box->bounds.height, BLACK, OUTLINE);

    bool is_empty = (text_box->line_count == 1 && strlen(text_box->text[0]) == 0);

    if (is_empty && !text_box->active) {
        DrawText(display, win, text_box->bounds.x + 5, text_box->bounds.y + text_box->font_size, text_box->placeholder, text_box->placeholder_color);
    } else {
        for (int i = 0; i < text_box->line_count; i++) {
            int y_offset = i * (text_box->font_size + 5);
            DrawText(display, win, text_box->bounds.x + 5, text_box->bounds.y + text_box->font_size + y_offset, text_box->text[i], text_box->text_color);
        }
    }
}

void update_text_box(TextBox *text_box, XEvent *event) {
    if (text_box->active && event->type == KeyPress) {
        char buffer[32];
        KeySym keysym;
        int len = XLookupString(&event->xkey, buffer, sizeof(buffer), &keysym, NULL);

        if (len > 0 && buffer[0] >= 32 && buffer[0] <= 126) {
            if (text_box->current_pos < MAX_LINE_LENGTH - 1) {
                text_box->text[text_box->current_line][text_box->current_pos] = buffer[0];
                text_box->current_pos++;
                text_box->text[text_box->current_line][text_box->current_pos] = '\0';
            }
        } else if (keysym == XK_BackSpace) {
            if (text_box->current_pos > 0) {
                text_box->current_pos--;
                text_box->text[text_box->current_line][text_box->current_pos] = '\0';
            }
        }
    }

    if (event->type == ButtonPress) {
        int mx = event->xbutton.x;
        int my = event->xbutton.y;
        text_box->active = (mx >= text_box->bounds.x && mx <= text_box->bounds.x + text_box->bounds.width && my >= text_box->bounds.y && my <= text_box->bounds.y + text_box->bounds.height);
    }
}

//--------------------------- Text Entry Struct ---------------------------
typedef struct {
    Rect bounds;
    char text[MAX_TEXT_LENGTH];
    int cursor_position;
    bool active;
    int font_size;
    Color text_color;
    Color background_color;
} TextEntry;

TextEntry create_text_entry(int x, int y, int width, int height, int font_size, Color text_color, Color background_color) {
    TextEntry entry = {0};
    entry.bounds = (Rect){x, y, width, height};
    entry.font_size = font_size;
    entry.text_color = text_color;
    entry.background_color = background_color;
    entry.active = false;
    entry.cursor_position = 0;
    memset(entry.text, 0, sizeof(entry.text));
    return entry;
}

void render_text_entry(Display *display, Window win, TextEntry *entry) {
    DrawRectangle(display, win, entry->bounds.x, entry->bounds.y, entry->bounds.width, entry->bounds.height, entry->background_color, FILLED);
    DrawRectangle(display, win, entry->bounds.x, entry->bounds.y, entry->bounds.width, entry->bounds.height, BLACK, OUTLINE);
    DrawText(display, win, entry->bounds.x + 5, entry->bounds.y + entry->font_size, entry->text, entry->text_color);
}

void update_text_entry(TextEntry *entry, XEvent *event) {
    if (entry->active && event->type == KeyPress) {
        char buffer[32];
        KeySym keysym;
        int len = XLookupString(&event->xkey, buffer, sizeof(buffer), &keysym, NULL);

        if (len > 0 && buffer[0] >= 32 && buffer[0] <= 126) {
            if (entry->cursor_position < MAX_TEXT_LENGTH - 1) {
                entry->text[entry->cursor_position] = buffer[0];
                entry->cursor_position++;
                entry->text[entry->cursor_position] = '\0';
            }
        } else if (keysym == XK_BackSpace) {
            if (entry->cursor_position > 0) {
                entry->cursor_position--;
                entry->text[entry->cursor_position] = '\0';
            }
        }
    }

    if (event->type == ButtonPress) {
        int mx = event->xbutton.x;
        int my = event->xbutton.y;
        entry->active = (mx >= entry->bounds.x && mx <= entry->bounds.x + entry->bounds.width && my >= entry->bounds.y && my <= entry->bounds.y + entry->bounds.height);
    }
}

//--------------------------- Main
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
            render_text_box(xiDisplay,win, &text_box);
        } else if (event.type == KeyPress) {
            // Handle key input for text box
        } else if (event.type == ButtonPress) {
            // Activate text box on click
        }
    }

    XCloseDisplay(xiDisplay);
    return 0;
}
