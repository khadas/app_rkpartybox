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
#ifdef __cplusplus
extern "C" {
#endif

// Function declarations
void pbox_multi_displayPlayPause(bool play);
void pbox_multi_displayPrevNext(bool next);
void pbox_multi_displayTrackInfo(const char* title, const char* artist);
void pbox_multi_displayTrackPosition(unsigned int mCurrent, unsigned int mDuration);
void pbox_multi_displayMainVolumeLevel(uint32_t mainVolume);
void pbox_multi_displayMicVolumeLevel(uint32_t micVolume);
void pbox_multi_displayAccompMusicLevel(uint32_t accomp_music_level);
void pbox_multi_displayHumanMusicLevel(uint32_t human_music_level);
void pbox_multi_displayGuitarLevel(uint32_t guitar_music_level);
void pbox_multi_displayMusicSeparateSwitch(bool enable, uint32_t hlevel, uint32_t mlevel, uint32_t glevel);

#ifdef __cplusplus
}
#endif
#endif  // _PBOX_MULTI_DISPLAY_H_
