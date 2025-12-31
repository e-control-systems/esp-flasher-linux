#pragma once

#include "esp_loader.h"
#include <stdio.h>

#define VERSION "0.1.3"
#define APPNAME "esp-flasher"
#define DEFAULT_BAUDRATE 115200

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *reset_chip;
	int reset_line;
	const char *bootsel_chip;
	int bootsel_line;
	const char *device;
	const char *fw_file;
	uint32_t transmission_rate;
} args_t;

void parse_args(args_t *args, int argc, char **argv);
void print_chip_name(target_chip_t chip);
void print_usage_oneline(FILE *file);
void print_help();
void print_missing_arg_error(const char *missing_arg);

#ifdef __cplusplus
}
#endif
