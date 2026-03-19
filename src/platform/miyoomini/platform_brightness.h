#ifndef PLATFORM_BRIGHTNESS_H__
#define PLATFORM_BRIGHTNESS_H__

// Miyoo Mini hardware paths
#define GPIO_DIR1 "/sys/class/gpio/"
#define GPIO_DIR2 "/sys/devices/gpiochip0/gpio/"
#define PWM_DIR   "/sys/devices/soc0/soc/1f003400.pwm/pwm/pwmchip0/"

// Track the last raw PWM value set so getBrightnessRaw() can return it
static uint32_t _display_brightness_raw = 50;

void platform_brightness_init(void)
{
    // No-op: Miyoo Mini brightness is controlled via PWM at runtime
}

void display_setBrightnessRaw(uint32_t value)
{
    FILE *fp;
    file_put_sync(fp, PWM_DIR "pwm0/duty_cycle", "%u", value);
    _display_brightness_raw = value;
    printf_debug("Raw brightness: %d\n", value);
}

int display_getBrightnessRaw(void)
{
    return (int)_display_brightness_raw;
}

int display_getBrightnessMax(void)
{
    // Approximate maximum raw PWM value for exponential curve at level 10:
    // round(3.0 * exp(0.350656 * 10)) ≈ 100
    return 100;
}

// Set display brightness (0 - 10) using exponential curve
void display_setBrightness(uint32_t value)
{
    int value_raw = (int)round(3.0 * exp(0.350656 * value));
    display_setBrightnessRaw(value_raw);
}

void display_setScreen(bool enabled)
{
    // export gpio4, direction: out
    file_write(GPIO_DIR1 "export", "4", 1);
    file_write(GPIO_DIR2 "gpio4/direction", "out", 3);

    // screen on/off
    file_write(GPIO_DIR2 "gpio4/value", enabled ? "1" : "0", 1);

    // unexport gpio4
    file_write(GPIO_DIR1 "unexport", "4", 1);

    if (enabled) {
        // re-enable brightness control
        file_write(PWM_DIR "export", "0", 1);
        file_write(PWM_DIR "pwm0/enable", "0", 1);
        file_write(PWM_DIR "pwm0/enable", "1", 1);
        display_restore();
    }
    else {
        display_save();
    }

    display_enabled = enabled;
}

#endif // PLATFORM_BRIGHTNESS_H__
