#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"

#include "Bluepad32.h"
#include "uni_main.h"

#include "SwitchDescriptors.h"
// #include "bsp/board.h"
#include "tusb_handler.h" // needed bc tusb.h has conflicts with btstack.h

#include <memory>
#include <stdio.h>

#include "hardware/pio.h"
#include "hardware/clocks.h"

static btstack_timer_source_t work_timer;

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

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
        // tusb_handler_tud_task(); // hangs program
        // if (!tusb_handler_tud_hid_ready())
        //     return;

        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, myGamepad->y() ? 1 : 0);

        SwitchOutReport out_report;
        out_report.buttons = 0;
        out_report.hat = SWITCH_HAT_NOTHING;
        out_report.lx = SWITCH_JOYSTICK_MID;
        out_report.ly = SWITCH_JOYSTICK_MID;
        out_report.rx = SWITCH_JOYSTICK_MID;
        out_report.ry = SWITCH_JOYSTICK_MID;
        // tusb_handler_tud_hid_report(&out_report, sizeof(out_report));
    }

    // set timer for next tick
    btstack_run_loop_set_timer(&work_timer, 100);
    btstack_run_loop_add_timer(&work_timer);
}

int main()
{
    stdio_init_all();
    tusb_handler_init();

    // initialize CYW43 driver architecture (will enable BT if/because CYW43_ENABLE_BLUETOOTH == 1)
    if (cyw43_arch_init())
    {
        printf("failed to initialise cyw43_arch\n");
        return -1;
    }

    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

    // set timer for workload
    btstack_run_loop_set_timer_handler(&work_timer, work_timer_handler);
    btstack_run_loop_set_timer(&work_timer, 100);
    btstack_run_loop_add_timer(&work_timer);

    uni_main(0, NULL);

    return 0;
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