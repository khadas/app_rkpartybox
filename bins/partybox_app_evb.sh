#! /bin/sh

export LD_LIBRARY_PATH=/data/:$LD_LIBRARY_PATH
export PATH=/data:$PATH

echo -1 > /proc/sys/kernel/sched_rt_runtime_us
#echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor

mkdir -p /oem/tone
if [ ! -e /oem/SmileySans-Oblique.ttf ]; then
	ln -s /etc/pbox/SmileySans-Oblique.ttf /oem/SmileySans-Oblique.ttf
fi
if [ ! -e /oem/eq_drc_player.bin ]; then
	ln -s /etc/pbox/eq_drc_player.bin /oem/eq_drc_player.bin
fi
if [ ! -e /oem/eq_drc_recorder.bin ]; then
	ln -s /etc/pbox/eq_drc_recorder.bin /oem/eq_drc_recorder.bin
fi
if [ ! -e /oem/tone/wozai-48k2ch.pcm ]; then
	ln -s /etc/pbox/wozai-48k2ch.pcm /oem/tone/wozai-48k2ch.pcm
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

if [ ! -e /oem/rkstudio.bin ]; then
    ln -s /etc/pbox/rkstudio.bin /oem/rkstudio.bin
fi

if [ ! -e /oem/tone/vocal_off.pcm ]; then
	ln -s /etc/pbox/vocal_off.pcm /oem/tone/vocal_off.pcm
fi
if [ ! -e /oem/tone/vocal_on.pcm ]; then
	ln -s /etc/pbox/vocal_on.pcm /oem/tone/vocal_on.pcm
fi
if [ ! -e /oem/tone/guitar_off.pcm ]; then
	ln -s /etc/pbox/guitar_off.pcm /oem/tone/guitar_off.pcm
fi
if [ ! -e /oem/tone/guitar_on.pcm ]; then
	ln -s /etc/pbox/guitar_on.pcm /oem/tone/guitar_on.pcm
fi
if [ ! -e /oem/tone/Stereo.pcm ]; then
	ln -s /etc/pbox/Stereo.pcm /oem/tone/Stereo.pcm
fi
if [ ! -e /oem/tone/Widen.pcm ]; then
	ln -s /etc/pbox/Widen.pcm /oem/tone/Widen.pcm
fi
if [ ! -e /oem/tone/Mono.pcm ]; then
	ln -s /etc/pbox/Mono.pcm /oem/tone/Mono.pcm
fi
if [ ! -e /oem/tone/Sense.pcm ]; then
	ln -s /etc/pbox/Sense.pcm /oem/tone/Sense.pcm
fi
if [ ! -e /oem/tone/doa.pcm ]; then
	ln -s /etc/pbox/doa.pcm /oem/tone/doa.pcm
fi
if [ ! -e /oem/tone/antifeedback_off.pcm ]; then
	ln -s /etc/pbox/antifeedback_off.pcm /oem/tone/antifeedback_off.pcm
fi
if [ ! -e /oem/tone/antifeedback_on.pcm ]; then
	ln -s /etc/pbox/antifeedback_on.pcm /oem/tone/antifeedback_on.pcm
fi
if [ ! -e /oem/tone/zero.pcm ]; then
	ln -s /etc/pbox/zero.pcm /oem/tone/zero.pcm
fi
if [ ! -e /oem/tone/one.pcm ]; then
	ln -s /etc/pbox/one.pcm /oem/tone/one.pcm
fi
if [ ! -e /oem/tone/two.pcm ]; then
	ln -s /etc/pbox/two.pcm /oem/tone/two.pcm
fi
if [ ! -e /oem/tone/three.pcm ]; then
	ln -s /etc/pbox/three.pcm /oem/tone/three.pcm
fi
if [ ! -e /oem/tone/four.pcm ]; then
	ln -s /etc/pbox/four.pcm /oem/tone/four.pcm
fi
if [ ! -e /oem/tone/five.pcm ]; then
	ln -s /etc/pbox/five.pcm /oem/tone/five.pcm
fi

