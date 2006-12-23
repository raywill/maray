;	Name:	kinit.asm
;	Copyright: GPL
;	Author: Raywill
;	Date: 22-12-06
;	Description: this file does lots of things
;				 (1) Move kernel to a safe place
;				 (2) Setup GDT
;				 (3) Setup Paging
;				 (4) Setup Interrupt(IDT)
;				 (5) Setup TSS
;				 (6) Essential Component For Task Switching
;



;first we load kernel image to 0x90000,then we move it to 0x0000
;Move Kernel to 0x0000 , Set Correct segment ,Enable Paging ,Set IDT ,Jump to Main.

[global kernel_entry]
[global  setvect]
[global soft_int]
[global start_first_process]


[extern set_console]
[extern save_console]
[extern osmain]
[extern common_interrupt]
[extern timer_irq]
[extern current]
[extern last]
[extern tss]

	%define _PAGE_OFFSET 	0xC0000000		;3G
	%define LOAD_BASE			0x90000				;phy_addr where we load our kernel to
	%define V_P_OFFSET (_PAGE_OFFSET-LOAD_BASE)
	%define IMAGE_SIZE 0x7D000	;=500K ,the total size of our kernel
	%define START_ADDR 0x0000
[section .text]


kernel_entry:

start:
	cli

;Let's move our kernel to 0x0000,0000 for simplicity
;MOVSD For legacy mode, move dword from address DS:(E)SI to ES:(E)DI.
	mov ax,ds	;Segment BASE Adress=0x0000 0000 
	mov es,ax
move_mem:
	xor edi,edi
	mov esi,LOAD_BASE
	mov ecx, IMAGE_SIZE>>2	;4 bytes per instruction
	cld			;move forward
	rep movsd
	;Done!
	lgdt [gdt_addr-_PAGE_OFFSET]			;
	jmp gdt_code_addr:after_move-_PAGE_OFFSET
after_move:
;Load our local GDT
	lgdt [gdt_addr-_PAGE_OFFSET]			;
	jmp	gdt_code_addr:flush_gdt-_PAGE_OFFSET

;Reset data segment	
flush_gdt:
	mov ax,gdt_data_addr
	mov ds,ax		;seg value is in the loader
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	mov	ss,ax
	mov	esp,0xFFFFF

;Now,Going to enable paging :-)
fill_page_directory:
	mov	edi,page_directory-_PAGE_OFFSET
	mov	ecx,1024
	mov	eax,0

fill_zero_page:		;fill all page directory entries as NULL
	mov	[edi],eax
	add edi,4
	dec ecx
	cmp ecx,0
	jnz fill_zero_page
	
;At first I didnt do this and kernel crash again and again.
;Thanks to the fellows in Mega-tokyo (Now in OSDev)
identity_map_kernel:
	mov	dword [page_directory-_PAGE_OFFSET+(START_ADDR>>22)*4],temp_kernel_page_table-_PAGE_OFFSET+7	;seccond param is compile time determined

	mov eax,7
	mov ebx,0
fill_temp_page_table:	
	mov	dword [temp_kernel_page_table-_PAGE_OFFSET+ebx*4],	eax
	add eax,0x1000
	inc ebx
	cmp ebx,1023
	jnz fill_temp_page_table


fill_presented_page:
	mov	dword [page_directory-_PAGE_OFFSET+(_PAGE_OFFSET>>22)*4],kernel_page_table-_PAGE_OFFSET+7	;seccond param is compile time determined

	;mov eax,90007
	mov eax,7
	mov ebx,0
fill_page_table:	
	mov	dword [kernel_page_table-_PAGE_OFFSET+ebx*4],	eax
	add eax,0x1000
	inc ebx
	cmp ebx,1023
	jnz fill_page_table
	
	mov eax,page_directory-_PAGE_OFFSET
	mov	cr3,eax

;Doing nothing here
;	hlt
;	nop
;	nop
	
enable_paging:
	mov	eax,cr0
	or	eax,1<<31
	mov	cr0,eax
	jmp flush_page0
flush_page0:
	jmp gdt_code_addr:flush_page
;ret can also flush the pre-frech queue :)
flush_page:
reset_data_seg:
	mov	ax,gdt_data_addr		;data segment descriptor
	mov	ds,ax
	mov	es,ax
	mov	fs,ax
	mov	gs,ax
	mov	ss,ax
	mov	esp,0x90000
	mov	dword [kernel_stack],0x90000		;save kernel esp
	
;install the irq handler
	mov ecx,(idt_end - idt) >> 3 ; number of exception handlers
	mov edi,idt
	mov esi,isr0		;first handler
