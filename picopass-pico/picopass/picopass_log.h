#pragma once

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

void pp_log_printf_(const char *file, int line, const char *fmt, ...);

// Convenience for printing hex bytes with the same prefix.
void pp_log_hexdump_(const char *file, int line, const char *label, const uint8_t *data,
                     size_t len);

#ifdef __cplusplus
}
#endif

#define PP_LOG(...) pp_log_printf_(__FILE__, __LINE__, __VA_ARGS__)
#define PP_LOG_HEX(label, data, len) pp_log_hexdump_(__FILE__, __LINE__, label, data, len)

#ifdef PICOPASS_DEBUG
#define PP_LOG_DBG(...) pp_log_printf_(__FILE__, __LINE__, __VA_ARGS__)
#else
#define PP_LOG_DBG(...) ((void)0)
#endif