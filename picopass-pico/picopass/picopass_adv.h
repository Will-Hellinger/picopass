#pragma once

#include <stdbool.h>
#include <stdint.h>

int picopass_adv_build(uint8_t out_adv_data[31]);

bool picopass_adv_payload_has_secret(const uint8_t *ad_data, uint8_t ad_len);
