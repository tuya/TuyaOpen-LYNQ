	.syntax	unified
	.arch armv7-m
	.cpu cortex-m3

.global	__isr_vector
.global	Reset_Handler

.global	__StackTop
.global	__StackLimit

	.section .stack
	.align	3
#ifdef __STACK_SIZE
	.equ	Stack_Size, __STACK_SIZE
#else
	.equ	Stack_Size, 0x1000
#endif
__StackLimit:
	.space	Stack_Size
	.size	__StackLimit, . - __StackLimit
__StackTop:
	.size	__StackTop, . - __StackTop

	.section .isr_vector,"a",%progbits
	.align	2
	.type  __isr_vector, %object
	.size __isr_vector, .- __isr_vector
__isr_vector:
	.long	__StackTop            /* Top of Stack */
	.long	Reset_Handler         /* Reset Handler */
	.long	NMI_Handler           /* NMI Handler */
	.long	HardFault_Handler     /* Hard Fault Handler */
	.long	MemManage_Handler     /* MPU Fault Handler */
	.long	BusFault_Handler      /* Bus Fault Handler */
	.long	UsageFault_Handler    /* Usage Fault Handler */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	0                     /* Reserved */
	.long	SVC_Handler           /* SVCall Handler */
	.long	DebugMon_Handler      /* Debug Monitor Handler */
	.long	0                     /* Reserved */
	.long	PendSV_Handler        /* PendSV Handler */
	.long	SysTick_Handler       /* SysTick Handler */
	
	/* External interrupts */
	.long	STUB_WakeupIntHandler     /* 0:  Pad0 Wakeup  */
	.long	STUB_WakeupIntHandler     /* 1:  Pad1 Wakeup */
	.long	STUB_WakeupIntHandler     /* 2:  Pad2 Wakeup */
	.long	STUB_WakeupIntHandler     /* 3:  Pad3 Wakeup */
	.long	STUB_WakeupIntHandler     /* 4:  Pad4 Wakeup */
	.long	STUB_WakeupIntHandler     /* 5:  Pad5 Wakeup */
	.long	STUB_WakeupIntHandler     /* 6:  LPUART Wakeup */
	.long	STUB_WakeupIntHandler     /* 7:  LPUSB Wakeup */
	.long	PwrKey_WakeupIntHandler   /* 8:  PwrKey Wakeup */ 
	.long	ChrgPad_WakeupIntHandler  /* 9:  ChrgPad Wakeup */
	.long	STUB_WakeupIntHandler     /* 10:  RTC Wakeup */
	.long	USB_IRQ_Handler           /* 11:  USB Wakeup */
	.long	STUB_WakeupIntHandler     /* 12:  TMU Systk */
	.long	STUB_WakeupIntHandler     /* 13:  XIC IRQ */
	.long	STUB_WakeupIntHandler     /* 14:  XIC IRQ */
	.long	STUB_WakeupIntHandler     /* 15:  XIC IRQ */
	.long	STUB_WakeupIntHandler     /* 16:  XIC IRQ */

	.text
	.thumb
	.thumb_func
	.align	2
	.type	Reset_Handler, %function
Reset_Handler:
	ldr	 r0, =0x4f020190
	ldr	 r1, =0x00000000
	str	 r1, [r0]
	
	ldr	 r0, =0x4f020130
	ldr	 r1, =0x00000002
	str	 r1, [r0]
	
	.extern SystemInit
	.extern ec_main
	ldr r0, =SystemInit
	blx	r0
	ldr r0, =ec_main
	bx	r0
	.size  Reset_Handler, .-Reset_Handler

	.thumb
	.thumb_func
	.align	2
	.type	HardFault_Handler, %function
HardFault_Handler:
	B .
	.size  HardFault_Handler, .-HardFault_Handler
	
	.align	1
	.thumb
	.thumb_func
	.weak	Default_Handler
	.type	Default_Handler, %function
Default_Handler:
	b	.
	.size  Default_Handler, .-Default_Handler
	
	.macro	def_irq_handler	handler_name
	.weak	\handler_name
	.thumb_set	\handler_name, Default_Handler
	.endm

	def_irq_handler	NMI_Handler
	def_irq_handler	HardFault_Handler
	def_irq_handler	MemManage_Handler
	def_irq_handler	BusFault_Handler
	def_irq_handler	UsageFault_Handler
	def_irq_handler	SVC_Handler
	def_irq_handler	DebugMon_Handler
	def_irq_handler	PendSV_Handler
	def_irq_handler	SysTick_Handler
	def_irq_handler	STUB_WakeupIntHandler
	def_irq_handler	STUB_WakeupIntHandler
	def_irq_handler	STUB_WakeupIntHandler
	def_irq_handler	STUB_WakeupIntHandler
	def_irq_handler	STUB_WakeupIntHandler
	def_irq_handler	STUB_WakeupIntHandler
	def_irq_handler	STUB_WakeupIntHandler
	def_irq_handler STUB_WakeupIntHandler
	def_irq_handler	STUB_WakeupIntHandler
	def_irq_handler	PwrKey_WakeupIntHandler
	def_irq_handler ChrgPad_WakeupIntHandler
	def_irq_handler USB_IRQ_Handler
	def_irq_handler STUB_WakeupIntHandler
	def_irq_handler	STUB_WakeupIntHandler

	.end
