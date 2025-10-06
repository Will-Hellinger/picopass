#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/rand.h"
#include "pico/stdlib.h"

/* ------------------ Constants & Config ------------------ */

static const uint8_t PICO_SECRET[8] = {'P', 'I', 'C', 'O', 'P', 'A', 'S', 'S'};
#define MANUF_ID_LO 0xFF
#define MANUF_ID_HI 0xFF
#define RSSI_THRESHOLD -100

/*
Flags:
0x02 = LE General Discoverable Mode
0x04 = BR/EDR Not Supported
0x06 = BR/EDR not supported + LE General Discoverable
*/

static const uint8_t adv_flags[] = {0x02, 0x01, 0x06};

// Manufacturer-specific AD structure (length/type filled at runtime)
static uint8_t adv_manuf[2 /*len+type*/ + 2 /*company*/ + 8 /*secret*/ + 1 /*dev type*/];

// Full advertising payload buffer (flags + manuf)
static uint8_t adv_data[31];
static int adv_len = 0;

/* ------------------ State & Helpers ------------------ */

typedef enum { PHASE_ADVERTISE, PHASE_SCAN } phase_t;

static phase_t phase = PHASE_ADVERTISE;

static btstack_timer_source_t phase_timer;

static int led_on = 0;

static inline uint32_t urand_range(uint32_t lo, uint32_t hi_exclusive) {
    uint32_t r = get_rand_32();
    return lo + (r % (hi_exclusive - lo));
}

static void led_set(int on) {
    led_on = on;
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, on ? 1 : 0);
}

/* ------------------ AD building & parsing ------------------ */

static void build_advertising_data(void) {
    // Manufacturer AD: [len][type=0xFF][company LSB][company MSB][secret
    // 8][device_type 1]
    uint8_t *p = adv_manuf;
    // length = 1(type) + 2(company) + 8(secret) + 1(device_type) = 12
    *p++ = 12;
    *p++ = 0xFF;         // AD type: Manufacturer Specific Data
    *p++ = MANUF_ID_LO;  // Company ID LSB
    *p++ = MANUF_ID_HI;  // Company ID MSB
    memcpy(p, PICO_SECRET, 8);
    p += 8;
    *p++ = 0x01;  // device type byte

    adv_len = 0;
    memcpy(&adv_data[adv_len], adv_flags, sizeof(adv_flags));
    adv_len += sizeof(adv_flags);
    memcpy(&adv_data[adv_len], adv_manuf, sizeof(adv_manuf));
    adv_len += sizeof(adv_manuf);
}

static bool ad_has_our_secret(const uint8_t *ad_data, uint8_t ad_len) {
    for (int i = 0; i <= ad_len - 8; i++) {
        if (memcmp(&ad_data[i], PICO_SECRET, 8) != 0) {
            continue;
        }
        return true;
    }

    return false;
}

/* ------------------ BLE packet handler ------------------ */

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {
    (void)channel;
    (void)size;

    if (packet_type != HCI_EVENT_PACKET) {
        return;
    }

    uint8_t event = hci_event_packet_get_type(packet);

    // Only handle LE advertising reports
    if (event != HCI_EVENT_LE_META) {
        return;
    }

    if (hci_event_le_meta_get_subevent_code(packet) != HCI_SUBEVENT_LE_ADVERTISING_REPORT) {
        return;
    }

    // Use raw packet RSSI (last byte) - BTStack API is unreliable
    int8_t rssi = (int8_t)packet[size - 1];
    
    bd_addr_t addr;
    gap_event_advertising_report_get_address(packet, addr);

    const uint8_t *data = gap_event_advertising_report_get_data(packet);
    int data_len = gap_event_advertising_report_get_data_length(packet);

    bool found_secret = ad_has_our_secret(data, data_len);

    if (rssi >= RSSI_THRESHOLD && found_secret) {
        printf("PICO: %s | RSSI: %d dBm\n", bd_addr_to_str(addr), rssi);

        led_set(1);
        led_set(0);
    } else if (found_secret) {
        printf("(PICO but weak signal: %d dBm)\n", rssi);
    }
}

/* ------------------ Phase machine: advertise <-> scan ------------------ */

static void stop_advertising(void) {
    gap_advertisements_enable(0);
}

static void start_advertising_burst(void) {
    // Random advertise interval: 10–50 ms → units of 0.625 ms
    uint32_t interval_ms = urand_range(10, 51);
    uint16_t interval_units = (uint16_t)((interval_ms * 1000) / 625);
    if (interval_units < 0x20) {
        interval_units = 0x20;  // spec min 20 ms
    }
    if (interval_units > 0x4000) {
        interval_units = 0x4000;
    }

    uint8_t adv_type = 0x03;
    bd_addr_t null_addr = {0, 0, 0, 0, 0, 0};

    gap_advertisements_set_params(interval_units, interval_units, adv_type, 0, null_addr, 0x07,
                                  0x00);
    gap_advertisements_set_data(adv_len, adv_data);
    gap_advertisements_enable(1);

    // Random advertising duration 10–50 ms, then stop and switch to scan
    uint32_t burst_ms = urand_range(10, 51);
    printf("[ADV] Starting advertising burst: interval=%ums, duration=%ums\n", interval_ms,
           burst_ms);
    btstack_run_loop_set_timer(&phase_timer, (int)burst_ms);
    btstack_run_loop_add_timer(&phase_timer);
}

