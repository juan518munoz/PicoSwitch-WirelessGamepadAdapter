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

static void work_timer_handler(btstack_timer_source_t *ts)
{
    BP32.update();

    GamepadPtr myGamepad = myGamepads[0];
    if (myGamepad && myGamepad->isConnected())
    {

        report.buttons = myGamepad->buttons();
        // THIS MAY CAUSE UNMASKING ON MULTIPLE BUTTON PRESSES
        // ADD FLAGS?
        // current xbox X needs to be mapped to Y
        if (report.buttons & SWITCH_MASK_A)
        {
            report.buttons &= ~SWITCH_MASK_A;
            report.buttons |= SWITCH_MASK_Y;
        }
        // current xbox B needs to be mapped to A
        else if (report.buttons & SWITCH_MASK_B)
        {
            report.buttons &= ~SWITCH_MASK_B;
            report.buttons |= SWITCH_MASK_A;
        }
        // current xbox A needs to be mapped to B
        else if (report.buttons & SWITCH_MASK_Y)
        {
            report.buttons &= ~SWITCH_MASK_Y;
            report.buttons |= SWITCH_MASK_B;
        }

        // current xbox Y is fine

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
    btstack_run_loop_set_timer(&work_timer, 30);
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
    btstack_run_loop_set_timer(&work_timer, 30);
    btstack_run_loop_add_timer(&work_timer);

    // uni_main(0, NULL);
    multicore_launch_core1(uni_main_core);
}

// // Another way to query the buttons, is by calling buttons(), or
// // miscButtons() which return a bitmask.
// // Some gamepads also have DPAD, axis and more.
// printf(
//     "idx=%d, dpad: 0x%02x, buttons: 0x%04x, axis L: %4d, %4d, axis R: %4d, "
//     "%4d, brake: %4d, throttle: %4d, misc: 0x%02x\n",
//     i,                       // Gamepad Index
//     myGamepad->dpad(),       // DPAD
//     myGamepad->buttons(),    // bitmask of pressed buttons
//     myGamepad->axisX(),      // (-511 - 512) left X Axis
//     myGamepad->axisY(),      // (-511 - 512) left Y axis
//     myGamepad->axisRX(),     // (-511 - 512) right X axis
//     myGamepad->axisRY(),     // (-511 - 512) right Y axis
//     myGamepad->brake(),      // (0 - 1023): brake button
//     myGamepad->throttle(),   // (0 - 1023): throttle (AKA gas) button
//     myGamepad->miscButtons() // bitmak of pressed "misc" buttons
// );