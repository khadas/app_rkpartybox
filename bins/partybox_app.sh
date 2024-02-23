#! /bin/sh

export LD_LIBRARY_PATH=/data/

echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor

ln -sf /oem/SmileySans-Oblique.ttf /data/SmileySans-Oblique.ttf

export rt_cfg_path_3a=/oem/config_3a.json
export rt_cfg_path_eqdrc_player=/oem/eq_drc_player.bin
export rt_cfg_path_eqdrc_recorder=/oem/eq_drc_recorder.bin
export rt_response_path=/oem/wozai-48k2ch.pcm
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
