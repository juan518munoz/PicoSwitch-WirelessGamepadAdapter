#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"

#include "Bluepad_main.h"
#include "SwitchDescriptors.h"

#include "bsp/board.h"
#include "tusb.h"


void send_switch_hid_report(SwitchOutGeneralReport report)
{
    tud_task();
    if (tud_suspended())
    {
        tud_remote_wakeup();
    }

    for (int i = 0; i < 4; i++) // BP32_MAX_GAMEPADS
    {
        if (tud_hid_n_ready(i))
        {
            tud_hid_n_report(i, 0, &report.gamepad[i], sizeof(report.gamepad[i]));
        }
    }
}

int main()
{
    stdio_init_all();

    init_bluepad();
    tusb_init();

    SwitchOutGeneralReport report;
    while (true)
    {
        get_button_state(&report);
        send_switch_hid_report(report);
    }

    return 0;
}
