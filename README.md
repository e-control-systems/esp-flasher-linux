<!-- omit in toc -->
# ESP Flasher Linux

Linux utility to program ESP Family of microcontrollers

<!-- omit in toc -->
## Table of Contents

- [Build](#build)
  - [Prerequisites](#prerequisites)
  - [Generate Binary](#generate-binary)
  - [Install](#install)
  - [Create self extracting package](#create-self-extracting-package)
  - [Create Deb Package](#create-deb-package)
- [Usage](#usage)

## Build
### Prerequisites
* cmake
* gcc
* libgpiod (version 1.6)

### Generate Binary
```sh
cmake -B build
cmake --build build
```

### Install
```sh
# you may have to run it using sudo
cmake --install build
```

### Create self extracting package 
```sh
pushd build
cpack
popd
```

### Create Deb Package
```sh
pushd build
cpack -G DEB
popd
```

## Usage

```sh
usage: esp-flasher [-h] [-v] [-R GPIOCHIP] -r GPIOLINE [-B GPIOCHIP] 
                   -b GPIOLINE -d DEVICE [-s BAUDRATE]
                   <address> <filename> [<address> <filename> ...]

Linux utility to program ESP Family of microcontrollers

positional arguments:
  <address> <filename>  Address followed by binary filename, separated by space

options:
  -h, --help            show this help message and exit
  -v, --version         show program's version number and exit
  -R GPIOCHIP, --reset-chip GPIOCHIP
                        reset pin GPIO character device (default: gpiochip0)
  -r GPIOLINE, --reset-line GPIOLINE
                        reset pin GPIO line 
  -B GPIOCHIP, --bootsel-chip GPIOCHIP
                        boot select pin GPIO character device (default:
                        gpiochip0)
  -b GPIOLINE, --bootsel-line GPIOLINE
                        boot select pin GPIO line 
  -d DEVICE, --device DEVICE
                        serial device for sending file 
  -s BAUDRATE, --speed BAUDRATE
                        transmission rate to use after initial sync to speed
                        up transfers (default: 115200)
```

