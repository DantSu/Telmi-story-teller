![Total Download](https://img.shields.io/github/downloads/DantSu/Telmi-story-teller/total.svg) [![v1.6.1 download](https://img.shields.io/github/downloads/DantSu/Telmi-story-teller/1.6.1/total.svg)](https://github.com/DantSu/Telmi-story-teller/releases/tag/1.6.1)

<p align="center"><img src="https://dantsu.com/files/Telmi_1280.png" alt="Telmi OS splash screen" /></p>

# Telmi - An open source story teller and MP3 player for Miyoo Mini

Telmi OS is an open source story teller and lite MP3 player for Miyoo Mini and Miyoo Mini Plus.
Telmi OS is for children 3~4 years old and older.

The story teller is compatible with stories exported from [STUdio](https://github.com/DantSu/studio).

## Installation

### Download the installation files

- [Download the latest version of Telmi](https://github.com/DantSu/Telmi-story-teller/releases/download/1.6.1/TelmiOS_v1.6.1.zip)

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

### Unzip TelmiOS_v1.6.1.zip

Put the content of `TelmiOS_v1.6.1.zip` at the root of the SD card.

### Power on your Miyoo

Insert the SD card in your Miyoo. Press the power button and the installation will run automatically.

**Enjoy !!**

# Add stories and music on Telmi OS

<p align="center"><img src="https://dantsu.com/files/Telmi_MiyooPC.jpg" alt="Telmi OS - Telmi Sync" /></p>

Download the latest version of [Telmi Sync](https://github.com/DantSu/Telmi-Sync/releases/) and install it (Supported OS : Windows, MacOS, Linux).
Drag and drop your stories exported from [STUdio](https://github.com/DantSu/studio) on Telmi Sync to import it. Do the same with audio files. 
Then plug SD card of Telmi OS on your computer, it will be reconized by TelmiSync. You can now transfer stories and music to Telmi OS.

# Buttons roles

<p align="center"><img src="https://dantsu.com/files/Telmi_selectingStory.png" alt="Telmi OS - buttons roles selecting story" /><img src="https://dantsu.com/files/Telmi_readingStory.png" alt="Telmi OS - buttons roles reading story" /><img src="https://dantsu.com/files/Telmi_playingMusic.png" alt="Telmi OS - buttons roles playing music" /><img src="https://dantsu.com/files/Telmi_selectingAlbum.png" alt="Telmi OS - buttons roles selecting music album" /></p>

# Youtube videos

### Vidéo d'installation et de prise en main de Telmi par @PhOeNiXv8.3 en français

<p align="center"><a href="https://www.youtube.com/watch?v=r7VK73ASUGo" taget="_blank"><img src="https://dantsu.com/files/Telmi_YoutubePhoenix.png" alt="Vidéo d'installation et d'utilisation de Telmi OS et Telmi Sync" width="340" /></a></p>

# Discord

Tu parles français et tu veux discuter avec moi ? Demander de l'aide ? Poser des questions ?
Rendez vous sur le discord de [Telmi](https://discord.gg/jkBr7pcQ).
