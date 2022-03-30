// SPDX-License-Identifier: MIT

#include "lv_port_indev.h"
#include "lvgl.h"
#include <io/pad.h>
#include <io/kb.h>

static lv_indev_drv_t indev_drv_gamepad;
static lv_indev_drv_t indev_drv_mouse;
static lv_indev_t *indev_mouse;
static lv_indev_t *indev_keypad;
static lv_obj_t *mouse_cursor;
static int mouse_x, mouse_y;
static lv_quit_event_t quit_event = LV_QUIT_NONE;
static bool mouse_event = false;
static bool mouse_pressed = false;
#ifndef MOUSE_SENSITIVITY
#define MOUSE_SENSITIVITY 50 // pixels per input poll LV_INDEV_DEF_READ_PERIOD
#endif
#ifndef MOUSE_DEADZONE
#define MOUSE_DEADZONE 10 // Percent
#endif

#ifndef LVGL_USE_CUSTOM_CONTROLLER_MAP
static gamecontroller_map_t lvgl_gamecontroller_map[] =
    {
        {.ps3_map = PAD_CTRL_CROSS, .lvgl_map = LV_KEY_ENTER},
        {.ps3_map = PAD_CTRL_CIRCLE, .lvgl_map = LV_KEY_ESC},
        {.ps3_map = PAD_CTRL_SQUARE, .lvgl_map = LV_KEY_BACKSPACE},
        {.ps3_map = PAD_CTRL_TRIANGLE, .lvgl_map = LV_KEY_HOME},
        {.ps3_map = PAD_CTRL_SELECT, .lvgl_map = LV_KEY_PREV},
        {.ps3_map = PAD_CTRL_START, .lvgl_map = LV_KEY_NEXT},
        {.ps3_map = PAD_CTRL_L3, .lvgl_map = LV_KEY_PREV},
        {.ps3_map = PAD_CTRL_R3, .lvgl_map = LV_KEY_NEXT},
        {.ps3_map = PAD_CTRL_L1, .lvgl_map = LV_KEY_PREV},
        {.ps3_map = PAD_CTRL_R1, .lvgl_map = LV_KEY_NEXT},
        {.ps3_map = PAD_CTRL_UP, .lvgl_map = LV_KEY_UP},
        {.ps3_map = PAD_CTRL_DOWN, .lvgl_map = LV_KEY_DOWN},
        {.ps3_map = PAD_CTRL_LEFT, .lvgl_map = LV_KEY_LEFT},
        {.ps3_map = PAD_CTRL_RIGHT, .lvgl_map = LV_KEY_RIGHT}};
#else
extern gamecontroller_map_t lvgl_gamecontroller_map[];
#endif

#ifndef LVGL_USE_CUSTOM_KEYBOARD_MAP
static keyboard_map_t lvgl_keyboard_map[] =
    {
        {.ps3_map = KB_RAWKEY_ESC, .lvgl_map = LV_KEY_ESC},
        {.ps3_map = KB_RAWKEY_BS, .lvgl_map = LV_KEY_BACKSPACE},
        {.ps3_map = KB_RAWKEY_HOME, .lvgl_map = LV_KEY_HOME},
        {.ps3_map = KB_RAWKEY_ENTER, .lvgl_map = LV_KEY_ENTER},
        {.ps3_map = KB_RAWKEY_PAGE_DOWN, .lvgl_map = LV_KEY_PREV},
        {.ps3_map = KB_RAWKEY_PAGE_UP, .lvgl_map = LV_KEY_NEXT},
        {.ps3_map = KB_RAWKEY_TAB, .lvgl_map = LV_KEY_NEXT},
        {.ps3_map = KB_RAWKEY_UP_ARROW, .lvgl_map = LV_KEY_UP},
        {.ps3_map = KB_RAWKEY_DOWN_ARROW, .lvgl_map = LV_KEY_DOWN},
        {.ps3_map = KB_RAWKEY_LEFT_ARROW, .lvgl_map = LV_KEY_LEFT},
        {.ps3_map = KB_RAWKEY_RIGHT_ARROW, .lvgl_map = LV_KEY_RIGHT}};
#else
extern keyboard_map_t lvgl_keyboard_map[];
#endif

lv_quit_event_t lv_get_quit()
{
    return quit_event;
}

void lv_set_quit(lv_quit_event_t event)
{
    quit_event = event;
}

