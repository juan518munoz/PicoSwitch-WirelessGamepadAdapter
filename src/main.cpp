#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"

#include "Bluepad_main.h"
#include "SwitchDescriptors.h"

#include "bsp/board.h"
#include "tusb.h"

void blink_task(uint8_t times)
{
    for (uint8_t i = 0; i < times; i++)
    {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(100);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(100);
    }
    sleep_ms(500);
}

void send_switch_hid_report(SwitchOutReport report)
{
    // try
    // {
    //     tud_task();
    //     if (tud_suspended())
    //     {
    //         blink_task(2);
    //         tud_remote_wakeup();
    //     }
    //     if (tud_hid_ready())
    //     {
    //         tud_hid_report(0, &report, sizeof(report));

    //         SwitchOutReport dummy_report = {
    //             .buttons = 0,
    //             .hat = SWITCH_HAT_NOTHING,
    //             .lx = SWITCH_JOYSTICK_MID,
    //             .ly = SWITCH_JOYSTICK_MID,
    //             .rx = SWITCH_JOYSTICK_MID,
    //             .ry = SWITCH_JOYSTICK_MID};
    //         tud_hid_report(1, &dummy_report, sizeof(dummy_report));
    //     }
    // }
    // catch (int e)
    // {
    //     tud_task();
    //     if (tud_suspended())
    //     {
    //         blink_task(3);
    //         tud_remote_wakeup();
    //     }
    // }
    tud_task();
    if (tud_suspended())
    {
        tud_remote_wakeup();
    }

    if (tud_hid_ready())
    {

        // tud_hid_report(0, &report, sizeof(report));

        // SwitchOutReport dummy_report = {
        //     .buttons = 0x01,
        //     .hat = SWITCH_HAT_NOTHING,
        //     .lx = SWITCH_JOYSTICK_MID,
        //     .ly = SWITCH_JOYSTICK_MID,
        //     .rx = SWITCH_JOYSTICK_MID,
        //     .ry = SWITCH_JOYSTICK_MID};
        // tud_hid_report(1, &report, sizeof(report));
    }
}

int main()
{
    stdio_init_all();

    init_bluepad();
    sleep_ms(1000);

    tusb_init();

    // int conn = 0;
    // uint16_t buttons;
    SwitchOutReport report;
    while (true)
    {
        get_button_state(&report);

        send_switch_hid_report(report);
    }

    return 0;
}
