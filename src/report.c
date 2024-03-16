#include "report.h"

#include <stdbool.h>

#include <pico/multicore.h>
#include <pico/async_context.h>
#include <pico/cyw43_arch.h>
#include <memory.h>
#include <uni.h>

#include "usb.h"

// used between threads
SwitchIdxOutReport shared_report;

void set_global_gamepad_report(SwitchIdxOutReport *src) {
    if (!src) {
        return;
    }
    logi("BLUEPAD: Setting report %d\n", src->idx);

    async_context_t *context = cyw43_arch_async_context();
    async_context_acquire_lock_blocking(context);
    memcpy(&shared_report, src, sizeof(shared_report));
    async_context_release_lock(context);
}

void get_global_gamepad_report(SwitchIdxOutReport *dest) {
    // logi("USB: Getting report %d\n", shared_report.idx);
    async_context_t *context = cyw43_arch_async_context();
    async_context_acquire_lock_blocking(context);
    memcpy(dest, &shared_report, sizeof(*dest));
    async_context_release_lock(context);
}