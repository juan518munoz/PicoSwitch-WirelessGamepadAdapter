#include "Bluepad_main.h"

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/async_context.h"

#include "btstack.h"
#include "Bluepad32.h"
#include "uni_main.h"

#include "SwitchDescriptors.h"

#include "pico/multicore.h"

#include <memory>
#include <stdio.h>

static btstack_timer_source_t work_timer;

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

// int conn;
uint16_t buttons;
void get_button_state(uint16_t *btns)
{
    async_context_t *context = cyw43_arch_async_context();
    async_context_acquire_lock_blocking(context);
    memcpy(btns, &buttons, sizeof(*btns));
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
        // pattern match conn on myGamepad->y()
        // if (myGamepad->y())
        //     conn = 1;
        // else
        //     conn = 0;
        buttons = myGamepad->buttons();
    }

    // set timer for next tick
    btstack_run_loop_set_timer(&work_timer, 100);
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
    buttons = 0;

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

    // set timer for workload
    btstack_run_loop_set_timer_handler(&work_timer, work_timer_handler);
    btstack_run_loop_set_timer(&work_timer, 100);
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