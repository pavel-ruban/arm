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
.org	0x68
.word	EXTI4_IRQHandler
.org	0xDC
.word	WRONG_IRQ_EXCEPTION
.org	0xE0
.word	EXTI15_10_IRQHandler


@ End of the interrupt vector table.
.org	253

.type EXTI15_10_IRQHandler "function"
.type EXTI4_IRQHandler "function"
.type WRONG_IRQ_EXCEPTION "function"

nmi_exception:
	b	nmi_exception		@ Infinite loop in case of hardware exception.
hard_fault_exception:
	b	hard_fault_exception		@ Infinite loop in case of hardware exception.
mem_manage_exception:
	b	mem_manage_exception		@ Infinite loop in case of hardware exception.
bus_fault_exception:
	b	bus_fault_exception		@ Infinite loop in case of hardware exception.
usage_fault_exception:
	b	usage_fault_exception		@ Infinite loop in case of hardware exception.
wrong_irq_exception:
	b	wrong_irq_exception		@ Infinite loop in case of hardware exception.

