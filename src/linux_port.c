#include "linux_port.h"
#include "esp_loader.h"
#include "esp_loader_io.h"
#include <fcntl.h>
#include <gpiod.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <time.h>
#include <unistd.h>
#include "protocol.h"

#if SERIAL_FLASHER_DEBUG_TRACE
static void transfer_debug_print(const uint8_t *data, uint16_t size, bool write)
{
	static bool write_prev = false;

	if (write_prev != write) {
		write_prev = write;
		printf("\n--- %s ---\n", write ? "WRITE" : "READ");
	}

	for (uint32_t i = 0; i < size; i++) {
		printf("%02x ", data[i]);
	}
}
#endif

static int64_t s_time_end;
static int serial = -1;
static struct gpiod_line *reset_trigger_line = NULL;
static struct gpiod_chip *reset_trigger_chip = NULL;
static struct gpiod_chip *bootsel_trigger_chip = NULL;
static struct gpiod_line *bootsel_trigger_line = NULL;

static speed_t convert_baudrate(int baud)
{
	switch (baud) {
	case 50:
		return B50;
	case 75:
		return B75;
	case 110:
		return B110;
	case 134:
		return B134;
	case 150:
		return B150;
	case 200:
		return B200;
	case 300:
		return B300;
	case 600:
		return B600;
	case 1200:
		return B1200;
	case 1800:
		return B1800;
	case 2400:
		return B2400;
	case 4800:
		return B4800;
	case 9600:
		return B9600;
	case 19200:
		return B19200;
	case 38400:
		return B38400;
	case 57600:
		return B57600;
	case 115200:
		return B115200;
	case 230400:
		return B230400;
	case 460800:
		return B460800;
	case 500000:
		return B500000;
	case 576000:
		return B576000;
	case 921600:
		return B921600;
	case 1000000:
		return B1000000;
	case 1152000:
		return B1152000;
	case 1500000:
		return B1500000;
	case 2000000:
		return B2000000;
	case 2500000:
		return B2500000;
	case 3000000:
		return B3000000;
	case 3500000:
		return B3500000;
	case 4000000:
		return B4000000;
	default:
		return -1;
	}
}

static int serial_open(const char *device, uint32_t baudrate)
{
	struct termios options;
	int status, fd;

	if ((fd = open(device, O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK)) ==
	    -1) {
		perror("Error occurred while opening serial port !");
		return -1;
	}

	fcntl(fd, F_SETFL, O_RDWR);

	// Get and modify current options:

	tcgetattr(fd, &options);
	speed_t baud = convert_baudrate(baudrate);

	if (baud < 0) {
		printf("Invalid baudrate!\n");
		return -1;
	}

	cfmakeraw(&options);
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);

	options.c_cflag |= (CLOCAL | CREAD);
	options.c_cflag &= ~(PARENB | CSTOPB | CSIZE);
	options.c_cflag |= CS8;
	options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
	options.c_oflag &= ~OPOST;
	options.c_iflag &= ~(IXON | IXOFF | IXANY); // Turn off s/w flow ctrl
	options.c_iflag &=
		~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR |
		  ICRNL); // Disable any special handling of received bytes

	options.c_cc[VMIN] = 0;
	options.c_cc[VTIME] = 10; // 1 Second

	tcsetattr(fd, TCSANOW, &options);

	ioctl(fd, TIOCMGET, &status);

	status |= TIOCM_DTR;
	status |= TIOCM_RTS;

	ioctl(fd, TIOCMSET, &status);

	usleep(10000); // 10mS

	return fd;
}

static esp_loader_error_t change_baudrate(int file_desc, int baudrate)
{
	struct termios options;
	speed_t baud = convert_baudrate(baudrate);

	if (baud < 0) {
		return ESP_LOADER_ERROR_INVALID_PARAM;
	}

	tcgetattr(file_desc, &options);

	cfmakeraw(&options);
	cfsetispeed(&options, baud);
	cfsetospeed(&options, baud);

	tcsetattr(file_desc, TCSANOW, &options);

	return ESP_LOADER_SUCCESS;
}

static void set_timeout(uint32_t timeout)
{
	struct termios options;

	timeout /= 100;
	timeout = MAX(timeout, 1);

	tcgetattr(serial, &options);
	options.c_cc[VTIME] = timeout;
	tcsetattr(serial, TCSANOW, &options);
}

