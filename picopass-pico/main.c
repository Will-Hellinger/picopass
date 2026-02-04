#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack.h"
#include "pico/cyw43_arch.h"
#include "pico/rand.h"
#include "pico/stdlib.h"
#include "picopass/picopass_adv.h"
#include "picopass/picopass_bonded.h"
#include "picopass/picopass_config.h"
#include "picopass/picopass_gap.h"
#include "picopass/picopass_led.h"
#include "picopass/picopass_log.h"
#include "picopass/picopass_secret.h"

typedef enum { PHASE_ADVERTISE, PHASE_SCAN } phase_t;

static phase_t phase = PHASE_ADVERTISE;

static btstack_timer_source_t phase_timer;

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

    bool found_secret = picopass_adv_payload_has_secret(data, data_len);

    if (rssi >= PICOPASS_RSSI_THRESHOLD && found_secret) {
        uint32_t last_seen = picopass_bonded_lookup(addr);
        PP_LOG("PICO: %s | RSSI: %d dBm", bd_addr_to_str(addr), rssi);

        if (last_seen == 0) {
            PP_LOG("NEW PICO: %s | RSSI: %d dBm", bd_addr_to_str(addr), rssi);
        } else {
            PP_LOG("SEEN BEFORE: %s | RSSI: %d dBm (last: %u ms ago)", bd_addr_to_str(addr), rssi,
                   to_ms_since_boot(get_absolute_time()) - last_seen);
        }

        picopass_bonded_update(addr, to_ms_since_boot(get_absolute_time()));

        picopass_led_flicker(0);
    } else if (found_secret) {
        PP_LOG("(PICO but weak signal: %d dBm)", rssi);
    }
}

static uint8_t adv_data[31];
static int adv_len = 0;

static void phase_timer_handler(btstack_timer_source_t *ts) {
    (void)ts;
    PP_LOG("[TIMER] Phase timer triggered - current phase: %s",
           phase == PHASE_ADVERTISE ? "ADVERTISE" : "SCAN");

    if (phase == PHASE_ADVERTISE) {
        PP_LOG("[STATE] Stopping advertising, switching to SCAN mode");
        picopass_gap_stop_advertising();
        phase = PHASE_SCAN;
        picopass_gap_start_scan_burst(&phase_timer);
    } else {  // PHASE_SCAN: just finished current scan window
        PP_LOG("[STATE] Stopping scanning, switching to ADVERTISE mode");
        picopass_gap_stop_scanning();
        phase = PHASE_ADVERTISE;
        picopass_gap_start_advertising_burst(&phase_timer, adv_data, adv_len);
    }
}

int main(void) {
    stdio_init_all();

    if (cyw43_arch_init()) {
        PP_LOG("cyw43_arch_init failed");
        return -1;
    }
    picopass_led_set(1);

    // Init BTstack
    PP_LOG("Initializing BTstack...");
    l2cap_init();
    sm_init();

    // Power on the HCI layer
    hci_power_control(HCI_POWER_ON);
    PP_LOG("BTstack initialized and HCI powered on");

    picopass_bonded_init();

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
            picopass_led_set(0);
            PP_LOG("MAC address initialized after %d attempts", retry + 1);
            break;
        }

        sleep_ms(10);  // Wait 10ms before retry
    }

    if (!mac_valid) {
        PP_LOG("ERROR: Failed to get valid MAC address after 500ms");
        return -1;
    }
    uint32_t seed = 0;

    for (int i = 0; i < 6; ++i) {
        seed += ((uint32_t)local_addr[i]) << (i * 4);
    }

    srand(seed);

    uint32_t s_mix = seed ^ get_rand_32();
    srand(s_mix);

    PP_LOG("=== PICOPASS STARTUP ===");
    PP_LOG("Device MAC: %s", bd_addr_to_str(local_addr));
    PP_LOG("Random seed: %u", s_mix);
    PP_LOG("RSSI threshold: %d dBm", PICOPASS_RSSI_THRESHOLD);
    PP_LOG("Starting PicoPASS BLE burst mode...");

    adv_len = picopass_adv_build(adv_data);

    const uint8_t *secret = picopass_secret_bytes();
    PP_LOG_HEX("Your advertising data", adv_data, (size_t)adv_len);
    PP_LOG_HEX("Expected secret", secret, 8);

    static btstack_packet_callback_registration_t hci_cb;
    hci_cb.callback = &packet_handler;
    hci_add_event_handler(&hci_cb);

    btstack_run_loop_set_timer_handler(&phase_timer, &phase_timer_handler);

    PP_LOG("[INIT] Starting in ADVERTISE mode");
    phase = PHASE_ADVERTISE;
    phase_timer_handler(NULL);

    btstack_run_loop_execute();

    cyw43_arch_deinit();
    return 0;
}