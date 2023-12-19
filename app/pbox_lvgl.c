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
#include "pbox_lvgl.h"

// Define a function pointer type for event handlers
typedef void (*LcdCmdHandler)(const pbox_lcd_msg_t*);

// Define a struct to associate opcodes with handlers
typedef struct {
    pbox_lcd_opcode_t opcode;
    LcdCmdHandler handler;
} LcdCmdHandler_t;

int unix_socket_lcd_notify(void *info, int length) {
    return unix_socket_notify_msg(PBOX_MAIN_LVGL, info, length);
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
void lcd_pbox_notifyEcho3A(bool echo3A_On) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_ECHO_3A_EVT,
    };
    msg.echo3A_On = echo3A_On;

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

// Notify function for the guitar music level event
void lcd_pbox_notifyGuitarMusicLevel(uint32_t guitar_music_level) {
    pbox_lcd_msg_t msg = {
        .type = PBOX_EVT,
        .msgId = PBOX_LCD_GUITAR_MUSIC_LEVEL_EVT,
    };
    msg.guitar_music_level = guitar_music_level;

    unix_socket_lcd_notify(&msg, sizeof(pbox_lcd_msg_t));
}

void pbox_lvgl_init(void) {
    //lvgl init. it leed to display the lvgl UI.
}

// Function to handle the play/pause command
void handleLcdPlayPauseCmd(const pbox_lcd_msg_t* msg) {
    bool play = msg->play;
    printf("Play/Pause Command: %s\n", play ? "Play" : "Pause");
}

// Function to handle the previous/next track command
void handleLcdPrevNextCmd(const pbox_lcd_msg_t* msg) {
    bool next = msg->next;
    printf("Prev/Next Command: %s\n", next ? "Next" : "Previous");
}

// Function to handle the track info command
void handleLcdTrackInfoCmd(const pbox_lcd_msg_t* msg) {
    char title[MAX_APP_NAME_LENGTH + 1];
    char artist[MAX_APP_NAME_LENGTH + 1];
    strncpy(title, msg->track.title, MAX_APP_NAME_LENGTH);
    strncpy(artist, msg->track.artist, MAX_APP_NAME_LENGTH);
    printf("Track Info Command: Title - %s, Artist - %s\n", title, artist);
}

// Function to handle the track position command
void handleLcdTrackPositionCmd(const pbox_lcd_msg_t* msg) {
    unsigned int mCurrent = msg->positions.mCurrent;
    unsigned int mDuration = msg->positions.mDuration;
    printf("Track Position Command: Current - %u, Duration - %u\n", mCurrent, mDuration);
}

// Function to handle the main volume level command
void handleLcdMainVolLevelCmd(const pbox_lcd_msg_t* msg) {
    uint32_t mainVolume = msg->mainVolume;
    printf("Main Volume Level Command: Level - %u\n", mainVolume);
}

// Function to handle the mic volume level command
void handleLcdMicVolLevelCmd(const pbox_lcd_msg_t* msg) {
    uint32_t micVolume = msg->micVolume;
    printf("Mic Volume Level Command: Level - %u\n", micVolume);
}

// Function to handle the accompaniment music level command
void handleLcdAccompMusicLevelCmd(const pbox_lcd_msg_t* msg) {
    uint32_t accomp_music_level = msg->accomp_music_level;
    printf("Accompaniment Music Level Command: Level - %u\n", accomp_music_level);
}

// Function to handle the human music level command
void handleLcdHumanMusicLevelCmd(const pbox_lcd_msg_t* msg) {
    uint32_t human_music_level = msg->human_music_level;
    printf("Human Music Level Command: Level - %u\n", human_music_level);
}

// Function to handle the music separate switch command
void handleLcdMusicSeparateSwitchCmd(const pbox_lcd_msg_t* msg) {
    pbox_vocal_t vocalSeparate = msg->vocalSeparate;
    printf("Music Separate Switch Command: Enable - %s, Human Level - %u, Guitar Level - %u, Other Level - %u\n", 
           vocalSeparate.enable ? "Enabled" : "Disabled", 
           vocalSeparate.u32HumanLevel, 
           vocalSeparate.u32GuitarLevel, 
           vocalSeparate.u32OtherLevel);
}

// Function to handle the echo 3A switch command
void handleLcdEcho3ASwitchCmd(const pbox_lcd_msg_t* msg) {
    bool echo3A_On = msg->echo3A_On;
    printf("Echo 3A Switch Command: %s\n", echo3A_On ? "On" : "Off");
}

// Function to handle the reverb mode command
void handleLcdReverbModeCmd(const pbox_lcd_msg_t* msg) {
    pbox_revertb_t reverbMode = msg->reverbMode;
    printf("Reverb Mode Command: Mode - %d\n", reverbMode);
}

