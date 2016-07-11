# setup
TARGET_OPTS = -mcpu=cortex-m3 -mthumb
COMPILE_OPTS = $(TARGET_OPTS) -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -Wall
INCLUDE_DIRS = -I include -I system/include/cmsis -I system/include/stm32f1-stdperiph -I system/include -I binds/include -I /usr/local/include/include -I lib/ethernet -I lib/enc28j60

LIBS_INCLUDE_DIRS = -I $(HARDWARE_LIBS_DIR)/rc522
INCLUDE_DIRS += $(LIBS_INCLUDE_DIRS)

LIBRARY_DIRS = -L lib -L /usr/local/lib/

LIBSTDC := $(shell arm-none-eabi-gcc $(TARGET_OPTS) -print-file-name=libgcov.a)

GDB = arm-none-eabi-gdb
CC = arm-none-eabi-gcc
CFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS) -std=gnu11

CXX = arm-none-eabi-g++
CXXFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS) -std=gnu++11 -fno-rtti -fno-exceptions

AS = arm-none-eabi-gcc
ASFLAGS = $(COMPILE_OPTS) $(INCLUDE_DIRS) -c

LD = arm-none-eabi-gcc

LDFLAGS = -Wl,--gc-sections,-Map=$@.map,-cref,-u,Reset_Handler $(INCLUDE_DIRS) $(LIBRARY_DIRS) -T mem.ld -T sections.ld -L ldscripts -nostartfiles -nodefaultlibs  -nostdlib

OBJCP = arm-none-eabi-objcopy
OBJCPFLAGS = -O binary

AR = arm-none-eabi-ar
ARFLAGS = cr

FIRMWARE = firmware
FIRMWARE_ELF = $(FIRMWARE).elf
FIRMWARE_BIN = $(FIRMWARE).bin

# all
all: $(FIRMWARE_ELF) $(FIRMWARE_BIN)

.PHONY: d
.PHONY: o
.PHONY: ctags

ctags:
	find . -type f -regex '.*\.\(s\|c\|cpp\|S\|h\|hpp\)' -follow | xargs ctags

d: COMPILE_OPTS += -gdwarf-2 -g3 -O0 -DDEBUG -DUSE_FULL_ASSERT
d: all

o: COMPILE_OPTS += -O2
o: all

# Archives
BINDS_DIR = binds
NEWLIB_DIR = system/src/newlib
STD_PERIPH_DIR = system/src/stm32f1-stdperiph
HARDWARE_LIBS_DIR = lib

NEWLIB_OUT = lib/newlib.a
LIBSTM32_OUT = lib/libstm32.a
BINDS_OUT = $(BINDS_DIR)/binds.a
HARDWARE_LIBS_OUT = $(HARDWARE_LIBS_DIR)/hardware_libs.a

FIRMWARE_OBJS = c_entry.o entry.o newlib_stubs.o
STDLIB_OBJS = memcpy-armv7m.o stdlibc.o

# main
$(FIRMWARE_ELF): $(FIRMWARE_OBJS) $(BINDS_OUT) $(LIBSTM32_OUT) $(NEWLIB_OUT) $(HARDWARE_LIBS_OUT) $(STDLIB_OBJS)
	$(LD) $(LDFLAGS) c_entry.o entry.o $(HARDWARE_LIBS_OUT) $(BINDS_OUT) $(LIBSTM32_OUT) $(NEWLIB_OUT) newlib_stubs.o $(STDLIB_OBJS) --output $@

$(FIRMWARE_BIN): $(FIRMWARE_ELF)
	$(OBJCP) $(OBJCPFLAGS) $< $@

# flash
flash: $(FIRMWARE_BIN)
	st-flash erase
	st-flash --reset write $(FIRMWARE_BIN) 0x08000000
	st-info --descr

.PHONY: gdbsrv

.PHONY: size

.PHONY: debug

.PHONY: fsize

gdbsrv:
	st-util
	#st-util > /dev/pts/5 2>&1 &

fsize:
	arm-none-eabi-nm --print-size --size-sort --radix=d $(FIRMWARE_ELF)

size:
	arm-none-eabi-size -A -d $(FIRMWARE_ELF)

debug:
	$(GDB)  -ex 'set confirm off' -iex 'target remote :4242' -iex 'file $(FIRMWARE_ELF)'

