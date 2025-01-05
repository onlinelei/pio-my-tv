#ifndef EEZ_LVGL_UI_VARS_H
#define EEZ_LVGL_UI_VARS_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// enum declarations



// Flow global variables

enum FlowGlobalVariables {
    FLOW_GLOBAL_VARIABLE_NONE
};

// Native global variables

extern const char *get_var_page_1_var_time();
extern void set_var_page_1_var_time(const char *value);
extern const char *get_var_page_1_var_message();
extern void set_var_page_1_var_message(const char *value);
extern const char *get_var_page_1_var_date();
extern void set_var_page_1_var_date(const char *value);
extern const char *get_var_page_easter_egg_title();
extern void set_var_page_easter_egg_title(const char *value);
extern const char *get_var_page_easter_egg_message();
extern void set_var_page_easter_egg_message(const char *value);
extern const char *get_var_page_loading_tips();
extern void set_var_page_loading_tips(const char *value);


#ifdef __cplusplus
}
#endif

#endif /*EEZ_LVGL_UI_VARS_H*/