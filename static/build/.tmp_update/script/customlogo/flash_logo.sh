#!/bin/sh
# Based on Miyoo Mini Easy LogoTweak v3.0.5 by schmurtz: https://github.com/schmurtzm/Miyoo-Mini-easy-logotweak
# Reduced version of flash_Logo.sh script, customized for TelmiOS with a fully automatic flashing procedure.
#
# Changelog
# 1.0: Initial version.
#   Flashes a logo whose name is given as parameter ($1), and optionally reboots after flashing ($2.

# Note:
# Beacon detection and logo name retrieving are done outside of this script,
# so that the script can be called either during installation or update of TelmiOS.

# ======================================= Input parameters ========================================

if [ $# -ne 2 ]; then
  exit 42
fi

logo=$1  # logo name (corresponds to a JPG file in this tool resources directory)
reboot=$2  # whether a reboot is requested after flash (1 = yes, 0 = no)

# =========================================== Constants ===========================================

currentdir=`pwd`  # backup current working dir, to be restored at the end of this script
sysdir="/mnt/SDCARD/.tmp_update"
scriptdir="$sysdir/script/customlogo"  # directory where this script and binaries are located
resdir="$sysdir/res/customlogo"  # directory where logos are located
cmpdir="$resdir/do_not_remove"  # directory where complementary images are located (needed for flashing)


# ====================================== Preliminary configs ======================================

# Cleanup on exit, whichever status code is returned
cleanup() {
  rm "$scriptdir/logo.img" "$scriptdir/*.jpg"
  cd "$currentdir"
}
trap cleanup EXIT

# Configure logger
logfile=$(basename "$0" .sh)
. "$sysdir/script/log.sh"


log "Requested logo: $logo (reboot=$reboot)"


# ====================================== Preliminary controls =====================================

# Stops if the logo image does not exist
if [ ! -f "$resdir/$logo.jpg" ]; then
  log "Logo image not found. Exiting without flash!"
  exit 1
fi

# Stops if mandatory images 2 and 3 are missing (bootlogo flashing needs all three images)
if [ ! -d "$cmpdir" ] || [ ! -f "$cmpdir/image2.jpg" ] || [ ! -f "$cmpdir/image3.jpg" ]; then
  log "Some complementary image is missing. Exiting without flash!"
  exit 2
fi

# Check firmware version
MIYOO_VERSION=`/etc/fw_printenv miyoo_version`
MIYOO_VERSION=${MIYOO_VERSION#miyoo_version=}
log "Current firmware version: $MIYOO_VERSION"

# Differenciate MM and MMP supported firmware
if [ -f "/customer/app/axp_test" ]; then
	MODEL="MMP"
	SUPPORTED_VERSION="202306282128"
else
	MODEL="MM"
	SUPPORTED_VERSION="202310271401"
fi

# Firmware not supported
if [ $MIYOO_VERSION -gt $SUPPORTED_VERSION ]; then
  log "Unsupported firmware version. Exiting without flash!"
  exit 3
fi


# =========================================== Functions ===========================================

HexEdit() {
	filename=$1
	offset=$2
	value="$3"
	binary_value=$(printf "%b" "\\x$value")
	printf "$binary_value" | dd of="$filename" bs=1 seek="$offset" conv=notrunc
}

checkjpg() {
	JpgFilePath=$1
	Filename=`basename "$JpgFilePath"`
	./bin/checkjpg "$JpgFilePath"
	CHECK_JPG=$?
	if [ $CHECK_JPG -eq 0 ]; then
		log "$Filename is a valid VGA JPG file"
	elif [ $CHECK_JPG -eq 1 ]; then
		log "$Filename is not a valid jpg file. Exiting without flash!"
		exit 4
	elif [ $CHECK_JPG -eq 2 ]; then
		log "$Filename doesn't have the right resolution; it should be 640x480 (VGA). Exiting without flash!"
		exit 5
	else
	  log "Unknown Checkjpg error occurred. Exiting without flash!"
	  exit 6
	fi
}


# =========================== images checks and logo.img generation ===============================

cd $scriptdir

# Check if the logo file really a jpg file (and not png files renamed)
checkjpg "$resdir/$logo.jpg"
# Check if the complementary images are also valid (in case of unwanted edit)
checkjpg "$cmpdir/image2.jpg"
checkjpg "$cmpdir/image3.jpg"

# Temporary copy the chosen logo and complementary images
cp "$resdir/$logo.jpg" "./image1.jpg"
cp "$cmpdir/image2.jpg" .
cp "$cmpdir/image3.jpg" .

# Create the new logo.img, then remove temporary .jpg files
rm "./logo.img"
./bin/logomake
rm "./image1.jpg" "./image2.jpg" "./image3.jpg"

# Patch screen offset for the MMP
if [ "$MODEL" = "MMP" ]; then
  HexEdit "./logo.img" 1086 2C
  HexEdit "./logo.img" 1088 4C
fi

# Check that the size of the created logo.img is correct
filesize=$(wc -c "./logo.img" | awk '{print $1}')
if [ "$filesize" != "131072" ]; then
  log "Wrong logo.img size. Exiting without flash!"
  exit 7
fi

# Special case for MM
if [ "$MODEL" = "MM" ]; then
  # Check for SPI write capability (for Mini with BoyaMicro chips)
  CHECK_WRITE=`./bin/checkwrite`
  CHECK_WRITE=$?

  # MM with no SPI write capability are not currently supported for Telmi (the procedure is not fully automatic).
  # See EasyLogoTweak application for a procedure to flash the logo on such device.
  if [ $CHECK_WRITE -ne 0 ]; then
    log "Unsupported device. Exiting without flash!"
    exit 8
  fi
fi


# ===================================== Actual logos flashing =====================================

log "Flashing..."
./bin/logowrite
log "Flash Done!"

cleanup

# Reboot only if requested
if [ "$reboot" -eq 1 ]; then
  log "Rebooting..."
  reboot
  sleep 10
fi

exit 0
