#ifndef _USB_H_
#define _USB_H_

#include "sdkconfig.h"
#include "SwitchDescriptors.h"
#include <stdbool.h>

typedef struct
{
    uint8_t connected;
    SwitchOutReport gamepad[CONFIG_BLUEPAD32_MAX_DEVICES];
} SwitchOutGeneralReport;

void init_usb();
void send_switch_hid_report(SwitchOutGeneralReport report);
bool usb_ready();

#endif