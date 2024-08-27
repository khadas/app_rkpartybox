#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <linux/input.h>

#if 0
#define KEY_MODE                0x175
#define KEY_VOLUMEDOWN          114
#define KEY_VOLUMEUP            115
#define KEY_PLAYPAUSE           164
#define KEY_PLAY                207
#define KEY_PREVIOUSSONG        165
#define KEY_NEXTSONG            163
#define KEY_MICMUTE             248
#endif

#define  key_event_name  "/dev/input/by-path/platform-adc-keys-event"

#define check_write_result(a) \
    do {if(a == -1) {fprintf(stderr, "%s %d: %s\n", __func__, __LINE__, strerror(errno));}} while (0)

void simulate_key(int fd,int keycode)
{
    struct input_event event;
    ssize_t result;

    event.type = EV_KEY;
    event.value = 1;
    event.code = keycode;
    gettimeofday(&event.time,0);
    result = write(fd,&event,sizeof(event)) ;
    check_write_result(result);

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    result = write(fd, &event, sizeof(event));
    check_write_result(result);

    memset(&event, 0, sizeof(event));
    gettimeofday(&event.time, NULL);
    event.type = EV_KEY;
    event.code = keycode;
    event.value = 0;
    result = write(fd, &event, sizeof(event));
    check_write_result(result);

    event.type = EV_SYN;
    event.code = SYN_REPORT;
    event.value = 0;
    result = write(fd, &event, sizeof(event));
    check_write_result(result);
}

int main(int argc, char **argv)
{
    int fd;
    int keycode = 0;
    fd = open(key_event_name, O_RDWR);
    if(fd <= 0){
        fprintf(stderr, "error open keyboard:\n");
        return -1;
    }

    if (argc < 2) {
	fprintf(stderr, "must input a keycode:\n" 
	"Supported key codes: \n" 
	"Mode     : 373\n"
	"Play     : 207\n"
	"VolumeUp : 115\n"
	"VolumeDn : 114\n"
	"MICMute  : 248\n"
	"for example:\n" 
	"inputkey 373\n"
	);
    close(fd);
	return -1;
    }

    fprintf(stderr, "keycode is %s\n", argv[1]);

    keycode = atoi(argv[1]); 

    simulate_key(fd, keycode);

    close(fd);

    return 0;
}
