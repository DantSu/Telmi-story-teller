#!/bin/sh
sysdir=$(
    cd -- "$(dirname "$0")" > /dev/null 2>&1
    pwd -P
)
miyoodir=/mnt/SDCARD/miyoo

CORE_PACKAGE_FILE="$sysdir/onion.pak"

export LD_LIBRARY_PATH="/lib:/config/lib:/customer/lib:$sysdir/lib"
export PATH="$sysdir/bin:$PATH"
unset LD_PRELOAD

MODEL_MM=283
MODEL_MMP=354

version() {
    echo "$@" | awk -F. '{ f=$1; if (substr(f,2,1) == "v") f = substr(f,3); printf("%d%03d%03d%03d\n", f,$2,$3,$4); }'
}

# globals
total_core=0

main() {
    # init_lcd
    if [ "$(ps | grep dev/l | grep -v grep)" == "" ]; then
        cat /proc/ls
        sleep 1
    fi

    check_device_model

    # Start the battery monitor
    if [ $DEVICE_ID -eq MODEL_MM ]; then
        # init charger detection
        gpiodir=/sys/devices/gpiochip0/gpio
        if [ ! -f $gpiodir/gpio59/direction ]; then
            echo 59 > /sys/class/gpio/export
            echo "in" > $gpiodir/gpio59/direction
        fi
    fi

    # init backlight
    pwmdir=/sys/class/pwm/pwmchip0
    echo 0 > $pwmdir/export
    echo 800 > $pwmdir/pwm0/period
    echo 80 > $pwmdir/pwm0/duty_cycle
    echo 1 > $pwmdir/pwm0/enable

    killall keymon

    check_firmware

    run_installation 1 0
    cleanup
}

cleanup() {
    echo ":: Cleanup"
    cd $sysdir
    rm -f \
        /tmp/.update_msg \
        .installed \
        install.sh

    # Remove dirs if empty
    rmdir /mnt/SDCARD/Backup/saves
    rmdir /mnt/SDCARD/Backup/states
    rmdir /mnt/SDCARD/Backup

    # Clean zips if still present
    rm -f $CORE_PACKAGE_FILE

    # Remove update trigger script
    rm -f /mnt/SDCARD/miyoo/app/MainUI

    rm -f /mnt/SDCARD/miyoo354/app/MainUI
    rmdir /mnt/SDCARD/miyoo354/app
    rmdir /mnt/SDCARD/miyoo354
}

DEVICE_ID=0

check_device_model() {
    DEVICE_ID=$([ -f /customer/app/axp_test ] && echo $MODEL_MMP || echo $MODEL_MM)
    echo -n "$DEVICE_ID" > /tmp/deviceModel
}

get_install_stats() {
    total_core=$(zip_total "$CORE_PACKAGE_FILE")

    echo "STATS"
    echo "Telmi total:" $total_core
}

