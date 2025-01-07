Window getWindowAndAdjustCoords(void *winOrContainer, int *x, int *y) {
    Container *container = (Container *)winOrContainer;
    
    if (container->containerWin) { // Check if it's a Container
        // Adjust coordinates for the container's absolute position
        *x += container->x;
        *y += container->y;
        return container->containerWin;
    }

    // Otherwise, it's a Window; no adjustment needed
    return *(Window *)winOrContainer;
}



void xiDrawRectangle(void *winOrContainer, int x, int y, int w, int h, Color color, DrawMode mode) {
    // Get the actual Window and adjust coordinates if necessary
    Window actualWin = getWindowAndAdjustCoords(winOrContainer, &x, &y);

    // Draw the rectangle with adjusted coordinates
    DrawRectangle(actualWin, x, y, w, h, color, mode);
}

// Create a container
Container container = xiCreateContainer(&mainWin, 50, 50, 200, 200, GRAY, false, "Container");

// Drawing inside a container (no need to pass .containerWin)
xiDrawRectangle(&container, 10, 20, 100, 50, RED, FILLED);

// Drawing in the main window
xiDrawRectangle(&mainWin, 300, 200, 150, 100, GREEN, OUTLINE);
