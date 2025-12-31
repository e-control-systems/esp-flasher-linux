#pragma once

#include "esp_loader.h"
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief structure to store parsed arguments
 * 
 */
typedef struct {
	const char *reset_chip; /**< esp reset trigger line's chip */
	int reset_line; /**< esp reset trigger line */
	const char *bootsel_chip; /**< esp bootsel trigger line's chip */
	int bootsel_line; /**< esp bootsel trigger line */
	const char *device; /**< serial device for flashing */
	const char *fw_file; /**< firmware file */
	uint32_t transmission_rate; /**< higher transmission rate for communicating with esp */
} args_t;

/**
 * @brief parses commandline arguments
 * 
 * @param args args_t pointer to store parsed arguments
 * @param argc argument count (argc from main)
 * @param argv argument value list (argv from main)
 */
void parse_args(args_t *args, int argc, char **argv);
/**
 * @brief prints string value of esp chip
 * 
 * @param chip esp chip type
 */
void print_chip_name(target_chip_t chip);

#ifdef __cplusplus
}
#endif
