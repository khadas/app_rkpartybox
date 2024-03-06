#! /bin/sh

export LD_LIBRARY_PATH=/data/

echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor

if [ ! -e /oem/SmileySans-Oblique.ttf ]; then
	ln -s /etc/SmileySans-Oblique.ttf /oem/SmileySans-Oblique.ttf
fi
if [ ! -e /oem/eq_drc_player.bin ]; then
	ln -s /etc/eq_drc_player.bin /oem/eq_drc_player.bin
fi
if [ ! -e /oem/eq_drc_recorder.bin ]; then
	ln -s /etc/eq_drc_recorder.bin /oem/eq_drc_recorder.bin
fi
if [ ! -e /oem/wozai-48k2ch.pcm ]; then
	ln -s /etc/wozai-48k2ch.pcm /oem/wozai-48k2ch.pcm
fi
if [ ! -e /oem/config_3a.json ]; then
	ln -s /etc/config_3a.json /oem/config_3a.json
fi
if [ ! -e /oem/Split_off.pcm ]; then
	ln -s /etc/Split_off.pcm /oem/Split_off.pcm
fi
if [ ! -e /oem/Split_on.pcm ]; then
	ln -s /etc/Split_on.pcm /oem/Split_on.pcm
fi
if [ ! -e /oem/Stereo.pcm ]; then
	ln -s /etc/Stereo.pcm /oem/Stereo.pcm
fi
if [ ! -e /oem/Widen.pcm ]; then
	ln -s /etc/Widen.pcm /oem/Widen.pcm
fi
if [ ! -e /oem/Mono.pcm ]; then
	ln -s /etc/Mono.pcm /oem/Mono.pcm
fi

if [ ! -e /oem/rkstudio.bin ]; then
    if [[ $(cat /proc/device-tree/acodec@ff560000/status 2>/dev/null) != "okay" ]]; then
        ln -s /etc/rkstudio_2in6out.bin /oem/rkstudio.bin
    else
        ln -s /etc/rkstudio_2in2out.bin /oem/rkstudio.bin
    fi
fi

export rt_cfg_path_3a=/oem/config_3a.json
export rt_cfg_path_eqdrc_player=/oem/eq_drc_player.bin
export rt_cfg_path_eqdrc_recorder=/oem/eq_drc_recorder.bin
export rt_cfg_path_rkstudio=/oem/rkstudio.bin
export rt_response_path=/oem/wozai-48k2ch.pcm
export player_weight=100
export rt_level_det_up=400
export rt_level_det_hold=400
export rt_level_det_down=400
export player_gain_0=0
export player_gain_1=-30
export player_gain_2=-30
export player_gain_3=0
export ai_period=128
export ao_period=128
export play_start_threshold=1
export ai_buf=1
export recorder_eqdrc_bypass=0
export player_eqdrc_bypass=1
rkpartybox
