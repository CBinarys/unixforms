#include"unixform.h"
int main() {
Window win;
win = xiCreateWindow(640,480,"window", 0,0);
Container con = xiCreateContainer(win, 200, 200, 320, 240, GRAY, false,"my container");

// Event loop
	while (1) {
    	XNextEvent(xiDisplay, &xiEvent);
        	switch (xiEvent.type) {
	            case Expose:
	                xiRenderContainer(&con);
	            	DrawRectangle(win, 10,10, 200, 20,RED, FILLED);
	                DrawText(win, 30,100, "haha hi you've been on my mind", GREEN);
	                printf("Window Exposed\n");
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
xiDestroyWindow(win);
    return 0;
}
