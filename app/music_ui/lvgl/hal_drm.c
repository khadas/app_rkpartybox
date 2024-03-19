#include <lvgl/lvgl.h>
#include <lvgl/lv_conf.h>
#include <lvgl/lv_drivers/display/drm.h>
extern void drm_disp_drv_init(int rot);

void hal_drm_init(lv_coord_t hor_res, lv_coord_t ver_res, int rotated)
{
    /*Create a display*/
    drm_disp_drv_init(rotated * 90 % 360);
}

