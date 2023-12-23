#ifndef __PBOX_LED_CJSON_H_
#define __PBOX_LED_CJSON_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <cJSON.h>
#include <ctype.h>
#include "pbox_ledctrl.h"


char *get_json_data( const char *jsonfile);
int get_led_effect_data(struct light_effect_ctrl * ctrl, struct led_effect* effect, char *led_effect_name);
int HextoDecimal (const char *Hexstr);
int base_light_config_init(struct light_effect_ctrl *ctrl, char *config_name);

#ifdef __cplusplus
}
#endif
#endif //_LED_CJSON_H_
