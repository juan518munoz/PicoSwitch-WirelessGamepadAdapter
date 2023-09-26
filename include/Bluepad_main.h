#ifndef BLUEPAD_MAIN_H
#define BLUEPAD_MAIN_H

#include <stdint.h>

void init_bluepad(void);

void get_button_state(uint16_t *btns);

#endif // BLUEPAD_MAIN_H