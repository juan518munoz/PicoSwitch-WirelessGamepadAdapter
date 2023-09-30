#ifndef BLUEPAD_MAIN_H
#define BLUEPAD_MAIN_H

#include "SwitchDescriptors.h"

typedef struct
{
    uint8_t connected;
    SwitchOutReport gamepad[4]; // BP32_MAX_GAMEPADS
} SwitchOutGeneralReport;

void init_bluepad(void);

void get_button_state(SwitchOutGeneralReport *rpt);

#endif // BLUEPAD_MAIN_H