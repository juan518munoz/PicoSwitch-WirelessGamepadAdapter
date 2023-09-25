#include "tusb_handler.h"

#include "tusb.h"
#include "SwitchDescriptors.h"

void tusb_handler_init()
{
    tusb_init();
}

void tusb_handler_tud_task()
{
    tud_task();
}

bool tusb_handler_tud_suspended()
{
    return tud_suspended();
}

void tusb_handler_tud_remote_wakeup()
{
    tud_remote_wakeup();
}

bool tusb_handler_tud_hid_ready()
{
    return tud_hid_ready();
}

void tusb_handler_tud_hid_report(SwitchOutReport *report, uint16_t report_size)
{
    tud_hid_report(0, report, report_size);
}
