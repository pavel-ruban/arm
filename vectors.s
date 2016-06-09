.thumb
.section .vectors
.org	0x0

.word	_estack
.word	reset_asm
.word	nmi_exception
.word	hard_fault_exception
.word	mem_manage_exception
.word	bus_fault_exception
.word	usage_fault_exception
.word	0, 0, 0, 0			@ Reserved addresses.


@ End of the interrupt vector table.
.org	253

nmi_exception:
	bl	nmi_exception		@ Infinite loop in case of hardware exception.
hard_fault_exception:
	bl	hard_fault_exception		@ Infinite loop in case of hardware exception.
mem_manage_exception:
	bl	mem_manage_exception		@ Infinite loop in case of hardware exception.
bus_fault_exception:
	bl	bus_fault_exception		@ Infinite loop in case of hardware exception.
usage_fault_exception:
	bl	usage_fault_exception		@ Infinite loop in case of hardware exception.

