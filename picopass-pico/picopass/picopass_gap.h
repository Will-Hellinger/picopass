#pragma once

#include <stdint.h>

#include "btstack_run_loop.h"

void picopass_gap_stop_advertising(void);
void picopass_gap_stop_scanning(void);

void picopass_gap_start_advertising_burst(btstack_timer_source_t *phase_timer, uint8_t *adv, int adv_len);

void picopass_gap_start_scan_burst(btstack_timer_source_t *phase_timer);
