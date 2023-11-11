#!/bin/sh
sysdir=/mnt/SDCARD/.tmp_update
miyoodir=/mnt/SDCARD/miyoo
export LD_LIBRARY_PATH="/lib:/config/lib:$miyoodir/lib:$sysdir/lib:$sysdir/lib/parasyte"
export PATH="$sysdir/bin:$PATH"

logfile=$(basename "$0" .sh)
. $sysdir/script/log.sh

MODEL_MM=283
MODEL_MMP=354

main() {
    # Set model ID
    axp 0 > /dev/null
    export DEVICE_ID=$([ $? -eq 0 ] && echo $MODEL_MMP || echo $MODEL_MM)
    echo -n "$DEVICE_ID" > /tmp/deviceModel

    touch /tmp/is_booting
    check_installer
    clear_logs

    init_system
    update_time

    # Remount passwd/group to add our own users
    mount -o bind $sysdir/config/passwd /etc/passwd
    mount -o bind $sysdir/config/group /etc/group

    # Start the battery monitor
    batmon &

    # Check is charging
    if [ $DEVICE_ID -eq $MODEL_MM ]; then
        is_charging=$(cat /sys/devices/gpiochip0/gpio/gpio59/value)
    elif [ $DEVICE_ID -eq $MODEL_MMP ]; then
        axp_status="0x$(axp 0 | cut -d':' -f2)"
        is_charging=$([ $(($axp_status & 0x4)) -eq 4 ] && echo 1 || echo 0)
    fi

    # Show charging animation
    if [ $is_charging -eq 1 ]; then
        cd $sysdir
        chargingState
    fi

    cd $sysdir
    bootScreen "Boot"

    # Init
    rm /tmp/.offOrder 2> /dev/null
    HOME=/mnt/SDCARD/StoryTeller/
    
    storyteller_jpg2png

    # start_networking
    sleep 1
    rm -rf /tmp/is_booting

    launch_storyteller
    while true; do
        check_off_order "End"
    done
}

storyteller_jpg2png() {
    for directory in /mnt/SDCARD/Stories/*; do
        if [ ! -d "$directory" ]; then
            continue
        fi

        for file in "$directory"/assets/*.jpg "$directory"/assets/*.jpeg "$directory"/assets/*.JPG "$directory"/assets/*.JPEG; do
            if [ ! -f "$file" ]; then
                continue
            fi
            jpg2png "$file" 320 240
            rm -f "$file" 2> /dev/null
        done
    done
}

set_prev_state() {
    echo "$1" > /tmp/prev_state
}

clear_logs() {
    mkdir -p $sysdir/logs

    cd $sysdir/logs
    rm -f \
    #    ./keymon.log \
        ./dnsmasq.log \
        ./ftp.log \
        ./runtime.log \
        ./update_networking.log \
        ./easy_netplay.log \
        2> /dev/null
}



is_running() {
    process_name="$1"
    pgrep "$process_name" > /dev/null
}

get_info_value() {
    echo "$1" | grep "$2\b" | awk '{split($0,a,"="); print a[2]}' | awk -F'"' '{print $2}' | tr -d '\n'
}


launch_storyteller() {
    log "\n:: Launch Story Teller"
    cd $sysdir
    LD_PRELOAD="$miyoodir/lib/libpadsp.so" storyTeller
    sync
}

check_off_order() {
    if [ -f /tmp/.offOrder ]; then
        bootScreen "$1" &
        sleep 1 # Allow the bootScreen to be displayed
        shutdown
    fi
}

init_system() {
    log "\n:: Init system"

    load_settings

    # init_lcd
    cat /proc/ls
    sleep 0.25

    if [ $DEVICE_ID -eq $MODEL_MMP ] && [ -f $sysdir/config/.lcdvolt ]; then
        $sysdir/script/lcdvolt.sh 2> /dev/null
    fi

    start_audioserver

    brightness=$(/customer/app/jsonval brightness)
    brightness_raw=$(awk "BEGIN { print int(3 * exp(0.350656 * $brightness) + 0.5) }")
    log "brightness: $brightness -> $brightness_raw"

    # init backlight
    echo 0 > /sys/class/pwm/pwmchip0/export
    echo 800 > /sys/class/pwm/pwmchip0/pwm0/period
    echo $brightness_raw > /sys/class/pwm/pwmchip0/pwm0/duty_cycle
    echo 1 > /sys/class/pwm/pwmchip0/pwm0/enable
}

device_uuid=$(read_uuid)
device_settings="/mnt/SDCARD/.tmp_update/config/system/$device_uuid.json"

load_settings() {
    if [ -f "$device_settings" ]; then
        cp -f "$device_settings" /mnt/SDCARD/system.json
    fi

    # link /appconfigs/system.json to SD card
    if [ -L /appconfigs/system.json ] && [ "$(readlink /appconfigs/system.json)" == "/mnt/SDCARD/system.json" ]; then
        rm /appconfigs/system.json
    fi
    ln -s /mnt/SDCARD/system.json /appconfigs/system.json

    if [ $DEVICE_ID -eq $MODEL_MM ]; then
        # init charger detection
        if [ ! -f /sys/devices/gpiochip0/gpio/gpio59/direction ]; then
            echo 59 > /sys/class/gpio/export
            echo in > /sys/devices/gpiochip0/gpio/gpio59/direction
        fi

        if [ $(/customer/app/jsonval vol) -ne 20 ] || [ $(/customer/app/jsonval mute) -ne 0 ]; then
            # Force volume and mute settings
            cat /mnt/SDCARD/system.json |
                sed 's/^\s*"vol":\s*[0-9][0-9]*/\t"vol":\t20/g' |
                sed 's/^\s*"mute":\s*[0-9][0-9]*/\t"mute":\t0/g' \
                    > temp
            mv -f temp /mnt/SDCARD/system.json
        fi
    fi
}

