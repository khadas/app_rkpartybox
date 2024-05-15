#! /bin/sh

export LD_LIBRARY_PATH=/data/:$LD_LIBRARY_PATH
export PATH=/data:$PATH

echo -1 > /proc/sys/kernel/sched_rt_runtime_us
#echo performance > /sys/devices/system/cpu/cpufreq/policy0/scaling_governor

if [ ! -e /oem/config_howling.json ]; then
	ln -s /etc/pbox/config_howling.json /oem/config_howling.json
fi
if [ ! -e /oem/config_reverb_doa_detect.json ]; then
	ln -s /etc/pbox/config_reverb_doa_detect.json /oem/config_reverb_doa_detect.json
fi
if [ ! -e /oem/config_gender_detect.json ]; then
	ln -s /etc/pbox/config_gender_detect.json /oem/config_gender_detect.json
fi
if [ ! -e /oem/eq_drc_player.bin ]; then
	ln -s /etc/pbox/eq_drc_player.bin /oem/eq_drc_player.bin
fi
if [ ! -e /oem/eq_drc_recorder.bin ]; then
	ln -s /etc/pbox/eq_drc_recorder.bin /oem/eq_drc_recorder.bin
fi
if [ ! -e /oem/rkstudio.bin ]; then
    ln -s /etc/pbox/rkstudio.bin /oem/rkstudio.bin
fi
if [ ! -e /oem/wozai-48k2ch.pcm ]; then
	ln -s /etc/pbox/wozai-48k2ch.pcm /oem/wozai-48k2ch.pcm
fi
if [ ! -e /oem/SmileySans-Oblique.ttf ]; then
	ln -s /etc/pbox/SmileySans-Oblique.ttf /oem/SmileySans-Oblique.ttf
fi
if [ ! -e /oem/vocal_off.pcm ]; then
	ln -s /etc/pbox/vocal_off.pcm /oem/vocal_off.pcm
fi
if [ ! -e /oem/vocal_on.pcm ]; then
	ln -s /etc/pbox/vocal_on.pcm /oem/vocal_on.pcm
fi
if [ ! -e /oem/guitar_off.pcm ]; then
	ln -s /etc/pbox/guitar_off.pcm /oem/guitar_off.pcm
fi
if [ ! -e /oem/guitar_on.pcm ]; then
	ln -s /etc/pbox/guitar_on.pcm /oem/guitar_on.pcm
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
if [ ! -e /oem/doa.pcm ]; then
	ln -s /etc/pbox/doa.pcm /oem/doa.pcm
fi
if [ ! -e /oem/antifeedback_off.pcm ]; then
	ln -s /etc/pbox/antifeedback_off.pcm /oem/antifeedback_off.pcm
fi
if [ ! -e /oem/antifeedback_on.pcm ]; then
	ln -s /etc/pbox/antifeedback_on.pcm /oem/antifeedback_on.pcm
fi
if [ ! -e /oem/zero.pcm ]; then
	ln -s /etc/pbox/zero.pcm /oem/zero.pcm
fi
if [ ! -e /oem/one.pcm ]; then
	ln -s /etc/pbox/one.pcm /oem/one.pcm
fi
if [ ! -e /oem/two.pcm ]; then
	ln -s /etc/pbox/two.pcm /oem/two.pcm
fi
if [ ! -e /oem/three.pcm ]; then
	ln -s /etc/pbox/three.pcm /oem/three.pcm
fi
if [ ! -e /oem/four.pcm ]; then
	ln -s /etc/pbox/four.pcm /oem/four.pcm
fi
if [ ! -e /oem/five.pcm ]; then
	ln -s /etc/pbox/five.pcm /oem/five.pcm
fi

export rt_cfg_path_3a=/oem/config_howling.json
export rt_cfg_path_reverb_doa_detect=/oem/config_reverb_doa_detect.json
export rt_cfg_path_gender_detect=/oem/config_gender_detect.json
export rt_cfg_path_eqdrc_player=/oem/eq_drc_player.bin
export rt_cfg_path_eqdrc_recorder=/oem/eq_drc_recorder.bin
export rt_cfg_path_rkstudio=/oem/rkstudio.bin
export rt_response_path=/oem/wozai-48k2ch.pcm
export mic_gain_0=5
export mic_gain_1=-80
export mic_gain_2=-80
export mic_gain_3=5
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
export recorder_eqdrc_bypass=0
export player_eqdrc_bypass=1
export player_gender_bypass=0
ulimit -c unlimited
echo "/tmp/core-%p-%e" > /proc/sys/kernel/core_pattern
rkpartybox
