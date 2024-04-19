#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include <pthread.h>
#include <fcntl.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include "pbox_common.h"
#include "pbox_lvgl.h"
#include "pbox_lvgl_app.h"
#include "pbox_app.h"
#include "pbox_socket.h"
#include "pbox_socketpair.h"
//xxx_app means it works in main thread...

int unix_socket_lcd_send(void *info, int length)
{
    #if ENABLE_LCD_DISPLAY
    return unix_socket_send_cmd(PBOX_CHILD_LVGL, info, length);
    #endif
}

void pbox_app_lcd_displayPlayPause(bool play) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_PLAY_PAUSE,
    };
    msg.play = play;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayPrevNext(bool next) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_PREV_NEXT,
    };
    msg.next = next;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayTrackInfo(const char* title, const char* artist) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_TRACK_INFO,
    };
    if(title)
    strncpy(msg.track.title, title, MAX_MUSIC_NAME_LENGTH);
    if(artist)
    strncpy(msg.track.artist, artist, MAX_APP_NAME_LENGTH);
    msg.track.title[MAX_MUSIC_NAME_LENGTH]  = 0;
    msg.track.artist[MAX_APP_NAME_LENGTH] = 0;
    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayTrackPosition(bool durationOnly, uint32_t mCurrent, uint32_t mDuration) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_TRACK_POSITION,
    };
    msg.positions.onlyDuration = durationOnly;
    msg.positions.mCurrent = mCurrent;
    msg.positions.mDuration = mDuration;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayMainVolumeLevel(uint32_t mainVolume) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_MAIN_VOL_LEVEL,
    };
    msg.mainVolume = mainVolume;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayMicVolumeLevel(uint32_t micVolume) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_MIC_VOL_LEVEL,
    };
    msg.micVolume = micVolume;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayMicMute(bool mute) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_MIC_MUTE,
    };
    msg.micmute = mute;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayAccompMusicLevel(uint32_t accomp_music_level) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_ACCOMP_MUSIC_LEVEL,
    };
    msg.accomp_music_level = accomp_music_level;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayHumanMusicLevel(uint32_t human_music_level) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_HUMAN_MUSIC_LEVEL,
    };
    msg.human_music_level = human_music_level;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayReservLevel(uint32_t reserv_music_level) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_RESERV_LEVEL,
    };
    msg.reserv_music_level = reserv_music_level;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayVocalFadeoutSwitch(bool enable, uint32_t hlevel, uint32_t alevel, uint32_t rlevel) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_MUSIC_SEPERATE_SWITCH,
    };
    pbox_vocal_t vocalSeparate = {
        .enable = enable,
        .humanLevel = hlevel,
        .accomLevel = alevel,
        .reservLevel = rlevel,
    };

    msg.vocalSeparate = vocalSeparate;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayEcho3A(bool on) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_ECHO_3A_SWITCH,
    };
    msg.echo3a = on;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_displayRevertbMode(pbox_revertb_t mode) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_REVERT_MODE,
    };
    msg.reverbMode = mode;

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_dispplayReflash(void) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_REFLASH,
    };

    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_dispplayEnergy(energy_info_t energy) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_ENERGY_INFO,
    };

    msg.energy_data = energy;
    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_dispplayUsbState(usb_state_t state) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_USB_STATE,
    };

    msg.usbState = state;
    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_dispplayUsbListupdate(uint32_t trackId) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_USB_LIST_UPDATE,
    };

    msg.trackId = trackId;
    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_app_lcd_dispplaybtState(btsink_state_t state) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_BT_STATE,
    };

    msg.btState = state;
    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}


void pbox_app_lcd_dispplayUacState(bool start) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_CMD,
        .msgId = PBOX_LCD_DISP_UAC_STATE,
    };

    msg.uac_start = start;
    unix_socket_lcd_send(&msg, sizeof(pbox_lcd_msg_t));
}

