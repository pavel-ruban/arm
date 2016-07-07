	.syntax unified
	.cpu cortex-m3
	.fpu softvfp
	.eabi_attribute 20, 1
	.eabi_attribute 21, 1
	.eabi_attribute 23, 3
	.eabi_attribute 24, 1
	.eabi_attribute 25, 1
	.eabi_attribute 26, 1
	.eabi_attribute 30, 2
	.eabi_attribute 34, 1
	.eabi_attribute 18, 4
	.thumb
	.file	"memset.c"
	.text
.Ltext0:
	.cfi_sections	.debug_frame
	.align	2
	.global	memset
	.thumb
	.thumb_func
	.type	memset, %function
memset:
.LFB0:
	.file 1 "/var/tmp/portage/cross-arm-cortexm3-eabi/newlib-2.0.0/work/newlib-2.0.0/newlib/libc/string/memset.c"
	.loc 1 47 0
	.cfi_startproc
	@ args = 0, pretend = 0, frame = 0
	@ frame_needed = 0, uses_anonymous_args = 0
.LVL0:
	push	{r3, r4, r5, r6, r7, lr}
	.cfi_def_cfa_offset 24
	.cfi_offset 3, -24
	.cfi_offset 4, -20
	.cfi_offset 5, -16
	.cfi_offset 6, -12
	.cfi_offset 7, -8
	.cfi_offset 14, -4
	.loc 1 47 0
	mov	r7, r0
.LVL1:
	.loc 1 57 0
	lsls	r0, r7, #30
.LVL2:
	beq	.L16
	.loc 1 59 0
	subs	r4, r2, #1
.LVL3:
	cmp	r2, #0
	beq	.L18
	uxtb	r5, r1
	mov	r3, r7
	b	.L4
.LVL4:
.L6:
	subs	r2, r4, #1
.LVL5:
	cmp	r4, #0
	beq	.L18
	mov	r4, r2
.LVL6:
.L4:
	.loc 1 60 0
	strb	r5, [r3], #1
.LVL7:
	.loc 1 57 0
	lsls	r2, r3, #30
	bne	.L6
.L2:
	.loc 1 65 0
	cmp	r4, #3
	bls	.L14
	.loc 1 54 0
	uxtb	r5, r1
.LVL8:
	.loc 1 72 0
	orr	r5, r5, r5, lsl #8
.LVL9:
	.loc 1 78 0
	cmp	r4, #15
	.loc 1 73 0
	orr	r5, r5, r5, lsl #16
.LVL10:
	.loc 1 78 0
	bls	.L13
	sub	r6, r4, #16
	lsrs	r6, r6, #4
	add	r0, r3, #16
	add	r0, r0, r6, lsl #4
	mov	r2, r3
.LVL11:
.L12:
	.loc 1 80 0
	str	r5, [r2]