// Function to handle the loop mode command
void handleLcdLoopModeCmd(const pbox_lcd_msg_t* msg) {
    bool loop = msg->loop;
    printf("Loop Mode Command: %s\n", loop ? "Looping" : "Not Looping");
}

// Function to handle the energy info command
void handleLcdEnergyInfoCmd(const pbox_lcd_msg_t* msg) {
    energy_info_t energyData = msg->energy_data;
    printf("\nEnergy Info Command, Size: %d\n", energyData.size);
    // For each energy data, print its value
    for (int i = 0; i < energyData.size; i++) {
        if(i==0) printf("freq  :\t");
        printf("%05d%c", energyData.energykeep[i].freq, i<(energyData.size-1)?'\t':' ');
    }
    printf("\n");
    for (int i = 0; i < energyData.size; i++) {
        if(i==0) printf("energy:\t");
        printf("%02d%c", energyData.energykeep[i].energy, i<(energyData.size-1)?'\t':' ');
    }
    printf("\n");
}

// Function to handle the guitar level command
void handleLcdGuitarLevelCmd(const pbox_lcd_msg_t* msg) {
    uint32_t guitar_music_level = msg->guitar_music_level;
    printf("Guitar Level Command: Level - %u\n", guitar_music_level);
}

// Function to handle the gui reflash command //exec lv_task_handler
void handleLcdGuiReflushCmd(const pbox_lcd_msg_t* msg) {
    (void*)(msg);
    printf("GUI reflash Command\n");
    //lv_task_handler();
}

// Array of event handlers
const LcdCmdHandler_t lcdEventHandlers[] = {
    { PBOX_LCD_DISP_PLAY_PAUSE, handleLcdPlayPauseCmd },
    { PBOX_LCD_DISP_PREV_NEXT, handleLcdPrevNextCmd },
    { PBOX_LCD_DISP_TRACK_INFO, handleLcdTrackInfoCmd },
    { PBOX_LCD_DISP_TRACK_POSITION, handleLcdTrackPositionCmd },
    { PBOX_LCD_DISP_MAIN_VOL_LEVEL, handleLcdMainVolLevelCmd },
    { PBOX_LCD_DISP_MIC_VOL_LEVEL, handleLcdMicVolLevelCmd },
    { PBOX_LCD_DISP_ACCOMP_MUSIC_LEVEL, handleLcdAccompMusicLevelCmd },
    { PBOX_LCD_DISP_HUMAN_MUSIC_LEVEL, handleLcdHumanMusicLevelCmd },
    { PBOX_LCD_DISP_MUSIC_SEPERATE_SWITCH, handleLcdMusicSeparateSwitchCmd },
    { PBOX_LCD_DISP_ECHO_3A_SWITCH, handleLcdEcho3ASwitchCmd },
    { PBOX_LCD_DISP_REVERT_MODE, handleLcdReverbModeCmd },
    { PBOX_LCD_DISP_LOOP_MODE, handleLcdLoopModeCmd },
    { PBOX_LCD_DISP_ENERGY_INFO, handleLcdEnergyInfoCmd },
    { PBOX_LCD_DISP_GUITAR_LEVEL, handleLcdGuitarLevelCmd },
    { PBOX_LCD_DISP_REFLASH, handleLcdGuiReflushCmd}
    // Add other as needed...
};

// Function to process an incoming pbox_lcd_msg_t event
void process_pbox_lcd_cmd(const pbox_lcd_msg_t* msg) {
    if (msg == NULL) {
        printf("Error: Null event message received.\n");
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

    printf("Warning: No handler found for event ID %d.\n", msg->msgId);
}

static void *pbox_touchLCD_server(void *arg)
{
    char buff[sizeof(pbox_lcd_msg_t)] = {0};
    pbox_lcd_msg_t *msg;
    pthread_setname_np(pthread_self(), "pbox_lcd");

    pbox_lvgl_init();

    int sock_fd = create_udp_socket(SOCKET_PATH_LVGL_SERVER);

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
            printf("select timeout or no data\n");
            continue;
        }

        int ret = recvfrom(sock_fd, buff, sizeof(buff), 0, NULL, NULL);
        if (ret <= 0) {
            if (ret == 0) {
                printf("Socket closed\n");
                break;
            } else {
                perror("recvfrom failed");
                continue;
            }
        }

        pbox_lcd_msg_t *msg = (pbox_lcd_msg_t *)buff;
        //printf("%s recv: type: %d, id: %d\n", __func__, msg->type, msg->msgId);

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
        printf("touchLCD server start failed\n");
    }

    return ret;
}