remove_configs() {
    echo ":: Remove configs"
    rm -f /mnt/SDCARD/RetroArch/.retroarch/retroarch.cfg

    rm -rf \
        /mnt/SDCARD/Saves/CurrentProfile/config/* \
        /mnt/SDCARD/Saves/GuestProfile/config/*
}

verify_file() {
    local file="/mnt/SDCARD/.tmp_update/onion.pak"

    if [ -f "$file" ]; then
        echo "File $file exists."

        if [ -w "$file" ]; then
            echo "File $file has write permission."
        else
            echo "File $file does not have write permission."
        fi

        if fuser "$file" > /dev/null; then
            echo "Something is currently writing to $file."
        else
            echo "No process is currently writing to $file."
        fi

        size=$(stat -c %s "$file")
        echo "File $file size: $size bytes."
    else
        echo "File $file does not exist."
    fi

    echo ""
}

run_installation() {
    reset_configs=$1
    system_only=$2

    killall batmon

    #get_install_stats

    rm -f /tmp/.update_msg 2> /dev/null
    rm -f $sysdir/config/currentSlide 2> /dev/null

    # Show installation progress
    cd $sysdir
    installUI &
    sync
    sleep 1

    verb="Updating"
    verb2="Update"

    if [ $reset_configs -eq 1 ]; then
        verb="Installing"
        verb2="Installation"
        echo "Preparing installation..." >> /tmp/.update_msg

        # Backup important stock files
        backup_system

        remove_configs

        # Remove stock folders.
        cd /mnt/SDCARD
        rm -rf App Emu RApp miyoo

    elif [ $system_only -ne 1 ]; then
        echo "Preparing update..." >> /tmp/.update_msg

        # Ensure packages are fresh !
        rm -rf /mnt/SDCARD/App/PackageManager 2> /dev/null

        remove_old_search
    fi

    verify_file

    install_core "1/1: $verb Telmi..."

    flash_custom_logo

    install_configs $reset_configs

    run_migration_scripts

    echo "Finishing up - Get ready!" >> /tmp/.update_msg

    if [ $reset_configs -eq 1 ]; then
        refresh_roms
    fi

    #########################################################################################
    #                                  Installation is done                                 #
    #                           Show quick guide and package manager                        #
    #########################################################################################

    if [ $system_only -ne 1 ]; then
        if [ $reset_configs -eq 1 ]; then
            cp -f $sysdir/res/miyoo${DEVICE_ID}_system.json /mnt/SDCARD/system.json
        fi

        # Start the battery monitor
        cd $sysdir
        batmon 2>&1 > /dev/null &

        # Reapply theme
        system_theme="$(/customer/app/jsonval theme)"
        active_theme="$(cat ./config/active_theme)"

        if [ -d "$(cat ./config/active_icon_pack)" ] && [ "$system_theme" == "$active_theme" ] && [ -d "$system_theme" ]; then
            themeSwitcher --update
        else
            themeSwitcher --update --reapply_icons
        fi

        touch $sysdir/.installed
        sync

        # Show quick guide
        if [ $reset_configs -eq 1 ]; then
            cd /mnt/SDCARD/App/Onion_Manual/
            ./launch.sh
        fi

        # Launch package manager
        cd /mnt/SDCARD/App/PackageManager/
        if [ $reset_configs -eq 1 ]; then
            packageManager --confirm
        else
            packageManager --confirm --auto_update
        fi
        free_mma

        cd $sysdir
        # ./config/boot_mod.sh # disabled because of possible incompatibility with new firmware

        # Show installation complete
        rm -f .installed
    fi

    #########################################################################################
    #                                  Wizards are completed                                #
    #                                           Exit                                        #
    #########################################################################################

    if [ $DEVICE_ID -eq MODEL_MM ]; then
        echo "$verb2 complete!" >> /tmp/.update_msg
        touch $sysdir/.waitConfirm
        touch $sysdir/.installed
        sync
    else
        echo "$verb2 complete - Rebooting..." >> /tmp/.update_msg
    fi

    installUI &
    sleep 1

    if [ $DEVICE_ID -eq MODEL_MM ]; then
        counter=10

        while [ -f $sysdir/.waitConfirm ] && [ $counter -ge 0 ]; do
            echo "Press A to turn off (""$counter""s)" >> /tmp/.update_msg
            counter=$((counter - 1))
            sleep 1
        done

        killall installUI
        bootScreen "End"
    else
        touch $sysdir/.installed
    fi

    rm -f $sysdir/config/currentSlide 2> /dev/null
    sync
}

install_core() {
    echo ":: Install Telmi"
    msg="$1"

    if [ ! -f "$CORE_PACKAGE_FILE" ]; then
        echo "CORE FILE MISSING"
        return
    fi

    rm -f \
        $sysdir/updater \
        $sysdir/bin/batmon \
        $sysdir/bin/prompt \
        /mnt/SDCARD/miyoo/app/.isExpert

    # Remove old lang files
    rm -rf /mnt/SDCARD/miyoo/app/lang_backup 2> /dev/null

    # Onion core installation / update.
    cd /
    unzip_progress "$CORE_PACKAGE_FILE" "$msg" /mnt/SDCARD

    # Cleanup
    rm -f $CORE_PACKAGE_FILE
}

flash_custom_logo() {
    echo "\n:: Flash custom logo"

    # At the time this function is called, all files needed for flashing (script and logos) should already be
    # present in their respective destinations (the unpacking of files has already happened)

    # "beacon" file (contains the logo name to be flashed, if file exists)
    beaconfile=".flashLogo"
    beacon="/mnt/SDCARD/${beaconfile}"

    echo "Looking for a beacon at $beacon"

    # Do not flash if the "beacon" file does not exist
    if [ ! -f "$beacon" ]; then
        echo "No beacon detected. Abort."
        echo
        return
    fi

    logo=$(cat "$beacon")
    echo "Beacon detected! Requested logo: $logo"
    rm -f "$beacon"  # The "beacon" file can now be safely deleted

    echo "Running flashing script... "
    echo "Flashing custom logo..." >> /tmp/.update_msg

    /mnt/SDCARD/.tmp_update/script/customlogo/flash_logo.sh "$logo" 0
    status=$?

    if [ $status -eq 0 ]; then  # flashing successful
        echo "Flashing successful!"
        echo "Flashing successful!" >> /tmp/.update_msg
    else  # flashing failed
        echo "Flashing failed with status=$status"
        echo "Flashing failed..." >> /tmp/.update_msg
    fi

    echo
    sleep 1
}


install_configs() {
    reset_configs=$1
    zipfile=$sysdir/config/configs.pak

    echo ":: Install configs (reset: $reset_configs)"

    if [ ! -f $zipfile ]; then
        return
    fi

    cd /mnt/SDCARD
    if [ $reset_configs -eq 1 ]; then
        # Overwrite all default configs
        rm -rf /mnt/SDCARD/Saves/CurrentProfile/config/*
        7z x -aoa $zipfile # aoa = overwrite existing files
    else
        # Extract config files without overwriting any existing files
        7z x -aos $zipfile # aos = skip existing files
    fi

    # Set X and Y button keymaps if empty
    cat /mnt/SDCARD/.tmp_update/config/keymap.json |
        sed 's/"mainui_button_x"\s*:\s*""/"mainui_button_x": "app:Search"/g' |
        sed 's/"mainui_button_y"\s*:\s*""/"mainui_button_y": "glo"/g' \
            > ./temp_keymap.json
    mv -f ./temp_keymap.json /mnt/SDCARD/.tmp_update/config/keymap.json
}

check_firmware() {
    echo ":: Check firmware"
    if [ ! -f /customer/lib/libpadsp.so ]; then
        cd $sysdir
        infoPanel -i "res/firmware.png"
        rm -rf $sysdir
        reboot
        sleep 10
        exit 0
    fi
}

backup_system() {
    echo ":: Backup system"

    # Imgs
    if [ -d /mnt/SDCARD/Imgs ]; then
        mv -f /mnt/SDCARD/Imgs /mnt/SDCARD/Backup/Imgs
    fi
}

remove_old_search() {
    echo ":: Remove old search"

    rm -rf /mnt/SDCARD/Emu/SEARCH 2> /dev/null
    rm -f /mnt/SDCARD/App/Search/data/data_cache6.db 2> /dev/null
}

run_migration_scripts() {
    local MIGRATION_DIR=$sysdir/script/migration
    local MIGRATION_STATE_FILE=$sysdir/config/migration_state
    local MIGRATION_LIST_FILE=/tmp/active_migrations
    local count=0
    local migration_state=$([ -f "$MIGRATION_STATE_FILE" ] 2> /dev/null && cat "$MIGRATION_STATE_FILE" || echo 0)

    [ -n "$migration_state" ] && [ "$migration_state" -eq "$migration_state" ] 2> /dev/null
    if [ $? -ne 0 ]; then
        migration_state=0
    fi

    if [ -d "$MIGRATION_DIR" ]; then
        for entry in "$MIGRATION_DIR"/*.sh; do
            if [ ! -f "$entry" ]; then
                continue
            fi

            migration_id=$(basename "$entry" | cut -d'_' -f1)

            # Validate migration ID
            [ -n "$migration_id" ] && [ "$migration_id" -eq "$migration_id" ] 2> /dev/null
            if [ $? -ne 0 ]; then
                continue
            fi

            if [ $migration_state -ge $migration_id ]; then
                continue
            fi

            echo "$entry" >> "$MIGRATION_LIST_FILE"
            count=$((count + 1))
        done
    fi

    if [ $count -gt 0 ] && [ -f "$MIGRATION_LIST_FILE" ]; then
        sort -f -o temp "$MIGRATION_LIST_FILE"
        rm -f "$MIGRATION_LIST_FILE"
        mv temp "$MIGRATION_LIST_FILE"

        echo "0/$count: Running migrations... 0%" >> /tmp/.update_msg
        local n=0
        local max_id=0

        while read entry; do
            n=$((n + 1))
            echo "Running migration script ($n/$count):" $(basename "$entry")
            migration_id=$(basename "$entry" | cut -d'_' -f1)

            cd $sysdir
            chmod a+x "$entry"
            eval "$entry"

            local percent=$((n * 100 / count))
            echo "$n/$count: Running migrations... $percent%" >> /tmp/.update_msg

            if [ $migration_id -gt $max_id ]; then
                max_id=$migration_id
                echo "$migration_id" > "$MIGRATION_STATE_FILE"
                sync
            fi

            sleep 0.1
        done < "$MIGRATION_LIST_FILE"

        rm $MIGRATION_LIST_FILE
    fi
}

refresh_roms() {
    echo ":: Refresh roms"
    # Force refresh the rom lists
    if [ -d /mnt/SDCARD/Roms ]; then
        cd /mnt/SDCARD/Roms
        find . -type f -name "*_cache[0-9].db" -exec rm -f {} \;
    fi
}

remove_everything_except() {
    find * .* -maxdepth 0 -not -name "$1" -exec rm -rf {} \;
}

md5hash() {
    echo $(md5sum "$1" | awk '{ print $1; }')
}

zip_total() {
    zipfile="$1"
    total=$(unzip -l "$zipfile" | tail -1 | grep -Eo "([0-9]+) files" | sed "s/[^0-9]*//g")
    echo $total
}

