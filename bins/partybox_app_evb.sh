#! /bin/sh

export LD_LIBRARY_PATH=/data/:$LD_LIBRARY_PATH
export PATH=/data:$PATH

echo -1 > /proc/sys/kernel/sched_rt_runtime_us
#echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor

if [ ! -e /oem/SmileySans-Oblique.ttf ]; then
	ln -s /etc/pbox/SmileySans-Oblique.ttf /oem/SmileySans-Oblique.ttf
fi
if [ ! -e /oem/eq_drc_player.bin ]; then
	ln -s /etc/pbox/eq_drc_player.bin /oem/eq_drc_player.bin
fi
if [ ! -e /oem/eq_drc_recorder.bin ]; then
	ln -s /etc/pbox/eq_drc_recorder.bin /oem/eq_drc_recorder.bin
fi
if [ ! -e /oem/wozai-48k2ch.pcm ]; then
	ln -s /etc/pbox/wozai-48k2ch.pcm /oem/wozai-48k2ch.pcm
fi
if [ ! -e /oem/config_howling.json ]; then
	ln -s /etc/pbox/config_howling.json /oem/config_howling.json
fi
if [ ! -e /oem/config_reverb_doa_detect.json ]; then
	ln -s /etc/pbox/config_reverb_doa_detect.json /oem/config_reverb_doa_detect.json
fi
if [ ! -e /oem/config_gender_detect.json ]; then
	ln -s /etc/pbox/config_gender_detect.json /oem/config_gender_detect.json
fi
if [ ! -e /oem/Split_off.pcm ]; then
	ln -s /etc/pbox/Split_off.pcm /oem/Split_off.pcm
fi
if [ ! -e /oem/Split_on.pcm ]; then
	ln -s /etc/pbox/Split_on.pcm /oem/Split_on.pcm
fi
if [ ! -e /oem/Stereo.pcm ]; then
	ln -s /etc/pbox/Stereo.pcm /oem/Stereo.pcm
fi
if [ ! -e /oem/Widen.pcm ]; then
	ln -s /etc/pbox/Widen.pcm /oem/Widen.pcm
fi
if [ ! -e /oem/Mono.pcm ]; then
	ln -s /etc/pbox/Mono.pcm /oem/Mono.pcm
fi

if [ ! -e /oem/rkstudio.bin ]; then
    ln -s /etc/pbox/rkstudio.bin /oem/rkstudio.bin
fi

if [ ! -e /oem/uac_config ]; then
	echo "Maybe first init, make link /oem/uac_config fisrt!"
	ln -s /etc/pbox/uac_config /oem/uac_config
fi

export rt_cfg_path_3a=/oem/config_howling.json
export rt_cfg_path_reverb_doa_detect=/oem/config_reverb_doa_detect.json
export rt_cfg_path_gender_detect=/oem/config_gender_detect.json
export rt_cfg_path_eqdrc_player=/oem/eq_drc_player.bin
export rt_cfg_path_eqdrc_recorder=/oem/eq_drc_recorder.bin
export rt_cfg_path_rkstudio=/oem/rkstudio.bin
export rt_response_path=/oem/wozai-48k2ch.pcm
export mic_gain_0=5
export mic_gain_1=5
export player_gain_0=0
export player_gain_1=-80
export player_gain_2=-80
export player_gain_3=0
export ai_period=128
export ai_count=2
export ai_buf=1
export ao_period=128
export ao_count=2
export play_start_threshold=1
export recorder_eqdrc_bypass=1
export player_eqdrc_bypass=1
export player_gender_bypass=1
ulimit -c unlimited
echo "/tmp/core-%p-%e" > /proc/sys/kernel/core_pattern
rkpartybox
