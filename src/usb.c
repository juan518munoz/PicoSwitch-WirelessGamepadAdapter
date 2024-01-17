#include "usb.h"
#include <tusb.h>
#include <stdint.h>
#include <stdbool.h>

#include <pico/stdlib.h>
#include <pico/cyw43_arch.h>
#include <pico/multicore.h>
#include <pico/async_context.h>

SwitchOutReportSerialized
serialize_report(SwitchIdxOutReport idx_r)
{
	SwitchOutReportSerialized serialized;
	serialized.low = 0;
	serialized.low |= idx_r.idx;
	serialized.low |= idx_r.report.buttons << 8;
	serialized.low |= idx_r.report.hat << 24;
	serialized.high = 0;
	serialized.high |= idx_r.report.lx;
	serialized.high |= idx_r.report.ly << 8;
	serialized.high |= idx_r.report.rx << 16;
	serialized.high |= idx_r.report.ry << 24;
	return serialized;
}

SwitchIdxOutReport
deserialize_report(SwitchOutReportSerialized serialized)
{
	SwitchIdxOutReport idx_r;
	idx_r.idx = serialized.low & 0xff;
	idx_r.report.buttons = (serialized.low >> 8) & 0xff;
	idx_r.report.hat = (serialized.low >> 24) & 0xff;
	idx_r.report.lx = serialized.high & 0xff;
	idx_r.report.ly = (serialized.high >> 8) & 0xff;
	idx_r.report.rx = (serialized.high >> 16) & 0xff;
	idx_r.report.ry = (serialized.high >> 24) & 0xff;
	return idx_r;
}

// add a function to verify that a report is "valid"

void
usb_core_task()
{
	tusb_init();

	SwitchOutReportSerialized serialized;
	SwitchIdxOutReport r;
	r.idx = 0;
	r.report.buttons = 0;
	r.report.hat = 0;
	r.report.lx = 0;
	r.report.ly = 0;
	r.report.rx = 0;
	r.report.ry = 0;

	while (1) {
		// low and high should have different magic numbers, in case we get a partial report
		// we can detect it and clear the poisoned FIFO
		serialized.low = multicore_fifo_pop_blocking();
		serialized.high = multicore_fifo_pop_blocking();
		r = deserialize_report(serialized);

		tud_task();
		if (tud_suspended()) {
			tud_remote_wakeup();
			continue;
		}

		if (tud_hid_n_ready(r.idx)) {
			tud_hid_n_report(r.idx, 0, &r.report, sizeof(r.report));
		}
		sleep_ms(10);
	}
}
