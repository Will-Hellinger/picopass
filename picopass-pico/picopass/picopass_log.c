#include "picopass_log.h"

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "pico/time.h"

static const char *pp_log_basename_(const char *path) {
    if (path == NULL) {
        return "?";
    }

    const char *base = path;
    for (const char *p = path; *p; p++) {
        if (*p == '/' || *p == '\\') {
            base = p + 1;
        }
    }
    return base;
}

static void pp_log_make_tag_(const char *file, char out_tag[16]) {
    const char *base = pp_log_basename_(file);

    size_t i = 0;
    for (; base[i] != '\0' && base[i] != '.' && i < 15; i++) {
        unsigned char c = (unsigned char)base[i];
        out_tag[i] = (char)toupper(c);
    }
    out_tag[i] = '\0';

    if (out_tag[0] == '\0') {
        strcpy(out_tag, "?");
    }
}

static uint32_t pp_log_now_ms_(void) {
    return to_ms_since_boot(get_absolute_time());
}

void pp_log_printf_(const char *file, int line, const char *fmt, ...) {
    char tag[16];
    pp_log_make_tag_(file, tag);

    uint32_t ms = pp_log_now_ms_();

    printf("[%lu] %s %d: ", (unsigned long)ms, tag, line);

    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    va_end(ap);

    printf("\n");
}

void pp_log_hexdump_(const char *file, int line, const char *label, const uint8_t *data,
                     size_t len) {
    char tag[16];
    pp_log_make_tag_(file, tag);

    uint32_t ms = pp_log_now_ms_();

    printf("[%lu] %s <%d>: %s (%u bytes): ", (unsigned long)ms, tag, line,
           (label != NULL) ? label : "data", (unsigned)len);

    for (size_t i = 0; i < len; i++) {
        printf("%02X ", data[i]);
    }
    printf("\n");
}
