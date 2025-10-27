TARGET = stm32_debounce
CC = arm-none-eabi-gcc
OBJCOPY = arm-none-eabi-objcopy

CFLAGS = -mcpu=cortex-m3 -mthumb -O2 -g -Wall
CFLAGS += -Tstm32f103.ld -nostdlib -ffreestanding

SRCS = main.c
OBJS = $(SRCS:.c=.o)

all: $(TARGET).bin

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET).elf: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@

$(TARGET).bin: $(TARGET).elf
	$(OBJCOPY) -O binary $< $@

flash: $(TARGET).bin
	@echo "=== KILL OLD QEMU ... ==="
	-pkill -9 qemu-stm32 2>/dev/null || true
	@sleep 1
	@echo "=== RUN QEMU ==="
	qemu-stm32 -M stm32-p103 -kernel $< -nographic \
		-serial tcp::1234,server,wait \
		-monitor null

clean:
	rm -f *.o *.elf *.bin

.PHONY: all flash clean