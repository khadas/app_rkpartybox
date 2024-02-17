#! /bin/sh

export LD_LIBRARY_PATH=/data/

echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor
ln -sf /oem/eq_drc_player.bin /data/eq_drc_player.bin
ln -sf /oem/eq_drc_recorder.bin /data/eq_drc_recorder.bin
ln -sf /oem/wozai-48k2ch.pcm /data/wozai-48k2ch.pcm
ln -sf /oem/SmileySans-Oblique.ttf /data/SmileySans-Oblique.ttf

export player_weight=100
export rt_level_det_up=400
export rt_level_det_hold=400
export rt_level_det_down=400
export mic_gain_0=0
export mic_gain_1=0
export player_gain_0=-6
export player_gain_1=-30
export player_gain_2=-30
export player_gain_3=-6
export bt_gain_0=-6
export bt_gain_1=-30
export bt_gain_2=-30
export bt_gain_3=-6
export ai_period=128
export ao_period=128
export play_start_threshold=1
export ai_buf=1
export ai_eqdrc_bypass=1
rkpartybox
