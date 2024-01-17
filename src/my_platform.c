// Example file - Public Domain
// Need help? https://tinyurl.com/bluepad32-help

#include <stdio.h>
#include <string.h>

#include <pico/cyw43_arch.h>
#include <pico/multicore.h>
#include <pico/async_context.h>

#include "sdkconfig.h"
#include "uni_bt.h"
#include "uni_gamepad.h"
#include "uni_hid_device.h"
#include "uni_log.h"
#include "uni_platform.h"
#include "usb.h"

// Sanity check
#ifndef CONFIG_BLUEPAD32_PLATFORM_CUSTOM
#error "Pico W must use BLUEPAD32_PLATFORM_CUSTOM"
#endif

#define AXIS_DEADZONE 0xa

// Declarations
static void trigger_event_on_gamepad(uni_hid_device_t *d);
SwitchOutReport report[CONFIG_BLUEPAD32_MAX_DEVICES];
SwitchIdxOutReport idx_r;
SwitchOutReportSerialized serialized;

// Helper functions
static void
empty_gamepad_report(SwitchOutReport *gamepad)
{
	gamepad->buttons = 0;
	gamepad->hat = SWITCH_HAT_NOTHING;
	gamepad->lx = SWITCH_JOYSTICK_MID;
	gamepad->ly = SWITCH_JOYSTICK_MID;
	gamepad->rx = SWITCH_JOYSTICK_MID;
	gamepad->ry = SWITCH_JOYSTICK_MID;
}

uint8_t
convert_to_switch_axis(int32_t bluepadAxis)
{
	// bluepad32 reports from -512 to 511 as int32_t
	// switch reports from 0 to 255 as uint8_t

	bluepadAxis += 513;  // now max possible is 1024
	bluepadAxis /= 4;    // now max possible is 255

	if (bluepadAxis < SWITCH_JOYSTICK_MIN)
		bluepadAxis = 0;
	else if ((bluepadAxis > (SWITCH_JOYSTICK_MID - AXIS_DEADZONE)) &&
	         (bluepadAxis < (SWITCH_JOYSTICK_MID + AXIS_DEADZONE))) {
		bluepadAxis = SWITCH_JOYSTICK_MID;
	} else if (bluepadAxis > SWITCH_JOYSTICK_MAX)
		bluepadAxis = SWITCH_JOYSTICK_MAX;

	return (uint8_t) bluepadAxis;
}

static void
fill_gamepad_report(int idx, uni_gamepad_t *gp)
{
	empty_gamepad_report(&report[idx]);

	// face buttons
	if ((gp->buttons & BUTTON_A)) {
		report[idx].buttons |= SWITCH_MASK_A;
	}
	if ((gp->buttons & BUTTON_B)) {
		report[idx].buttons |= SWITCH_MASK_B;
	}
	if ((gp->buttons & BUTTON_X)) {
		report[idx].buttons |= SWITCH_MASK_X;
	}
	if ((gp->buttons & BUTTON_Y)) {
		report[idx].buttons |= SWITCH_MASK_Y;
	}

	// shoulder buttons
	if ((gp->buttons & BUTTON_SHOULDER_L)) {
		report[idx].buttons |= SWITCH_MASK_L;
	}
	if ((gp->buttons & BUTTON_SHOULDER_R)) {
		report[idx].buttons |= SWITCH_MASK_R;
	}

	// dpad
	switch (gp->dpad) {
	case DPAD_UP:
		report[idx].hat = SWITCH_HAT_UP;
		break;
	case DPAD_DOWN:
		report[idx].hat = SWITCH_HAT_DOWN;
		break;
	case DPAD_LEFT:
		report[idx].hat = SWITCH_HAT_LEFT;
		break;
	case DPAD_RIGHT:
		report[idx].hat = SWITCH_HAT_RIGHT;
		break;
	case DPAD_UP | DPAD_RIGHT:
		report[idx].hat = SWITCH_HAT_UPRIGHT;
		break;
	case DPAD_DOWN | DPAD_RIGHT:
		report[idx].hat = SWITCH_HAT_DOWNRIGHT;
		break;
	case DPAD_DOWN | DPAD_LEFT:
		report[idx].hat = SWITCH_HAT_DOWNLEFT;
		break;
	case DPAD_UP | DPAD_LEFT:
		report[idx].hat = SWITCH_HAT_UPLEFT;
		break;
	default:
		report[idx].hat = SWITCH_HAT_NOTHING;
		break;
	}

	// sticks
	report[idx].lx = convert_to_switch_axis(gp->axis_x);
	report[idx].ly = convert_to_switch_axis(gp->axis_y);
	report[idx].rx = convert_to_switch_axis(gp->axis_rx);
	report[idx].ry = convert_to_switch_axis(gp->axis_ry);
	if ((gp->buttons & BUTTON_THUMB_L))
		report[idx].buttons |= SWITCH_MASK_L3;
	if ((gp->buttons & BUTTON_THUMB_R))
		report[idx].buttons |= SWITCH_MASK_R3;

	// triggers
	if (gp->brake)
		report[idx].buttons |= SWITCH_MASK_ZL;
	if (gp->throttle)
		report[idx].buttons |= SWITCH_MASK_ZR;

	// misc buttons
	if (gp->misc_buttons & MISC_BUTTON_SYSTEM)
		report[idx].buttons |= SWITCH_MASK_HOME;
	if (gp->misc_buttons & MISC_BUTTON_CAPTURE)
		report[idx].buttons |= SWITCH_MASK_CAPTURE;
	if (gp->misc_buttons & MISC_BUTTON_BACK)
		report[idx].buttons |= SWITCH_MASK_MINUS;
	if (gp->misc_buttons & MISC_BUTTON_HOME)
		report[idx].buttons |= SWITCH_MASK_PLUS;
}

