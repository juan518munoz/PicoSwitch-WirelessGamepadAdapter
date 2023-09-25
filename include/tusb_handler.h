#ifndef TUSB_HANDLER_H
#define TUSB_HANDLER_H

#include "SwitchDescriptors.h"

void tusb_handler_init();
void tusb_handler_tud_task();
bool tusb_handler_tud_suspended();
void tusb_handler_tud_remote_wakeup();
bool tusb_handler_tud_hid_ready();
void tusb_handler_tud_hid_report(SwitchOutReport *report, uint16_t report_size);

#endif // TUSB_HANDLER_H