;Default to be ring 0,interrupt gate.
do_idt:
	mov eax,esi				; EAX=offset of entry point
	mov [edi],ax			; set low 16 bits of gate offset
	shr eax,16				; get high 16 of the handler address
	mov [edi + 6],ax	; set high 16 bits of gate offset
	add edi,8					; 8 bytes/interrupt gate
	add esi,(isr1 - isr0)		; bytes/stub
	dec ecx
	jnz do_idt

;Override IDT[20],which control the timer.
;Reinstall timer_isr.It is different from other ISR
;I need to do something special in it :)
;Default to be ring 0,interrupt gate.So just tell IDT the isr offset.
	mov edi,idt+20h*8
	mov esi,timer_isr		;first handler
	mov eax,esi				; EAX=offset of entry point
	mov [edi],ax			; set low 16 bits of gate offset
	shr eax,16				; get high 16 of the handler address
	mov [edi + 6],ax	; set high 16 bits of gate offset


	lidt	[idt_addr-_PAGE_OFFSET]	;load null interrupt describe table

setup_TSS_Selector:
	mov eax,tss-_PAGE_OFFSET
	mov word [TR1_seg_sel+2],ax
	shr eax,16
	mov byte [TR1_seg_sel+4],al
	shr eax,8
	mov byte [TR1_seg_sel+7],al
	
;	mov	eax,TR1_seg_addr
;	ltr ax
;Everything have done!Let's go!
	;sti(we can do this in C,after everything initialized)
	call	osmain
hang:
	jmp hang


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;here is the gdt data for the kernel;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
align 8			;can fast the speed
gdt:
 gdt_null:
	dd	0
	dd	0
 gdt_data_addr	equ $-gdt
 gdt_data:
 	dw	0xffff
	dw	0
	db	0
	db	10010010b	;(7)seg exist,(6-5)privileg,(4)0:sys seg,1:data or code seg,(3-0)seg attr,2:data seg,read and write.
	db	11001111b	;(7)limit len,0:1 byte,1:4K,(6)B bit,control stack,B=1,use esp,B=0,use sp,(5-4) reserved,(3-0)seg limit 19-16.
	db	0
 gdt_code_addr	equ $-gdt
 gdt_code:
 	dw	0xffff
	dw	0
	db	0
	db	10011010b	;(7)seg exist,(6-5)privileg,(4)0:sys seg,1:data or code seg,(3-0)seg attr,10:code seg,execute and read.
	db	11001111b
	db	0

 user_gdt_data_addr	equ $-gdt
 user_gdt_data:
 	dw	0xffff
	dw	0
	db	0
	db	11110010b	;(7)seg exist,(6-5)privileg,(4)0:sys seg,1:data or code seg,(3-0)seg attr,2:data seg,read and write.
	db	11001111b	;(7)limit len,0:1 byte,1:4K,(6)B bit,control stack,B=1,use esp,B=0,use sp,(5-4) reserved,(3-0)seg limit 19-16.
	db	0
 user_gdt_code_addr	equ $-gdt
 user_gdt_code:
 	dw	0xffff
	dw	0
	db	0
	db	11111010b	;(7)seg exist,(6-5)privileg,(4)0:sys seg,1:data or code seg,(3-0)seg attr,10:code seg,execute and read.
	db	11001111b
	db	0

TR1_seg_addr	equ $-gdt
 TR1_seg_sel:
 	dw	0x67
	dw	0xffff
	db	0xff
	db	10001001b	;(7)seg exist,(6-5)privileg,(4)0:sys seg,1:data or code seg,(3-0)seg attr,10:code seg,execute and read.
	db	0
	db	0xff
 
	gdt_end:
 gdt_addr:
 	dw	gdt_end - gdt - 1	;gdt total length,here it contains 3 item
	dd	gdt-_PAGE_OFFSET	;gdt start address
	
;set null interrupt 
idt:
%rep 256
	dw 0				; offset 15:0
	dw gdt_code_addr		; selector
	db 0				; (always 0 for interrupt gates)
	db 8Eh			; present,ring 0,'386 interrupt gate
	dw 0				; offset 31:16
%endrep
idt_end:
 idt_addr:
	dw idt_end - idt - 1		; IDT limit
	dd idt-_PAGE_OFFSET				; linear adr of IDT

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
%macro PUSHB 1
	db 6Ah
	db %1
%endmacro

%macro INTR 1				; (byte offset from start of stub)
isr%1:
	push byte 0			; ( 0) fake error code
	PUSHB %1			; ( 2) exception number
	push gs				; ( 4) push segment registers
	push fs				; ( 6)
	push es				; ( 8)
	push ds				; ( 9)
	pusha				; (10) push GP registers
		mov ax,gdt_data_addr	; (11) put known-good values...
		mov ds,eax		; (15) ...in segment registers
		mov es,eax		; (17)
		mov fs,eax		; (19)
		mov gs,eax		; (21)
		mov eax,esp		; (23)
		push eax		; (25) push pointer to regs_t
