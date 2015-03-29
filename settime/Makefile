UNIXTIME := $(shell date +%s)

all:
	arm-none-eabi-gcc -Os -g -Wextra -Wshadow -Wimplicit-function-declaration -Wredundant-decls -Wmissing-prototypes -Wstrict-prototypes -fno-common -ffunction-sections -fdata-sections -MD -Wall -Wundef -I../libopencm3//include -DSTM32F1 -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd -o settime.o -c settime.c -D__COUNTERVAL__=$(UNIXTIME)
	arm-none-eabi-gcc --static -nostartfiles -L../libopencm3//lib -T../wortuhr.ld -Wl,-Map=settime.map -Wl,--gc-sections -mthumb -mcpu=cortex-m3 -msoft-float -mfix-cortex-m3-ldrd settime.o -lopencm3_stm32f1 -Wl,--start-group -lc -lgcc -lnosys -Wl,--end-group -o settime.elf
	arm-none-eabi-objcopy -Obinary settime.elf settime.bin
	st-flash write settime.bin 0x08000000

clean:
	rm -f settime.o settime.bin settime.elf settime.d settime.map