unzip_progress() {
    zipfile="$1"
    msg="$2"
    dest="$3"

    echo "   - Extract '$zipfile' into $dest"

    verify_file

    echo "Verifying package..." > /tmp/.update_msg
    sleep 3

    if [ -f "/mnt/SDCARD/.tmp_update/onion.pak" ]; then
        echo "onion.pak exists"
        sleep 1
    else
        echo "onion.pak is missing, extraction will fail"
    fi

    verify_file

    sleep 1

    # Run the 7z extraction command in the background and redirect output to /tmp/.extraction_output
    (
        7z x -aoa -o"$dest" "$zipfile" -bsp1 -bb > /tmp/.extraction_output
        echo $? > "/tmp/extraction_status"
    ) &

    sleep 1

    if pgrep 7z > /dev/null; then
        echo "7Z is running"
        sleep 1
    else
        echo "7Z is NOT running and should be"
        sleep 1
    fi

    if [ -f "/mnt/SDCARD/.tmp_update/onion.pak" ]; then
        echo "onion.pak still exists"
        sleep 1
    else
        echo "onion.pak is still missing, extraction has probably failed"
        sleep 1
        echo "the build will say it's complete but isn't"
    fi

    # Continuously update /tmp/.update_msg every 500 milliseconds until the command line finishes
    a=$(pgrep 7z)
    while [ -n "$a" ]; do
        last_line=$(tail -n 1 /tmp/.extraction_output)
        value=$(echo "$last_line" | sed 's/.* \([0-9]\+\)%.*/\1/')
        if [ "$value" -eq "$value" ] 2> /dev/null; then # check if the value is numeric
            if [ $value -eq 0 ]; then
                echo "Preparing file system..." > /tmp/.update_msg # It gets stuck a bit at 0%, so don't show percentage yet
            else
                echo "$msg $value%" > /tmp/.update_msg # Now we can show completion percentage
            fi
        fi
        > /tmp/.extraction_output # to avoid to parse a too big file
        sleep 0.5
        a=$(pgrep 7z)
    done

    # Check the exit status of the extraction command
    extraction_status=$(cat "/tmp/extraction_status")
    if [ "$extraction_status" -ne 0 ]; then
        touch $sysdir/.installFailed
        echo ":: Installation failed!"
        sync
        reboot
        sleep 10
        exit 0
    else
        echo "$msg 100%" >> /tmp/.update_msg
    fi

    rm /tmp/extraction_status
    rm /tmp/.extraction_output
}

free_mma() {
    /mnt/SDCARD/.tmp_update/bin/freemma
}

main
sync
reboot
sleep 10
