<p align="center"><img = src="https://dantsu.com/files/Telmi_1280.png" alt="Telmi OS splash screen" /></p>

# Telmi - An open source story teller and MP3 player for Miyoo Mini

TelmiOS is an open source story teller and lite MP3 player for Miyoo Mini and Miyoo Mini Plus.
TelmiOS is for children 3~4 years old and older.

The story teller is compatible with stories exported from [STUdio](https://github.com/DantSu/studio).

## Installation

### Download the installation files

- [Download the latest version of Telmi](https://github.com/DantSu/Telmi-story-teller/releases/download/1.0.0/TelmiOS_v1.0.0.zip)

### Format your SD card as FAT32 (not exFAT!)

#### Windows
If your SD card is larger than 32 GB, you need to use a third-party tool like [Rufus](https://rufus.ie/):

1. In Rufus, select the device that corresponds with the SD card you want to format
2. Set **Boot selection** to `Non bootable`
3. **Partition scheme** should already be set to `MBR`
4. Set **File system** to `Large FAT32`
5. **Cluster size** at `32 kilobytes` should be fine (`16 kilobytes` might extend the life of your SD card)
6. Click **START** to format the card, when it's done you can close the window

#### MacOS

On Mac, you can format your SD card with the built-in Disk Utilities:

1. Open **Application** › **Utilities** › **Disk Utilities**
2. From the sidebar, select the USB drive that corresponds with the SD card you want to format (Choose to format the root element [as on this picture](https://onionui.github.io/assets/files/format-usb-to-fat32-on-mac-6244645c5513220bacdeec4aaa541bc8.webp))
3. Choose **Erase** from the toolbar
4. Set **Format** › `MS-DOS (FAT)`
5. Set **Scheme** (if it exists) › `Master Boot Record`
6. Click **Erase** and wait for the process to finish

#### Linux

There are lots of ways to format an SD card on Linux. 3 methods are presented in [this blog post](https://www.golinuxcloud.com/steps-to-format-sd-card-in-linux/).

Make sure you choose: `For use with all systems and devices (FAT)`

#### Chrome OS

Chrome already has a tool to format an SD card. Insert the card into your Chromebook, right-click it and click **Format Device**, make sure `FAT32` is selected under **Format** and click or tap **Erase & Format**. 

### Unzip TelmiOS_v1.0.0.zip

Put the content of `TelmiOS_v1.0.0.zip` at the root of the SD card.

### Power on your Miyoo

Insert the SD card in your Miyoo. Press the power button and the installation will run automatically.

**Enjoy !!**

# Add stories and music on TelmiOS

Download the latest version of [Telmi Sync](https://github.com/DantSu/Telmi-Sync/releases/) and install it (Supported OS : Windows, MacOS, Linux).
Drag and drop your stories exported from [STUdio](https://github.com/DantSu/studio) on Telmi Sync to import it. Do the same with audio files. 
Then plug SD card of TelmiOS on your computer, it will be reconized by TelmiSync. You can now transfer stories and music to TelmiOS.

# Buttons roles

- **Power** : Power on and power off
- **LEFT** : Previous action or previous Song
- **RIGHT** : Next action or next Song
- **A or B** : Validate
- **X or Y** : Back
- **Select or Start** : Pause
- **Volume UP** : Increase volume
- **Volume DOWN** : Decrease volume
- **Home + Volume UP** : Increase brightness
- **Home + Volume DOWN** : Decrease brightness

# Demo

<p align="center"><a href="https://www.youtube.com/watch?v=0kqYJ-VKsNU" taget="_blank"><img = src="https://dantsu.com/files/Telmi_youtube.png" alt="Telmi OS demonstration video" /></a></p>