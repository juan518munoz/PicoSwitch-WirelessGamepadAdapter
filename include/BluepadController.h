#ifndef BluepadController_h
#define BluepadController_h

#include <string.h>
#include "hardware/pio.h"
#include "SwitchDescriptors.h"
#include "BluepadController.pio.h"

// This masks might be different for the bluepad32
// BP First Byte
#define BP_MASK_A (0x1)
#define BP_MASK_B (0x1 << 1)
#define BP_MASK_X (0x1 << 2)
#define BP_MASK_Y (0x1 << 3)
#define BP_MASK_START (0x1 << 4)

// BP Second Byte
#define BP_MASK_DPAD (0xF)
#define BP_MASK_Z (0x1 << 4)
#define BP_MASK_R (0x1 << 5)
#define BP_MASK_L (0x1 << 6)
#define BP_MASK_DPAD_UP 0x8
#define BP_MASK_DPAD_UPRIGHT 0xA
#define BP_MASK_DPAD_RIGHT 0x2
#define BP_MASK_DPAD_DOWNRIGHT 0x6
#define BP_MASK_DPAD_DOWN 0x4
#define BP_MASK_DPAD_DOWNLEFT 0x5
#define BP_MASK_DPAD_LEFT 0x1
#define BP_MASK_DPAD_UPLEFT 0x9