//
// Platform Overrides
//

static void my_platform_init(int argc, const char** argv) {
    ARG_UNUSED(argc);
    ARG_UNUSED(argv);

    logi("my_platform: init()\n");

	uni_gamepad_mappings_t mappings = GAMEPAD_DEFAULT_MAPPINGS;

	// remaps
	mappings.button_b = UNI_GAMEPAD_MAPPINGS_BUTTON_A;
	mappings.button_a = UNI_GAMEPAD_MAPPINGS_BUTTON_B;
	mappings.button_y = UNI_GAMEPAD_MAPPINGS_BUTTON_X;
	mappings.button_x = UNI_GAMEPAD_MAPPINGS_BUTTON_Y;

	uni_gamepad_set_mappings(&mappings);
}

static void my_platform_on_init_complete(void) {
    logi("my_platform: on_init_complete()\n");

    // Safe to call "unsafe" functions since they are called from BT thread

    // Start scanning
    uni_bt_enable_new_connections_unsafe(true);

    // Based on runtime condition you can delete or list the stored BT keys.
    if (1)
        uni_bt_del_keys_unsafe();
    else
        uni_bt_list_keys_unsafe();

    // Turn off LED once init is done.
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
}

static void my_platform_on_device_connected(uni_hid_device_t* d) {
    logi("my_platform: device connected: %p\n", d);
}

static void my_platform_on_device_disconnected(uni_hid_device_t* d) {
    logi("my_platform: device disconnected: %p\n", d);
}

static uni_error_t my_platform_on_device_ready(uni_hid_device_t* d) {
    logi("my_platform: device ready: %p\n", d);

    // You can reject the connection by returning an error.
    return UNI_ERROR_SUCCESS;
}

static void my_platform_on_controller_data(uni_hid_device_t* d, uni_controller_t* ctl)
{
	uni_gamepad_t *gp;
	uint8_t idx = uni_hid_device_get_idx_for_instance(d);

	// Print device Id before dumping gamepad.
	logi("(%p) ", d);
	uni_controller_dump(ctl);

	switch (ctl->klass) {
	case UNI_CONTROLLER_CLASS_GAMEPAD:
		gp = &ctl->gamepad;
		fill_gamepad_report(idx, gp);
		idx_r.idx = idx;
		idx_r.report = report[idx];

		serialized = serialize_report(idx_r);
		if (multicore_fifo_push_timeout_us(serialized.low, 10)) {
			if (!multicore_fifo_push_timeout_us(serialized.high, 10)) {
				// if we fail to send the second part, clear poisoned FIFO
				multicore_fifo_drain();
			}
		}
		break;
	case UNI_CONTROLLER_CLASS_BALANCE_BOARD:
		// Do something
		uni_balance_board_dump(&ctl->balance_board);
		break;
	case UNI_CONTROLLER_CLASS_MOUSE:
		// Do something
		uni_mouse_dump(&ctl->mouse);
		break;
	case UNI_CONTROLLER_CLASS_KEYBOARD:
		// Do something
		uni_keyboard_dump(&ctl->keyboard);
		break;
	default:
		loge("Unsupported controller class: %d\n", ctl->klass);
		break;
	}
}

static int32_t my_platform_get_property(uni_platform_property_t key) {
    // Deprecated
    ARG_UNUSED(key);
    return 0;
}

static void my_platform_on_oob_event(uni_platform_oob_event_t event, void* data) {
    switch (event) {
        case UNI_PLATFORM_OOB_GAMEPAD_SYSTEM_BUTTON:
            // Optional: do something when "system" button gets pressed.
            trigger_event_on_gamepad((uni_hid_device_t*)data);
            break;

        case UNI_PLATFORM_OOB_BLUETOOTH_ENABLED:
            // When the "bt scanning" is on / off. Could by triggered by different events
            // Useful to notify the user
            logi("my_platform_on_oob_event: Bluetooth enabled: %d\n", (bool)(data));
            break;

        default:
            logi("my_platform_on_oob_event: unsupported event: 0x%04x\n", event);
    }
}

//
// Helpers
//
static void trigger_event_on_gamepad(uni_hid_device_t* d) {
    if (d->report_parser.set_rumble != NULL) {
        d->report_parser.set_rumble(d, 0x80 /* value */, 15 /* duration */);
    }

    if (d->report_parser.set_player_leds != NULL) {
        static uint8_t led = 0;
        led += 1;
        led &= 0xf;
        d->report_parser.set_player_leds(d, led);
    }

    if (d->report_parser.set_lightbar_color != NULL) {
        static uint8_t red = 0x10;
        static uint8_t green = 0x20;
        static uint8_t blue = 0x40;

        red += 0x10;
        green -= 0x20;
        blue += 0x40;
        d->report_parser.set_lightbar_color(d, red, green, blue);
    }
}

//
// Entry Point
//
struct uni_platform* get_my_platform(void) {
    static struct uni_platform plat = {
        .name = "My Platform",
        .init = my_platform_init,
        .on_init_complete = my_platform_on_init_complete,
        .on_device_connected = my_platform_on_device_connected,
        .on_device_disconnected = my_platform_on_device_disconnected,
        .on_device_ready = my_platform_on_device_ready,
        .on_oob_event = my_platform_on_oob_event,
        .on_controller_data = my_platform_on_controller_data,
        .get_property = my_platform_get_property,
    };

    return &plat;
}