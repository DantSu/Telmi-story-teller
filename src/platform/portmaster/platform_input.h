#ifndef PLATFORM_INPUT_H__
#define PLATFORM_INPUT_H__

#include "SDL2/SDL.h"

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

static SDL_GameController *_platform_input_controllers[8] = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};
static int _platform_input_opened_controllers = 0;

// Double-tap SELECT+START to quit
static bool _select_held = false;
static bool _start_held = false;
static Uint32 _last_combo_release_time = 0;
static int _combo_tap_count = 0;
#define DOUBLE_TAP_TIMEOUT_MS 500

static InputAction _key_to_action(SDL_Keycode key)
{
    switch (key) {
    case SDLK_ESCAPE:    return INPUT_ACTION_MENU;
    case SDLK_LEFT:      return INPUT_ACTION_LEFT;
    case SDLK_RIGHT:     return INPUT_ACTION_RIGHT;
    case SDLK_UP:        return INPUT_ACTION_UP;
    case SDLK_DOWN:      return INPUT_ACTION_DOWN;
    case SDLK_RETURN:    return INPUT_ACTION_START;
    case SDLK_BACKSPACE: return INPUT_ACTION_SELECT;
    case SDLK_z:         return INPUT_ACTION_A;
    case SDLK_x:         return INPUT_ACTION_B;
    case SDLK_a:         return INPUT_ACTION_Y;
    case SDLK_s:         return INPUT_ACTION_X;
    case SDLK_q:         return INPUT_ACTION_L2;
    case SDLK_w:         return INPUT_ACTION_R2;
    case SDLK_MINUS:     return INPUT_ACTION_VOL_DOWN;
    case SDLK_EQUALS:    return INPUT_ACTION_VOL_UP;
    default:             return INPUT_ACTION_NONE;
    }
}

static InputAction _controller_to_action(Uint8 button)
{
    switch (button) {
    case SDL_CONTROLLER_BUTTON_BACK:        return INPUT_ACTION_MENU;
    case SDL_CONTROLLER_BUTTON_DPAD_LEFT:   return INPUT_ACTION_LEFT;
    case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:  return INPUT_ACTION_RIGHT;
    case SDL_CONTROLLER_BUTTON_DPAD_UP:     return INPUT_ACTION_UP;
    case SDL_CONTROLLER_BUTTON_DPAD_DOWN:   return INPUT_ACTION_DOWN;
    case SDL_CONTROLLER_BUTTON_START:       return INPUT_ACTION_START;
    case SDL_CONTROLLER_BUTTON_A:           return INPUT_ACTION_A;
    case SDL_CONTROLLER_BUTTON_B:           return INPUT_ACTION_B;
    case SDL_CONTROLLER_BUTTON_X:           return INPUT_ACTION_X;
    case SDL_CONTROLLER_BUTTON_Y:           return INPUT_ACTION_Y;
    case SDL_CONTROLLER_BUTTON_LEFTSHOULDER:  return INPUT_ACTION_L2;
    case SDL_CONTROLLER_BUTTON_RIGHTSHOULDER: return INPUT_ACTION_R2;
    default:                                return INPUT_ACTION_NONE;
    }
}

static InputAction _joystick_button_to_action(Uint8 button)
{
    switch (button) {
    case 0: return INPUT_ACTION_A;
    case 1: return INPUT_ACTION_B;
    case 2: return INPUT_ACTION_X;
    case 3: return INPUT_ACTION_Y;
    case 4: return INPUT_ACTION_L2;
    case 5: return INPUT_ACTION_R2;
    case 6: return INPUT_ACTION_SELECT;
    case 7: return INPUT_ACTION_START;
    case 8: return INPUT_ACTION_MENU;
    default: return INPUT_ACTION_NONE;
    }
}

void platform_input_init(void)
{
    SDL_InitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
    SDL_GameControllerEventState(SDL_ENABLE);
    SDL_JoystickEventState(SDL_ENABLE);

    int joystickCount = SDL_NumJoysticks();
    for (int i = 0; i < joystickCount && _platform_input_opened_controllers < 8; ++i) {
        if (!SDL_IsGameController(i)) {
            continue;
        }
        SDL_GameController *controller = SDL_GameControllerOpen(i);
        if (controller != NULL) {
            _platform_input_controllers[_platform_input_opened_controllers++] = controller;
        }
    }
}

