#include <string.h>  
#include <errno.h>  
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <time.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <stdlib.h>
#include <pthread.h>
#include <signal.h>

#include "pbox_ledctrl.h"
#include "pbox_led_cjson.h"

int led_fd[LED_NUMBER] = {};
light_effect_ctrl_t light_effect_ctrl;

struct led_effect *leffect;

int led_ctrl_fd = -1;

int led_set_status(int status,struct led_effect *effect)
{
	if (ioctl(led_ctrl_fd, status, effect) == 0) {
		// printf("%s, fd %d set status success\n", __func__,led_ctrl_fd);
	} else {
		printf("%s,fd %d set fail \n", __func__,led_ctrl_fd);
		return -1;
	}

	return 0;
}

void* led_handle_command(void *led_effect_name)
{
	if(get_led_effect_data(leffect,led_effect_name) < 0) {
		return ((void *)-1);
	}

	led_set_status(RK_ECHO_SET_LED_EFFECT,leffect);

	return ((void *)0);
}

int leds_ctrl_init(void)
{
	if (light_effect_ctrl.driver_ctrl_init)
		return 0;

	leffect = malloc(sizeof(struct led_effect));
	memset(leffect,0,sizeof(struct led_effect));

	led_ctrl_fd = open(LEDS_CTRL_FILE, O_RDONLY);
	printf("%s, fd %d \n", __func__,led_ctrl_fd);
	if (led_ctrl_fd < 0) {
		fprintf(stderr,"%s,can't open file %s\n",__func__, LEDS_CTRL_FILE);
		free(leffect);
		return -1;
	}
	
	light_effect_ctrl.driver_ctrl_init = 1;

	return 0;
}

void led_ctrl_exit(void)
{
	close(led_ctrl_fd);
	free(leffect);
	
	light_effect_ctrl.driver_ctrl_init = 0;
}

pthread_t ledeffect_id;
int set_led_effect(char *led_effect_name)
{
	if(led_ctrl_fd < 0) {
		printf("can't open file %s \n",LEDS_CTRL_FILE);
		return -1;
	}
	printf("set_led_status start \n");
	int ret=pthread_create(&ledeffect_id,NULL,led_handle_command,led_effect_name);
	if(ret != 0){
		printf ("set_led_status create thread error!\n");
		return -1;
	}
	pthread_detach(ledeffect_id);

	return 0;
}

// int get_led_num()
// {
//     int *num = NULL;
//     if (ioctl(led_ctrl_fd, RK_ECHO_GET_LED_NUM, num) == 0) {

//         printf("%s, num=%d\n", __func__,num);
//     } else {
//         printf("%s,fd %d set fail \n", __func__,led_ctrl_fd);
//     }

//     return 0;
// }
#ifndef TUBE_LED

int set_led_status(char *led_effect_name)
{
	light_effect_ctrl.ctrl_mode = 0;
	return set_led_effect(led_effect_name);
}

int leds_multi_init()
{
	return leds_ctrl_init();
}

#else
int multi_ctrl_fd = -1;

int set_led_status(int status)
{
	struct tube_time tt;
	if(RK_ECHO_TIME == status) {
		tt.tm_hour = strtol(optarg, NULL, 10) / 100;
		tt.tm_min = strtol(optarg, NULL, 10) % 100;
		/* set time */
		ioctl(multi_ctrl_fd, RK_ECHO_TIME, &tt);
	} else {
		if (ioctl(multi_ctrl_fd, status, NULL) == 0) {
			printf("%s, fd %d set status %d success\n", __func__,multi_ctrl_fd, status);

		} else {
			printf("%s,fd %d set fail %d\n", __func__,multi_ctrl_fd, status);
		}
	}

	return 0;
}

int leds_multi_init()
{
	multi_ctrl_fd = open(LEDS_SIMPLE_CTRL_FILE, O_RDONLY);
	if(multi_ctrl_fd < 0)
	{
		printf("%s,can't open file %s\n", __func__, LEDS_SIMPLE_CTRL_FILE);
		return -1;
	}

	printf("%s,init success %d\n", __func__, multi_ctrl_fd);

	return multi_ctrl_fd;
}

#endif

int led_userspace_ctrl_init(int num)
{
	int i;
	char str[64];

	if (light_effect_ctrl.userspace_ctrl_init)
		return 0;

	for (i = 0; i < num; i++) {
		memset(str, 0x00, sizeof(str));

		snprintf(str, sizeof(str), "%sled%d/brightness",PATH_LED, i + 1);
		//printf("=========%s=========\n", str);
		led_fd[i] = open(str, O_WRONLY | O_CREAT, 0644);
		if (led_fd[i] < 0) {
			printf("Error opening file %s\n", str);
			return led_userspace_ctrl_deinit(num);
		}
	}

	printf("user space led ctrl init OK !!!\n");
	light_effect_ctrl.userspace_ctrl_init = 1;
	return 0;
}

int led_userspace_ctrl_deinit(int num)
{
	printf("led userspace ctrl deinit\n");

	int i;
	for (i = 0; i < num; i++) {
		if (led_fd[i] != 0)
			close(led_fd[i]);
	}

	light_effect_ctrl.userspace_ctrl_init = 0;

	printf("led userspace ctrl deinit ok\n");
	return 0;
}

int userspace_set_led_color(uint32_t rgb_index, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t led_index;
	char str[16];

	if (!light_effect_ctrl.userspace_ctrl_init && !light_effect_ctrl.ctrl_mode)
		return 0;

	if (rgb_index > RGB_LED_NUMBER -1)
		rgb_index = RGB_LED_NUMBER -1;

	led_index = rgb_index * 3;

	memset(str, 0x00, sizeof(str));
	snprintf(str, sizeof(str), "%d", g);
	// green
	if (write(led_fd[led_index], str, strlen(str)) != strlen(str)) {
		printf("Error writing to file\n");
		close(led_fd[led_index]);
		return 1;
	}

	memset(str, 0x00, sizeof(str));
	snprintf(str, sizeof(str), "%d", r);
	// red
	if (write(led_fd[led_index + 1], str, strlen(str)) != strlen(str)) {
		printf("Error writing to file\n");
		close(led_fd[led_index + 1]);
		return 1;
	}

	memset(str, 0x00, sizeof(str));
	snprintf(str, sizeof(str), "%d", b);
	// blue
	if (write(led_fd[led_index + 2], str, strlen(str)) != strlen(str)) {
		printf("Error writing to file\n");
		close(led_fd[led_index+ 2]);
		return 1;
	}

	return 0;
}

