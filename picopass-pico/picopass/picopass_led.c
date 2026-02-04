#include "picopass_led.h"

#include "pico/cyw43_arch.h"

static int led_on = 0;

void picopass_led_set(int on) {
    led_on = on;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on);
}

void picopass_led_toggle(void) {
    led_on = !led_on;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, led_on);
}

void picopass_led_flicker(int ms) {
    picopass_led_set(1);
    sleep_ms(ms);
    picopass_led_set(0);
}