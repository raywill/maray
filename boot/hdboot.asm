; myboot.asm   bootstrap for real mode loader
;Aug,01,2005

%define IMAGE_SEG	0x9000
%define TOTAL_SECTOR	90
%if IMAGE_SEG & 31
  %error "IMAGE_SEG must be divisible by 0x20"
%endif
[org	0x7C00]
[bits 16]
entry:
	jmp	short begin
	;reserved as my own param area

gdt:
 gdt_null:
	dd	0
	dd	0
 gdt_data_addr	equ $-gdt
 gdt_data:
 	dw	0xffff
	dw	0
	db	0				;we got baseaddr=0x90000
	db	10010010b	;(7)seg exist,(6-5)privileg,(4)0:sys seg,1:data or code seg,(3-0)seg attr,2:data seg,read and write.
	db	11001111b	;(7)limit len,0:1 byte,1:4K,(6)B bit,control stack,B=1,use esp,B=0,use sp,(5-4) reserved,(3-0)seg limit 19-16.
	db	0
 gdt_code_addr	equ $-gdt
 gdt_code:
 	dw	0xffff
	dw	0
	db	0			;we got baseaddr=0x90000
	db	10011010b	;(7)seg exist,(6-5)privileg,(4)0:sys seg,1:data or code seg,(3-0)seg attr,10:code seg,execute and read.
	db	11001111b
	db	0
gdt_end:
 gdt_addr:
 	dw	gdt_end - gdt - 1	;gdt total length,here it contains 3 item
	dd	gdt		;gdt start address

head:	db 0
sector:	db 0
cyl:    db 0

begin:
	xor	ax, ax
	mov	ds, ax
	cli
	mov	ss, ax
	mov	sp, 0x7C00
	mov	bp, sp
	call	read
eof:
	mov	ax,sucmsg		;Leaving boot
	mov	si,ax
	call	disps
	
enter_pm:
	lgdt	[gdt_addr]
open_A20:
  in      al,92h
  or      al,00000010b
  out     92h,al
set_PM_bit:
	mov	eax,cr0
	or	eax,1
	mov	cr0,eax
	
	jmp gdt_code_addr:mode_32
[bits 32]
mode_32:
	mov	ax,gdt_data_addr
	mov	ds,ax							;set data segment base at 0x90000
	jmp dword gdt_code_addr:0x90000	;0x10=10 000 b ,now CS was pointing to gdt[2]
	
;	jmp	IMAGE_SEG:0		;jump to 0x9000
;===================================================================
[bits 16]
error:
	add	al,'0'
	call	disp
	mov	ax,errmsg		;Leaving boot
	mov	si,ax
	call	disps
	
	xor	ah, ah
	int	16h		;Wait for a key
	int	19h		;Try to reboot
;===================================================================
	;���������� ������ 
	;���ڲ����� AH��02H
	;AL��������
	;CH��?���
	;CL������
	;DH����ͷ
	;DL����������00H~7FH�����̣�80H~0FFH��Ӳ��
	;ES:BX���������ĵ�ַ 
	;���ڲ����� 
	;CF��0���������ɹ���AH��00H��AL����������������������AH��״̬���룬�μ����ܺ�01H�е�˵�� 
;===================================================================
read:	
	push	si
	mov	ax,loadmsg		;Leaving boot
	mov	si,ax
	call	disps
	pop	si

; we should get disk info first
; 0) boot from hd or fd
; 1) int 13/ah=08h, driver parameters

	mov	dx, [si]
; si point to the partition entry
; [si] should be 0x0(fd) or 0x80(hd)
	xor	ax, ax
	mov	es, ax
	mov	ax, 0x0800
	xor	di, di
; according to manual, it guard against BIOS bugs
	int	0x13
	jc	error

	mov	[head], dh
	and	cl, 0x3f ; 0b00111111  mask high two bits
	;mov	[sector], cl
	;mov	[cyl], ch  ; not used
	mov	[sector], cx ; in a time

; only deal with hd for convenience
start_read:
	mov	dx, [si]
	mov	cx, [si+2]
	xor	di, di
	xor	bx, bx
	mov	ax, IMAGE_SEG
	mov	es, ax  ; es:bx -> data buffer
update:
	inc	di
	cmp	di, TOTAL_SECTOR
	je	read_done
	inc	cl
	cmp	cl, [sector]
	jle	do_read
	mov	cl, 1
	inc	dh
	cmp	dh, [head]
	jle	do_read
	xor	dh, dh
	inc	ch
do_read:
	mov	ax, 0x0201
	int	13h
	jc	error
	add	bx, 512
	jmp	update

read_done:
	ret

disp:
	mov	ah, 0x0e 	;���ֻ�ģʽ������BIOS�����ַ���������Ļ��
	mov	bh ,0x00	;ҳ��
	mov	bl,7		; ��������
	int	10h
	ret

;In:	DS:SI points to the string to be displayed.
;Out:	None
disps:
	mov	ax, 0x0e0D 	;���ֻ�ģʽ������BIOS�����ַ���������Ļ��
	mov	bh ,0x00	;ҳ��
	mov	bl,7		; ��������
.1	int	10h
	lodsb
	test	al,al
	jnz	.1
	ret
	
errmsg	db	10,"Error Executing bootsector",13
	db	10,"Press any key to reboot",13,10,0
loadmsg	db	10,"Maray:Loading kernel...",13,10,0
sucmsg	db	10,"Ready to jump to kernel",13,10,0
size	equ	$ - entry
%if size+11+2 > 512
  %error "code is too large for boot sector"
%endif
	times	(512 - size  - 2) db 0

	db	0x55, 0xAA		;2  byte boot signature

