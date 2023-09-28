#include "Bluepad_main.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/async_context.h"

#include "btstack.h"
#include "Bluepad32.h"
#include "uni_main.h"
#include "uni_gamepad.h"

#include "SwitchDescriptors.h"

#include "pico/multicore.h"

#include <memory>
#include <stdio.h>

#define BTSTACK_WORK_TIMER_MS 5

static btstack_timer_source_t work_timer;

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

// int conn;
// uint16_t buttons;
SwitchOutReport report;
void get_button_state(SwitchOutReport *rpt)
{
    async_context_t *context = cyw43_arch_async_context();
    async_context_acquire_lock_blocking(context);
    memcpy(rpt, &report, sizeof(*rpt));
    async_context_release_lock(context);
}

// This callback gets called any time a new gamepad is connected.
// Up to 4 gamepads can be connected at the same time.
void onConnectedGamepad(GamepadPtr gp)
{
    bool foundEmptySlot = false;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++)
    {
        if (myGamepads[i] == nullptr)
        {
            printf("CALLBACK: Gamepad is connected, index=%d\n", i);
            // Additionally, you can get certain gamepad properties like:
            // Model, VID, PID, BTAddr, flags, etc.
            GamepadProperties properties = gp->getProperties();
            printf("Gamepad model: %s, VID=0x%04x, PID=0x%04x\n", gp->getModelName(), properties.vendor_id,
                   properties.product_id);
            myGamepads[i] = gp;
            foundEmptySlot = true;
            break;
        }
    }
    if (!foundEmptySlot)
    {
        printf("CALLBACK: Gamepad connected, but could not found empty slot\n");
    }
}

void onDisconnectedGamepad(GamepadPtr gp)
{
    bool foundGamepad = false;

    for (int i = 0; i < BP32_MAX_GAMEPADS; i++)
    {
        if (myGamepads[i] == gp)
        {
            printf("CALLBACK: Gamepad is disconnected from index=%d\n", i);
            myGamepads[i] = nullptr;
            foundGamepad = true;
            break;
        }
    }

    if (!foundGamepad)
    {
        printf("CALLBACK: Gamepad disconnected, but not found in myGamepads\n");
    }
}

uint8_t convert_to_switch_axis(int32_t bluepadAxis)
{
    // bluepad32 reports from -512 to 511 as int32_t
    // switch reports from 0 to 255 as uint8_t

    bluepadAxis += 513; // now max possible is 1024
    bluepadAxis /= 4;   // now max possible is 255

    if (bluepadAxis < SWITCH_JOYSTICK_MIN)
        bluepadAxis = 0;
    else if (bluepadAxis > SWITCH_JOYSTICK_MAX)
        bluepadAxis = SWITCH_JOYSTICK_MAX;

    return (uint8_t)bluepadAxis;
}

static void work_timer_handler(btstack_timer_source_t *ts)
{
    BP32.update();

    GamepadPtr myGamepad = myGamepads[0];
    if (myGamepad && myGamepad->isConnected())
    {

        // report.buttons = myGamepad->buttons();
        uint16_t buttons = myGamepad->buttons();
        uint16_t buttons_aux = buttons; // keep initial state

        buttons &= ~SWITCH_MASK_A;
        buttons &= ~SWITCH_MASK_B;
        buttons &= ~SWITCH_MASK_X;
        buttons &= ~SWITCH_MASK_Y;
        buttons &= ~SWITCH_MASK_MINUS;
        buttons &= ~SWITCH_MASK_PLUS;

        // now apply correct masking
        if (buttons_aux & SWITCH_MASK_A)
        {
            buttons |= SWITCH_MASK_Y;
        }
        if (buttons_aux & SWITCH_MASK_B)
        {
            buttons |= SWITCH_MASK_A;
        }
        if (buttons_aux & SWITCH_MASK_Y)
        {
            buttons |= SWITCH_MASK_B;
        }
        if (buttons_aux & SWITCH_MASK_X)
        {
            buttons |= SWITCH_MASK_X;
        }
        if (buttons_aux & SWITCH_MASK_MINUS)
        {
            buttons |= SWITCH_MASK_L3;
        }
        if (buttons_aux & SWITCH_MASK_PLUS)
        {
            buttons |= SWITCH_MASK_R3;
        }
        report.buttons = buttons;

        report.lx = convert_to_switch_axis(myGamepad->axisX());
        report.ly = convert_to_switch_axis(myGamepad->axisY());
        report.rx = convert_to_switch_axis(myGamepad->axisRX());
        report.ry = convert_to_switch_axis(myGamepad->axisRY());

        if (myGamepad->brake() > 0)
        {
            report.buttons |= SWITCH_MASK_ZL;
        }
        if (myGamepad->throttle() > 0)
        {
            report.buttons |= SWITCH_MASK_ZR;
        }
        if (myGamepad->miscSystem())
        {
            report.buttons |= SWITCH_MASK_HOME;
        }
        if (false) // bluepad doesn't detect this
        {
            report.buttons |= SWITCH_MASK_CAPTURE;
        }
        if (myGamepad->miscBack()) // xbox series doesn't detect this
        {
            report.buttons |= SWITCH_MASK_MINUS;
        }
        if (myGamepad->miscHome())
        {
            report.buttons |= SWITCH_MASK_PLUS;
        }

        switch (myGamepad->dpad())
        {
        case DPAD_UP:
            report.hat = SWITCH_HAT_UP;
            break;
        case DPAD_DOWN:
            report.hat = SWITCH_HAT_DOWN;
            break;
        case DPAD_LEFT:
            report.hat = SWITCH_HAT_LEFT;
            break;
        case DPAD_RIGHT:
            report.hat = SWITCH_HAT_RIGHT;
            break;
        case DPAD_UP | DPAD_RIGHT:
            report.hat = SWITCH_HAT_UPRIGHT;
            break;
        case DPAD_DOWN | DPAD_RIGHT:
            report.hat = SWITCH_HAT_DOWNRIGHT;
            break;
        case DPAD_DOWN | DPAD_LEFT:
            report.hat = SWITCH_HAT_DOWNLEFT;
            break;
        case DPAD_UP | DPAD_LEFT:
            report.hat = SWITCH_HAT_UPLEFT;
            break;
        default:
            report.hat = SWITCH_HAT_NOTHING;
            break;
        }
    }

    // set timer for next tick
    btstack_run_loop_set_timer(&work_timer, BTSTACK_WORK_TIMER_MS);
    btstack_run_loop_add_timer(&work_timer);
}

void init_bluepad(void)
{
    // initialize CYW43 driver architecture (will enable BT if/because CYW43_ENABLE_BLUETOOTH == 1)
    if (cyw43_arch_init())
    {
        printf("failed to initialise cyw43_arch\n");
        return;
    }

    // buttons = 0;

    // Initialize on zero
    report.buttons = 0;
    report.hat = SWITCH_HAT_NOTHING;
    report.lx = SWITCH_JOYSTICK_MID;
    report.ly = SWITCH_JOYSTICK_MID;
    report.rx = SWITCH_JOYSTICK_MID;
    report.ry = SWITCH_JOYSTICK_MID;

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

    // set timer for workload
    btstack_run_loop_set_timer_handler(&work_timer, work_timer_handler);
    btstack_run_loop_set_timer(&work_timer, BTSTACK_WORK_TIMER_MS);
    btstack_run_loop_add_timer(&work_timer);

    // uni_main(0, NULL);
    multicore_launch_core1(uni_main_core);
}
