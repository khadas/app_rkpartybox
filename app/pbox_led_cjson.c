#include "pbox_led_cjson.h"
#include "slog.h"
//获取json文本的数据

const char *jsonfile = "/etc/led_effect.json";
const char *jsonfile1 = "/userdata/led_effect.json";
// static struct led_config led_config;

int HextoDecimal (const char *Hexstr)
{
	long sum=0;
	int t;
	int i = 0;
	for (i = 0; i < strlen(Hexstr); i++) {
		if (Hexstr[i] <= '9')
			t = Hexstr[i] - '0';
		else if(Hexstr[i] >= 'a')
			t = Hexstr[i] - 'a' + 10;
		else
			t = Hexstr[i] - 'A' + 10;
		sum = sum * 16 + t;
	}
	return sum;
}

char *get_json_data(const char *jsonfile)
{
	FILE *f_json = NULL;

	long json_size;

	char *json_data = NULL;
	ALOGD("open %s\n", jsonfile);
	f_json = fopen(jsonfile, "r");
	if (f_json == NULL) {
		ALOGE("open %s  failed\n", jsonfile);
		ALOGE("open %s\n", jsonfile1);
		f_json = fopen(jsonfile1, "r");
		if (f_json == NULL){
			ALOGE("try open %s  failed, return\n", jsonfile1);
			return NULL;
		}
	}
	fseek(f_json, 0, SEEK_END); //将指针移动到文件尾部

	json_size = ftell(f_json); //当前指针位置相对于文件首部偏移的字节数

	fseek(f_json, 0, SEEK_SET); //将指针移动到文件首部

	json_data = (char *) malloc(json_size + 1); //向系统申请分配指定size个字节的内存空间
	memset(json_data, 0, json_size + 1);
	fread((void *) json_data, json_size, 1, f_json); //将f_json中的数据读入中json_data中

	fclose(f_json);

	f_json = NULL;

	return (json_data);
}

static void dump_led_effect(struct led_effect* effect)
{
	ALOGI("----------led effect json dump start------------\n");
	ALOGI("back_color 0x%06x\n", effect->back_color);
	ALOGI("fore_color 0x%06x\n", effect->fore_color);
	ALOGI("period %d\n", effect->period);
	ALOGI("start %d\n", effect->start);
	ALOGI("num %d\n", effect->num);
	ALOGI("type %d\n", effect->led_effect_type);
	ALOGI("actions_per_period %d\n", effect->actions_per_period);
	// 	for (int i = 0; i < LED_NUM; i++)
	//  ALOGI("0x%06x ", effect->leds_color[i]);
	ALOGI("--------------------end----------------------\n\n");
}

int get_led_effect_data(struct light_effect_ctrl * ctrl, struct led_effect* effect, char *led_effect_name)
{
	ALOGD("get_led_effect_data: %s\n", led_effect_name);
	char *p = get_json_data(jsonfile);
	if (NULL == p){
		ALOGE("get_json_data failed, return\n");
		return -1;
	}
	cJSON * pJson = cJSON_Parse(p);
	if (NULL == pJson) {
		ALOGE("parse %s led_effect failed, return\n", led_effect_name);
		free(p);
		return -1;
	}
	cJSON * root = cJSON_GetObjectItem(pJson, led_effect_name);
	if (root) {
		cJSON *pback_color = cJSON_GetObjectItem(root, "back_color");
		if(pback_color) {
			effect->back_color =  HextoDecimal(pback_color->valuestring);
		}
		cJSON *pfore_color = cJSON_GetObjectItem(root, "fore_color");
		if(pfore_color){
			effect->fore_color =  HextoDecimal(pfore_color->valuestring);
		}
		cJSON *pperiod = cJSON_GetObjectItem(root, "period");
		if(pperiod){
			effect->period = pperiod->valueint;
		}
		cJSON *pstart = cJSON_GetObjectItem(root, "start");
		if(pstart){
			effect->start = pstart->valueint;
		}
		cJSON *pnum = cJSON_GetObjectItem(root, "num");
		if(pnum){
			effect->num = pnum->valueint;
			if (effect->num > ctrl->unit_num)
				effect->num = ctrl->unit_num;
		}
		cJSON *pscroll_num = cJSON_GetObjectItem(root, "scroll_num");
		if(pscroll_num){
			effect->scroll_num = pscroll_num->valueint;
		}
		cJSON *per_period = cJSON_GetObjectItem(root, "actions_per_period");
		if(per_period){
			effect->actions_per_period = per_period->valueint;
		}
		cJSON *ptype = cJSON_GetObjectItem(root, "led_effect_type");
		if(ptype){
			effect->led_effect_type = ptype->valueint;
		}	
	} else {
		ALOGE("cJSON_GetObjectItem led_effect %s failed, return\n",led_effect_name);
		cJSON_Delete(pJson);
		free(p);
		return -1;
	}
	ALOGD("parse %s led_effect success\n", led_effect_name);
	dump_led_effect(effect);
	cJSON_Delete(pJson);
	free(p);
	return 0;
}

