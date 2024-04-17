#include <stdbool.h>
#include <stdint.h>
#include <assert.h>

#include "hal_hw.h"

// 总音量参数数组
static const float MAIN_GAIN[DSP_MAIN_MAX_VOL+1] = {-800,-400,-380,-360,-340,-320,-300,\
-280,-260,-240,-220,-200,-180,-160,-140,-120,-110,-100,-90,-80,-70,-60,-50,-45,-40,-35,-30,-25,-20,-15,-10,-5,0};

// 话筒音量参数数组
//static const float MIC_GAIN[DSP_MIC_MAX_VOL+1] = {-800,-400,-380,-360,-340,-320,-300,\
-280,-260,-240,-220,-200,-180,-160,-140,-120,-110,-100,-90,-80,-70,-60,-50,-45,-40,-35,-30,-25,-20,-15,-10,-5,0};

// GT音量参数数组
//static const float GT_GAIN[DSP_MIC_MAX_VOL+1] = {-800,-400,-380,-360,-340,-320,-300,\
-280,-260,-240,-220,-200,-180,-160,-140,-120,-110,-100,-90,-80,-70,-60,-50,-45,-40,-35,-30,-25,-20,-15,-10,-5,0};

// 音乐音量参数数组
static const float MUSIC_GAIN[DSP_MUSIC_MAX_VOL+1] = {-800,-400,-380,-360,-340,-320,-300,\
-280,-260,-240,-220,-200,-180,-160,-140,-120,-110,-100,-90,-80,-70,-60,-50,-45,-40,-35,-30,-25,-20,-15,-10,-5,0};

// 话筒混响高低音音量参数数组
static const float MIC_REVERB[DSP_MIC_REVERB_MAX_VOL+1] = {0,3,6,9,12,15,18,21,24,27,30,33,\
34,37,40,43,47,50,53,56,59,62,65,68,71,75,79,83,87,91,95,98,100};

static const float MIC_TREBLE[DSP_MIC_TREBLE_MAX_VOL+1] = {-120,-115,-110,-105,-100,-95,-90,\
-85,-80,-70, -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60,70,80,85,90,95,100,105,110,115,120};

static const float MIC_BASS[DSP_MIC_BASS_MAX_VOL+1] = {-120,-115,-110,-105,-100,-95,-90,-85,\
-80,-70, -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60,70,80,85,90,95,100,105,110,115,120};

// GT混响高低音音量参数数组
static const float GT_REVERB[DSP_MIC_REVERB_MAX_VOL+1] = {0,3,6,9,12,15,18,21,24,27,30,33,34,\
37,40,43,47,50,53,56,59,62,65,68,71,75,79,83,87,91,95,98,100};

static const float GT_TREBLE[DSP_MIC_TREBLE_MAX_VOL+1] = {-120,-115,-110,-105,-100,-95,-90,\
-85,-80,-70, -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60,70,80,85,90,95,100,105,110,115,120};

static const float GT_BASS[DSP_MIC_BASS_MAX_VOL+1] = {-120,-115,-110,-105,-100,-95,-90,-85,\
-80,-70, -60, -50, -40, -30, -20, -10, 0, 10, 20, 30, 40, 50, 60,70,80,85,90,95,100,105,110,115,120};

float HW_MAIN_GAIN(uint8_t index) {
    assert(index <= DSP_MAIN_MAX_VOL);
    return MAIN_GAIN[index];
}

float HW_MUSIC_GAIN(uint8_t index) {
    assert(index <= DSP_MUSIC_MAX_VOL);
    return MUSIC_GAIN[index];
}

float HW_MIC_REVERB(uint8_t index) {
    assert(index <= DSP_MIC_REVERB_MAX_VOL);
    return MIC_REVERB[index];
}

float HW_MIC_TREBLE(uint8_t index) {
    assert(index <= DSP_MIC_TREBLE_MAX_VOL);
    return MIC_TREBLE[index];
}

float HW_MIC_BASS(uint8_t index) {
    return MIC_BASS[index];
    assert(index <= DSP_MIC_BASS_MAX_VOL);
}

float HW_GT_REVERB(uint8_t index) {
    assert(index <= DSP_MIC_TREBLE_MAX_VOL);
    return GT_REVERB[index];
}

float HW_GT_TREBLE(uint8_t index) {
    assert(index <= DSP_MIC_TREBLE_MAX_VOL);
    return GT_TREBLE[index];
}

float HW_GT_BASS(uint8_t index) {
    assert(index <= DSP_MIC_BASS_MAX_VOL);
    return GT_BASS[index];
}

const float* hw_get_main_volume_table(uint16_t* size) {
    *size = sizeof(MAIN_GAIN)/sizeof(float);
    return &MAIN_GAIN[0];
}

const float* hw_get_music_volume_table(uint16_t* size) {
    *size = sizeof(MUSIC_GAIN)/sizeof(float);
    return &MUSIC_GAIN[0];
}

const float* hw_get_mic_reverb_table(uint16_t* size) {
    *size = sizeof(MIC_REVERB)/sizeof(float);
    return &MIC_REVERB[0];
}

const float* hw_get_mic_treble_table(uint16_t* size) {
    *size = sizeof(MIC_TREBLE)/sizeof(float);
    return &MIC_TREBLE[0];
}

const float* hw_get_mic_bass_table(uint16_t* size) {
    *size = sizeof(MIC_BASS)/sizeof(float);
    return &MIC_BASS[0];
}

const float* hw_get_guitar_reverb_table(uint16_t* size) {
    *size = sizeof(GT_REVERB)/sizeof(float);
    return &GT_REVERB[0];
}

const float* hw_get_guitar_treble_table(uint16_t* size) {
    *size = sizeof(GT_TREBLE)/sizeof(float);
    return &GT_TREBLE[0];
}

const float* hw_get_guitar_bass_table(uint16_t* size) {
    *size = sizeof(GT_BASS)/sizeof(float);
    return &GT_BASS[0];
}