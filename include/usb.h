#ifndef _USB_H_
#define _USB_H_

#include "sdkconfig.h"
#include <stdint.h>

typedef struct
{
	uint16_t buttons;
	uint8_t hat;
	uint8_t lx;
	uint8_t ly;
	uint8_t rx;
	uint8_t ry;
} SwitchOutReport;

typedef struct {
    uint8_t idx;
    SwitchOutReport report;
} SwitchIdxOutReport;

void usb_core_task();

#endif