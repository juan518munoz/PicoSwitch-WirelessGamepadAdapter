#include "usb.h"

#include <stdint.h>
#include <stdbool.h>
#include <uni.h>

#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <pico/multicore.h>
#include <pico/async_context.h>

#include "report.h"

void
usb_core_task()
{
	// tusb_init();

	SwitchIdxOutReport r;
	r.idx = 0;

	// send empty reports while bluepad32 is still not set
	while (multicore_fifo_get_status() & 1 == 0)
	{
		logi("USB: Waiting for bluepad32 to be set\n");
		sleep_ms(100);
	}
	
	while (1) {
		get_global_gamepad_report(&r);

		// tud_task();
		// if (tud_suspended()) {
		// 	tud_remote_wakeup();
		// 	continue;
		// }

		// if (tud_hid_n_ready(r.idx)) {
		// 	tud_hid_n_report(r.idx, 0, &r.report, sizeof(r.report));
		// }
		sleep_ms(1);
	}
}