.LVL12:
	.loc 1 81 0
	str	r5, [r2, #4]
.LVL13:
	.loc 1 82 0
	str	r5, [r2, #8]
.LVL14:
	.loc 1 83 0
	str	r5, [r2, #12]
	adds	r2, r2, #16
.LVL15:
	.loc 1 78 0
	cmp	r2, r0
	bne	.L12
	and	r4, r4, #15
	adds	r6, r6, #1
	.loc 1 87 0 discriminator 1
	cmp	r4, #3
	add	r3, r3, r6, lsl #4
.LVL16:
	bls	.L14
.L13:
	.loc 1 57 0
	mov	r6, r3
	mov	r2, r4
.LVL17:
.L10:
	.loc 1 90 0
	subs	r2, r2, #4
.LVL18:
	.loc 1 87 0
	cmp	r2, #3
	.loc 1 89 0
	str	r5, [r6], #4
.LVL19:
	.loc 1 87 0
	bhi	.L10
	subs	r2, r4, #4
.LVL20:
	bic	r2, r2, #3
	adds	r2, r2, #4
	add	r3, r3, r2
	and	r4, r4, #3
.LVL21:
.L14:
	.loc 1 98 0 discriminator 1
	cbz	r4, .L18
	mov	r0, r3
	uxtb	r1, r1
.LVL22:
	mov	r2, r4
	bl	memset
.LVL23:
.L18:
	.loc 1 102 0
	mov	r0, r7
	pop	{r3, r4, r5, r6, r7, pc}
.LVL24:
.L16:
	.loc 1 57 0
	mov	r4, r2
	mov	r3, r7
	b	.L2
	.cfi_endproc
.LFE0:
	.size	memset, .-memset
.Letext0:
	.file 2 "/usr/lib/gcc/arm-cortexm3-eabi/4.8.2/include/stddef.h"
	.file 3 "/var/tmp/portage/cross-arm-cortexm3-eabi/newlib-2.0.0/work/newlib-2.0.0/newlib/libc/include/string.h"
	.file 4 "<built-in>"
	.section	.debug_info,"",%progbits
.Ldebug_info0:
	.4byte	0x140
	.2byte	0x4
	.4byte	.Ldebug_abbrev0
	.byte	0x4
	.uleb128 0x1
	.4byte	.LASF13
	.byte	0x1
	.4byte	.LASF14
	.4byte	.Ltext0
	.4byte	.Letext0-.Ltext0
	.4byte	.Ldebug_line0
	.uleb128 0x2
	.byte	0x1
	.byte	0x6
	.4byte	.LASF0
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF1
	.uleb128 0x2
	.byte	0x2
	.byte	0x5
	.4byte	.LASF2
	.uleb128 0x2
	.byte	0x2
	.byte	0x7
	.4byte	.LASF3
	.uleb128 0x3
	.byte	0x4
	.byte	0x5
	.ascii	"int\000"
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF4
	.uleb128 0x2
	.byte	0x8
	.byte	0x5
	.4byte	.LASF5
	.uleb128 0x2
	.byte	0x8
	.byte	0x7
	.4byte	.LASF6
	.uleb128 0x2
	.byte	0x4
	.byte	0x5
	.4byte	.LASF7
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF8
	.uleb128 0x4
	.byte	0x4
	.uleb128 0x2
	.byte	0x4
	.byte	0x7
	.4byte	.LASF9
	.uleb128 0x5
	.byte	0x4
	.4byte	0x76
	.uleb128 0x2
	.byte	0x1
	.byte	0x8
	.4byte	.LASF10
	.uleb128 0x6
	.4byte	.LASF15
	.byte	0x2
	.byte	0xd4
	.4byte	0x44
	.uleb128 0x7
	.4byte	.LASF16
	.byte	0x3
	.byte	0x19
	.4byte	0x67
	.4byte	.LFB0
	.4byte	.LFE0-.LFB0
	.uleb128 0x1
	.byte	0x9c
	.4byte	0x11e
	.uleb128 0x8
	.ascii	"m\000"
	.byte	0x1
	.byte	0x2b
	.4byte	0x67
	.4byte	.LLST0
	.uleb128 0x8
	.ascii	"c\000"
	.byte	0x1
	.byte	0x2b
	.4byte	0x3d
	.4byte	.LLST1
	.uleb128 0x8
	.ascii	"n\000"
	.byte	0x1
	.byte	0x2b
	.4byte	0x7d
	.4byte	.LLST2
	.uleb128 0x9
	.ascii	"s\000"
	.byte	0x1
	.byte	0x30
	.4byte	0x70
	.4byte	.LLST3
	.uleb128 0x9
	.ascii	"i\000"
	.byte	0x1
	.byte	0x33
	.4byte	0x3d
	.4byte	.LLST4
	.uleb128 0xa
	.4byte	.LASF11
	.byte	0x1
	.byte	0x34
	.4byte	0x69
	.4byte	.LLST5
	.uleb128 0xa
	.4byte	.LASF12
	.byte	0x1
	.byte	0x35
	.4byte	0x11e
	.4byte	.LLST6
	.uleb128 0x9
	.ascii	"d\000"
	.byte	0x1
	.byte	0x36
	.4byte	0x44
	.4byte	.LLST7
	.uleb128 0xb
	.4byte	.LVL23
	.4byte	0x124
	.uleb128 0xc
	.uleb128 0x1
	.byte	0x52
	.uleb128 0x2
	.byte	0x74
	.sleb128 0
	.byte	0
	.byte	0
	.uleb128 0x5
	.byte	0x4
	.4byte	0x69
	.uleb128 0xd
	.4byte	.LASF17
	.byte	0x4
	.byte	0
	.4byte	.LASF16
	.4byte	0x67
	.uleb128 0xe
	.4byte	0x67
	.uleb128 0xe
	.4byte	0x3d
	.uleb128 0xe
	.4byte	0x44
	.byte	0
	.byte	0
	.section	.debug_abbrev,"",%progbits
.Ldebug_abbrev0:
	.uleb128 0x1
	.uleb128 0x11
	.byte	0x1
	.uleb128 0x25
	.uleb128 0xe
	.uleb128 0x13
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x10
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x2
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0xe
	.byte	0
	.byte	0
	.uleb128 0x3
	.uleb128 0x24
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x3e
	.uleb128 0xb
	.uleb128 0x3
	.uleb128 0x8
	.byte	0
	.byte	0
	.uleb128 0x4
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.byte	0
	.byte	0
	.uleb128 0x5
	.uleb128 0xf
	.byte	0
	.uleb128 0xb
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x6
	.uleb128 0x16
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x7
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x12
	.uleb128 0x6
	.uleb128 0x40
	.uleb128 0x18
	.uleb128 0x2117
	.uleb128 0x19
	.uleb128 0x1
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0x8
	.uleb128 0x5
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0x9
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0x8
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xa
	.uleb128 0x34
	.byte	0
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x2
	.uleb128 0x17
	.byte	0
	.byte	0
	.uleb128 0xb
	.uleb128 0x4109
	.byte	0x1
	.uleb128 0x11
	.uleb128 0x1
	.uleb128 0x31
	.uleb128 0x13
	.byte	0
	.byte	0
	.uleb128 0xc
	.uleb128 0x410a
	.byte	0
	.uleb128 0x2
	.uleb128 0x18
	.uleb128 0x2111
	.uleb128 0x18
	.byte	0
	.byte	0
	.uleb128 0xd
	.uleb128 0x2e
	.byte	0x1
	.uleb128 0x3f
	.uleb128 0x19
	.uleb128 0x3
	.uleb128 0xe
	.uleb128 0x3a
	.uleb128 0xb
	.uleb128 0x3b
	.uleb128 0xb
	.uleb128 0x6e
	.uleb128 0xe
	.uleb128 0x27
	.uleb128 0x19
	.uleb128 0x49
	.uleb128 0x13
	.uleb128 0x3c
	.uleb128 0x19
	.byte	0
	.byte	0
	.uleb128 0xe
	.uleb128 0x5
	.byte	0
	.uleb128 0x49
	.uleb128 0x13
	.byte	0
	.byte	0
	.byte	0
	.section	.debug_loc,"",%progbits
.Ldebug_loc0:
.LLST0:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL2-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL2-.Ltext0
	.4byte	.LFE0-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	0
	.4byte	0
.LLST1:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL22-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	.LVL22-.Ltext0
	.4byte	.LVL24-.Ltext0
	.2byte	0x4
	.byte	0xf3
	.uleb128 0x1
	.byte	0x51
	.byte	0x9f
	.4byte	.LVL24-.Ltext0
	.4byte	.LFE0-.Ltext0
	.2byte	0x1
	.byte	0x51
	.4byte	0
	.4byte	0
.LLST2:
	.4byte	.LVL0-.Ltext0
	.4byte	.LVL3-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL3-.Ltext0
	.4byte	.LVL5-.Ltext0
	.2byte	0x1
	.byte	0x54
	.4byte	.LVL5-.Ltext0
	.4byte	.LVL6-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL6-.Ltext0
	.4byte	.LVL11-.Ltext0
	.2byte	0x1
	.byte	0x54
	.4byte	.LVL16-.Ltext0
	.4byte	.LVL17-.Ltext0
	.2byte	0x1
	.byte	0x54
	.4byte	.LVL17-.Ltext0
	.4byte	.LVL18-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL18-.Ltext0
	.4byte	.LVL19-.Ltext0
	.2byte	0x3
	.byte	0x72
	.sleb128 4
	.byte	0x9f
	.4byte	.LVL19-.Ltext0
	.4byte	.LVL20-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL21-.Ltext0
	.4byte	.LVL23-.Ltext0
	.2byte	0x3
	.byte	0x74
	.sleb128 -1
	.byte	0x9f
	.4byte	.LVL24-.Ltext0
	.4byte	.LFE0-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	0
	.4byte	0
.LLST3:
	.4byte	.LVL1-.Ltext0
	.4byte	.LVL2-.Ltext0
	.2byte	0x1
	.byte	0x50
	.4byte	.LVL2-.Ltext0
	.4byte	.LVL4-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	.LVL4-.Ltext0
	.4byte	.LVL16-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL21-.Ltext0
	.4byte	.LVL23-1-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL24-.Ltext0
	.4byte	.LFE0-.Ltext0
	.2byte	0x1
	.byte	0x57
	.4byte	0
	.4byte	0
.LLST4:
	.4byte	.LVL10-.Ltext0
	.4byte	.LVL21-.Ltext0
	.2byte	0x3
	.byte	0x8
	.byte	0x20
	.byte	0x9f
	.4byte	0
	.4byte	0
.LLST5:
	.4byte	.LVL9-.Ltext0
	.4byte	.LVL21-.Ltext0
	.2byte	0x1
	.byte	0x55
	.4byte	0
	.4byte	0
.LLST6:
	.4byte	.LVL8-.Ltext0
	.4byte	.LVL11-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL11-.Ltext0
	.4byte	.LVL12-.Ltext0
	.2byte	0x3
	.byte	0x72
	.sleb128 4
	.byte	0x9f
	.4byte	.LVL12-.Ltext0
	.4byte	.LVL13-.Ltext0
	.2byte	0x3
	.byte	0x72
	.sleb128 8
	.byte	0x9f
	.4byte	.LVL13-.Ltext0
	.4byte	.LVL14-.Ltext0
	.2byte	0x3
	.byte	0x72
	.sleb128 12
	.byte	0x9f
	.4byte	.LVL14-.Ltext0
	.4byte	.LVL15-.Ltext0
	.2byte	0x3
	.byte	0x72
	.sleb128 16
	.byte	0x9f
	.4byte	.LVL15-.Ltext0
	.4byte	.LVL16-.Ltext0
	.2byte	0x1
	.byte	0x52
	.4byte	.LVL16-.Ltext0
	.4byte	.LVL17-.Ltext0
	.2byte	0x1
	.byte	0x53
	.4byte	.LVL17-.Ltext0
	.4byte	.LVL21-.Ltext0
	.2byte	0x1
	.byte	0x56
	.4byte	0
	.4byte	0
.LLST7:
	.4byte	.LVL1-.Ltext0
	.4byte	.LVL22-.Ltext0
	.2byte	0x6
	.byte	0x71
	.sleb128 0
	.byte	0x8
	.byte	0xff
	.byte	0x1a
	.byte	0x9f
	.4byte	.LVL22-.Ltext0
	.4byte	.LVL24-.Ltext0
	.2byte	0x7
	.byte	0xf3
	.uleb128 0x1
	.byte	0x51
	.byte	0x8
	.byte	0xff
	.byte	0x1a
	.byte	0x9f
	.4byte	.LVL24-.Ltext0
	.4byte	.LFE0-.Ltext0
	.2byte	0x6
	.byte	0x71
	.sleb128 0
	.byte	0x8
	.byte	0xff
	.byte	0x1a
	.byte	0x9f
	.4byte	0
	.4byte	0
	.section	.debug_aranges,"",%progbits
	.4byte	0x1c
	.2byte	0x2
	.4byte	.Ldebug_info0
	.byte	0x4
	.byte	0
	.2byte	0
	.2byte	0
	.4byte	.Ltext0
	.4byte	.Letext0-.Ltext0
	.4byte	0
	.4byte	0
	.section	.debug_line,"",%progbits
.Ldebug_line0:
	.section	.debug_str,"MS",%progbits,1
.LASF17:
	.ascii	"__builtin_memset\000"
.LASF3:
	.ascii	"short unsigned int\000"
.LASF15:
	.ascii	"size_t\000"
.LASF14:
	.ascii	"/var/tmp/portage/cross-arm-cortexm3-eabi/newlib-2.0"
	.ascii	".0/work/newlib-2.0.0/newlib/libc/string/memset.c\000"
.LASF13:
	.ascii	"GNU C 4.8.2 -fpreprocessed -mcpu=cortex-m3 -mfloat-"
	.ascii	"abi=soft -mthumb -g -O2 -O3 -std=gnu99 -fno-builtin"
	.ascii	" -fomit-frame-pointer\000"
.LASF0:
	.ascii	"signed char\000"
.LASF9:
	.ascii	"long unsigned int\000"
.LASF6:
	.ascii	"long long unsigned int\000"
.LASF12:
	.ascii	"aligned_addr\000"
.LASF1:
	.ascii	"unsigned char\000"
.LASF10:
	.ascii	"char\000"
.LASF7:
	.ascii	"long int\000"
.LASF5:
	.ascii	"long long int\000"
.LASF4:
	.ascii	"unsigned int\000"
.LASF2:
	.ascii	"short int\000"
.LASF16:
	.ascii	"memset\000"
.LASF11:
	.ascii	"buffer\000"
.LASF8:
	.ascii	"sizetype\000"
	.ident	"GCC: (Gentoo 4.8.2 p1.0, pie-0.5.8) 4.8.2"
