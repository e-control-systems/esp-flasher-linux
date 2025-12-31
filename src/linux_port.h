#pragma once

#include "esp_loader.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *device;
	uint32_t baudrate;
	const char *reset_trigger_chip;
	uint32_t reset_trigger_line;
	const char *bootsel_trigger_chip;
	uint32_t bootsel_trigger_line;
} loader_linux_config_t;

esp_loader_error_t linux_loader_port_init(const loader_linux_config_t *config);
void linux_loader_port_deinit(void);

#ifdef __cplusplus
}
#endif