save_settings() {
    if [ -f /mnt/SDCARD/system.json ]; then
        cp -f /mnt/SDCARD/system.json "$device_settings"
    fi
}

update_time() {
    timepath=/mnt/SDCARD/Saves/CurrentProfile/saves/currentTime.txt
    currentTime=0
    # Load current time
    if [ -f $timepath ]; then
        currentTime=$(cat $timepath)
    fi
    date +%s -s @$currentTime

    # Ensure that all play activities are closed
    playActivity stop_all

    #Add 4 hours to the current time
    hours=4
    if [ -f $sysdir/config/startup/addHours ]; then
        hours=$(cat $sysdir/config/startup/addHours)
    fi
    addTime=$(($hours * 3600))
    if [ ! -f $sysdir/config/.ntpState ]; then
        currentTime=$(($currentTime + $addTime))
    fi
    date +%s -s @$currentTime
}

start_audioserver() {
    defvol=$(echo $(/customer/app/jsonval vol) | awk '{ printf "%.0f\n", 48 * (log(1 + $1) / log(10)) - 60 }')
    runifnecessary "audioserver" $miyoodir/app/audioserver $defvol
}

runifnecessary() {
    cnt=0
    #a=`ps | grep $1 | grep -v grep`
    a=$(pgrep $1)
    while [ "$a" == "" ] && [ $cnt -lt 8 ]; do
        log "try to run: $2"
        $2 $3 &
        sleep 0.5
        cnt=$(expr $cnt + 1)
        a=$(pgrep $1)
    done
}

start_networking() {
    rm $sysdir/config/.hotspotState # dont start hotspot at boot

    touch /tmp/network_changed
    sync

    check_networking
}

check_networking() {
    if [ $DEVICE_ID -ne $MODEL_MMP ] || [ ! -f /tmp/network_changed ] && [ -f /tmp/ntp_synced ]; then
        check_timezone
        return
    fi

    if pgrep -f update_networking.sh; then
        log "update_networking already running"
    else
        rm /tmp/network_changed
        $sysdir/script/network/update_networking.sh check
    fi

    check_timezone
}

check_timezone() {
    export TZ=$(cat "$sysdir/config/.tz")
}

check_installer() {
    :
}

if [ -f $sysdir/config/.logging ]; then
    main
else
    main 2>&1 > /dev/null
fi
