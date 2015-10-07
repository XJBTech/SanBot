# Sanbot Firmware

This project contains the source code for the sanbot. 

## Environment

### OS X

```
# install Homebrew(https://github.com/Homebrew/homebrew)
$ brew —version

# install dependencies
$ sudo brew install libusb libusb-compat
$ sudo brew install pkg-config
$ sudo brew install autoconf automake libtool

# install stlink
$ cd ~/
$ git clone https://github.com/texane/stlink stlink.git
$ cd stlink.git
$ ./autogen.sh
$ ./configure
$ make

# install gcc-arm-none-eabi
$ cd ~/
$ curl -O http://launchpad.net/gcc-arm-embedded/4.9/4.9-2015-q2-update/+download/gcc-arm-none-eabi-4_9-2015q2-20150609-mac.tar.bz2
$ tar jxvf gcc-arm-none-eabi-4_9-2015q2-20150609-mac.tar.bz2
$ mv gcc-arm-none-eabi-4_9-2015q2 gcc-arm-none-eabi

# PATH(replace yourusername)
echo "export PATH=/Users/yourusername/gcc-arm-none-eabi/bin:\$PATH" >> ~/.profile
echo "export PATH=/Users/yourusername/stlink.git:\$PATH" >> ~/.profile

# restart the terminal.
```

## Compiling

### config.mk
To create custom build options create a file called config.mk in the root folder 
(same as Makefile) and fill it with options. E.g. 
```
PLATFORM = ...
DEBUG = 1
```

## Folder description:
```
./              | Root, contains the Makefile
 + init         | Contains the main.c
 + config       | Configuration files
 + drivers      | Hardware driver layer
 |  + src       | Drivers source code
 |  + interface | Drivers header files. Interface to the HAL layer
 + hal          | Hardware abstaction layer
 |  + src       | HAL source code
 |  + interface | HAL header files. Interface with the other parts of the program
 + modules      | Firmware operating code and headers
 |  + src       | Firmware tasks source code and main.c
 |  + interface | Operating headers. Configure the firmware environement
 + utils        | Utils code. Implement utility block like the debug.
 |  + src       | Utils source code
 |  + interface | Utils header files. Interface with the other parts of the program
 + platform     | Platform specific files. Not really used yet
 + scripts      | Scripts for LD, OpenOCD, make, version control, ...
 + lib          | Libraries
 |  + STM32...  | Library folders of the ST STM32 peripheral libs
 |  + CMSIS     | Core abstraction layer
```

## Make targets:
```
all        : Shortcut for build
build      : compile
clean      : Clean every compiled files
# cload    :
flash      : Flash .bin using stlink
erase      : Erase the controller
```
