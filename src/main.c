#include <stdio.h>
#include <stdlib.h>
#include "linux_port.h"
#include "esp_loader_io.h"
#include "flasher_utils.h"
#include "utils.h"
#include "config.h"

/**
 * @brief read contents of a file to memory
 * 
 * @note this function allocates memory in the heap, 
 * 		 make sure to free memory
 * 
 * @param file_path file to read contents from
 * @param file_size pointer to store file length
 * 
 * @return pointer with file contents on success
 * @return NULL on failure 
 */
uint8_t *read_file(const char *file_path, size_t *file_size)
{
	FILE *fp = fopen(file_path, "rb");
	if (fp == NULL) {
		perror("Failed to open firmware file");
		return NULL;
	}
	fseek(fp, 0, SEEK_END);
	*file_size = ftell(fp);
	rewind(fp);

	uint8_t *buffer = malloc(*file_size);
	fread(buffer, *file_size, 1, fp);

	fclose(fp);

	return buffer;
}

int main(int argc, char **argv)
{
	args_t args;
	parse_args(&args, argc, argv);

	printf("Reset         => chip: %s \t line: %d\r\n", args.reset_chip,
	       args.reset_line);
	printf("Boot select   => chip: %s \t line: %d\r\n", args.bootsel_chip,
	       args.bootsel_line);
	printf("Serial Device => %s\r\n", args.device);
	printf("Firmware      => %s\r\n", args.fw_file);

	size_t fw_size;
	uint8_t *fw_data = read_file(args.fw_file, &fw_size);
	if (!fw_data) {
		return EXIT_FAILURE;
	}

	const loader_linux_config_t config = {
		.device = args.device,
		.baudrate = DEFAULT_BAUDRATE,
		.bootsel_trigger_chip = args.bootsel_chip,
		.bootsel_trigger_line = args.bootsel_line,
		.reset_trigger_chip = args.reset_chip,
		.reset_trigger_line = args.reset_line,
	};

	if (linux_loader_port_init(&config) != ESP_LOADER_SUCCESS) {
		free(fw_data);
		return EXIT_FAILURE;
	}

	if (connect_to_target(args.transmission_rate) != ESP_LOADER_SUCCESS) {
		free(fw_data);
		return EXIT_FAILURE;
	}
	printf("Idetifying chip...\n");
	target_chip_t chip = esp_loader_get_target();
	printf("Chip identified as ");
	print_chip_name(chip);

	printf("Flashing firmware...\n");
	flash_binary(fw_data, fw_size, 0x00);
	printf("Done!\n");
	esp_loader_reset_target();

	linux_loader_port_deinit();

	free(fw_data);
	return EXIT_SUCCESS;
}