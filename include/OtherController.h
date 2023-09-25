#ifndef Controller_h
#define Controller_h

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "SwitchDescriptors.h"

class OtherController
{
public:
    OtherController(uint8_t pin, uint8_t stateBytes)
    {
        _pin = pin;
        _stateBytes = stateBytes;
        _controllerState = new uint8_t[_stateBytes];
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
                throw 0;
            }
            uint32_t data = pio_sm_get(_pio, _sm);
            response[responseLength - remainingBytes] = (uint8_t)(data & 0xFF); // & (0xFF << i * 8);
            remainingBytes--;
        }
    }
    void init();
    void updateState();
    SwitchReport *getSwitchReport();

private:
    uint8_t getScaledAnalogAxis(uint8_t axisPos, uint8_t *minAxis, uint8_t *maxAxis);
    uint8_t _maxAnalogX = 0xC0;
    uint8_t _minAnalogX = 0x40;
    uint8_t _maxAnalogY = 0xC0;
    uint8_t _minAnalogY = 0x40;
    uint8_t _maxCX = 0xC0;
    uint8_t _minCX = 0x40;
    uint8_t _maxCY = 0xC0;
    uint8_t _minCY = 0x40;
    // bool _isOneToOne = strcmp(gcControllerType, "oneToOne") == 0; always one to

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

#endif