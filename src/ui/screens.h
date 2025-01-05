#ifndef EEZ_LVGL_UI_SCREENS_H
#define EEZ_LVGL_UI_SCREENS_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _objects_t {
    lv_obj_t *page_main;
    lv_obj_t *page_loading;
    lv_obj_t *page_upgrade_easter_egg;
    lv_obj_t *obj0;
    lv_obj_t *obj1;
    lv_obj_t *obj2;
    lv_obj_t *obj3;
    lv_obj_t *obj4;
    lv_obj_t *obj5;
    lv_obj_t *obj6;
    lv_obj_t *page_1_label_time;
} objects_t;

extern objects_t objects;

enum ScreensEnum {
    SCREEN_ID_PAGE_MAIN = 1,
    SCREEN_ID_PAGE_LOADING = 2,
    SCREEN_ID_PAGE_UPGRADE_EASTER_EGG = 3,
};

void create_screen_page_main();
void tick_screen_page_main();

void create_screen_page_loading();
void tick_screen_page_loading();

void create_screen_page_upgrade_easter_egg();
void tick_screen_page_upgrade_easter_egg();

void create_screens();
void tick_screen(int screen_index);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_SCREENS_H*/