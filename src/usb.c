#include "usb.h"

#include <tusb.h>
#include <stdint.h>
#include <stdbool.h>

#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <pico/multicore.h>
#include <pico/async_context.h>

#include "report.h"
#include "SwitchDescriptors.h"

void
usb_core_task()
{
	tusb_init();

	SwitchIdxOutReport r;
	r.idx = 0;
	r.report.buttons = 0;
	r.report.hat = SWITCH_HAT_NOTHING;
	r.report.lx = 0;
	r.report.ly = 0;
	r.report.rx = 0;
	r.report.ry = 0;

	// send empty reports while bluepad32 is still not set
	while (multicore_fifo_get_status() & 1 == 0) {
		if (tud_hid_n_ready(r.idx)) {
			tud_hid_n_report(r.idx, 0, &r.report, sizeof(r.report));
		}
		sleep_ms(100);
	}

	while (1) {
		get_global_gamepad_report(&r);

		tud_task();
		if (tud_suspended()) {
			tud_remote_wakeup();
			continue;
		}

		if (tud_hid_n_ready(r.idx)) {
			tud_hid_n_report(r.idx, 0, &r.report, sizeof(r.report));
		}
		sleep_ms(5);  // we need to sleep here to avoid starving bluepad core
	}
}
