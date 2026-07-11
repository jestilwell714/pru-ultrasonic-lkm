# Target names based on your compilation command
PRU_OUT = pru0_firmware.out

# Your exact source files
ASM_SRC = pru_program_multiple_sensor.asm
C_SRC   = main.c

# TI PRU Code Generation Tools
CC = clpru

# Include paths from your command
INCLUDES = --include_path=/usr/lib/ti/pru-software-support-package-v6.0/include/am335x/ \
           --include_path=/usr/lib/ti/pru-software-support-package-v6.0/include/ \
           --include_path=/usr/share/ti/cgt-pru/include/ \
           --include_path=/usr/share/ti/cgt-pru/lib

# Libraries from your command
LIBS = --library=libc.a \
       --library=/usr/lib/ti/pru-software-support-package-v6.0/lib/rpmsg_lib.lib

.PHONY: all clean install stop start config_pins

# Running 'make' or 'make all' now just compiles the binary
all: $(PRU_OUT)

$(PRU_OUT): $(ASM_SRC) $(C_SRC)
	$(CC) $(ASM_SRC) $(C_SRC) $(INCLUDES) --run_linker $(LIBS) -o $(PRU_OUT)

# Your installation steps including the firmware name registration
install: $(PRU_OUT)
	-sudo rm -vf /lib/firmware/am335x-pru0-fw/*.out
	sudo cp -v $(PRU_OUT) /lib/firmware/am335x-pru0-fw
	echo 'am335x-pru0-fw' > /sys/class/remoteproc/remoteproc1/firmware

# Automatically configure hardware pins for PRU 0 multiplexing
config_pins:
	sudo config-pin P9_27 pruout
	sudo config-pin P9_29 pruin
	sudo config-pin P9_31 pruin
	sudo config-pin P9_25 pruout

# Stops the active PRU core instance
stop:
	sudo echo 'stop' > /sys/class/remoteproc/remoteproc1/state

# Configures the pins first, then boots the core instance
start: config_pins
	sudo echo 'start' > /sys/class/remoteproc/remoteproc1/state

# Your exact local cleanup routine
clean:
	rm -vf *.obj *.out