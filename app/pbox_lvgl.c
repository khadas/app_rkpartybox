/******************************************************
 * In ideal mode, this file can display ui info base on
 * LVGL or other display frameworks.
 * as now, we just display on LVGL.
 * ***************************************************/
#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/un.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/time.h>
#include "pbox_common.h"
#include "pbox_socketpair.h"
#include "pbox_lvgl.h"
#if ENABLE_LCD_DISPLAY
#include "lv_demo_music.h"
#endif

#if (ENABLE_LCD_DISPLAY == 0)
#define _lv_demo_music_update_ui_info(...)         ((void) 0)
#define _lv_demo_music_update_list(...)         ((void) 0)
#endif

// Define a function pointer type for event handlers
typedef void (*LcdCmdHandler)(const pbox_lcd_msg_t*);

// Define a struct to associate opcodes with handlers
typedef struct {
    pbox_lcd_opcode_t opcode;
    LcdCmdHandler handler;
} LcdCmdHandler_t;

int unix_socket_lcd_notify(void *info, int length) {
    #if ENABLE_LCD_DISPLAY
    return unix_socket_notify_msg(PBOX_MAIN_LVGL, info, length);
    #endif
}

// Notify function for update trackid event
void lcd_pbox_notifyTrackid(uint32_t id) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_PLAY_TRACKID_EVT,
    };
    msg.trackId = id;
    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the stop event
void lcd_pbox_notifyPlayStop() {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_PLAY_STOP_EVT,
    };

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the play/pause event
void lcd_pbox_notifyPlayPause(bool play) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_PLAY_PAUSE_EVT,
    };
    msg.play = play;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the previous/next track event
void lcd_pbox_notifyPrevNext(bool next) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_PREV_NEXT_EVT,
    };
    msg.next = next;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the loop mode event
void lcd_pbox_notifyLoopMode(bool loop) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_LOOP_MODE_EVT,
    };
    msg.loop = loop;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the seek position event
void lcd_pbox_notifySeekPosition(unsigned int mCurrent, unsigned int mDuration) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_SEEK_POSITION_EVT,
    };
    msg.positions.mCurrent = mCurrent;
    msg.positions.mDuration = mDuration;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the main volume level event
void lcd_pbox_notifyMainVolLevel(uint32_t mainVolume) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_MAIN_VOL_LEVEL_EVT,
    };
    msg.mainVolume = mainVolume;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the microphone volume level event
void lcd_pbox_notifyMicVolLevel(uint32_t micVolume) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_MIC_VOL_LEVEL_EVT,
    };
    msg.micVolume = micVolume;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the accompaniment music level event
void lcd_pbox_notifyAccompMusicLevel(uint32_t accomp_music_level) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_ACCOMP_MUSIC_LEVEL_EVT,
    };
    msg.accomp_music_level = accomp_music_level;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the human voice music level event
void lcd_pbox_notifyHumanMusicLevel(uint32_t human_music_level) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_HUMAN_MUSIC_LEVEL_EVT,
    };
    msg.human_music_level = human_music_level;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the music separate switch event
void lcd_pbox_notifySeparateSwitch(bool enable) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_SEPERATE_SWITCH_EVT,
    };
    msg.enable = enable;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the echo 3A event
void lcd_pbox_notifyEcho3A(bool echo3a) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_ECHO_3A_EVT,
    };
    msg.echo3a = echo3a;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the reverb mode event
void lcd_pbox_notifyReverbMode(pbox_revertb_t reverbMode) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_REVERT_MODE_EVT,
    };
    msg.reverbMode = reverbMode;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

// Notify function for the reserv music level event
void lcd_pbox_notifyReservMusicLevel(uint32_t reserv_music_level) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_RESERV_MUSIC_LEVEL_EVT,
    };
    msg.reserv_music_level = reserv_music_level;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_lvgl_init(void) {
    //lvgl init. it leed to display the lvgl UI.
#if LV_USE_DEMO_MUSIC
    lv_demo_music();
#endif
}

// Function to handle the play/pause command
void handleLcdPlayPauseCmd(const pbox_lcd_msg_t* msg) {
    bool play = msg->play;
    ALOGD("Play/Pause Command: %s\n", play ? "Play" : "Pause");
    _lv_demo_music_update_ui_info(UI_WIDGET_PLAY_PAUSE, msg);
}

