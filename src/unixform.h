/// HEADERS AND LIBS CONTAINED IN UNIXFORM.H ARE:
#include <X11/Xlib.h> /// X11/XLIB
#include<stdio.h> /// stdio.h 
#include<stdlib.h> /// stdlib.h
#include<stdbool.h>
/// in your example program you may choose to include the above and it will work fine, but they are already done for you, and it may slow down your program due to multiple re-including

///------------------------- GLOBAL VAR -------------------------
Display *xiDisplay;
int xiScreen;
Window xiWindow;
XEvent xiEvent;
///------------------------- FUNCTIONS -------------------------
int xiCreateWindow(){
    // Step 1: Open a connection to the X server
    xiDisplay = XOpenDisplay(NULL);
    if (!xiDisplay) {
        fprintf(stderr, "Unable to open X display\n");
        exit(1);
    }

    // Step 2: Create a window
    xiScreen = DefaultScreen(xiDisplay);
    xiWindow = XCreateSimpleWindow(xiDisplay, RootWindow(xiDisplay, xiScreen),10, 10, 500, 300, 1,BlackPixel(xiDisplay, xiScreen),WhitePixel(xiDisplay, xiScreen));
    // Step 4: Make the window visible
    XMapWindow(xiDisplay, xiWindow);

}

void xiDestroyWindow(){
//  Clean up and close
	XUnmapWindow(xiDisplay, xiWindow);
	XDestroyWindow(xiDisplay, xiWindow);
    XCloseDisplay(xiDisplay);
}

void xiUpdate(){
	while(true){
		
	}
}
