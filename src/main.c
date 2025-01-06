#include"unixform.h"
#define RED (Color){255,0,44}
int main() {
Window win;
win = xiCreateWindow(640,480,"window", 0,0);
    // Event loop
    while (1) {
        XNextEvent(xiDisplay, &xiEvent);

        switch (xiEvent.type) {
            case Expose:
                // Handle window expose (redraw)
                printf("Window Exposed\n");
                DrawRectangle(win, 10,10, 200, 200,RED, FILLED);
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
xiDestroyWindow(win);
    return 0;
}
