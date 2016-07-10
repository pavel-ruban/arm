## ARM-NONE-EABI-GCC STM32F103C8 RC522 ENC28J60

**Author**  - Pavel Ruban http://pavelruban.org

This sources contain my and other people work (STM32 (CMSIS, headers), lifelover (great tutorial for ethernet ip / tcp and other stacks), and etc).

**STM32f103c8t6 MCU** sources for arm-none-eabi-gcc toolchain
**STM32 CMSIS libraray**, **newlib** stubs, support for -std=gnu++11 (classes, auto, templates, without dynamical memory allocation and standard c, c++ libraries)

**ASM** implementations of **memcpy** / **memset**

**Makefile** build system

### C++ Queue Template
Implemented queue with start / end pointers, with iterators and STL like interface, it used for cache purposes and other dynamically stored stuff, it avoids use of dynamic allocation as it impacts real time system solutions, but using it's template allows allocate array with x items of T type, and use push_back(), end(), first(), ++it etc interface **@see include/queue.h**.

### SPI IRQ RC522, ENC28j60 Drivers

**RC522** has receive interrupt, but as it's triggered only when PICC (external NFC tag) answers and according to **ISO-1443** we have to periodically send by EM field data to PICC (REQALL) otherwise device won't response. So it uses RC522 internal timer with interrupt, every 40ms timer up interrupt, then host resends REQALL command, and returns control to programm, after receive interrupt is up, that allows use IRQ driven interrupt interface.

**ENC28j60** uses dhcp, tcp, upd, arp, ip stacks (thanks to easyelectronics.ru:@lifelover). It's a bit changed, also fixed some bugs (dhcpd filter flow, port headers sorting, etc).

Also here you can find examples of STM32 CMSIS GPIO remapp, several spi configuration, timer interrupts, exti IRQ lines configuring, hybrid asm\c\c++ sources, interrupt table in separate assembler file, custom asm entry point.
