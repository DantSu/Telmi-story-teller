#ifndef MP3_HELPER_
#define MP3_HELPER_

#include <fcntl.h>
#include <stdint.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Tableaux des bitrates Layer III des frames MP3, indexés par bitRateIndex
static const int mpeg1Layer3Bitrates[15] = {0, 32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 256, 320};
static const int mpeg2Layer3Bitrates[15] = {0, 8, 16, 24, 32, 40, 48, 56, 64, 80, 96, 112, 128, 144, 160};

// Recherche la première frame MP3 (Layer III) valide dans data et retourne son
// bitrate en kbps, ou -1 si aucune frame valide n'est trouvée.
static int mp3_frame_bitrate(const uint8_t *data, size_t length) {
    int bitrateKbps = -1;
    for (size_t offset = 0; offset + 3 < length && bitrateKbps < 0; ++offset) {
        uint8_t b0 = data[offset];
        uint8_t b1 = data[offset + 1];
        uint8_t b2 = data[offset + 2];
        if (b0 != 0xFF || (b1 & 0xF0) != 0xF0) {
            continue; // pas de sync de frame
        }
        int version = (b1 >> 3) & 0x03; // 3 = MPEG1, 0 = MPEG2, 1 = MPEG2.5, 2 = réservé
        int layer = (b1 >> 1) & 0x03; // 1 = Layer III (MP3)
        int bitRateIndex = (b2 >> 4) & 0x0F;
        int sampleRateIndex = (b2 >> 2) & 0x03;
        if (layer != 1 || bitRateIndex == 0 || bitRateIndex == 15 || sampleRateIndex == 3) {
            continue;
        }
        if (version == 3) { // MPEG1
            bitrateKbps = mpeg1Layer3Bitrates[bitRateIndex];
        } else if (version == 1 || version == 0) { // MPEG2 / MPEG2.5
            bitrateKbps = mpeg2Layer3Bitrates[bitRateIndex];
        }
    }
    return bitrateKbps;
}

// Estimation rapide de la durée d'un MP3 CBR : durée = octetsAudio * 8 / bitrate.
// On lit seulement le premier frame pour récupérer le bitrate, et on déduit la
// taille des tags ID3 (qui ne sont pas de l'audio). La détection VBR échantillonne
// une frame au milieu de la zone audio : si son bitrate diffère du premier frame,
// le fichier n'est pas CBR et la formule n'est plus valable. Retourne -1 si aucune
// frame MP3 valide n'est trouvée ou si VBR est détecté (le caller doit alors replier
// sur Mix_MusicDuration).
double mp3_duration_estimate(const char *path) {
    double duration = -1.0;
    int fd = open(path, O_RDONLY);
    if (fd >= 0) {
        struct stat fileStat;
        if (fstat(fd, &fileStat) == 0) {
            off_t fileSize = fileStat.st_size;
            uint8_t buffer[4096];
            ssize_t bytesRead = read(fd, buffer, sizeof(buffer));

            if (bytesRead >= 4 && fileSize >= bytesRead) {
                // Tag ID3v2 en tête : taille syncsafe (4 octets de 7 bits)
                off_t audioStart = 0;
                if (bytesRead >= 10 && buffer[0] == 'I' && buffer[1] == 'D' && buffer[2] == '3') {
                    audioStart = 10
                               + ((off_t)(buffer[6] & 0x7F) << 21)
                               + ((off_t)(buffer[7] & 0x7F) << 14)
                               + ((off_t)(buffer[8] & 0x7F) << 7)
                               + (off_t)(buffer[9] & 0x7F);
                    if ((buffer[5] & 0x10) != 0) {
                        audioStart += 10; // footer ID3v2
                    }
                }

                if (audioStart < fileSize) {
                    // Tag ID3v1 en queue
                    off_t audioEnd = fileSize;
                    if (fileSize >= 128) {
                        uint8_t tail[128];
                        if (lseek(fd, -128, SEEK_END) >= 0
                            && read(fd, tail, sizeof(tail)) == 128
                            && tail[0] == 'T' && tail[1] == 'A' && tail[2] == 'G') {
                            audioEnd -= 128;
                        }
                    }

                    // Première frame : on la lit après le tag si elle dépasse le buffer de 4Ko
                    uint8_t frameBuffer[4096];
                    off_t frameRead = 0;
                    if (audioStart < bytesRead) {
                        frameRead = bytesRead - audioStart;
                        memcpy(frameBuffer, buffer + audioStart, frameRead);
                    } else {
                        if (lseek(fd, audioStart, SEEK_SET) >= 0) {
                            ssize_t frameBytes = read(fd, frameBuffer, sizeof(frameBuffer));
                            if (frameBytes > 0) {
                                frameRead = frameBytes;
                            }
                        }
                    }

                    // Détection VBR : on échantillonne une frame au milieu de la zone audio
                    // et on compare son bitrate au premier frame.
                    off_t middlePosition = audioStart + (audioEnd - audioStart) / 2;
                    uint8_t middleBuffer[4096];
                    ssize_t middleRead = 0;
                    if (lseek(fd, middlePosition, SEEK_SET) >= 0) {
                        middleRead = read(fd, middleBuffer, sizeof(middleBuffer));
                    }

                    int firstBitrateKbps = mp3_frame_bitrate(frameBuffer, (size_t) frameRead);
                    int middleBitrateKbps = (middleRead > 3) ? mp3_frame_bitrate(middleBuffer, (size_t) middleRead) : -1;

                    if (firstBitrateKbps > 0 && middleBitrateKbps == firstBitrateKbps
                        && audioEnd - audioStart >= 4) {
                        duration = ((double)(audioEnd - audioStart) * 8.0) / ((double)firstBitrateKbps * 1000.0);
                        if (duration <= 0.0) {
                            duration = -1.0;
                        }
                    }
                }
            }
        }
        close(fd); // unique point de sortie : impossible d'oublier la fermeture
    }
    return duration;
}

#endif // MP3_HELPER_
