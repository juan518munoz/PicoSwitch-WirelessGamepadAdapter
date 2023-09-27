#ifndef BLUEPAD_MAIN_H
#define BLUEPAD_MAIN_H

// #include <stdint.h>
#include "SwitchDescriptors.h"

void init_bluepad(void);

void get_button_state(SwitchOutReport *rpt);

#endif // BLUEPAD_MAIN_H