TSS_ENTRY	equ 0x05
global getCR2
getCR2:
	mov eax,cr2
	ret
	
global getCR3
getCR3:
	mov eax,cr3
	ret

global k_ltr
k_ltr:
	mov ax,TSS_ENTRY<<3
	ltr ax
	ret
	
global debug_hlt
debug_hlt:
	cli			;we must clear interrupt first.
	hlt			;otherwise hlt will also be interrupted by timer
	nop
	nop
	sti			;for debugging mode.
	ret
	
global enable
enable:
	sti
	ret

global disable
disable:
	cli
	ret

;***********************;
;*****比较垃圾的********;
;*****内核调用**********;
;***********************;
global sys_lock
sys_lock:
	int 0x81
	ret

global sys_unlock
sys_unlock:
	int 0x80
	ret

global debug_halt
debug_halt:
	int 0x82
	ret
	
global sys_wait
sys_wait:
	int 0x83
	ret