.1:
; setvect() changes the operand of the CALL instruction at run-time,
; so we need its location = 27 bytes from start of stub. We also want
; the CALL to use absolute addressing instead of EIP-relative, so:
;		hlt
;		nop
;		nop
			mov eax,common_interrupt; (26)
			call eax	; (31)

		pop eax
		popa				; pop GP registers
		pop ds				; pop segment registers
		pop es
		pop fs
		pop gs
		
		nop
		nop

			
	add esp,8			; drop exception number and error code
iret
%endmacro				; (38)

%macro INTR_EC 1
isr%1:
	nop				; error code already pushed
	nop				; nop+nop=same length as push byte
	PUSHB %1			; ( 2) exception number
	push gs				; ( 4) push segment registers
	push fs				; ( 6)
	push es				; ( 8)
	push ds				; ( 9)
	pusha				; (10) push GP registers
		mov ax,gdt_data_addr	; (11) put known-good values...
		mov ds,eax		; (15) ...in segment registers
		mov es,eax		; (17)
		mov fs,eax		; (19)
		mov gs,eax		; (21)
		mov eax,esp		; (23)
		push eax		; (25) push pointer to regs_t
.1:
; setvect() changes the operand of the CALL instruction at run-time,
; so we need its location = 27 bytes from start of stub. We also want
; the CALL to use absolute addressing instead of EIP-relative, so:

;	hlt
;	nop
;	nop

			mov eax,common_interrupt	; (26)
			call eax	; (31)

		pop eax
		popa				; pop GP registers
		pop ds				; pop segment registers
		pop es
		pop fs
		pop gs
		
		nop
		nop

	add esp,8			; drop exception number and error code
iret
%endmacro				; (38)

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; interrupt/exception stubs
; *** CAUTION: these must be consecutive, and must all be the same size.
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	INTR 0		; zero divide (fault)
	INTR 1		; debug/single step
	INTR 2		; non-maskable interrupt (trap)
	INTR 3		; INT3 (trap)
	INTR 4		; INTO (trap)
	INTR 5		; BOUND (fault)
	INTR 6		; invalid opcode (fault)
	INTR 7		; coprocessor not available (fault)
	INTR_EC 8	; double fault (abort w/ error code)
	INTR 9		; coproc segment overrun (abort; 386/486SX only)
	INTR_EC 0Ah	; bad TSS (fault w/ error code)
	INTR_EC 0Bh	; segment not present (fault w/ error code)
	INTR_EC 0Ch	; stack fault (fault w/ error code)
	INTR_EC 0Dh	; GPF (fault w/ error code)
	INTR_EC 0Eh	; page fault
	INTR 0Fh	; reserved
	INTR 10h	; FP exception/coprocessor error (trap)
	INTR 11h	; alignment check (trap; 486+ only)
	INTR 12h	; machine check (Pentium+ only)
	INTR 13h
	INTR 14h
	INTR 15h
	INTR 16h
	INTR 17h
	INTR 18h
	INTR 19h
	INTR 1Ah
	INTR 1Bh
	INTR 1Ch
	INTR 1Dh
	INTR 1Eh
	INTR 1Fh

; isr20 through isr2F are hardware interrupts. The 8259 programmable
; interrupt controller (PIC) chips must be reprogrammed to make these work.


; INTR 20 is dealed with specially.
;	INTR 20h	; IRQ 0/timer interrupt
	INTR 21h	; IRQ 1/keyboard interrupt
	INTR 22h
	INTR 23h
	INTR 24h
	INTR 25h
	INTR 26h	; IRQ 6/floppy interrupt
	INTR 27h
	INTR 28h	; IRQ 8/real-time clock interrupt
	INTR 29h
	INTR 2Ah
	INTR 2Bh
	INTR 2Ch
	INTR 2Dh	; IRQ 13/math coprocessor interrupt
	INTR 2Eh	; IRQ 14/primary ATA ("IDE") drive interrupt
	INTR 2Fh	; IRQ 15/secondary ATA drive interrupt

; syscall software interrupt
	INTR 30h

; the other 207 vectors are undefined

%assign i 31h
%rep (0FFh - 30h)

	INTR i

%assign i (i + 1)
%endrep

