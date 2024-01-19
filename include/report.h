#ifndef _REPORT_H_
#define _REPORT_H_

#include "usb.h"
#include "SwitchDescriptors.h"

void set_global_gamepad_report(SwitchIdxOutReport *rpt);
void get_global_gamepad_report(SwitchIdxOutReport *rpt);

#endif