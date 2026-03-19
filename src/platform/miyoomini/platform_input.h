#ifndef PLATFORM_INPUT_H__
#define PLATFORM_INPUT_H__

#include <fcntl.h>
#include <linux/input.h>
#include <poll.h>

#include "system/keymap_hw.h"

typedef enum InputAction {
    INPUT_ACTION_NONE = 0,
    INPUT_ACTION_QUIT,
    INPUT_ACTION_MENU,
    INPUT_ACTION_POWER,
    INPUT_ACTION_LEFT,
    INPUT_ACTION_RIGHT,
    INPUT_ACTION_UP,
    INPUT_ACTION_DOWN,
    INPUT_ACTION_START,
    INPUT_ACTION_SELECT,
    INPUT_ACTION_A,
    INPUT_ACTION_B,
    INPUT_ACTION_X,
    INPUT_ACTION_Y,
    INPUT_ACTION_L2,
    INPUT_ACTION_R2,
    INPUT_ACTION_VOL_DOWN,
    INPUT_ACTION_VOL_UP,
} InputAction;

static int _platform_input_fd = -1;
static struct input_event _platform_input_ev;
static struct pollfd _platform_input_fds[1];

static InputAction _miyoo_code_to_action(unsigned int code)
{
    switch (code) {
    case HW_BTN_MENU:        return INPUT_ACTION_MENU;
    case HW_BTN_POWER:       return INPUT_ACTION_POWER;
    case HW_BTN_LEFT:        return INPUT_ACTION_LEFT;
    case HW_BTN_RIGHT:       return INPUT_ACTION_RIGHT;
    case HW_BTN_UP:          return INPUT_ACTION_UP;
    case HW_BTN_DOWN:        return INPUT_ACTION_DOWN;
    case HW_BTN_START:       return INPUT_ACTION_START;
    case HW_BTN_SELECT:      return INPUT_ACTION_SELECT;
    case HW_BTN_A:           return INPUT_ACTION_A;
    case HW_BTN_B:           return INPUT_ACTION_B;
    case HW_BTN_X:           return INPUT_ACTION_X;
    case HW_BTN_Y:           return INPUT_ACTION_Y;
    case HW_BTN_L2:          return INPUT_ACTION_L2;
    case HW_BTN_R2:          return INPUT_ACTION_R2;
    case HW_BTN_VOLUME_DOWN: return INPUT_ACTION_VOL_DOWN;
    case HW_BTN_VOLUME_UP:   return INPUT_ACTION_VOL_UP;
    default:                 return INPUT_ACTION_NONE;
    }
}

void platform_input_init(void)
{
    _platform_input_fd = open("/dev/input/event0", O_RDONLY);
    memset(_platform_input_fds, 0, sizeof(_platform_input_fds));
    _platform_input_fds[0].fd = _platform_input_fd;
    _platform_input_fds[0].events = POLLIN;
}

void platform_input_quit(void)
{
    if (_platform_input_fd >= 0) {
        close(_platform_input_fd);
        _platform_input_fd = -1;
    }
}

// Poll for the next input event. Returns true if an event is available.
// Skips REPEAT events (value=2) and non-key events.
bool platform_input_poll(InputAction *action, bool *is_pressed)
{
    while (poll(_platform_input_fds, 1, 0) > 0) {
        read(_platform_input_fd, &_platform_input_ev, sizeof(_platform_input_ev));
        if (_platform_input_ev.type != EV_KEY || _platform_input_ev.value > 1) {
            continue; // skip non-key events and REPEAT (value=2)
        }
        *is_pressed = (_platform_input_ev.value == 1);
        *action = _miyoo_code_to_action(_platform_input_ev.code);
        return true;
    }
    return false;
}

#endif // PLATFORM_INPUT_H__