timer_isr:
;For more infomation ,see /root/man/

	push gs				; ( 4) push segment registers
	push fs				; ( 6)
	push es				; ( 8)
	push ds				; ( 9)
	pusha					; (10) push GP registers
								;`EAX, ECX, EDX, EBX, EBP, ESP (original value), EBP, ESI,EDI

	;now switch the stack to kernel stack
;	mov eax,[current+4]	;	save esp	,_current is an pointer
;	mov	[eax],esp				; move esp to address indecated by eax
;	mov	ebx,esp
;	mov	eax,[kernel_stack]
;	mov esp,eax					;use kernel stack in irq_handler
;	push ebx
	
		mov ax,gdt_data_addr	; (11) put known-good values...
		mov ds,eax		; (15) ...in segment registers
		mov es,eax		; (17)
		mov fs,eax		; (19)
		mov gs,eax		; (21)

;		mov eax,esp		;isr,please do not ocurrpt my stack.^__^
;		push eax	
		
;		mov eax,[_current]
;		mov	esp,[eax+8]		;use task's kstack
		
			mov eax,timer_irq
			call eax		;Current tast changed , using RB method.
		
;		pop eax
;		mov esp,eax
;current process changed? If no,
;Jump to no_switch label
		mov	eax,[last]
		cmp eax,[current]
		jz no_switch			;if the same task,do less work.


;stack changed ,so we should save the old stack .
;next time we need to use it!
;otherwise,the task will be restart when it is rescheduled.
		mov	eax,[last]
		mov [eax],ss						;We had better use LSS instruction
		mov	[eax+4],esp					;use current process's stack.This process is selected in schedule.		

		; push last
		; call save_console
		; add	esp,4

		; push current
		; call set_console
		; add esp,4



		
		mov eax,[current]
		mov [last],eax

ret_from_sys_call:
;Note:who is responsible for updating _current's ss and esp?
;***********************************************************
;We do not updating esp's content!
;BUT,content in the stack that esp pointing to is updated.

		mov	eax,[current]
		mov ss,[eax]						;We had better use LSS instruction
		mov	esp,[eax+4]					;use current process's stack.This process is selected in schedule.

;该死!问题就出在这里!!!!这里每次执行后esp总会长大~~~TMD!
		;update process's kernel stack in TSS
		mov	ebx, tss
;5*4+9*4+4*4= 18*4
;		mov eax,esp
;		add eax,18*4
;		mov	[ebx+4],eax		;store esp
;		mov	eax,ss
;		mov	[ebx+8],eax		;store ss
;Let's try something new:
;		mov	eax,[ current]		;this have been done.
		mov edx,[eax+2*4]	;esp0
		mov	[ebx+4],edx
		mov eax,ss
		mov [ebx+8],eax
		
no_switch:
		popa					; pop GP registers
		pop ds				; pop segment registers
		pop es
		pop fs
		pop gs

iret

 start_first_process:
	add esp,4				;drop out return address of start_first_process() call.
;	mov eax,[ current+4]
;	mov [ last],eax
;we have set  last,_current,TSS correctly in C code before we call start_first_process
	sti											;now setup interrupt flag.
	jmp ret_from_sys_call		;What if interrupt ocurr before we jump?Get ready for it?YES!
	
 soft_int:
	push ebp
		mov  ebp,esp
		push eax
		 mov eax,[ebp+8]
		 ;int ax
		 int 0x20
		pop eax
		pop ebp
	ret

 setvect:
	push ebp
		mov ebp,esp
		push esi
		push ebx
			mov esi,[ebp + 8]

; store accessdbyte in IDT[i]
			mov eax,[esi + 0]
			xor ebx,ebx
			mov bl,[ebp + 12]
			cmp	bl,20h	;I do not want you to reinstall timer_isr
			jz	no_timer_isr		;just return
			shl ebx,3									;ebx+5+idt=accessdbyte addr
			mov [idt + ebx + 5],al		;in al,8Eh=Present Ring0 386mode

; store handler address in stub
			mov eax,isr1
			sub eax,isr0	; assume stub size < 256 bytes
			mul byte [ebp + 12]				;which intr? 0x20 like so
			mov ebx,eax								;because the mul source is byte(al),the result is (AX)
			add ebx,isr0							;ebx=Delta_dis*No+Start
			mov eax,[esi + 4]					;[esi+4]=eip
			mov [ebx + (isr0.1 - isr0 + 1)],eax

no_timer_isr:
		pop ebx
		pop esi
	pop ebp
	ret





align 0x1000
GLOBAL  page_directory
	 page_directory:
		times	0x1000 db 0
	
align 0x1000
GLOBAL  kernel_page_table	
	 kernel_page_table:
		times	0x1000 db 0
		
;this is used for indentity-mapping the kernel
align 0x1000
GLOBAL  temp_kernel_page_table	
	 temp_kernel_page_table:
		times	0x1000 db 0
		

[section .data]
kernel_stack dd 0		;save kernel esp
[section .bss]
knrl_stack:

; vim: syntax=nasm ts=8 st=4