static void mouse_read(lv_indev_drv_t *indev_drv_gamepad, lv_indev_data_t *data)
{

    data->state = (mouse_pressed) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;

    padData pad_data;

    ioPadGetData(0, &pad_data);

    int x = pad_data.ANA_L_H - 127;
    int y = pad_data.ANA_L_V - 127;

    if (SDL_abs(x) > (MOUSE_DEADZONE * 255) / 100)
    {
        mouse_x += (x * MOUSE_SENSITIVITY) / 255;
        if (mouse_x < 0)
            mouse_x = 0;
        if (mouse_x > 640)
            mouse_x = 640;
    }

    if (SDL_abs(y) > (MOUSE_DEADZONE * 255) / 100)
    {
        mouse_y += (y * MOUSE_SENSITIVITY) / 255;
        if (mouse_y < 0)
            mouse_y = 0;
        if (mouse_y > 640)
            mouse_y = 640;
    }

    data->point.x = (int16_t)mouse_x;
    data->point.y = (int16_t)mouse_y;

}

static void keypad_read(lv_indev_drv_t *indev_drv_gamepad, lv_indev_data_t *data)
{
    data->key = 0;

    static SDL_Event e;
    if (SDL_PollEvent(&e))
    {
        if (e.type == SDL_WINDOWEVENT)
        {
            if (e.window.event == SDL_WINDOWEVENT_CLOSE)
            {
                quit_event = true;
            }
        }

        // Handle controller hotplugging
        if (e.type == SDL_CONTROLLERDEVICEADDED)
        {
            SDL_GameController *new_pad = SDL_GameControllerOpen(e.cdevice.which);
            if (pad == NULL)
            {
                pad = new_pad;
            }
        }

        if (e.type == SDL_CONTROLLERDEVICEREMOVED)
        {
            if (pad == SDL_GameControllerFromInstanceID(e.cdevice.which))
            {
                pad = NULL;
            }
            SDL_GameControllerClose(SDL_GameControllerFromInstanceID(e.cdevice.which));
        }

        // Parse some mouse events while we are here.
        if (e.type == SDL_MOUSEMOTION || e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP)
        {
            mouse_event = true;
        }

        if ((e.type == SDL_MOUSEBUTTONDOWN || e.type == SDL_MOUSEBUTTONUP) && e.button.button == SDL_BUTTON_LEFT)
        {
            mouse_event = true;
            mouse_pressed = (e.type == SDL_MOUSEBUTTONDOWN);
        }

        if ((e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP) && e.cbutton.button == SDL_CONTROLLER_BUTTON_LEFTSTICK)
        {
            mouse_pressed = (e.type == SDL_CONTROLLERBUTTONDOWN);
        }

        padData pad_data;

        ioPadGetData(0, &pad_data);

        // Handle controller button events
        if (e.type == SDL_CONTROLLERBUTTONDOWN || e.type == SDL_CONTROLLERBUTTONUP)
        {
            // If we hit this assert, lvgl_gamecontroller_map isnt right
            LV_ASSERT(lvgl_gamecontroller_map[e.cbutton.button].sdl_map == e.cbutton.button);
            data->key = lvgl_gamecontroller_map[e.cbutton.button].lvgl_map;
            data->state = (e.type == SDL_CONTROLLERBUTTONDOWN) ? LV_INDEV_STATE_PRESSED : LV_INDEV_STATE_RELEASED;
        }

        KbData kb_data;

        ioKbRead(0, &kb_data);

        for (int i = 0; i < (sizeof(lvgl_keyboard_map) / sizeof(keyboard_map_t)); i++)
        {
            if (lvgl_keyboard_map[i].ps3_map == kb_data.nb_keycode)
            {
                data->key = lvgl_keyboard_map[i].lvgl_map;
                data->state = true;
            }
        }
    }
    //Is there more input events?
    data->continue_reading = false;
}

void lv_port_indev_init(bool use_mouse_cursor)
{
    ioPadInit(4);
    ioKbInit(1);
    SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER);
    for (int i = 0; i < SDL_NumJoysticks(); i++)
    {
        SDL_GameControllerOpen(i);
    }

    // Register the gamepad as a keypad
    lv_indev_drv_init(&indev_drv_gamepad);
    indev_drv_gamepad.type = LV_INDEV_TYPE_KEYPAD;
    indev_drv_gamepad.read_cb = keypad_read;
    indev_keypad = lv_indev_drv_register(&indev_drv_gamepad);

    // Register a mouse cursor
    if (use_mouse_cursor)
    {
        lv_indev_drv_init(&indev_drv_mouse);
        indev_drv_mouse.type = LV_INDEV_TYPE_POINTER;
        indev_drv_mouse.read_cb = mouse_read;
        indev_mouse = lv_indev_drv_register(&indev_drv_mouse);
        mouse_cursor = lv_img_create(lv_scr_act());
        lv_img_set_src(mouse_cursor, LV_SYMBOL_PLUS);
        lv_indev_set_cursor(indev_mouse, mouse_cursor);
    }
    quit_event = false;
}

void lv_port_indev_deinit(void)
{
    ioPadEnd();
    ioKbEnd();
}