// Function to handle the previous/next track command
void handleLcdPrevNextCmd(const pbox_lcd_msg_t* msg) {
    bool next = msg->next;
    ALOGD("Prev/Next Command: %s\n", next ? "Next" : "Previous");
}

// Function to handle the track info command
void handleLcdTrackInfoCmd(const pbox_lcd_msg_t* msg) {
    char title[MAX_MUSIC_NAME_LENGTH + 1] = {0};
    char artist[MAX_APP_NAME_LENGTH + 1] = {0};
    if(msg->track.title)
        strncpy(title, msg->track.title, MAX_MUSIC_NAME_LENGTH);
    if(msg->track.artist)
        strncpy(artist, msg->track.artist, MAX_APP_NAME_LENGTH);
    ALOGD("Track Info Command: Title - %s, Artist - %s\n", title, artist);
    _lv_demo_music_update_ui_info(UI_WIDGET_TRACK_INFO, msg);
}

// Function to handle the track position command
void handleLcdTrackPositionCmd(const pbox_lcd_msg_t* msg) {
    uint32_t mCurrent = msg->positions.mCurrent;
    uint32_t mDuration = msg->positions.mDuration;
    //ALOGD("Track Position Command: Current - %u, Duration - %u\n", mCurrent, mDuration);
    _lv_demo_music_update_ui_info(UI_WIDGET_POSITION_INFO, msg);
}

//Function to handle the track list update commmand
void handleLcdUsbStateUpdateCmd(const pbox_lcd_msg_t *msg) {
    ALOGD("%s \n", __func__);
    switch (msg->usbState) {
        case USB_SCANNED:
        case USB_CONNECTED: {
            ALOGW("USB connected\n");
            _lv_demo_music_update_ui_info(UI_WIDGET_DEVICE_STATE, msg);
        } break;
        case USB_DISCONNECTED: {
            ALOGW("USB Disk Removed\n");
            _lv_demo_music_update_ui_info(UI_WIDGET_DEVICE_STATE, msg);
        } break;
    }
}

void handleLcdUsbListUpdateCmd(const pbox_lcd_msg_t *msg) {
    ALOGD("%s trackId:%d\n", __func__, msg->trackId);
    _lv_demo_music_update_list(msg->trackId);
}

void handleLcdBtStateUpdateCmd(const pbox_lcd_msg_t *msg) {
    ALOGD("%s \n", __func__);
    switch (msg->btState) {
        case APP_BT_NONE:
        case APP_BT_TURNING_TRUNNING_OFF:
        case APP_BT_INIT_ON:
        case APP_BT_CONNECTING:
        case APP_BT_DISCONNECT: {
            ALOGD("BT DISCONNECT\n");
            _lv_demo_music_update_ui_info(UI_WIDGET_DEVICE_STATE, msg);
        } break;
        case APP_BT_CONNECTED: {
            ALOGD("BT CONNECTED\n");
            _lv_demo_music_update_ui_info(UI_WIDGET_DEVICE_STATE, msg);
            //_lv_demo_music_update_list();
        } break;
    }
}

void handleLcdUacStateUpdateCmd(const pbox_lcd_msg_t *msg) {
    ALOGD("%s uac started:%d\n", __func__, msg->uac_start);
    _lv_demo_music_update_ui_info(UI_WIDGET_DEVICE_STATE, msg);
}
// Function to handle the main volume level command
void handleLcdMainVolLevelCmd(const pbox_lcd_msg_t* msg) {
    uint32_t mainVolume = msg->mainVolume;
    ALOGD("Main Volume Level Command: Level - %u\n", mainVolume);
    _lv_demo_music_update_ui_info(UI_WIDGET_MAIN_VOLUME, msg);
}

// Function to handle the mic volume level command
void handleLcdMicVolLevelCmd(const pbox_lcd_msg_t* msg) {
    uint32_t micVolume = msg->micVolume;
    ALOGD("Mic Volume Level Command: Level - %u\n", micVolume);
    _lv_demo_music_update_ui_info(UI_WIDGET_MIC_VOLUME, msg);
}

// Function to handle the mic mute command
void handleLcdMicmuteCmd(const pbox_lcd_msg_t* msg) {
    bool mute = msg->micmute;
    ALOGD("Mic mute Command State - %s\n", mute? "on":"off");
    _lv_demo_music_update_ui_info(UI_WIDGET_MIC_MUTE, msg);
}