int maintask_touch_lcd_data_recv(pbox_lcd_msg_t *msg)
{
    assert(msg);
    switch (msg->msgId) {
        case PBOX_LCD_PLAY_PAUSE_EVT: {
            bool play = msg->play;
            if(play) {
                pbox_app_music_resume(DISP_All);
            } else {
                pbox_app_music_pause(DISP_All);
            }
        } break;
        case PBOX_LCD_PLAY_TRACKID_EVT: {
            int32_t id = msg->trackId;
            pbox_app_music_trackid(id, DISP_All);
        } break;
        case PBOX_LCD_PLAY_STOP_EVT: {
            pbox_app_music_stop(DISP_All);
        } break;
        case PBOX_LCD_PREV_NEXT_EVT: {
            bool next = msg->next;
            pbox_app_music_album_next(next, DISP_LED);
        } break;
        case PBOX_LCD_LOOP_MODE_EVT: {
            bool mode = msg->loop;
            pbox_app_music_album_loop((uint32_t)(mode), DISP_LED);
        } break;
        case PBOX_LCD_SEEK_POSITION_EVT: {
            unsigned int msecSeekTo = msg->positions.mCurrent;
            unsigned int msecDuration = msg->positions.mDuration;
            if (msecSeekTo <= msecDuration)
                pbox_app_music_seek_position(msecSeekTo, msecDuration, DISP_LED);
        } break;
        case PBOX_LCD_MAIN_VOL_LEVEL_EVT: {
            float volume = msg->mainVolume;
            volume = (MAX_MAIN_VOLUME-MIN_MAIN_VOLUME)*volume/100 + MIN_MAIN_VOLUME; //covert to real db volume.
            if(volume == pboxUIdata->mainVolumeLevel) break;
            pbox_app_music_set_main_volume(volume, DISP_LED|DISP_FS);
        } break;
        case PBOX_LCD_MIC_VOL_LEVEL_EVT: {
            float mic_volume = msg->micVolume;
            mic_volume = (MAX_MIC_PHONE_VOLUME-MIN_MIC_PHONE_VOLUME)*mic_volume/100 + MIN_MIC_PHONE_VOLUME; //covert to real db volume.
            if(mic_volume == pboxUIdata->micData[0].micVolume) break;
            pbox_app_music_set_mic_volume(0, mic_volume, DISP_LED|DISP_FS);
        } break;
        case PBOX_LCD_ACCOMP_MUSIC_LEVEL_EVT: {
            int32_t accomp_level = msg->accomp_music_level;
            if(accomp_level == pboxUIdata->accomLevel) break;
            pbox_app_music_set_accomp_music_level(accomp_level, DISP_LED);
        } break;
        case PBOX_LCD_HUMAN_MUSIC_LEVEL_EVT: {
            int32_t humanLevel = msg->human_music_level;
            if(humanLevel == pboxUIdata->humanLevel) break;
            pbox_app_music_set_human_music_level(humanLevel, DISP_LED);
        } break;
        case PBOX_LCD_RESERV_MUSIC_LEVEL_EVT: {
            int32_t reserv_level = msg->reserv_music_level;
            if(reserv_level == pboxUIdata->reservLevel) break;
            pbox_app_music_set_reserv_music_level(reserv_level, DISP_LED);
        } break;
        case PBOX_LCD_SEPERATE_SWITCH_EVT: {
            bool enable = msg->enable;
            if(enable == pboxUIdata->vocalSplit) break;
            pbox_app_music_original_singer_open(!enable, DISP_LED);
        } break;
        case PBOX_LCD_ECHO_3A_EVT: {
            bool enable = msg->enable;
            if(enable == pboxUIdata->micData[0].echo3a) break;
            pbox_app_music_set_echo_3a(0, enable, DISP_LED);
        } break;
        case PBOX_LCD_REVERT_MODE_EVT: {
            pbox_revertb_t revertb = msg->reverbMode;
            if(revertb == pboxUIdata->micData[0].reverbMode) break;
            pbox_app_music_set_recoder_revert(0, revertb, DISP_LED|DISP_FS);
        } break;
        default: break;
    } //end switch (msg->msgId)
}

void maintask_lvgl_fd_process(int fd) {
    int bytesAvailable = -1;
    char buff[sizeof(pbox_lcd_msg_t)] = {0};
    int ret = recv(fd, buff, sizeof(buff), 0);
    if (ret <= 0) {
        if (ret == 0) {
            ALOGW("%s: Connection closed\n", __func__);
        } else if (errno != EINTR) {
            perror("recvfrom");
        }
        return;
    }

    pbox_lcd_msg_t *msg = (pbox_lcd_msg_t *)buff;
    ALOGD("%s: Socket received - type: %d, id: %d\n", __func__, msg->type, msg->msgId);

    if (msg->type != PBOX_EVT) {
        ALOGW("%s: Invalid message type\n", __func__);
        return;
    }

    maintask_touch_lcd_data_recv(msg);
}