# newlib.a
NEWLIB_OBJS = 					\
	$(NEWLIB_DIR)/assert.o			\
	$(NEWLIB_DIR)/_exit.o			\
	$(NEWLIB_DIR)/_sbrk.o			\
	$(NEWLIB_DIR)/_startup.o		\
	$(NEWLIB_DIR)/_syscalls.o

$(NEWLIB_OUT): $(NEWLIB_OBJS)
	$(AR) $(ARFLAGS) $@ $(NEWLIB_OBJS)

# libstm32.a
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

BINDS_OBJS = 				\
	$(BINDS_DIR)/spi_binds.o	\
	$(BINDS_DIR)/rc522_binds.o	\
	$(BINDS_DIR)/ethernet_binds.o

$(BINDS_OUT): $(BINDS_OBJS)
	$(AR) $(ARFLAGS) $@ $(BINDS_OBJS)

HARDWARE_LIBS_OBJS = 					\
	$(HARDWARE_LIBS_DIR)/rc522/mfrc522.o		\
	$(HARDWARE_LIBS_DIR)/ethernet/arp.o		\
	$(HARDWARE_LIBS_DIR)/ethernet/ethernet.o	\
	$(HARDWARE_LIBS_DIR)/ethernet/icmp.o		\
	$(HARDWARE_LIBS_DIR)/ethernet/ip.o		\
	$(HARDWARE_LIBS_DIR)/ethernet/tcp.o		\
	$(HARDWARE_LIBS_DIR)/ethernet/dhcp.o		\
	$(HARDWARE_LIBS_DIR)/ethernet/udp.o		\
	$(HARDWARE_LIBS_DIR)/ethernet/dns.o		\
	$(HARDWARE_LIBS_DIR)/enc28j60/enc28j60.o

$(HARDWARE_LIBS_OUT): $(HARDWARE_LIBS_OBJS)
	$(AR) $(ARFLAGS) $@ $(HARDWARE_LIBS_OBJS)


# @todo integrate autodependencies to this makefile
#BUILD_DIRS := $(BINDS_DIR) $(STD_PERIPH_DIR) $(HARDWARE_LIBS_DIR) $(NEWLIB_DIR) .
#
#DEPDIR := .d
#$(shell mkdir -p $(DEPDIR) > /dev/null)
#DEPFLAGS = -MT $@ -MMD -MP -MF $(DEPDIR)/$(notdir $*).Td
#
#COMPILE.c = $(CC) $(DEPFLAGS) $(CFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
#COMPILE.cc = $(CXX) $(DEPFLAGS) $(CXXFLAGS) $(CPPFLAGS) $(TARGET_ARCH) -c
#POSTCOMPILE = mv -f $(DEPDIR)/$$*.Td $(DEPDIR)/$$*.d
#
#% : %.c
#% : %.c $(DEPDIR)/%.d
#	$(COMPILE.c) $(OUTPUT_OPTION) $<
#	$(POSTCOMPILE)
#
#$(OBJDIR)/%.o : %.c
#$(OBJDIR)/%.o : %.c $(DEPDIR)/%.d
#	$(COMPILE.c) $(OUTPUT_OPTION) $<
#	$(POSTCOMPILE)

#define make-cpp-goal
#$1/%.o : %.cpp
#$1/%.o : %.cpp $(DEPDIR)/%.d
#	$(COMPILE.cc) $(OUTPUT_OPTION) $$*.o $$<
#	$(POSTCOMPILE)
#endef
#
#$(DEPDIR)/%.d: ;
#.PRECIOUS: $(DEPDIR)/%.d

#$(foreach bdir,$(BUILD_DIRS),$(eval $(call make-c-goal,$(bdir))))
#$(foreach bdir,$(BUILD_DIRS),$(eval $(call make-cpp-goal,$(bdir))))

#ALL_OBJS := $(HARDWARE_LIBS_OBJS) $(BINDS_OBJS) $(LIBSTM32_OBJS) $(NEWLIB_OBJS) $(FIRMWARE_OBJS) $(STD_LIB_OBJS)
#
#-include $(patsubst %,$(DEPDIR)/%.d,$(basename $(ALL_OBJS)))

clean:
	-find  -type f -a \( -name '*.o' -o -name '*.d'  -o -name '*.Td' \) -exec rm "{}" \;
	-rm $(BINDS_OUT) $(HARDWARE_LIBS_OUT) $(NEWLIB_OUT) $(LIBSTM32_OUT) $(FIRMWARE_ELF) $(FIRMWARE_BIN) *.map
