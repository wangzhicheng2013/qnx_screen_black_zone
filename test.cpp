#include "qnx_screen_color_zone.hpp"
int main(int argc, const char** argv) {
    qnx_screen_color_zone blue_zone;
    if (3 == argc) {
        blue_zone.screen_width = atoi(argv[1]);
        blue_zone.screen_height = atoi(argv[2]);
    }
    blue_zone.set_blue();
    int error = blue_zone.init();
    if (error) {
        printf("blue zone init failed, error:%d\n", error);
        return -1;
    }
    printf("blue result:%d\n", blue_zone.set_bg_color(100, 100, 300, 300));
    
    return 0;
}