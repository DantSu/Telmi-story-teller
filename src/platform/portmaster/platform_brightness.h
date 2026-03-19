#ifndef PLATFORM_BRIGHTNESS_H__
#define PLATFORM_BRIGHTNESS_H__

// Linux sysfs backlight interface
#define BACKLIGHT_SYS_PATH      "/sys/class/backlight/backlight/"
#define BACKLIGHT_BRIGHTNESS     BACKLIGHT_SYS_PATH "brightness"
#define BACKLIGHT_MAX_BRIGHTNESS BACKLIGHT_SYS_PATH "max_brightness"

static uint32_t display_backlight_max = 255;
static uint32_t display_backlight_current = 255;

void platform_brightness_init(void)
{
    FILE *fp;
    file_get(fp, BACKLIGHT_MAX_BRIGHTNESS, "%u", &display_backlight_max);
    if (display_backlight_max == 0) {
        display_backlight_max = 255;
    }
    file_get(fp, BACKLIGHT_BRIGHTNESS, "%u", &display_backlight_current);
    if (display_backlight_current == 0) {
        display_backlight_current = display_backlight_max;
    }
}

void display_setBrightnessRaw(uint32_t value)
{
    if (value > display_backlight_max) {
        value = display_backlight_max;
    }
    display_backlight_current = value;
    FILE *fp;
    file_put_sync(fp, BACKLIGHT_BRIGHTNESS, "%u", value);
    printf_debug("Raw brightness: %d\n", value);
}

int display_getBrightnessRaw(void)
{
    return (int)display_backlight_current;
}

int display_getBrightnessMax(void)
{
    return (int)display_backlight_max;
}

// Set display brightness (0 - 10) scaled linearly to hardware range
void display_setBrightness(uint32_t value)
{
    if (value > 10) {
        value = 10;
    }
    int value_raw = (int)round(((double)value / 10.0) * (double)display_backlight_max);
    if (value > 0 && value_raw < 1) {
        value_raw = 1;
    }
    display_setBrightnessRaw(value_raw);
}

void display_setScreen(bool enabled)
{
    if (enabled == display_enabled) {
        return;
    }
    if (enabled) {
        if (display_backlight_current == 0) {
            display_backlight_current = display_backlight_max / 3;
        }
        display_setBrightnessRaw(display_backlight_current);
    }
    else {
        display_backlight_current = display_getBrightnessRaw();
        display_setBrightnessRaw(0);
    }
    display_enabled = enabled;
}

#endif // PLATFORM_BRIGHTNESS_H__