class BluepadController
{
private:
    // uint8_t getScaledAnalogAxis(uint8_t axisPos, uint8_t *minAxis, uint8_t *maxAxis);
    // this values might be different for the bluepad32
    uint8_t _maxLeftAnalogX = 0xC0;
    uint8_t _minLeftAnalogX = 0x40;
    uint8_t _maxLeftAnalogY = 0xC0;
    uint8_t _minLeftAnalogY = 0x40;
    uint8_t _maxRightAnalogX = 0xC0;
    uint8_t _minRightAnalogX = 0x40;
    uint8_t _maxRightAnalogY = 0xC0;
    uint8_t _minRightAnalogY = 0x40;

public:
    BluepadController(uint8_t pin)
    {
        _pin = pin;
        _stateBytes = 10;
        _controllerState = new uint8_t[10];
    }
    void emptyRxFifo()
    {
        while (pio_sm_get_rx_fifo_level(_pio, _sm))
        {
            pio_sm_get(_pio, _sm);
        }
    }
    void updatePioOutputSize(uint8_t autoPullLength)
    {
        // Pull mask = 0x3E000000
        // Push mask = 0x01F00000
        pio_sm_set_enabled(_pio, _sm, false);
        _pio->sm[_sm].shiftctrl = (_pio->sm[_sm].shiftctrl & 0xA00FFFFF) | (0x8 << 20) | (((autoPullLength + 5) & 0x1F) << 25);
        // Restart the state machine to avoid 16 0 bits being auto-pulled
        _pio->ctrl |= 1 << (4 + _sm);
        pio_sm_set_enabled(_pio, _sm, true);
    }
    void sendData(uint8_t *request, uint8_t dataLength, uint8_t *response, uint8_t responseLength)
    {
        uint32_t dataWithResponseLength = ((responseLength - 1) & 0x1F) << 27;
        for (int i = 0; i < dataLength; i++)
        {
            dataWithResponseLength |= *(request + i) << (19 - i * 8);
        }
        pio_sm_put_blocking(_pio, _sm, dataWithResponseLength);

        int16_t remainingBytes = responseLength;
        while (remainingBytes > 0)
        {
            absolute_time_t timeout_us = make_timeout_time_us(600);
            bool timedOut = false;
            while (pio_sm_is_rx_fifo_empty(_pio, _sm) && !timedOut)
            {
                timedOut = time_reached(timeout_us);
            }
            if (timedOut)
            {
                // throw 0;
                return;
            }
            uint32_t data = pio_sm_get(_pio, _sm);
            response[responseLength - remainingBytes] = (uint8_t)(data & 0xFF); // & (0xFF << i * 8);
            remainingBytes--;
        }
    }
    void init()
    {
        _pio = pio0;
        _offset = pio_add_program(_pio, &controller_program);
        _sm = pio_claim_unused_sm(_pio, true);
        pio_sm_config tmpConfig = controller_program_get_default_config(_offset);
        _c = &tmpConfig;
        controller_program_init(_pio, _sm, _offset, _pin, _c);
        uint8_t data[1] = {0x00};
        uint8_t response[3];
        sendData(data, 1, response, 3);
        sleep_us(200);
        updatePioOutputSize(24);
    };
    void updateState()
    {
        uint8_t data[3] = {0x40, 0x03, 0x0};
        sendData(data, 3, _controllerState, 8);
        sleep_us(500);
    }
    SwitchReport *getSwitchReport()
    {
        _switchReport.hat = SWITCH_HAT_NOTHING;
        _switchReport.rx = SWITCH_JOYSTICK_MID;
        _switchReport.ry = SWITCH_JOYSTICK_MID;
        _switchReport.lx = SWITCH_JOYSTICK_MID;
        _switchReport.ly = SWITCH_JOYSTICK_MID;

        _switchReport.buttons =
            (BP_MASK_START & _controllerState[0] ? SWITCH_MASK_PLUS : 0) |
            (BP_MASK_Y & _controllerState[0] ? SWITCH_MASK_Y : 0) |
            (BP_MASK_X & _controllerState[0] ? SWITCH_MASK_X : 0) |
            (BP_MASK_B & _controllerState[0] ? SWITCH_MASK_B : 0) |
            (BP_MASK_A & _controllerState[0] ? SWITCH_MASK_A : 0) |
            (BP_MASK_L & _controllerState[1] ? SWITCH_MASK_ZL : 0) |
            (BP_MASK_R & _controllerState[1] ? SWITCH_MASK_R : 0) |
            (!(BP_MASK_R & _controllerState[1]) && _controllerState[7] > 0x30 ? SWITCH_MASK_ZR : 0) |
            (!(BP_MASK_L & _controllerState[1]) && _controllerState[6] > 0x30 ? SWITCH_MASK_ZL : 0) |
            (BP_MASK_Z & _controllerState[1] ? SWITCH_MASK_L : 0) |
            ((BP_MASK_L & _controllerState[1]) && (BP_MASK_R & _controllerState[1]) && (BP_MASK_START & _controllerState[0]) ? SWITCH_MASK_HOME : 0);

        switch (BP_MASK_DPAD & _controllerState[1])
        {
        case BP_MASK_DPAD_UP:
            _switchReport.hat = SWITCH_HAT_UP;
            break;
        case BP_MASK_DPAD_UPRIGHT:
            _switchReport.hat = SWITCH_HAT_UPRIGHT;
            break;
        case BP_MASK_DPAD_RIGHT:
            _switchReport.hat = SWITCH_HAT_RIGHT;
            break;
        case BP_MASK_DPAD_DOWNRIGHT:
            _switchReport.hat = SWITCH_HAT_DOWNRIGHT;
            break;
        case BP_MASK_DPAD_DOWN:
            _switchReport.hat = SWITCH_HAT_DOWN;
            break;
        case BP_MASK_DPAD_DOWNLEFT:
            _switchReport.hat = SWITCH_HAT_DOWNLEFT;
            break;
        case BP_MASK_DPAD_LEFT:
            _switchReport.hat = SWITCH_HAT_LEFT;
            break;
        case BP_MASK_DPAD_UPLEFT:
            _switchReport.hat = SWITCH_HAT_UPLEFT;
            break;
        }

        // Scale for joystick insensitivity if needed https://GCsquid.com/GC-joystick-360-degrees/
        // GC Y axis is inverted relative to Switch
        _switchReport.lx = getScaledAnalogAxis(_controllerState[2], &_minLeftAnalogX, &_maxLeftAnalogX);
        _switchReport.ly = getScaledAnalogAxis((1 - _controllerState[3] / 255.) * 255., &_minLeftAnalogY, &_maxLeftAnalogY);
        _switchReport.rx = getScaledAnalogAxis(_controllerState[4], &_minRightAnalogX, &_maxRightAnalogX);
        _switchReport.ry = getScaledAnalogAxis((1 - _controllerState[5] / 255.) * 255., &_minRightAnalogY, &_maxRightAnalogY);

        return &_switchReport;
    }
    uint8_t getScaledAnalogAxis(uint8_t axisPos, uint8_t *minAxis, uint8_t *maxAxis)
    {
        *maxAxis = axisPos > *maxAxis ? axisPos : *maxAxis;
        *minAxis = axisPos < *minAxis ? axisPos : *minAxis;
        return axisPos > 0x80 ? SWITCH_JOYSTICK_MID - 1 + (axisPos - SWITCH_JOYSTICK_MID) / (float)(*maxAxis - SWITCH_JOYSTICK_MID) * (float)SWITCH_JOYSTICK_MID : SWITCH_JOYSTICK_MID - (SWITCH_JOYSTICK_MID - axisPos) / (float)(SWITCH_JOYSTICK_MID - *minAxis) * (float)SWITCH_JOYSTICK_MID;
    }

protected:
    uint8_t _pin;
    PIO _pio;
    uint _sm;
    pio_sm_config *_c;
    uint _offset;
    uint8_t _stateBytes;
    uint8_t *_controllerState;
    SwitchReport _switchReport{
        .buttons = 0,
        .hat = SWITCH_HAT_NOTHING,
        .lx = SWITCH_JOYSTICK_MID,
        .ly = SWITCH_JOYSTICK_MID,
        .rx = SWITCH_JOYSTICK_MID,
        .ry = SWITCH_JOYSTICK_MID,
        .vendor = 0,
    };
};

