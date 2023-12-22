/************* UI include two part:*************************
***************************1. LED effect show **************
***************************2. LCD screnn display ***********
* mainly here, almost are commands to display something,
* when key pressed, touch screem pressed, or bt info feedback,
* we may call api in this page to display something.
************************************************************/
#ifndef _PBOX_MULTI_DISPLAY_H_
#define _PBOX_MULTI_DISPLAY_H_
#include <stdint.h>
#include <stdbool.h>
#include "pbox_common.h"
#include "pbox_app.h"
#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
void pbox_multi_displayIsPlaying(bool play, display_t policy);
void pbox_multi_displayPrevNext(bool next, display_t policy);
void pbox_multi_displayTrackInfo(const char* title, const char* artist, display_t policy);
void pbox_multi_displayTrackPosition(unsigned int mCurrent, unsigned int mDuration, display_t policy);
void pbox_multi_displayMainVolumeLevel(uint32_t mainVolume, display_t policy);
void pbox_multi_displayMicVolumeLevel(uint32_t micVolume, display_t policy);
void pbox_multi_displayAccompMusicLevel(uint32_t accomp_music_level, display_t policy);
void pbox_multi_displayHumanMusicLevel(uint32_t human_music_level, display_t policy);
void pbox_multi_displayReservLevel(uint32_t reserv_music_level, display_t policy);
void pbox_multi_displayMusicSeparateSwitch(bool enable, uint32_t hlevel, uint32_t mlevel, uint32_t rlevel, display_t policy);
void pbox_multi_displayEnergyInfo(energy_info_t energy, display_t policy);
void pbox_multi_displayEcho3A(bool enable, display_t policy);
void pbox_multi_displayRevertMode(pbox_revertb_t mode, display_t policy);
void pbox_multi_displayUsbState(usb_state_t state, display_t policy);

#ifdef __cplusplus
}
#endif
#endif  // _PBOX_MULTI_DISPLAY_H_
