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
#define AXIS_DEADZONE 0xa

static btstack_timer_source_t work_timer;

GamepadPtr myGamepads[BP32_MAX_GAMEPADS];

SwitchOutGeneralReport report;
void get_button_state(SwitchOutGeneralReport *rpt)
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
    else if ((bluepadAxis > (SWITCH_JOYSTICK_MID - AXIS_DEADZONE)) && (bluepadAxis < (SWITCH_JOYSTICK_MID + AXIS_DEADZONE)))
    {
        bluepadAxis = SWITCH_JOYSTICK_MID;
    }
    else if (bluepadAxis > SWITCH_JOYSTICK_MAX)
        bluepadAxis = SWITCH_JOYSTICK_MAX;

    return (uint8_t)bluepadAxis;
}

void fill_report(GamepadPtr myGamepad, int index)
{

    uint16_t buttons = myGamepad->buttons();
    uint16_t buttons_aux = buttons; // keep initial state

    // remove wrong masking
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
    report.gamepad[index].buttons = buttons;

    report.gamepad[index].lx = convert_to_switch_axis(myGamepad->axisX());
    report.gamepad[index].ly = convert_to_switch_axis(myGamepad->axisY());
    report.gamepad[index].rx = convert_to_switch_axis(myGamepad->axisRX());
    report.gamepad[index].ry = convert_to_switch_axis(myGamepad->axisRY());

    if (myGamepad->brake() > 0)
    {
        report.gamepad[index].buttons |= SWITCH_MASK_ZL;
    }
    if (myGamepad->throttle() > 0)
    {
        report.gamepad[index].buttons |= SWITCH_MASK_ZR;
    }
    if (myGamepad->miscSystem())
    {
        report.gamepad[index].buttons |= SWITCH_MASK_HOME;
    }
    if (myGamepad->miscCapture())
    {
        report.gamepad[index].buttons |= SWITCH_MASK_CAPTURE;
    }
    if (myGamepad->miscBack())
    {
        report.gamepad[index].buttons |= SWITCH_MASK_MINUS;
    }
    if (myGamepad->miscHome())
    {
        report.gamepad[index].buttons |= SWITCH_MASK_PLUS;
    }

    switch (myGamepad->dpad())
    {
    case DPAD_UP:
        report.gamepad[index].hat = SWITCH_HAT_UP;
        break;
    case DPAD_DOWN:
        report.gamepad[index].hat = SWITCH_HAT_DOWN;
        break;
    case DPAD_LEFT:
        report.gamepad[index].hat = SWITCH_HAT_LEFT;
        break;
    case DPAD_RIGHT:
        report.gamepad[index].hat = SWITCH_HAT_RIGHT;
        break;
    case DPAD_UP | DPAD_RIGHT:
        report.gamepad[index].hat = SWITCH_HAT_UPRIGHT;
        break;
    case DPAD_DOWN | DPAD_RIGHT:
        report.gamepad[index].hat = SWITCH_HAT_DOWNRIGHT;
        break;
    case DPAD_DOWN | DPAD_LEFT:
        report.gamepad[index].hat = SWITCH_HAT_DOWNLEFT;
        break;
    case DPAD_UP | DPAD_LEFT:
        report.gamepad[index].hat = SWITCH_HAT_UPLEFT;
        break;
    default:
        report.gamepad[index].hat = SWITCH_HAT_NOTHING;
        break;
    }
}

static void work_timer_handler(btstack_timer_source_t *ts)
{
    BP32.update();

    uint8_t connected = 0;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++)
    {
        GamepadPtr myGamepad = myGamepads[i];
        if (myGamepad && myGamepad->isConnected())
        {
            fill_report(myGamepad, i);
            connected++;
        }
    }
    report.connected = connected;

    // set timer for next tick
    btstack_run_loop_set_timer(&work_timer, BTSTACK_WORK_TIMER_MS);
    btstack_run_loop_add_timer(&work_timer);
}

void init_report(void)
{
    report.connected = 0;
    for (int i = 0; i < BP32_MAX_GAMEPADS; i++)
    {
        report.gamepad[i].buttons = 0;
        report.gamepad[i].hat = SWITCH_HAT_NOTHING;
        report.gamepad[i].lx = SWITCH_JOYSTICK_MID;
        report.gamepad[i].ly = SWITCH_JOYSTICK_MID;
        report.gamepad[i].rx = SWITCH_JOYSTICK_MID;
        report.gamepad[i].ry = SWITCH_JOYSTICK_MID;
    }
}

void init_bluepad(void)
{
    // initialize CYW43 driver architecture (will enable BT if/because CYW43_ENABLE_BLUETOOTH == 1)
    if (cyw43_arch_init())
    {
        printf("failed to initialise cyw43_arch\n");
        return;
    }

    // Setup the Bluepad32 callbacks
    BP32.setup(&onConnectedGamepad, &onDisconnectedGamepad);

    // Set report to default values
    init_report();

    // set timer for workload
    btstack_run_loop_set_timer_handler(&work_timer, work_timer_handler);
    btstack_run_loop_set_timer(&work_timer, BTSTACK_WORK_TIMER_MS);
    btstack_run_loop_add_timer(&work_timer);

    // uni_main(0, NULL);
    multicore_launch_core1(uni_main_core);
}
