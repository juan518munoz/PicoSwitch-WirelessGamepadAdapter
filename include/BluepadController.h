#ifndef BluepadController_h
#define BluepadController_h

#include <string.h>
#include "Controller.h"

class BluepadController: public Controller {
    public:
        BluepadController(uint8_t pin): Controller(pin, 10) {};
        void init();
        void updateState();
        SwitchReport *getSwitchReport();
    private:
}

// define masks here!

#endif