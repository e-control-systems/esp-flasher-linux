// SPDX-FileCopyrightText: 2026 E-Control Systems
//
// SPDX-License-Identifier: Apache-2.0

#include "utils.h"
#include "esp_loader.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include "config.h"
#include <stdbool.h>
#include <errno.h>

#define DEFAULT_RESET_CHIP "gpiochip0"
#define DEFAULT_BOOTSEL_CHIP "gpiochip0"

static const char *exec_name;

void print_chip_name(target_chip_t chip)
{
	switch (chip) {
	case ESP8266_CHIP:
		(void)printf("ESP8266\n");
		break;
	case ESP32_CHIP:
		(void)printf("ESP32\n");
		break;
	case ESP32S2_CHIP:
		(void)printf("ESP32 S2\n");
		break;
	case ESP32C3_CHIP:
		(void)printf("ESP32 C3\n");
		break;
	case ESP32S3_CHIP:
		(void)printf("ESP32 S3\n");
		break;
	case ESP32C2_CHIP:
		(void)printf("ESP32 C2\n");
		break;
	case ESP32C5_CHIP:
		(void)printf("ESP32 C5\n");
		break;
	case ESP32H2_CHIP:
		(void)printf("ESP32 H2\n");
		break;
	case ESP32C6_CHIP:
		(void)printf("ESP32 C6\n");
		break;
	case ESP32P4_CHIP:
		(void)printf("ESP32 P4\n");
		break;
	case ESP_MAX_CHIP:
		(void)printf("ESP32 MAX\n");
		break;
	default:
		(void)printf("Unknown Chip");
	}
}

void print_help()
{
	(void)printf(
		"\nusage: %s [-h] [-v] [-R GPIOCHIP] -r GPIOLINE [-B GPIOCHIP] "
		"\n                   -b GPIOLINE -d DEVICE [-s BAUDRATE]"
		"\n                   <address> <filename> [<address> <filename> ...]"
		"\n",
		exec_name);
	(void)printf(
		"\nLinux utility to program ESP Family of microcontrollers"
		"\n"
		"\npositional arguments:"
		"\n  <address> <filename>  Address followed by binary filename, separated by space"
		"\n"
		"\noptions:"
		"\n  -h, --help            show this help message and exit"
		"\n  -v, --version         show program's version number and exit"
		"\n  -R GPIOCHIP, --reset-chip GPIOCHIP"
		"\n                        reset pin GPIO character device (default: gpiochip0)"
		"\n  -r GPIOLINE, --reset-line GPIOLINE"
		"\n                        reset pin GPIO line "
		"\n  -B GPIOCHIP, --bootsel-chip GPIOCHIP"
		"\n                        boot select pin GPIO character device (default:"
		"\n                        gpiochip0)"
		"\n  -b GPIOLINE, --bootsel-line GPIOLINE"
		"\n                        boot select pin GPIO line "
		"\n  -d DEVICE, --device DEVICE"
		"\n                        serial device for sending file "
		"\n  -s BAUDRATE, --speed BAUDRATE"
		"\n                        transmission rate to use after initial sync to speed"
		"\n                        up transfers (default: 115200)"
		"\n");
}

void print_usage_err()
{
	(void)fprintf(
		stderr,
		"usage: %s [-h] [-v] [-R GPIOCHIP] -r GPIOLINE [-B GPIOCHIP] -b GPIOLINE -d DEVICE [-s DEVICE] <address> <filename> [<address> <filename> ...]\n",
		exec_name);
}

void print_missing_arg_error(const char *missing_arg)
{
	print_usage_err();
	(void)fprintf(stderr, "%s: error: argument '%s' is required.\n",
		      PROJECT_NAME, missing_arg);
}

int parse_args(args_t *args, int argc, char **argv)
{
	exec_name = argv[0];
	*args = (args_t){
		.reset_chip = DEFAULT_RESET_CHIP,
		.reset_line = -1,
		.bootsel_chip = DEFAULT_BOOTSEL_CHIP,
		.bootsel_line = -1,
		.device = NULL,
		.transmission_rate = DEFAULT_BAUDRATE,
	};

	int opt;
	struct option long_options[] = {
		{ "reset-chip", required_argument, NULL, 'R' },
		{ "reset-line", required_argument, NULL, 'r' },
		{ "bootsel-chip", required_argument, NULL, 'B' },
		{ "bootsel-line", required_argument, NULL, 'b' },
		{ "device", required_argument, NULL, 'd' },
		{ "speed", required_argument, NULL, 's' },
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'v' },
		{ NULL, 0, NULL, 0 },
	};

	while ((opt = getopt_long(argc, argv, "R:r:B:b:d:s:hv", long_options,
				  NULL)) != -1) {
		switch (opt) {
		case 'h':
			print_help(stdout);
			exit(EXIT_SUCCESS);
			break;

		case 'v':
			(void)printf("%s %s\r\n", PROJECT_NAME,
				     PROJECT_VERSION);
			exit(EXIT_SUCCESS);
			break;

		case 'R':
			args->reset_chip = optarg;
			break;

		case 'r':
			args->reset_line = atoi(optarg);
			break;

		case 'B':
			args->bootsel_chip = optarg;
			break;

		case 'b':
			args->bootsel_line = atoi(optarg);
			break;

		case 'd':
			args->device = optarg;
			break;

		case 's':
			args->transmission_rate = strtoul(optarg, NULL, 0);
			break;
		}
	}

	if (args->reset_line == -1) {
		print_missing_arg_error("-r/--reset-line");
		exit(EXIT_FAILURE);
	}
	if (args->bootsel_line == -1) {
		print_missing_arg_error("-b/--bootsel-line");
		exit(EXIT_FAILURE);
	}
	if (args->device == NULL) {
		print_missing_arg_error("-d/--device");
		exit(EXIT_FAILURE);
	}
	if (argc == optind) {
		print_missing_arg_error("<address> <filename>");
		exit(EXIT_FAILURE);
	}
	if ((argc - optind) % 2 != 0) {
		print_usage_err();
		(void)fprintf(
			stderr,
			"%s: error: argument <address> <filename>: Must be pairs of an "
			"address and the binary filename to write there\n",
			PROJECT_NAME);
		exit(EXIT_FAILURE);
	}

	return optind;
}