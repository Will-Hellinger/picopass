#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "btstack.h"

#define PICOPASS_MAX_BONDED 256

typedef struct {
    bd_addr_t address;
    uint32_t last_seen_ms;
} bonded_device_t;
void picopass_bonded_init(void);

uint32_t picopass_bonded_lookup(const bd_addr_t addr);

void picopass_bonded_update(const bd_addr_t addr, uint32_t now_ms);

int picopass_bonded_count(void);

void picopass_bonded_dump(void);