#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "btstack.h"

#include "Bluepad32.h"
#include "uni_main.h"

// #include "BluepadController.h"
#include "SwitchDescriptors.h"
#include "tusb_handler.h" // needed bc tusb.h has conflicts with btstack.h
#include "Controller.pio.h"

#include <memory>
#include <stdio.h>

#include "hardware/pio.h"
#include "hardware/clocks.h"

static btstack_timer_source_t work_timer;
SwitchReport _switchReport{
    .buttons = 0,
    .hat = SWITCH_HAT_NOTHING,
    .lx = SWITCH_JOYSTICK_MID,
    .ly = SWITCH_JOYSTICK_MID,
    .rx = SWITCH_JOYSTICK_MID,
    .ry = SWITCH_JOYSTICK_MID,
    .vendor = 0,
};

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

// BluepadController *bluepadController;
SwitchDescriptors switchController;

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
        try
        {
            tusb_handler_tud_task();
            if (tusb_handler_tud_suspended())
            {
                tusb_handler_tud_remote_wakeup();
            }
            if (tusb_handler_tud_hid_ready())
            {
                cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, myGamepad->y() ? 1 : 0);

                // _switchReport.buttons |= SWITCH_MASK_L;
                // _switchReport.buttons |= SWITCH_MASK_R;

                tusb_handler_tud_hid_report(&_switchReport, sizeof(SwitchReport));
            }
        }
        catch (int e)
        {
            tusb_handler_tud_task();
            if (tusb_handler_tud_suspended())
            {
                tusb_handler_tud_remote_wakeup();
            }
        }
    }

    // set timer for next tick
    btstack_run_loop_set_timer(&work_timer, 100);
    btstack_run_loop_add_timer(&work_timer);
}

void updatePioOutputSize(uint8_t autoPullLength, PIO _pio, uint _sm)
{
    // Pull mask = 0x3E000000
    // Push mask = 0x01F00000
    pio_sm_set_enabled(_pio, _sm, false);
    _pio->sm[_sm].shiftctrl = (_pio->sm[_sm].shiftctrl & 0xA00FFFFF) | (0x8 << 20) | (((autoPullLength + 5) & 0x1F) << 25);
    // Restart the state machine to avoid 16 0 bits being auto-pulled
    _pio->ctrl |= 1 << (4 + _sm);
    pio_sm_set_enabled(_pio, _sm, true);
}

void sendData(uint8_t *request, uint8_t dataLength, uint8_t *response, uint8_t responseLength, PIO _pio, uint _sm)
{
    uint32_t dataWithResponseLength = ((responseLength - 1) & 0x1F) << 27;
    for (int i = 0; i < dataLength; i++)
    {
        dataWithResponseLength |= *(request + i) << (19 - i * 8);
    }
    pio_sm_put_blocking(_pio, _sm, dataWithResponseLength);

    int16_t remainingBytes = responseLength;
    while (remainingBytes > 0)
    {
        absolute_time_t timeout_us = make_timeout_time_us(600);
        bool timedOut = false;
        while (pio_sm_is_rx_fifo_empty(_pio, _sm) && !timedOut)
        {
            timedOut = time_reached(timeout_us);
        }
        if (timedOut)
        {
            throw 0;
        }
        uint32_t data = pio_sm_get(_pio, _sm);
        response[responseLength - remainingBytes] = (uint8_t)(data & 0xFF); // & (0xFF << i * 8);
        remainingBytes--;
    }
}

void init_pio()
{
    // Init Params
    uint8_t pin = 18;
    PIO pio;
    uint sm;
    pio_sm_config *c;
    uint offset;
}

int main()
{
    stdio_init_all();
    init_pio();
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