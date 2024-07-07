#!/bin/bash
ACODEC_FILE_PATH="/sys/devices/platform/ff560000.acodec/rk3308-acodec-dev/dac_output"

sleep 2
if [ ! -x "$ACODEC_FILE_PATH" ]; then
	echo "Error: $ACODEC_FILE_PATH does not exist. Exiting script!"
	if [[ $(cat /proc/device-tree/acodec@ff560000/status 2>/dev/null) != "okay" ]]; then
		echo "Error: dts acodec is not 'okay'. Exiting script."
		exit 1
	fi
fi

amixer -c 0 cset numid=12 3
amixer -c 0 cset numid=13 3
amixer -c 0 cset numid=34 12
amixer -c 0 cset numid=40 1
amixer -c 0 cset numid=41 1
amixer -c 0 cset numid=44 3
amixer -c 0 cset numid=45 3
amixer -c 0 cset numid=46 30
amixer -c 0 cset numid=47 30

#echo 1 > "$ACODEC_FILE_PATH"
#sleep 1
#echo 11 > "$ACODEC_FILE_PATH"
#sleep 1
#while true; do
#    dac_output=$(cat "$ACODEC_FILE_PATH")
#    if [ "$dac_output" != "dac path: both line out and hp out" ]; then
#        echo "Content is not 'dac path: both line out and hp out'. Setting dac_output to 1."
#        echo 11 > "$ACODEC_FILE_PATH"
#    fi
#    sleep 1
#done
