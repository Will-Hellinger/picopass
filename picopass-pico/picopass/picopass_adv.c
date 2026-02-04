#include "picopass_adv.h"

#include <string.h>

#include "picopass_config.h"
#include "picopass_secret.h"

/*
Flags
- 0x02 : General Discoverable Mode
- 0x01 : BR/EDR Not Supported
- 0x06 : LE General Discoverable Mode, BR/EDR Not Supported
*/
static const uint8_t adv_flags[] = {0x02, 0x01, 0x06};

int picopass_adv_build(uint8_t out_adv_data[31]) {
    // Manufacturer AD: [len][type=0xFF][company LSB][company MSB][secret 8][device_type 1]
    uint8_t manuf[2 /*len+type*/ + 2 /*company*/ + 8 /*secret*/ + 1 /*dev type*/];

    const uint8_t *secret = picopass_secret_bytes();

    uint8_t *p = manuf;
    // length = 1(type) + 2(company) + 8(secret) + 1(device_type) = 12
    *p++ = 12;
    *p++ = 0xFF;
    *p++ = PICOPASS_MANUF_ID_LO;
    *p++ = PICOPASS_MANUF_ID_HI;
    memcpy(p, secret, 8);
    p += 8;
    *p++ = PICOPASS_DEVICE_TYPE;

    int len = 0;
    memcpy(&out_adv_data[len], adv_flags, sizeof(adv_flags));
    len += (int)sizeof(adv_flags);
    memcpy(&out_adv_data[len], manuf, sizeof(manuf));
    len += (int)sizeof(manuf);

    return len;
}

bool picopass_adv_payload_has_secret(const uint8_t *ad_data, uint8_t ad_len) {
    const uint8_t *secret = picopass_secret_bytes();

    for (int i = 0; i <= (int)ad_len - 8; i++) {
        if (memcmp(&ad_data[i], secret, 8) != 0) {
            continue;
        }
        return true;
    }

    return false;
}
