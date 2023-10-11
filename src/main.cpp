#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/multicore.h"

#include "Bluepad_main.h"
#include "SwitchDescriptors.h"

#include "bsp/board.h"
#include "tusb.h"

bool led_on;
void blink_task(uint8_t connected) {
    // keep LED on if connected
    if (!led_on && connected > 0) {
        led_on = true;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    }
    else if (connected == 0) {
        led_on = false;
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
        sleep_ms(200);
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 0);
        sleep_ms(200);
    }
}

void send_switch_hid_report(SwitchOutGeneralReport report)
{
    tud_task();
    if (tud_suspended())
    {
        tud_remote_wakeup();
    }

    blink_task(report.connected);

    for (int i = 0; i < 4; i++) // BP32_MAX_GAMEPADS
    {
        if (tud_hid_n_ready(i))
        {
            tud_hid_n_report(i, 0, &report.gamepad[i], sizeof(report.gamepad[i]));
        }
    }
}

int main()
{
    stdio_init_all();
    init_bluepad();
    tusb_init();

    led_on = false;

    SwitchOutGeneralReport report;
    while (true)
    {
        get_button_state(&report);
        send_switch_hid_report(report);
    }

    return 0;
}
