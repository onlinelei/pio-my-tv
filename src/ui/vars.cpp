#include "ui/vars.h"

#include <string.h>

// 定义存储变量的静态缓冲区
static char date_value[11] = "";                            // 日期
static char time_value[9] = "";                             // 时间
static char message_value[256] = "";                        // 消息
static char page_easter_egg_title[64] = "Upgrade summary";  // 彩蛋标题
static char page_easter_egg_message[128] =
    "1. update memory \n2. update bettry show \n3. fix bugs\n... ...";  // 彩蛋消息
static char page_loading_tips[256] = "Wait for loading...";             // 默认提示信息

// 实现时间变量的 getter 和 setter
const char* get_var_page_1_var_time() {
    return time_value;
}

void set_var_page_1_var_time(const char* value) {
    if (value) {
        strncpy(time_value, value, sizeof(time_value) - 1);
        time_value[sizeof(time_value) - 1] = '\0';
    }
}

const char* get_var_page_1_var_date() {
    return date_value;
}
void set_var_page_1_var_date(const char* value) {
    if (value) {
        strncpy(date_value, value, sizeof(date_value) - 1);
        date_value[sizeof(date_value) - 1] = '\0';
    }
}

// 实现消息变量的 getter 和 setter
const char* get_var_page_1_var_message() {
    return message_value;
}

void set_var_page_1_var_message(const char* value) {
    if (value) {
        strncpy(message_value, value, sizeof(message_value) - 1);
        message_value[sizeof(message_value) - 1] = '\0';
    }
}

// 获取彩蛋标题
const char* get_var_page_easter_egg_title() {
    return page_easter_egg_title;
}

// 设置彩蛋标题
void set_var_page_easter_egg_title(const char* value) {
    if (value) {
        strncpy(page_easter_egg_title, value, sizeof(page_easter_egg_title) - 1);
        page_easter_egg_title[sizeof(page_easter_egg_title) - 1] = '\0';  // 确保字符串结束
    }
}

// 获取彩蛋消息
const char* get_var_page_easter_egg_message() {
    return page_easter_egg_message;
}

// 设置彩蛋消息
void set_var_page_easter_egg_message(const char* value) {
    if (value) {
        strncpy(page_easter_egg_message, value, sizeof(page_easter_egg_message) - 1);
        page_easter_egg_message[sizeof(page_easter_egg_message) - 1] = '\0';  // 确保字符串结束
    }
}

// 获取等待页面提示信息
const char* get_var_page_loading_tips() {
    return page_loading_tips;
}

// 设置等待页面提示信息
void set_var_page_loading_tips(const char* value) {
    if (value) {
        strncpy(page_loading_tips, value, sizeof(page_loading_tips) - 1);
        page_loading_tips[sizeof(page_loading_tips) - 1] = '\0';  // 确保字符串结束
    }
}