static void dump_config(struct light_effect_ctrl *ctrl)
{
	ALOGE("----------base config dump start------------\n");
	ALOGE("light_unit_type %d\n", ctrl->unit_type);
	ALOGE("light_unit_num %d\n", ctrl->unit_num);
	ALOGE("rgb_order %d\n", ctrl->rgb_order);
	for(int i = 0; i < get_led_total_num(ctrl); i++) {
		ALOGE("==position_mapp[%d]:%d==\n", i, ctrl->position_mapp[i]);
	}
	ALOGE("--------------------end----------------------\n\n");
}

pbox_light_unit_type_t get_light_unit_type(char* str)
{
	if (strcmp(str,"rgb") == 0)
		return RK_LIGHT_UNIT_RGB;
	else if (strcmp(str,"led") == 0)
		return RK_LIGHT_UNIT_LED;
	else
		return RK_LIGHT_UNIT_UNKNOW;
}

pbox_rgb_order_t get_light_rgb_order(char* str)
{
	if (strcmp(str,"rgb") == 0)
		return RK_RGB_ORDER_RGB;
	else if (strcmp(str,"rbg") == 0)
		return RK_RGB_ORDER_RBG;
	else if (strcmp(str,"gbr") == 0)
		return RK_RGB_ORDER_GBR;
	else if (strcmp(str,"grb") == 0)
		return RK_RGB_ORDER_GRB;
	else if (strcmp(str,"brg") == 0)
		return RK_RGB_ORDER_BRG;
	else if (strcmp(str,"bgr") == 0)
		return RK_RGB_ORDER_BGR;
	else
		return RK_RGB_ORDER_UNKNOW;
}

int get_led_total_num(struct light_effect_ctrl *ctrl)
{
	if (ctrl->unit_type == RK_LIGHT_UNIT_LED)
		return ctrl->unit_num;
	else if (ctrl->unit_type == RK_LIGHT_UNIT_RGB)
		return ctrl->unit_num * 3;
}

int base_light_config_init(struct light_effect_ctrl *ctrl, char *config_name)
{
	int total_num;
	char str[16];

	ALOGD("get_config_data: %s\n", config_name);
	char *p = get_json_data(jsonfile);
	if (NULL == p){
		ALOGE("get_json_data failed, return\n");
		return -1;
	}
	cJSON * pJson = cJSON_Parse(p);
	if (NULL == pJson) {
		ALOGE("parse %s led_effect failed, return\n", config_name);
		free(p);
		return -1;
	}
	cJSON * root = cJSON_GetObjectItem(pJson, config_name);
	if (root) {
		cJSON *punit_type = cJSON_GetObjectItem(root, "light_unit_type");
		if(punit_type)
			ctrl->unit_type =  get_light_unit_type(punit_type->valuestring);
		else
			goto failed;

		cJSON *punit_num = cJSON_GetObjectItem(root, "light_unit_num");
		if(punit_num)
			ctrl->unit_num =  punit_num->valueint;
		else
			goto failed;

		cJSON *prgb_order = cJSON_GetObjectItem(root, "rgb_order");
		if(prgb_order)
			ctrl->rgb_order = get_light_rgb_order(prgb_order->valuestring);
		else
			goto failed;

		total_num = get_led_total_num(ctrl);

		ctrl->position_mapp = (int *)malloc(sizeof(int) * total_num);
		if (ctrl->position_mapp == NULL) {
			ALOGE("position_mapp malloc fail\n");
			goto failed;
		}
		memset(ctrl->position_mapp, 0, total_num * sizeof(int));

		for (int i = 0; i < total_num; i++) {
			memset(str, 0x00, sizeof(str));
			snprintf(str, sizeof(str), "position_mapp%d", i + 1);
			cJSON *pposition_mapp = cJSON_GetObjectItem(root, str);
			if(pposition_mapp){
				ctrl->position_mapp[i] = pposition_mapp->valueint;
			} else {
				ALOGE("could not find logic mapp %d\n", i + 1);
				free(ctrl->position_mapp);
				goto failed;
			}
		}

		ctrl->unit_fd = (int *)malloc(sizeof(int) * total_num);
		if (ctrl->unit_fd == NULL) {
			ALOGE("unit fd malloc fail\n");
			return -1;
		}
		memset(ctrl->unit_fd, 0, total_num *sizeof(int));

	} else {
		ALOGE("cJSON_GetObjectItem led_effect %s failed, return\n",config_name);
failed:
		cJSON_Delete(pJson);
		free(p);
		return -1;
	}
	ALOGD("parse %s led_effect success\n", config_name);
	dump_config(ctrl);
	cJSON_Delete(pJson);
	free(p);
	return 0;
}
