#include "picopass_secret.h"

static const uint8_t PICO_SECRET[8] = {'P', 'I', 'C', 'O', 'P', 'A', 'S', 'S'};

const uint8_t *picopass_secret_bytes(void) {
    return PICO_SECRET;
}
