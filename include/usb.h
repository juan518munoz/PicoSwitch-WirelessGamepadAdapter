#ifndef _USB_H_
#define _USB_H_

#include "sdkconfig.h"
#include "SwitchDescriptors.h"

// move to SwitchDescriptors.h ?
typedef struct {
    uint8_t idx;
    SwitchOutReport report;
} SwitchIdxOutReport;

// serialize report to two uint32_t
typedef struct {
    uint32_t low;
    uint32_t high;
} SwitchOutReportSerialized;

void send_switch_hid_report(int idx, SwitchOutReport report);
SwitchOutReportSerialized serialize_report(SwitchIdxOutReport idx_r);
SwitchIdxOutReport deserialize_report(SwitchOutReportSerialized serialized);
void usb_core_task();

#endif