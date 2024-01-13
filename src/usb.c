#include "usb.h"
#include <tusb.h>
#include <stdint.h>
#include <stdbool.h>

#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"
#include "pico/async_context.h"

void
send_switch_hid_report(SwitchOutGeneralReport report)
{
	tud_task();
	if (tud_suspended()) {
		tud_remote_wakeup();
	}

	for (int i = 0; i < CONFIG_BLUEPAD32_MAX_DEVICES; i++) {
		if (tud_hid_n_ready(i)) {
			tud_hid_n_report(i,
			                 0,
			                 &report.gamepad[i],
			                 sizeof(report.gamepad[i]));
		}
	}
}

void
init_usb()
{
	tusb_init();
}

bool
usb_ready()
{
	tud_task();
	return tud_ready();
}
