#include "qnx_screen_black_zone.hpp"
int main() {
    qnx_screen_black_zone black_zone;
    int error = black_zone.init();
    if (error) {
        printf("black zone init failed, error:%d\n", error);
        return -1;
    }
    printf("black result:%d\n", black_zone.black(100, 100, 300, 300));
    
    return 0;
}