#endif

// OLD: REMOVE ONCE BLUEPAD32 IS WORKING
// #ifndef BluepadController_h
// #define BluepadController_h

// #include <string.h>
// #include "OtherController.h"

// class BluepadController: public OtherController {
//     public:
//         BluepadController(uint8_t pin): OtherController(pin, 10) {};
//         void init();
//         void updateState();
//         SwitchReport *getSwitchReport();
//     private:
//         uint8_t getScaledAnalogAxis(uint8_t axisPos, uint8_t *minAxis, uint8_t *maxAxis);
//         // this values might be different for the bluepad32
//         uint8_t _maxLeftAnalogX = 0xC0;
// 		uint8_t _minLeftAnalogX = 0x40;
// 		uint8_t _maxLeftAnalogY = 0xC0;
// 		uint8_t _minLeftAnalogY = 0x40;
//         uint8_t _maxRightAnalogX = 0xC0;
// 		uint8_t _minRightAnalogX = 0x40;
// 		uint8_t _maxRightAnalogY = 0xC0;
// 		uint8_t _minRightAnalogY = 0x40;
//         bool _isOneToOne = true; // one to one mapping to switch controller
//                                  // remove it asap, we always want one to one
// };

// // This masks might be different for the bluepad32
// // BP First Byte
// #define BP_MASK_A      (0x1)
// #define BP_MASK_B      (0x1 << 1)
// #define BP_MASK_X      (0x1 << 2)
// #define BP_MASK_Y      (0x1 << 3)
// #define BP_MASK_START  (0x1 << 4)

// // BP Second Byte
// #define BP_MASK_DPAD   (0xF)
// #define BP_MASK_Z      (0x1 << 4)
// #define BP_MASK_R      (0x1 << 5)
// #define BP_MASK_L      (0x1 << 6)
// #define BP_MASK_DPAD_UP        0x8
// #define BP_MASK_DPAD_UPRIGHT   0xA
// #define BP_MASK_DPAD_RIGHT     0x2
// #define BP_MASK_DPAD_DOWNRIGHT 0x6
// #define BP_MASK_DPAD_DOWN      0x4
// #define BP_MASK_DPAD_DOWNLEFT  0x5
// #define BP_MASK_DPAD_LEFT      0x1
// #define BP_MASK_DPAD_UPLEFT    0x9

// #endif