// Function to handle the accompaniment music level command
void handleLcdAccompMusicLevelCmd(const pbox_lcd_msg_t* msg) {
    uint32_t accomp_music_level = msg->accomp_music_level;
    ALOGD("Accompaniment Music Level Command: Level - %u\n", accomp_music_level);
}

// Function to handle the human music level command
void handleLcdHumanMusicLevelCmd(const pbox_lcd_msg_t* msg) {
    uint32_t human_music_level = msg->human_music_level;
    ALOGD("Human Music Level Command: Level - %u\n", human_music_level);
}

// Function to handle the music separate switch command
void handleLcdMusicSeparateSwitchCmd(const pbox_lcd_msg_t* msg) {
    pbox_vocal_t vocalSeparate = msg->vocalSeparate;
    ALOGD("Music Separate Switch Command: Enable - %s, Human Level - %u, Music Level - %u, Reserv Level - %u\n",
           vocalSeparate.enable ? "Enabled" : "Disabled", 
           vocalSeparate.humanLevel,
            vocalSeparate.accomLevel,
           vocalSeparate.reservLevel);
    _lv_demo_music_update_ui_info(UI_WIDGET_VOCAL_SEPERATE, msg);
}

// Function to handle the echo 3A switch command
void handleLcdEcho3ASwitchCmd(const pbox_lcd_msg_t* msg) {
    bool echo3a = msg->echo3a;
    ALOGD("Echo 3A Switch Command: %s\n", echo3a ? "On" : "Off");
    _lv_demo_music_update_ui_info(UI_WIDGET_3A_SWITCH, msg);
}

// Function to handle the reverb mode command
void handleLcdReverbModeCmd(const pbox_lcd_msg_t* msg) {
    pbox_revertb_t reverbMode = msg->reverbMode;
    ALOGD("Reverb Mode Command: Mode - %d\n", reverbMode);
    _lv_demo_music_update_ui_info(UI_WIDGET_REVERTB_MODE, msg);
}

// Function to handle the loop mode command
void handleLcdLoopModeCmd(const pbox_lcd_msg_t* msg) {
    bool loop = msg->loop;
    ALOGD("Loop Mode Command: %s\n", loop ? "Looping" : "Not Looping");
}

// Function to handle the energy info command
void handleLcdEnergyInfoCmd(const pbox_lcd_msg_t* msg) {
    energy_info_t energyData = msg->energy_data;
    //ALOGD("Energy Info Command, Size: %d\n", energyData.size);
    // For each energy data, print its value
    #if 0
    for (int i = 0; i < energyData.size; i++) {
        if(i==0) ALOGD("freq  :\t");
        ALOGD("%05d%c", energyData.energykeep[i].freq, i<(energyData.size-1)?'\t':' ');
    }
    ALOGD("\n");
    for (int i = 0; i < energyData.size; i++) {
        if(i==0) ALOGD("energy:\t");
        ALOGD("%02d%c", energyData.energykeep[i].energy, i<(energyData.size-1)?'\t':' ');
    }
    ALOGD("\n");
    #endif
    _lv_demo_music_update_ui_info(UI_WIDGET_SPECTRUM_CHART, msg);
}

// Function to handle the reserv level command
void handleLcdReservLevelCmd(const pbox_lcd_msg_t* msg) {
    uint32_t reserv_music_level = msg->reserv_music_level;
    ALOGD("Reserv Level Command: Level - %u\n", reserv_music_level);
}

// Function to handle the gui reflash command //exec lv_task_handler
void handleLcdGuiReflushCmd(const pbox_lcd_msg_t* msg) {
    (void*)(msg);
    //ALOGD("GUI reflash Command\n");
#if LV_USE_DEMO_MUSIC
    lv_task_handler();
#endif
}