void platform_input_quit(void)
{
    for (int i = 0; i < _platform_input_opened_controllers; ++i) {
        if (_platform_input_controllers[i] != NULL) {
            SDL_GameControllerClose(_platform_input_controllers[i]);
            _platform_input_controllers[i] = NULL;
        }
    }
    _platform_input_opened_controllers = 0;
    SDL_QuitSubSystem(SDL_INIT_JOYSTICK | SDL_INIT_GAMECONTROLLER);
}

// Poll for the next input event. Returns true if an event is available.
// Sets action=INPUT_ACTION_QUIT for SDL_QUIT events.
bool platform_input_poll(InputAction *action, bool *is_pressed)
{
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        InputAction evt_action = INPUT_ACTION_NONE;
        bool evt_pressed = false;
        bool evt_released = false;

        if (event.type == SDL_QUIT) {
            *action = INPUT_ACTION_QUIT;
            *is_pressed = false;
            return true;
        }
        else if (event.type == SDL_CONTROLLERBUTTONDOWN) {
            evt_action = _controller_to_action(event.cbutton.button);
            evt_pressed = true;
        }
        else if (event.type == SDL_CONTROLLERBUTTONUP) {
            evt_action = _controller_to_action(event.cbutton.button);
            evt_released = true;
        }
        else if (event.type == SDL_JOYBUTTONDOWN) {
            // Skip joystick events for devices opened as game controllers
            // (SDL sends both controller and joystick events for the same input)
            if (SDL_IsGameController(event.jbutton.which)) {
                continue;
            }
            evt_action = _joystick_button_to_action(event.jbutton.button);
            evt_pressed = true;
        }
        else if (event.type == SDL_JOYBUTTONUP) {
            if (SDL_IsGameController(event.jbutton.which)) {
                continue;
            }
            evt_action = _joystick_button_to_action(event.jbutton.button);
            evt_released = true;
        }
        else if (event.type == SDL_JOYHATMOTION) {
            if (SDL_IsGameController(event.jhat.which)) {
                continue;
            }
            if (event.jhat.value & SDL_HAT_LEFT) {
                evt_action = INPUT_ACTION_LEFT;
                evt_pressed = true;
            }
            else if (event.jhat.value & SDL_HAT_RIGHT) {
                evt_action = INPUT_ACTION_RIGHT;
                evt_pressed = true;
            }
            else if (event.jhat.value & SDL_HAT_UP) {
                evt_action = INPUT_ACTION_UP;
                evt_pressed = true;
            }
            else if (event.jhat.value & SDL_HAT_DOWN) {
                evt_action = INPUT_ACTION_DOWN;
                evt_pressed = true;
            }
        }
        else if (event.type == SDL_KEYDOWN && event.key.repeat == 0) {
            evt_action = _key_to_action(event.key.keysym.sym);
            evt_pressed = true;
        }
        else if (event.type == SDL_KEYUP) {
            evt_action = _key_to_action(event.key.keysym.sym);
            evt_released = true;
        }

        if (evt_action == INPUT_ACTION_NONE || (!evt_pressed && !evt_released)) {
            continue;
        }

        // Track SELECT+START or MENU+START double-tap for quit
        // Note: SELECT button may be mapped as MENU when using controller events
        if (evt_action == INPUT_ACTION_SELECT || evt_action == INPUT_ACTION_MENU) {
            _select_held = evt_pressed;
        }
        if (evt_action == INPUT_ACTION_START) {
            _start_held = evt_pressed;
        }

        // Check if both buttons are held together
        bool combo_held = _select_held && _start_held;

        // When combo is released (both were held, now at least one is released)
        static bool _last_combo_held = false;
        if (_last_combo_held && !combo_held) {
            Uint32 now = SDL_GetTicks();
            if (now - _last_combo_release_time < DOUBLE_TAP_TIMEOUT_MS) {
                _combo_tap_count++;
                if (_combo_tap_count >= 2) {
                    // Double-tap detected! Send quit action
                    _combo_tap_count = 0;
                    *action = INPUT_ACTION_QUIT;
                    *is_pressed = false;
                    return true;
                }
            } else {
                _combo_tap_count = 1;
            }
            _last_combo_release_time = now;
        }
        _last_combo_held = combo_held;

        *action = evt_action;
        *is_pressed = evt_pressed;
        return true;
    }
    return false;
}

#endif // PLATFORM_INPUT_H__
