#pragma once

#include "esp_loader.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief structure for storing esp loader configuration for linux
 * 
 */
typedef struct {
	const char *device; /**< serial deivce for communication */
	uint32_t baudrate; /**< base baudrate */
	const char *reset_trigger_chip; /**< gpiochip for reset trigger */
	uint32_t reset_trigger_line; /**< gpiochip offset for reset trigger */
	const char *bootsel_trigger_chip; /**< gpiochip for bootsel trigger */
	uint32_t bootsel_trigger_line; /**< gpiochip offset for bootsel trigger */
} loader_linux_config_t;

/**
 * @brief initialize gpios and serial port
 * 
 * @param config configuration for initilization
 * 
 * @attention make sure to call linux_loader_port_deinit afterwards
 * 
 * @return ESP_LOADER_SUCCESS on Success
 * @return ESP_LOADER_FAILURE on Failure
 */
esp_loader_error_t linux_loader_port_init(const loader_linux_config_t *config);
/**
 * @brief deinitilize gpios and serial port
 * 
 */
void linux_loader_port_deinit(void);

#ifdef __cplusplus
}
#endif