if [ ! -e /oem/tone/eq_balledmode.pcm ]; then
	ln -s /etc/pbox/eq_balledmode.pcm /oem/tone/eq_balledmode.pcm
fi
if [ ! -e /oem/tone/eq_bluesmode.pcm ]; then
	ln -s /etc/pbox/eq_bluesmode.pcm /oem/tone/eq_bluesmode.pcm
fi
if [ ! -e /oem/tone/eq_classicmode.pcm ]; then
	ln -s /etc/pbox/eq_classicmode.pcm /oem/tone/eq_classicmode.pcm
fi
if [ ! -e /oem/tone/eq_countrymode.pcm ]; then
	ln -s /etc/pbox/eq_countrymode.pcm /oem/tone/eq_countrymode.pcm
fi
if [ ! -e /oem/tone/eq_dancemode.pcm ]; then
	ln -s /etc/pbox/eq_dancemode.pcm /oem/tone/eq_dancemode.pcm
fi
if [ ! -e /oem/tone/eq_elecmode.pcm ]; then
	ln -s /etc/pbox/eq_elecmode.pcm /oem/tone/eq_elecmode.pcm
fi
if [ ! -e /oem/tone/eq_jazzmode.pcm ]; then
	ln -s /etc/pbox/eq_jazzmode.pcm /oem/tone/eq_jazzmode.pcm
fi
if [ ! -e /oem/tone/eq_offmode.pcm ]; then
	ln -s /etc/pbox/eq_offmode.pcm /oem/tone/eq_offmode.pcm
fi
if [ ! -e /oem/tone/eq_popmode.pcm ]; then
	ln -s /etc/pbox/eq_popmode.pcm /oem/tone/eq_popmode.pcm
fi
if [ ! -e /oem/tone/eq_rockmode.pcm ]; then
	ln -s /etc/pbox/eq_rockmode.pcm /oem/tone/eq_rockmode.pcm
fi
if [ ! -e /oem/tone/res_indoor.pcm ]; then
	ln -s /etc/pbox/res_indoor.pcm /oem/tone/res_indoor.pcm
fi
if [ ! -e /oem/tone/res_outdoor.pcm ]; then
	ln -s /etc/pbox/res_outdoor.pcm /oem/tone/res_outdoor.pcm
fi
if [ ! -e /oem/tone/horizon_placement.pcm ]; then
	ln -s /etc/pbox/horizon_placement.pcm /oem/tone/horizon_placement.pcm
fi
if [ ! -e /oem/tone/vertical_placement.pcm ]; then
	ln -s /etc/pbox/vertical_placement.pcm /oem/tone/vertical_placement.pcm
fi

if [ ! -e /oem/uac_config ]; then
	echo "Maybe first init, make link /oem/uac_config fisrt!"
	ln -s /etc/pbox/uac_config /oem/uac_config
fi

export rt_cfg_path_3a=/oem/config_howling.json
export rt_cfg_path_reverb_doa_detect=/oem/config_reverb_doa_detect.json
export rt_cfg_path_gender_detect=/oem/config_gender_detect.json
export rt_cfg_path_rkstudio_player=/oem/eq_drc_player.bin
export rt_cfg_path_rkstudio_recorder=/oem/eq_drc_recorder.bin
export rt_cfg_path_rkstudio=/oem/rkstudio.bin
export rt_response_path=/oem/tone/wozai-48k2ch.pcm
export ref_player_only=0
export mic_gain_0=0
export mic_gain_1=0
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
export recorder_howling_bypass=0
export recorder_level_bypass=1
export recorder_reverb_bypass=0
export recorder_rkstudio_bypass=1
export player_rkstudio_bypass=0
export player_gender_bypass=0

amixer -c 0 cset numid=34 12
amixer -c 0 cset numid=40 1
amixer -c 0 cset numid=41 1
amixer -c 0 cset numid=44 3
amixer -c 0 cset numid=45 3
amixer -c 0 cset numid=46 30
amixer -c 0 cset numid=47 30

ulimit -c unlimited
echo "/tmp/core-%p-%e" > /proc/sys/kernel/core_pattern
rkpartybox
