#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "linux_port.h"
#include "esp_loader_io.h"
#include "flasher_utils.h"

#define DEFAULT_RESET_CHIP "gpiochip0"
#define DEFAULT_TRIGGER_CHIP "gpiochip0"

#define DEFAULT_BAUDRATE 115200

static const char *VERSION = "0.1.3";
static const char *APPNAME = "esp-flasher-linux";
static const char *exec_name;

void print_usage_oneline(FILE *file)
{
	fprintf(file,
		"usage: %s [-h] [-v] [-R GPIOCHIP] -r GPIOLINE [-B GPIOCHIP] -b GPIOLINE "
		"-d DEVICE firmware\n",
		exec_name);
}

void print_help()
{
	print_usage_oneline(stdout);
	printf("\nLinux utility to program ESP Family of microcontrollers"
	       "\n"
	       "\npositional arguments:"
	       "\n  firmware             merged firmware file for flashing"
	       "\n"
	       "\noptions:"
	       "\n  -h, --help            show this help message and exit"
	       "\n  -v, --version         show program's version number and exit"
	       "\n  -R GPIOCHIP, --reset-chip GPIOCHIP"
	       "\n                        reset pin GPIO character device (default: gpiochip0)"
	       "\n  -r GPIOLINE, --reset-line GPIOLINE"
	       "\n                        reset pin GPIO line"
	       "\n  -B GPIOCHIP, --bootsel-chip GPIOCHIP"
	       "\n                        boot select pin GPIO character device (default: gpiochip0)"
	       "\n  -b GPIOLINE, --bootsel-line GPIOLINE"
	       "\n                        boot select pin GPIO line"
	       "\n  -d DEVICE, --device DEVICE"
	       "\n                        serial device for sending file"
	       "\n");
}

void print_missing_arg_error(const char *missing_arg)
{
	print_usage_oneline(stderr);
	fprintf(stderr, "%s: error: argument '%s' is required.\n", APPNAME,
		missing_arg);
}

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

int main(int argc, char **argv)
{
	int opt;
	const char *reset_chip = DEFAULT_RESET_CHIP;
	int reset_line = -1;
	const char *bootsel_chip = DEFAULT_TRIGGER_CHIP;
	int bootsel_line = -1;
	const char *device = NULL;
	const char *fw_file = NULL;

	exec_name = argv[0];

	struct option long_options[] = {
		{ "reset-chip", required_argument, NULL, 'R' },
		{ "reset-line", required_argument, NULL, 'r' },
		{ "bootsel-chip", required_argument, NULL, 'B' },
		{ "bootsel-line", required_argument, NULL, 'b' },
		{ "device", required_argument, NULL, 'd' },
		{ "help", no_argument, NULL, 'h' },
		{ "version", no_argument, NULL, 'v' },
		{ NULL, 0, NULL, 0 },
	};

	while ((opt = getopt_long(argc, argv, "R:r:B:b:d:hv", long_options,
				  NULL)) != -1) {
		switch (opt) {
		case 'h':
			print_help(stdout);
			return EXIT_SUCCESS;

		case 'v':
			printf("%s %s\r\n", APPNAME, VERSION);
			return EXIT_SUCCESS;
			break;

		case 'R':
			reset_chip = optarg;
			break;

		case 'r':
			reset_line = atoi(optarg);
			break;

		case 'B':
			bootsel_chip = optarg;
			break;

		case 'b':
			bootsel_line = atoi(optarg);
			break;

		case 'd':
			device = optarg;
			break;
		}
	}

	if (reset_line == -1) {
		print_missing_arg_error("-r/--reset-line");
		return EXIT_FAILURE;
	}
	if (bootsel_line == -1) {
		print_missing_arg_error("-t/--trigger-line");
		return EXIT_FAILURE;
	}
	if (device == NULL) {
		print_missing_arg_error("-d/--device");
		return EXIT_FAILURE;
	}
	if (argc == optind) {
		print_missing_arg_error("firmware");
		return EXIT_FAILURE;
	}
	fw_file = argv[optind];

	printf("Reset     => chip: %s \t line: %d\r\n", reset_chip, reset_line);
	printf("Bootselct => chip: %s \t line: %d\r\n", bootsel_chip,
	       bootsel_line);
	printf("Device    => %s\r\n", device);
	printf("Firmware  => %s\r\n", fw_file);

	FILE *fp = fopen(fw_file, "rb");
	if (fp == NULL) {
		perror("Failed to open firmware file");
		return EXIT_FAILURE;
	}
	fseek(fp, 0, SEEK_END);
	size_t fw_size = ftell(fp);
	rewind(fp);

	uint8_t *buffer = malloc(fw_size);
	fread(buffer, fw_size, 1, fp);

	fclose(fp);

	const loader_linux_config_t config = {
		.device = device,
		.baudrate = DEFAULT_BAUDRATE,
		.bootsel_trigger_chip = bootsel_chip,
		.bootsel_trigger_line = bootsel_line,
		.reset_trigger_chip = reset_chip,
		.reset_trigger_line = reset_line,
	};

	if (linux_loader_port_init(&config) != ESP_LOADER_SUCCESS) {
		free(buffer);
		return EXIT_FAILURE;
	}
	// loader_port_reset_target();
	if (connect_to_target(DEFAULT_BAUDRATE) == ESP_LOADER_SUCCESS) {
		printf("Idetifying chip...\n");
		target_chip_t chip = esp_loader_get_target();
		printf("Chip identified as: ");
		print_chip_name(chip);

		printf("Loading merged firmware...\n");
		flash_binary(buffer, fw_size, 0x00);
		printf("Done!\n");
		esp_loader_reset_target();
	}
	linux_loader_port_deinit();

	free(buffer);
	return EXIT_SUCCESS;
}