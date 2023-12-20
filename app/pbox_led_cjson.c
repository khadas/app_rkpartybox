#include "pbox_led_cjson.h"
//获取json文本的数据

const char *jsonfile = "/oem/led_effect.json";
const char *jsonfile1 = "/userdata/led_effect.json";
// static struct led_config led_config;

int HextoDecimal (const char *Hexstr){
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

char *get_json_data(const char *jsonfile) {
	FILE *f_json = NULL;

	long json_size;

	char *json_data = NULL;
	printf("open %s\n", jsonfile);
	f_json = fopen(jsonfile, "r");
	if (f_json == NULL) {
		printf("open %s  failed\n", jsonfile);
		printf("open %s\n", jsonfile1);
		f_json = fopen(jsonfile1, "r");
		if (f_json == NULL){
			printf("try open %s  failed, return\n", jsonfile1);
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

static void dump_led_effect(struct led_effect* effect) {
	printf("----------led effect json dump start------------\n");
	printf("back_color 0x%06x\n", effect->back_color);
	printf("fore_color 0x%06x\n", effect->fore_color);
	printf("period %d\n", effect->period);
	printf("start %d\n", effect->start);
	printf("num %d\n", effect->num);
	printf("type %d\n", effect->led_effect_type);
	printf("actions_per_period %d\n", effect->actions_per_period);
	// 	for (int i = 0; i < LED_NUM; i++)
	//  printf("0x%06x ", effect->leds_color[i]);
	printf("--------------------end----------------------\n\n");
}

int get_led_effect_data(struct led_effect* effect, char *led_effect_name)
{
    printf("get_led_effect_data: %s\n", led_effect_name);
    char *p = get_json_data(jsonfile);
    if (NULL == p){
        printf("get_json_data failed, return\n");
        return -1;
    }
    cJSON * pJson = cJSON_Parse(p);
    if (NULL == pJson) {
        printf("parse %s led_effect failed, return\n", led_effect_name);
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
        printf("cJSON_GetObjectItem led_effect %s failed, return\n",led_effect_name);
		cJSON_Delete(pJson);
		free(p);
		return -1;
    }
	printf("parse %s led_effect success\n", led_effect_name);
	dump_led_effect(effect);
    cJSON_Delete(pJson);
    free(p);
	return 0;
}
