#include"unixform.h"
int main() {
Window win;
win = xiCreateWindow(640,480,"window", 0,0);
xiUpdate();
xiDestroyWindow(win);
    return 0;
}