static void stop_scanning(void) {
    gap_stop_scan();
}

static void start_scan_burst(void) {
    uint32_t window_ms = urand_range(10, 51);
    uint32_t interval_ms = urand_range(80, 151);

    if (window_ms > interval_ms) {
        window_ms = interval_ms - 1;
    }

    uint16_t window_units = (uint16_t)((window_ms * 1000) / 625);
    uint16_t interval_units = (uint16_t)((interval_ms * 1000) / 625);

    if (window_units == 0) {
        window_units = 1;
    }
    if (interval_units == 0) {
        interval_units = 1;
    }

    printf("[SCAN] Starting scan burst: window=%ums, interval=%ums (units: %u/%u)\n", window_ms,
           interval_ms, window_units, interval_units);

    gap_set_scan_parameters(1 /* active */, interval_units, window_units);
    gap_start_scan();

    // Stop scan after the window; next phase will be scheduled by timer callback
    btstack_run_loop_set_timer(&phase_timer, (int)window_ms);
    btstack_run_loop_add_timer(&phase_timer);
}

static void phase_timer_handler(btstack_timer_source_t *ts) {
    (void)ts;
    printf("[TIMER] Phase timer triggered - current phase: %s\n",
           phase == PHASE_ADVERTISE ? "ADVERTISE" : "SCAN");

    if (phase == PHASE_ADVERTISE) {
        printf("[STATE] Stopping advertising, switching to SCAN mode\n");
        stop_advertising();
        phase = PHASE_SCAN;
        start_scan_burst();
    } else {  // PHASE_SCAN: just finished current scan window
        printf("[STATE] Stopping scanning, switching to ADVERTISE mode\n");
        stop_scanning();
        phase = PHASE_ADVERTISE;
        start_advertising_burst();
    }
}

/* ------------------ Main ------------------ */

int main(void) {
    stdio_init_all();

    if (cyw43_arch_init()) {
        printf("cyw43_arch_init failed\n");
        return -1;
    }
    led_set(1);

    // Init BTstack
    printf("Initializing BTstack...\n");
    l2cap_init();
    sm_init();

    // Power on the HCI layer
    hci_power_control(HCI_POWER_ON);
    printf("BTstack initialized and HCI powered on\n");

    // Wait for valid MAC address
    bd_addr_t local_addr;
    bool mac_valid = false;

    for (int retry = 0; retry < 50; retry++) {
        gap_local_bd_addr(local_addr);

        bool all_zero = true;
        for (int i = 0; i < 6; i++) {
            if (local_addr[i] != 0) {
                all_zero = false;
                break;
            }
        }

        if (!all_zero) {
            mac_valid = true;
            led_set(0);
            printf("MAC address initialized after %d attempts\n", retry + 1);
            break;
        }

        sleep_ms(10);  // Wait 10ms before retry
    }

    if (!mac_valid) {
        printf("ERROR: Failed to get valid MAC address after 500ms\n");
        return -1;
    }
    uint32_t seed = 0;

    for (int i = 0; i < 6; ++i) {
        seed += ((uint32_t)local_addr[i]) << (i * 4);
    }

    srand(seed);

    uint32_t s_mix = seed ^ get_rand_32();
    srand(s_mix);

    printf("=== PICOPASS STARTUP ===\n");
    printf("Device MAC: %s\n", bd_addr_to_str(local_addr));
    printf("Random seed: %u\n", s_mix);
    printf("RSSI threshold: %d dBm\n", RSSI_THRESHOLD);
    printf("Starting PicoPASS BLE burst mode...\n");

    build_advertising_data();

    printf("Your advertising data (%d bytes): ", adv_len);
    for (int i = 0; i < adv_len; i++) {
        printf("%02X ", adv_data[i]);
    }
    printf("\nExpected secret: ");
    for (int i = 0; i < 8; i++) {
        printf("%02X ", PICO_SECRET[i]);
    }
    printf("\n");

    static btstack_packet_callback_registration_t hci_cb;
    hci_cb.callback = &packet_handler;
    hci_add_event_handler(&hci_cb);

    btstack_run_loop_set_timer_handler(&phase_timer, &phase_timer_handler);

    printf("[INIT] Starting in ADVERTISE mode\n");
    phase = PHASE_ADVERTISE;
    phase_timer_handler(NULL);

    btstack_run_loop_execute();

    cyw43_arch_deinit();
    return 0;
}