esp_loader_error_t linux_loader_port_init(const loader_linux_config_t *config)
{
	esp_loader_error_t retval;

	serial = serial_open(config->device, config->baudrate);
	if (serial < 0) {
		perror("Serial port could not be opened!");
		retval = ESP_LOADER_ERROR_FAIL;
		goto fail;
	}

	reset_trigger_chip =
		gpiod_chip_open_by_name(config->reset_trigger_chip);
	if (reset_trigger_chip == NULL) {
		perror("Reset trigger chip could not be opened!");
		retval = ESP_LOADER_ERROR_FAIL;
		goto fail_close_serial;
	}

	reset_trigger_line = gpiod_chip_get_line(reset_trigger_chip,
						 config->reset_trigger_line);
	if (reset_trigger_line == NULL) {
		perror("Could not get Reset trigger line");
		retval = ESP_LOADER_ERROR_FAIL;
		goto fail_close_reset_chip;
	}

	if (gpiod_line_request_output(reset_trigger_line, "esp-reset-pin",
				      SERIAL_FLASHER_RESET_INVERT ? 1 : 0) ==
	    -1) {
		perror("Failed to set reset trigger line to output");
		retval = ESP_LOADER_ERROR_FAIL;
		goto fail_release_reset_line;
	}

	bootsel_trigger_chip =
		gpiod_chip_open_by_name(config->bootsel_trigger_chip);
	if (bootsel_trigger_chip == NULL) {
		perror("Bootsel trigger chip could not be opened!");
		retval = ESP_LOADER_ERROR_FAIL;
		goto fail_release_reset_line;
	}

	bootsel_trigger_line = gpiod_chip_get_line(
		bootsel_trigger_chip, config->bootsel_trigger_line);
	if (bootsel_trigger_line == NULL) {
		perror("Could not get Bootsel trigger line");
		retval = ESP_LOADER_ERROR_FAIL;
		goto fail_close_bootsel_chip;
	}

	if (gpiod_line_request_output(bootsel_trigger_line, "esp-bootsel-pin",
				      SERIAL_FLASHER_BOOT_INVERT ? 0 : 1) ==
	    -1) {
		perror("Failed to set bootsel trigger line to output");
		retval = ESP_LOADER_ERROR_FAIL;
		goto fail_release_bootsel_line;
	}

	return ESP_LOADER_SUCCESS;

fail_release_bootsel_line:
	gpiod_line_release(bootsel_trigger_line);
	bootsel_trigger_line = NULL;

fail_close_bootsel_chip:
	gpiod_chip_close(bootsel_trigger_chip);
	bootsel_trigger_chip = NULL;

fail_release_reset_line:
	gpiod_line_release(reset_trigger_line);
	reset_trigger_line = NULL;

fail_close_reset_chip:
	gpiod_chip_close(reset_trigger_chip);
	reset_trigger_chip = NULL;

fail_close_serial:
	close(serial);
	serial = -1;

fail:
	return retval;
}

void linux_loader_port_deinit(void)
{
	if (serial > 0) {
		close(serial);
	}
	if (reset_trigger_line) {
		gpiod_line_release(reset_trigger_line);
	}
	if (reset_trigger_chip) {
		gpiod_chip_close(reset_trigger_chip);
	}
	if (bootsel_trigger_line) {
		gpiod_line_release(bootsel_trigger_line);
	}
	if (bootsel_trigger_chip) {
		gpiod_chip_close(bootsel_trigger_chip);
	}
}

static esp_loader_error_t read_byte(uint8_t *c, uint32_t timeout)
{
	set_timeout(timeout);
	int read_bytes = read(serial, c, 1);

	if (read_bytes == 1) {
		return ESP_LOADER_SUCCESS;
	} else if (read_bytes == 0) {
		return ESP_LOADER_ERROR_TIMEOUT;
	} else {
		return ESP_LOADER_ERROR_FAIL;
	}
}

static esp_loader_error_t read_data(uint8_t *buffer, uint32_t size)
{
	for (int i = 0; i < size; i++) {
		uint32_t remaining_time = loader_port_remaining_time();
		RETURN_ON_ERROR(read_byte(&buffer[i], remaining_time));
	}

	return ESP_LOADER_SUCCESS;
}

esp_loader_error_t loader_port_read(uint8_t *data, uint16_t size,
				    uint32_t timeout)
{
	RETURN_ON_ERROR(read_data(data, size));

#if SERIAL_FLASHER_DEBUG_TRACE
	transfer_debug_print(data, size, false);
#endif

	return ESP_LOADER_SUCCESS;
}

esp_loader_error_t loader_port_write(const uint8_t *data, uint16_t size,
				     uint32_t timeout)
{
	int written = write(serial, data, size);

	if (written < 0) {
		return ESP_LOADER_ERROR_FAIL;
	} else if (written < size) {
#if SERIAL_FLASHER_DEBUG_TRACE
		transfer_debug_print(data, written, true);
#endif
		return ESP_LOADER_ERROR_TIMEOUT;
	} else {
#if SERIAL_FLASHER_DEBUG_TRACE
		transfer_debug_print(data, written, true);
#endif
		return ESP_LOADER_SUCCESS;
	}
}

void loader_port_enter_bootloader(void)
{
	gpiod_line_set_value(bootsel_trigger_line,
			     SERIAL_FLASHER_BOOT_INVERT ? 1 : 0);
	loader_port_reset_target();
	loader_port_delay_ms(SERIAL_FLASHER_BOOT_HOLD_TIME_MS);
	gpiod_line_set_value(bootsel_trigger_line,
			     SERIAL_FLASHER_BOOT_INVERT ? 0 : 1);
}

void loader_port_reset_target(void)
{
	gpiod_line_set_value(reset_trigger_line,
			     SERIAL_FLASHER_RESET_INVERT ? 1 : 0);
	loader_port_delay_ms(SERIAL_FLASHER_RESET_HOLD_TIME_MS);
	gpiod_line_set_value(reset_trigger_line,
			     SERIAL_FLASHER_RESET_INVERT ? 0 : 1);
}

void loader_port_delay_ms(uint32_t ms)
{
	usleep(ms * 1000);
}

void loader_port_start_timer(uint32_t ms)
{
	s_time_end = clock() + (ms * (CLOCKS_PER_SEC / 1000));
}

uint32_t loader_port_remaining_time(void)
{
	int64_t remaining = (s_time_end - clock()) / 1000;
	return (remaining > 0) ? (uint32_t)remaining : 0;
}

void loader_port_debug_print(const char *str)
{
	printf("DEBUG: %s\n", str);
}

esp_loader_error_t loader_port_change_transmission_rate(uint32_t baudrate)
{
	return change_baudrate(serial, baudrate);
}