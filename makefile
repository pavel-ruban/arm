# setup

COMPILE_OPTS = -mcpu=cortex-m3 -mthumb -Wall -g -O0 -DDEBUG -DUSE_FULL_ASSERT -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000
INCLUDE_DIRS = -I include -I system/include/cmsis -I system/include/stm32f1-stdperiph -I system/include
LIBRARY_DIRS = -L lib

GDB = arm-none-eabi-gdb
CC = arm-none-eabi-gcc
CFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS) -std=gnu11

CXX = arm-none-eabi-g++
CXXFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS) -std=gnu++11

AS = arm-none-eabi-gcc
ASFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS) -c

LD = arm-none-eabi-gcc

LDFLAGS = -Wl,--gc-sections,-Map=$@.map,-cref,-u,Reset_Handler $(INCLUDE_DIRS) $(LIBRARY_DIRS) -T mem.ld -T sections.ld -L ldscripts -nostartfiles

OBJCP = arm-none-eabi-objcopy
OBJCPFLAGS = -O binary

AR = arm-none-eabi-ar
ARFLAGS = cr

FIRMWARE = firmware
FIRMWARE_ELF = $(FIRMWARE).elf
FIRMWARE_BIN = $(FIRMWARE).bin

# all
all: $(FIRMWARE_ELF) $(FIRMWARE_BIN)

NEWLIB_OUT = lib/newlib.a
LIBSTM32_OUT = lib/libstm32.a

# main
$(FIRMWARE_ELF): c_entry.o entry.o $(LIBSTM32_OUT) $(NEWLIB_OUT)
	$(LD) $(LDFLAGS) c_entry.o entry.o $(LIBSTM32_OUT) $(NEWLIB_OUT) --output $@

$(FIRMWARE_BIN): $(FIRMWARE_ELF)
	$(OBJCP) $(OBJCPFLAGS) $< $@

# flash
flash: $(FIRMWARE_BIN)
	st-flash erase
	st-flash --reset write $(FIRMWARE_BIN) 0x08000000
	st-info --descr

gdbsrv:
	st-util

debug:
	$(GDB)  -ex 'set confirm off' -ex 'target remote :4242' -ex 'file $(FIRMWARE_ELF)'

# newlib.a
NEWLIB_DIR = system/src/newlib

NEWLIB_OBJS = 					\
	$(NEWLIB_DIR)/assert.o			\
	$(NEWLIB_DIR)/_exit.o			\
	$(NEWLIB_DIR)/_sbrk.o			\
	$(NEWLIB_DIR)/_startup.o		\
	$(NEWLIB_DIR)/_syscalls.o

$(NEWLIB_OUT): $(NEWLIB_OBJS)
	$(AR) $(ARFLAGS) $@ $(NEWLIB_OBJS)

# libstm32.a
STD_PERIPH_DIR = system/src/stm32f1-stdperiph

LIBSTM32_OBJS =					\
	$(STD_PERIPH_DIR)/stm32f10x_adc.o	\
	$(STD_PERIPH_DIR)/stm32f10x_bkp.o	\
	$(STD_PERIPH_DIR)/stm32f10x_can.o	\
	$(STD_PERIPH_DIR)/stm32f10x_cec.o	\
	$(STD_PERIPH_DIR)/stm32f10x_crc.o	\
	$(STD_PERIPH_DIR)/stm32f10x_dac.o	\
	$(STD_PERIPH_DIR)/stm32f10x_dbgmcu.o	\
	$(STD_PERIPH_DIR)/stm32f10x_dma.o	\
	$(STD_PERIPH_DIR)/stm32f10x_exti.o	\
	$(STD_PERIPH_DIR)/stm32f10x_flash.o	\
	$(STD_PERIPH_DIR)/stm32f10x_fsmc.o	\
	$(STD_PERIPH_DIR)/stm32f10x_gpio.o	\
	$(STD_PERIPH_DIR)/stm32f10x_i2c.o	\
	$(STD_PERIPH_DIR)/stm32f10x_iwdg.o	\
	$(STD_PERIPH_DIR)/stm32f10x_pwr.o	\
	$(STD_PERIPH_DIR)/stm32f10x_rcc.o	\
	$(STD_PERIPH_DIR)/stm32f10x_rtc.o	\
	$(STD_PERIPH_DIR)/stm32f10x_sdio.o	\
	$(STD_PERIPH_DIR)/stm32f10x_spi.o	\
	$(STD_PERIPH_DIR)/stm32f10x_tim.o	\
	$(STD_PERIPH_DIR)/stm32f10x_usart.o	\
	$(STD_PERIPH_DIR)/misc.o		\
	$(STD_PERIPH_DIR)/stm32f10x_wwdg.o

$(LIBSTM32_OBJS): include/stm32f10x_conf.h

$(LIBSTM32_OUT): $(LIBSTM32_OBJS)
	$(AR) $(ARFLAGS) $@ $(LIBSTM32_OBJS)


clean:
	-rm *.o $(STD_PERIPH_DIR)/*.o /src/*.o $(NEWLIB_OUT) $(LIBSTM32_OUT) $(FIRMWARE_ELF) $(FIRMWARE_BIN) *.map
