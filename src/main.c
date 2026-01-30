// SPDX-FileCopyrightText: 2026 E-Control Systems
//
// SPDX-License-Identifier: Apache-2.0

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
 * @attention this function allocates memory in the heap, 
 * 		 make sure to free the memory
 * 
 * @param file_path file to read contents from
 * @param file_size pointer to store file length
 * 
 * @return 
 *     - pointer with file contents on Success
 *     - NULL on failure
 */
uint8_t *read_file(const char *file_path, size_t *file_size)
{
	FILE *fp = fopen(file_path, "rb");
	if (fp == NULL) {
		perror("Failed to open file");
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

/**
 * @brief upload a file to the given address in flash
 * 
 * @param addr address to written to
 * @param file_path file to write
 * @return ESP_LOADER_SUCCESS on success, or error codes on failure.
 */
esp_loader_error_t upload_file(const size_t addr, const char *file_path)
{
	size_t fw_size;
	uint8_t *fw_data = read_file(file_path, &fw_size);
	if (!fw_data) {
		return ESP_LOADER_ERROR_FAIL;
	}
	(void)printf("Flashing '%s' at 0x%zx...\n", file_path, addr);
	esp_loader_error_t err = flash_binary(fw_data, fw_size, addr);
	free(fw_data);

	return err;
}

int main(int argc, char **argv)
{
	args_t args;
	int fw_list_index = parse_args(&args, argc, argv);

	(void)printf("Reset: %s; %d\r\n", args.reset_chip, args.reset_line);
	(void)printf("Boot select: %s; %d\r\n", args.bootsel_chip,
		     args.bootsel_line);
	(void)printf("Serial port: %s\r\n", args.device);

	const loader_linux_config_t config = {
		.device = args.device,
		.baudrate = DEFAULT_BAUDRATE,
		.bootsel_trigger_chip = args.bootsel_chip,
		.bootsel_trigger_line = args.bootsel_line,
		.reset_trigger_chip = args.reset_chip,
		.reset_trigger_line = args.reset_line,
	};

	if (linux_loader_port_init(&config) != ESP_LOADER_SUCCESS) {
		return EXIT_FAILURE;
	}

	if (connect_to_target(args.transmission_rate) != ESP_LOADER_SUCCESS) {
		linux_loader_port_deinit();
		return EXIT_FAILURE;
	}
	(void)printf("Idetifying chip...\r\n");
	target_chip_t chip = esp_loader_get_target();
	(void)printf("Chip identified as ");
	print_chip_name(chip);

	for (; fw_list_index < argc; fw_list_index += 2) {
		unsigned long addr = strtoul(argv[fw_list_index], NULL, 0);
		const char *filename = argv[fw_list_index + 1];
		(void)printf("Writing %s to 0x%lx\r\n", filename, addr);
		esp_loader_error_t err = upload_file(addr, filename);
		if (err != ESP_LOADER_SUCCESS) {
			perror("Failed to write file");
			esp_loader_reset_target();
			linux_loader_port_deinit();
			return EXIT_FAILURE;
		}
	}
	(void)printf("Done!\r\n");

	esp_loader_reset_target();
	linux_loader_port_deinit();

	return EXIT_SUCCESS;
}