// Array of event handlers
const LcdCmdHandler_t lcdEventHandlers[] = {
    { PBOX_LCD_DISP_PLAY_PAUSE, handleLcdPlayPauseCmd },
    { PBOX_LCD_DISP_PREV_NEXT, handleLcdPrevNextCmd },
    { PBOX_LCD_DISP_TRACK_INFO, handleLcdTrackInfoCmd },
    { PBOX_LCD_DISP_TRACK_POSITION, handleLcdTrackPositionCmd },
    { PBOX_LCD_DISP_USB_LIST_UPDATE, handleLcdUsbListUpdateCmd },
    { PBOX_LCD_DISP_USB_STATE, handleLcdUsbStateUpdateCmd },
    { PBOX_LCD_DISP_BT_STATE, handleLcdBtStateUpdateCmd },
    { PBOX_LCD_DISP_UAC_STATE, handleLcdUacStateUpdateCmd},
    { PBOX_LCD_DISP_MAIN_VOL_LEVEL, handleLcdMainVolLevelCmd },
    { PBOX_LCD_DISP_MIC_VOL_LEVEL, handleLcdMicVolLevelCmd },
    { PBOX_LCD_DISP_MIC_MUTE, handleLcdMicmuteCmd },
    { PBOX_LCD_DISP_ACCOMP_MUSIC_LEVEL, handleLcdAccompMusicLevelCmd },
    { PBOX_LCD_DISP_HUMAN_MUSIC_LEVEL, handleLcdHumanMusicLevelCmd },
    { PBOX_LCD_DISP_MUSIC_SEPERATE_SWITCH, handleLcdMusicSeparateSwitchCmd },
    { PBOX_LCD_DISP_ECHO_3A_SWITCH, handleLcdEcho3ASwitchCmd },
    { PBOX_LCD_DISP_REVERT_MODE, handleLcdReverbModeCmd },
    { PBOX_LCD_DISP_LOOP_MODE, handleLcdLoopModeCmd },
    { PBOX_LCD_DISP_ENERGY_INFO, handleLcdEnergyInfoCmd },
    { PBOX_LCD_DISP_RESERV_LEVEL, handleLcdReservLevelCmd },
    { PBOX_LCD_DISP_REFLASH, handleLcdGuiReflushCmd},

    // Add other as needed...
};

// Function to process an incoming pbox_lcd_msg_t event
void process_pbox_lcd_cmd(const pbox_lcd_msg_t* msg) {
    if (msg == NULL) {
        ALOGD("Error: Null event message received.\n");
        return;
    }

    // Iterate over the LcdCmdHandlers array
    for (int i = 0; i < sizeof(lcdEventHandlers) / sizeof(lcdEventHandlers[0]); ++i) {
        if (lcdEventHandlers[i].opcode == msg->msgId) {
            // Call the corresponding event handler
            if (lcdEventHandlers[i].handler != NULL) {
                lcdEventHandlers[i].handler(msg);
            }
            return; // Exit after handling the event
        }
    }

    ALOGD("Warning: No handler found for event ID %d.\n", msg->msgId);
}

static void *pbox_touchLCD_server(void *arg)
{
    int sock_fd;
    char buff[sizeof(pbox_lcd_msg_t)] = {0};
    pbox_lcd_msg_t *msg;
    pthread_setname_np(pthread_self(), "pbox_lcd");

    pbox_lvgl_init();
    #if ENABLE_LCD_DISPLAY
    sock_fd = get_server_socketpair_fd(PBOX_SOCKPAIR_LVGL);
    #endif

    if (sock_fd < 0) {
        perror("Failed to create UDP socket");
        return (void *)-1;
    }

    fd_set read_fds;
    FD_ZERO(&read_fds);
    FD_SET(sock_fd, &read_fds);

    while(true) {
        fd_set read_set = read_fds;

        int result = select(sock_fd+1, &read_set, NULL, NULL, NULL);
        if (result < 0) {
            if (errno != EINTR) {
                perror("select failed");
                break;
            }
            continue; // Interrupted by signal, restart select
        } else if (result == 0) {
            ALOGW("select timeout or no data\n");
            continue;
        }
        int ret = recv(sock_fd, buff, sizeof(buff), 0);
        if (ret <= 0) {
            if (ret == 0) {
                ALOGW("Socket closed\n");
                break;
            } else {
                perror("recvfrom failed");
                continue;
            }
        }

        pbox_lcd_msg_t *msg = (pbox_lcd_msg_t *)buff;
        //ALOGD("%s recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

        if(msg->type == PBOX_EVT)
            continue;

        process_pbox_lcd_cmd(msg);
    }
}

pthread_t touchLcd_server_task_id;

int pbox_create_lvglTask(void)
{
    int ret;

    ret = pthread_create(&touchLcd_server_task_id, NULL, pbox_touchLCD_server, NULL);
    if (ret < 0)
    {
        ALOGE("touchLCD server start failed\n");
    }

    return ret;
}
