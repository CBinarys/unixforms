#include"unixform.h"
int main() {
Window win;
win = xiCreateWindow(640,480,"window", 0,0);
Container con = xiCreateContainer(win, 0, 0, 100, 100, RED, false,false,"my container");

    // Event loop
    while (1) {
        XNextEvent(xiDisplay, &xiEvent);

        switch (xiEvent.type) {
            case Expose:
                // Handle window expose (redraw)
                printf("Window Exposed\n");
                // xiRenderContainer(&con);
                DrawRectangle(win, 10,10, 200, 200,RED, FILLED);
                DrawText(win, 0,0, "haha hi you've been on my mind", BLACK);
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
