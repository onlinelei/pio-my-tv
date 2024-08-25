// /**
//  * @file display.cpp
//  *
//  */

// /*********************
//  *      INCLUDES
//  *********************/
// #include "display.h"
// #include <stdbool.h>
// #include <TFT_eSPI.h>

// /*********************
//  *      DEFINES
//  *********************/
// #ifndef MY_DISP_HOR_RES
// #define MY_DISP_HOR_RES (240)
// #endif

// #ifndef MY_DISP_VER_RES
// #define MY_DISP_VER_RES (240)
// #endif

// /**********************
//  *  STATIC VARIABLES
//  **********************/
// TFT_eSPI tft = TFT_eSPI();

// /**********************
//  *  STATIC PROTOTYPES
//  **********************/
// static void disp_flush(lv_disp_drv_t *disp_drv, const lv_area_t *area, lv_color_t *color_p)
// {
//     uint32_t w = (area->x2 - area->x1 + 1);
//     uint32_t h = (area->y2 - area->y1 + 1);

//     tft.startWrite();
//     tft.setAddrWindow(area->x1, area->y1, w, h);
//     tft.pushColors(&color_p->full, w * h, true);
//     tft.endWrite();

//     lv_disp_flush_ready(disp_drv);
// }

// /**********************
//  *   GLOBAL FUNCTIONS
//  **********************/

// void Display::init(void)
// {
//     /* Set Backlight mode */
//     ledcSetup(LCD_BL_PWM_CHANNEL, 5000, 8);
//     ledcAttachPin(LCD_BL_PIN, LCD_BL_PWM_CHANNEL);

//     /* Init lvgl */
//     lv_init();

//     /* Init display device */
//     tft.begin();
//     tft.setRotation(4); /* mirror */

//     /*-----------------------------
//      * Create a buffer for drawing
//      *----------------------------*/
//     static lv_disp_draw_buf_t draw_buf_dsc_1;
//     static lv_color_t buf_1[MY_DISP_HOR_RES * 10];                             /*A buffer for 10 rows*/
//     lv_disp_draw_buf_init(&draw_buf_dsc_1, buf_1, NULL, MY_DISP_HOR_RES * 10); /*Initialize the display buffer*/

//     /*-----------------------------------
//      * Register the display in LVGL
//      *----------------------------------*/
//     static lv_disp_drv_t disp_drv; /*Descriptor of a display driver*/
//     lv_disp_drv_init(&disp_drv);   /*Basic initialization*/

//     /*Set up the functions to access to your display*/

//     /*Set the resolution of the display*/
//     disp_drv.hor_res = MY_DISP_HOR_RES;
//     disp_drv.ver_res = MY_DISP_VER_RES;

//     /*Used to copy the buffer's content to the display*/
//     disp_drv.flush_cb = disp_flush;

//     /*Set a display buffer*/
//     disp_drv.draw_buf = &draw_buf_dsc_1;

//     /*Finally register the driver*/
//     lv_disp_drv_register(&disp_drv);
// }

// void Display::routine(void)
// {
//     lv_task_handler();
// }

// void Display::setBackLight(float duty)
// {
//     duty = constrain(duty, 0, 1);
//     duty = 1 - duty;
//     ledcWrite(LCD_BL_PWM_CHANNEL, (int)(duty * 255));
// }
