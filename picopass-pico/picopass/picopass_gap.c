#include "picopass_gap.h"

#include "btstack.h"
#include "pico/rand.h"
#include "picopass_log.h"

static inline uint32_t urand_range(uint32_t lo, uint32_t hi_exclusive) {
    uint32_t r = get_rand_32();
    return lo + (r % (hi_exclusive - lo));
}

void picopass_gap_stop_advertising(void) {
    gap_advertisements_enable(0);
}

void picopass_gap_stop_scanning(void) {
    gap_stop_scan();
}

void picopass_gap_start_advertising_burst(btstack_timer_source_t *phase_timer, uint8_t *adv, int adv_len) {
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
    gap_advertisements_set_data(adv_len, adv);
    gap_advertisements_enable(1);

    // Random advertising duration 10–50 ms, then stop and switch to scan
    uint32_t burst_ms = urand_range(10, 51);
    PP_LOG("[ADV] Starting advertising burst: interval=%ums, duration=%ums", interval_ms, burst_ms);
    btstack_run_loop_set_timer(phase_timer, (int)burst_ms);
    btstack_run_loop_add_timer(phase_timer);
}

void picopass_gap_start_scan_burst(btstack_timer_source_t *phase_timer) {
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

    PP_LOG("[SCAN] Starting scan burst: window=%ums, interval=%ums (units: %u/%u)", window_ms, interval_ms, window_units, interval_units);

    gap_set_scan_parameters(1 /* active */, interval_units, window_units);
    gap_start_scan();

    // Stop scan after the window; next phase will be scheduled by timer callback
    btstack_run_loop_set_timer(phase_timer, (int)window_ms);
    btstack_run_loop_add_timer(phase_timer);
}
