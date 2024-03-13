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
#include "slog.h"

int led_userspace_ctrl_init(struct light_effect_ctrl * ctrl)
{
	int i;
	char str[64];
	int total_num;

	if (ctrl->userspace_ctrl_init)
		return 0;

	total_num = get_led_total_num(ctrl);

	for(int i = 0; i < total_num; i++) {
		ALOGD("==unit_fd[%d]:%d==\n", i, ctrl->unit_fd[i]);
	}
	for (i = 0; i < total_num; i++) {
		memset(str, 0x00, sizeof(str));

		snprintf(str, sizeof(str), "%sled%d/brightness",PATH_LED, ctrl->position_mapp[i]);//leds节点从1开始
		ALOGD("=========%s=========\n", str);
		ctrl->unit_fd[i] = open(str, O_WRONLY | O_CREAT, 0644);

		if (ctrl->unit_fd[i] < 0) {
			ALOGW("Error opening file %s\n", str);
			return led_userspace_ctrl_deinit(ctrl);
		}
	}

	for(int i = 0; i < total_num; i++) {
		ALOGI("==unit_fd[%d]:%d==\n", i, ctrl->unit_fd[i]);
	}
	ALOGD("user space led ctrl init OK !!!\n");
	ctrl->userspace_ctrl_init = 1;
	return 0;
}

int led_userspace_ctrl_deinit(struct light_effect_ctrl * ctrl)
{
	int i;
	int total_num;

	total_num = get_led_total_num(ctrl);

	for (i = 0; i < total_num; i++) {
		if (ctrl->unit_fd[i] != 0)
			close(ctrl->unit_fd[i]);
	}

	ctrl->userspace_ctrl_init = 0;

	ALOGW("led userspace ctrl deinit ok\n");
	return 0;
}

int userspace_set_rgb_color(struct light_effect_ctrl * ctrl, uint32_t rgb_index, uint8_t r, uint8_t g, uint8_t b)
{
	uint32_t led_index;
	char str[16];

	if (!ctrl->userspace_ctrl_init)
		return 0;

	if (rgb_index > ctrl->unit_num -1)
		rgb_index = ctrl->unit_num -1;

	led_index = rgb_index * 3;

	memset(str, 0x00, sizeof(str));
	snprintf(str, sizeof(str), "%d", g);
	// green
	if (write(ctrl->unit_fd[led_index], str, strlen(str)) != strlen(str)) {
		ALOGW("Error writing to file\n");
		close(ctrl->unit_fd[led_index]);
		return -1;
	}

	memset(str, 0x00, sizeof(str));
	snprintf(str, sizeof(str), "%d", r);
	// red
	if (write(ctrl->unit_fd[led_index + 1], str, strlen(str)) != strlen(str)) {
		ALOGW("Error writing to file\n");
		close(ctrl->unit_fd[led_index + 1]);
		return -1;
	}

	memset(str, 0x00, sizeof(str));
	snprintf(str, sizeof(str), "%d", b);
	// blue
	if (write(ctrl->unit_fd[led_index + 2], str, strlen(str)) != strlen(str)) {
		ALOGW("Error writing to file\n");
		close(ctrl->unit_fd[led_index+ 2]);
		return -1;
	}

	return 0;
}

int userspace_set_led_brightness(struct light_effect_ctrl * ctrl, uint32_t led_index, uint8_t brightness)
{
	char str[16];

	if (!ctrl->userspace_ctrl_init)
		return 0;

	memset(str, 0x00, sizeof(str));
	snprintf(str, sizeof(str), "%d", brightness);

	if (write(ctrl->unit_fd[led_index], str, strlen(str)) != strlen(str)) {
		ALOGW("Error writing to file\n");
		close(ctrl->unit_fd[led_index]);
		return 1;
	}

	return 0;
}
