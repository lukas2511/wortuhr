UNIXTIME ?= $(shell date +%s)

all: build

build: libopencm3/lib/stm32/f1/timer.o
	python generate_wortuhr_h.py
	arm-none-eabi-gcc -Os -g -Wextra -Wshadow -Wimplicit-function-declaration -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -fno-common -ffunction-sections -fdata-sections -MD -Wall -Wundef -I./libopencm3/include -DSTM32F1 -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd -o wortuhr.o -c wortuhr.c
	arm-none-eabi-gcc --static -nostartfiles -L./libopencm3/lib -Twortuhr.ld -Wl,-Map=wortuhr.map -Wl,--gc-sections -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd wortuhr.o -lopencm3_stm32f1 -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group -o wortuhr.elf
	arm-none-eabi-objcopy -Obinary wortuhr.elf wortuhr.bin

install: build
	make -C settime UNIXTIME=$(UNIXTIME)
	st-flash write wortuhr.bin 0x08000000

libopencm3/lib/stm32/f1/timer.o:
	git submodule init
	git submodule update
	make -C libopencm3 -j

clean:
	rm -f wortuhr.o wortuhr.h wortuhr.bin wortuhr.elf wortuhr.d wortuhr.map
	make -C settime clean
