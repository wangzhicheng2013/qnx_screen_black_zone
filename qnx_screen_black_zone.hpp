#pragma once
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <assert.h>
#include <screen/screen.h>
struct qnx_screen_black_zone {
    screen_context_t screen_ctx = nullptr;
    screen_display_t *screen_disps = nullptr;
    screen_display_t screen_disp = nullptr;
    int screen_size[2] = { 0 };
    int display_count = 0;
    // Screen resolution
    int screen_width = 1920;
    int screen_height = 1080;
    int display_zorder = 999;
    screen_window_t win = { 0 };

    qnx_screen_black_zone() = default;
    virtual ~qnx_screen_black_zone() {
        uninit_win();
        unit_context();
    }
    int init() {
        int error = init_context();
        if (error) {
            return error;
        }
        error = init_win();
        return error;
    }
    int black(int left,            // left margin
              int top,             // top margin
              int right,           // right margin
              int bottom) {        // bottom margin
        if ((left < 0) || (top < 0) || (right < 0) || (bottom < 0)) {
            printf("unvalid margin left:%d, top:%d, right:%d, bottom:%d\n", left, top, right, bottom);
            return -1002;
        }
        if ((left + right > screen_size[0]) || (top + bottom > screen_size[1])) {
            printf("unvalid margin left:%d, top:%d, right:%d, bottom:%d, screen width:%d, screen height:%d\n", left, top, right, bottom, screen_size[0], screen_size[1]);
            return -1003;
        }
        int display_rect[4] = { 0 };
        display_rect[0] = left;
        display_rect[1] = top;
        printf("display zone position: %dx%d\n", display_rect[0], display_rect[1]);
        int error = screen_set_window_property_iv(win, SCREEN_PROPERTY_POSITION, display_rect);
        if (error) {
            printf("screen_set_window_property_iv for SCREEN_PROPERTY_POSITION failed, error:%s\n", strerror(errno));
            return error;
        }
        display_rect[2] = screen_size[0] - left - right;
        display_rect[3] = screen_size[1] - top - bottom;
        printf("display zone size: %dx%d\n", display_rect[2], display_rect[3]);
        error = screen_set_window_property_iv(win, SCREEN_PROPERTY_SIZE, display_rect + 2);
        if (error) {
            printf("screen_set_window_property_iv for SCREEN_PROPERTY_SIZE failed, error:%s\n", strerror(errno));
            return error;
        }
        screen_buffer_t render_buf[2] = { 0 };
        error = screen_get_window_property_pv(win, SCREEN_PROPERTY_RENDER_BUFFERS, (void **)render_buf);
        if (error) {
            printf("screen_get_window_property_pv for SCREEN_PROPERTY_RENDER_BUFFERS failed, error:%s\n", strerror(errno));
            return error;
        }
        const static int black_color = 0xff000000;
        const static int bg_color[] = { SCREEN_BLIT_COLOR, black_color, SCREEN_BLIT_END };
        screen_fill(screen_ctx, render_buf[0], bg_color);
        screen_post_window(win, render_buf[0], 0, nullptr, 0);
        sleep(10);      // wait to see the change of screen
        return 0;
    }
private:
    int init_context() {
        int error = screen_create_context(&screen_ctx, SCREEN_APPLICATION_CONTEXT);
        if (error) {
            printf("screen_create_context failed:%s\n", strerror(errno));
            return error;
        }
        // Using the number of displays returned by the query,
        // allocate enough memory to retrieve an array of pointers to screen_display_t
        error = screen_get_context_property_iv(screen_ctx, SCREEN_PROPERTY_DISPLAY_COUNT, &display_count);
        if (error) {
            printf("screen_get_context_property_iv failed:%s\n", strerror(errno));
            return error;
        }
        printf("qnx get screen count:%d\n", display_count);
        if (display_count < 1) {
            printf("SCREEN_PROPERTY_DISPLAY_COUNT is less than 1!\n");
            return -1000;
        }
        screen_disps = (screen_display_t *)calloc(display_count, sizeof(screen_display_t));
        assert(screen_disps != nullptr);
        error = screen_get_context_property_pv(screen_ctx, SCREEN_PROPERTY_DISPLAYS, (void **)screen_disps);
        if (error) {
            printf("screen_get_context_property_pv failed:%s\n", strerror(errno));
            return error;
        }
        int i = 0;
        // choose the correct display screen
        for (;i < display_count;i++) {
            screen_disp = screen_disps[i];
            error = screen_get_display_property_iv(screen_disp, SCREEN_PROPERTY_SIZE, screen_size);
            if (error) {
                printf("screen_get_display_property_iv failed:%s\n", strerror(errno));
                return error;
            }
            printf("qnx_screen_context get screen index:%d, size:%dx%d\n", i, screen_size[0], screen_size[1]);
            if ((screen_width == screen_size[0]) && (screen_height == screen_size[1])) {
                printf("get correct screen display index:%d\n", i);
                break;
            }
        }
        if (i >= display_count) {
            printf("no find the target screen resolution width:%d,height:%d\n", screen_width, screen_height);
            return -1001;
        }
        printf("qnx screen context init ok!\n");
        return 0;
    }
    void unit_context() {
        if (screen_ctx != nullptr) {
            screen_destroy_context(screen_ctx);
            screen_ctx = nullptr;
        }
        if (screen_disps != nullptr) {
            free(screen_disps);
            screen_disps = nullptr;
        }
    }
    int init_win() {
        int error = screen_create_window(&win, screen_ctx);
        if (error) {
            printf("screen_create_window failed, error:%s\n", strerror(errno));
            return error;
        }
        static const int vis = 1;
        error = screen_set_window_property_iv(win, SCREEN_PROPERTY_VISIBLE, &vis);
        if (error) {
            printf("screen_set_window_property_iv for SCREEN_PROPERTY_VISIBLE failed, error:%s\n", strerror(errno));
            return error;
        }
        error = screen_set_window_property_pv(win, SCREEN_PROPERTY_DISPLAY, (void**)&screen_disp);
        if (error) {
            printf("screen_set_window_property_iv for SCREEN_PROPERTY_DISPLAY failed, error:%s\n", strerror(errno));
            return error;
        }
        static const int usage = SCREEN_USAGE_WRITE;
        error =  screen_set_window_property_iv(win, SCREEN_PROPERTY_USAGE, &usage);
        if (error) {
            printf("screen_set_window_property_iv for SCREEN_PROPERTY_USAGE failed, error:%s\n", strerror(errno));
            return error;
        }
        error = screen_set_window_property_iv(win, SCREEN_PROPERTY_ZORDER, &display_zorder);
        if (error) {
            printf("screen_set_window_property_iv for SCREEN_PROPERTY_ZORDER failed, error:%s\n", strerror(errno));
            return error;
        }
        // create screen window buffers
        error = screen_create_window_buffers(win, 1);
        if (error) {
            printf("screen_create_window_buffers failed, error:%s\n", strerror(errno));
            return error;
        }
        printf("qnx screen win init ok!\n");
        return 0;
    }
    void uninit_win() {
        screen_destroy_window(win);
    }
};