#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include "pbox_app.h"
#include "pbox_keyadcSara.h"
#include "pbox_keyadcSara_app.h"

#if ENABLE_SARAADC
void keyscan_knob_data_recv(struct _keyinfo keyinfo) {
    uint32_t value = keyinfo.value;
    switch(keyinfo.keycode) {
        case MIC1_BUTTON_BASS: {
            pbox_app_music_set_mic_bass(1, value, DISP_All);
        } break;
        case MIC1_BUTTON_TREBLE: {
            pbox_app_music_set_mic_treble(1, value, DISP_All);
        } break;
        case MIC1_BUTTON_REVERB: {
            pbox_app_music_set_mic_reverb(1, value, DISP_All);
        } break;
        case MIC2_BUTTON_BASS: {
            pbox_app_music_set_mic_bass(2, value, DISP_All);
        } break;
        case MIC2_BUTTON_TREBLE: {
            pbox_app_music_set_mic_treble(2, value, DISP_All);
        } break;
        case MIC2_BUTTON_REVERB: {
            pbox_app_music_set_mic_reverb(2, value, DISP_All);
        } break;
    }
}

void maintask_keyscan_fd_process(int fd) {
    char buff[sizeof(pbox_keyscan_msg_t)] = {0};
    int ret = recv(fd, buff, sizeof(buff), 0);
    int i = 0;
    if (ret <= 0) {
        if (ret == 0) {
            printf("%s: fd closed or EOF\n", __func__, fd);
        } else if (errno != EINTR) {
            perror("recvfrom");
        }
        return;
    }

    pbox_keyscan_msg_t *msg = (pbox_keyscan_msg_t *)buff;
    printf("%s recv: msgId:%d, keycode: %d\n", __func__, msg->msgId, msg->keyinfo.keycode);

    if (msg->type != PBOX_EVT)
        return;

    switch(msg->msgId) {
        case PBOX_KEYSCAN_BUTTON_EVENT: {

        } break;
        case PBOX_KEYSCAN_KNOB_EVENT: {
            keyscan_knob_data_recv(msg->keyinfo);
        } break;
    }

    return;
}
#endif