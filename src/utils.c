#include "utils.h"
#include "esp_loader.h"
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>
#include <errno.h>

#define DEFAULT_RESET_CHIP "gpiochip0"
#define DEFAULT_BOOTSEL_CHIP "gpiochip0"

static const char *exec_name;

void print_chip_name(target_chip_t chip)
{
	switch (chip) {
	case ESP8266_CHIP:
		printf("ESP8266\n");
		break;
	case ESP32_CHIP:
		printf("ESP32\n");
		break;
	case ESP32S2_CHIP:
		printf("ESP32 S2\n");
		break;
	case ESP32C3_CHIP:
		printf("ESP32 C3\n");
		break;
	case ESP32S3_CHIP:
		printf("ESP32 S3\n");
		break;
	case ESP32C2_CHIP:
		printf("ESP32 C2\n");
		break;
	case ESP32C5_CHIP:
		printf("ESP32 C5\n");
		break;
	case ESP32H2_CHIP:
		printf("ESP32 H2\n");
		break;
	case ESP32C6_CHIP:
		printf("ESP32 C6\n");
		break;
	case ESP32P4_CHIP:
		printf("ESP32 P4\n");
		break;
	case ESP_MAX_CHIP:
		printf("ESP32 MAX\n");
		break;
	default:
		printf("Unknown Chip");
	}
}

void print_usage_oneline(FILE *file)
{
	fprintf(file,
		"usage: %s [-h] [-v] [-R GPIOCHIP] -r GPIOLINE [-B GPIOCHIP] -b GPIOLINE "
		"-d DEVICE firmware\n",
		exec_name);
}

void print_help()
{
	printf("\nusage: %s [-h] [-v] [-R GPIOCHIP] -r GPIOLINE [-B GPIOCHIP] -b"
	       "\n                   GPIOLINE -d DEVICE [-s BAUDRATE]"
	       "\n                   firmware"
	       "\n",
	       exec_name);
	printf("\nLinux utility to program ESP Family of microcontrollers"
	       "\n"
	       "\npositional arguments:"
	       "\n  firmware              merged firmware file for flashing"
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

void print_missing_arg_error(const char *missing_arg)
{
	fprintf(stderr,
		"usage: %s [-h] [-v] [-R GPIOCHIP] -r GPIOLINE [-B GPIOCHIP] -b GPIOLINE -d DEVICE [-s DEVICE] firmware\n",
		exec_name);
	fprintf(stderr, "%s: error: argument '%s' is required.\n", APPNAME,
		missing_arg);
}

void parse_args(args_t *args, int argc, char **argv)
{
	exec_name = argv[0];
	*args = (args_t){
		.reset_chip = DEFAULT_RESET_CHIP,
		.reset_line = -1,
		.bootsel_chip = DEFAULT_BOOTSEL_CHIP,
		.bootsel_line = -1,
		.device = NULL,
		.fw_file = NULL,
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
			printf("%s %s\r\n", APPNAME, VERSION);
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

		case 's': {
			char *end;
			errno = 0;
			args->transmission_rate = strtoul(optarg, &end, 10);
			if (end == optarg) {
				fprintf(stderr,
					"error parsing transmission rate. no digits were found.\n");
				exit(EXIT_FAILURE);
			} else if (*end != '\0') {
				fprintf(stderr,
					"error parsing transmission rate. extra characters at the end of the string.\n");
				exit(EXIT_FAILURE);
			} else if (errno == ERANGE) {
				fprintf(stderr,
					"error parsing transmission rate. out of range.\n");
				exit(EXIT_FAILURE);
			}
		} break;
		}
	}

	if (args->reset_line == -1) {
		print_missing_arg_error("-r/--reset-line");
		exit(EXIT_FAILURE);
	}
	if (args->bootsel_line == -1) {
		print_missing_arg_error("-t/--trigger-line");
		exit(EXIT_FAILURE);
	}
	if (args->device == NULL) {
		print_missing_arg_error("-d/--device");
		exit(EXIT_FAILURE);
	}
	if (argc == optind) {
		print_missing_arg_error("firmware");
		exit(EXIT_FAILURE);
	}
	args->fw_file = argv[optind];
}