#include "report.h"

#include <stdbool.h>

#include <pico/multicore.h>
#include <pico/async_context.h>
#include <pico/cyw43_arch.h>
#include <memory.h>

#include "usb.h"
#include "SwitchDescriptors.h"

// used between threads
SwitchIdxOutReport shared_report;

void set_global_gamepad_report(SwitchIdxOutReport *src) {
    if (!src) {
        return;
    }

    async_context_t *context = cyw43_arch_async_context();
    async_context_acquire_lock_blocking(context);
    memcpy(&shared_report, src, sizeof(shared_report));
    multicore_fifo_push_blocking(0);
    async_context_release_lock(context);
}

void get_global_gamepad_report(SwitchIdxOutReport *dest) {
    multicore_fifo_pop_blocking();
    async_context_t *context = cyw43_arch_async_context();
    async_context_acquire_lock_blocking(context);
    memcpy(dest, &shared_report, sizeof(*dest));
    async_context_release_lock(context);
}