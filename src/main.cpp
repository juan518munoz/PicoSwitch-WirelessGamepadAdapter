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
    tud_task();
    if (tud_suspended())
    {
        tud_remote_wakeup();
    }

    if (tud_hid_n_ready(0))
    {
        tud_hid_n_report(0, 0, &report, sizeof(report));
    }
    if (tud_hid_n_ready(1))
    {
        tud_hid_n_report(1, 0, &report, sizeof(report));
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
