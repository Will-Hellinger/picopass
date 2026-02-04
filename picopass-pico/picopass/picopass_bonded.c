#include "picopass_bonded.h"
#include "picopass_log.h"
#include <string.h>

static bonded_device_t bonded_list[PICOPASS_MAX_BONDED];
static int bonded_count = 0;

void picopass_bonded_init(void) {
    memset(bonded_list, 0, sizeof(bonded_list));
    bonded_count = 0;
    PP_LOG("Bonded devices tracker initialized (max %d)", PICOPASS_MAX_BONDED);
}

uint32_t picopass_bonded_lookup(const bd_addr_t addr) {
    for (int i = 0; i < bonded_count; i++) {
        if (memcmp(bonded_list[i].address, addr, 6) == 0) {
            return bonded_list[i].last_seen_ms;
        }
    }
    
    return 0;  // Not found
}

void picopass_bonded_update(const bd_addr_t addr, uint32_t now_ms) {
    // Check if already exists
    for (int i = 0; i < bonded_count; i++) {
        if (memcmp(bonded_list[i].address, addr, 6) == 0) {
            bonded_list[i].last_seen_ms = now_ms;
            return;
        }
    }

    // Not found, add new
    if (bonded_count < PICOPASS_MAX_BONDED) {
        memcpy(bonded_list[bonded_count].address, addr, 6);
        bonded_list[bonded_count].last_seen_ms = now_ms;
        bonded_count++;
        PP_LOG("Added bonded device #%d: %s", bonded_count, bd_addr_to_str(addr));
    } else {
        PP_LOG("Bonded list full! Ignoring new device");
    }
}

int picopass_bonded_count(void) {
    return bonded_count;
}

void picopass_bonded_dump(void) {
    PP_LOG("=== Bonded Devices (%d) ===", bonded_count);
    for (int i = 0; i < bonded_count; i++) {
        PP_LOG("  [%d] %s (last seen %u ms)", i, bd_addr_to_str(bonded_list[i].address),
               bonded_list[i].last_seen_ms);
    }
}