; myboot.asm   bootstrap for real mode loader
;Aug,01,2005

; move image to 0x90000 first
; then move it to 0x100000 after init for PM
; make consistent for hdboot
; Feb,17,2007

%define IMAGE_SEG	0x9000
%if IMAGE_SEG & 31
  %error "IMAGE_SEG must be divisible by 0x20"
%endif

%define LOAD_BASE 0x100000
%define IMAGE_SIZE 51200

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
	mov	ds,ax	;set data segment base at 0x90000
; ds:esi -> es:edi
        mov     es, ax
move_mem:
        mov     esi, IMAGE_SEG << 4
        mov     edi, LOAD_BASE
        mov     ecx, IMAGE_SIZE >> 2
        cld
        rep movsd        

	jmp dword gdt_code_addr:LOAD_BASE ;0x10=10 000 b ,now CS was pointing to gdt[2]
	
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
	;功能描述： 读扇区 
	;入口参数： AH＝02H
	;AL＝扇区数
	;CH＝柱面
	;CL＝扇区
	;DH＝磁头
	;DL＝驱动器，00H~7FH：软盘；80H~0FFH：硬盘
	;ES:BX＝缓冲区的地址 
	;出口参数： 
	;CF＝0——操作成功，AH＝00H，AL＝传输的扇区数，否则，AH＝状态代码，参见功能号01H中的说明 
;===================================================================
;BIOS_sector_num = 1 + (DOS_sector_num MOD Sectors_per_track)
;BIOS_Head_num   = (DOS_sector_num DIV Sectors_per_track) MOD Total_heads
;BIOS_Track_num  = (DOS_sector_num DIV Sectors_per_track) DIV Total_heads

;Format   Size   Cyls   Heads  Sec/Trk   FATs   Sec/FAT   Sec/Root   Media
;1.2M     5 1/4   80      2      15       2       14        14        F9
;1.44M    3 1/2   80      2      18       2       18        14        F0

;BIOS INT13H
;AH = 02h
;AL = number of sectors to read (must be nonzero)
;CH = low eight bits of cylinder number
;CL = sector number 1-63 (bits 0-5) 
;     high two bits of cylinder (bits 6-7, hard disk only)
;DH = head number
;DL = drive number (bit 7 set for hard disk)
;ES:BX -> data buffer


read:
        mov     ax,loadmsg              ;Leaving boot
        mov     si,ax
        call    disps

        mov     di, 1
        mov     dh, 0
        mov     dl, 0
        mov     ch, 0
        mov     cl, 2  ; range from 1 to 18, sector 1 loaded by bios

        xor     bx, bx
        mov     ax, IMAGE_SEG
        mov     es, ax  ; es:bx -> data buffer
rp_read:
        mov     al, 1
        mov     ah, 2
        int     13h
        jc      error

        inc     di
        cmp     di, IMAGE_SIZE >> 9  ; how many sectors wanna read
        je      read_done
        add     bx, 512 ; 512B per sector
        inc     cl
        cmp     cl, 18  ; 18 sectors per track
        jle     rp_read
        mov     cl, 1

        xor     dh, 1   ; head 0/1
        jnz     rp_read
        inc     ch      ; cyls
        jmp     rp_read

read_done:
        ret


disp:
	mov	ah, 0x0e 	;打字机模式，告诉BIOS，把字符输出到屏幕上
	mov	bh ,0x00	;页码
	mov	bl,7		; 文字属性
	int	10h
	ret

;In:	DS:SI points to the string to be displayed.
;Out:	None
disps:
	mov	ax, 0x0e0D 	;打字机模式，告诉BIOS，把字符输出到屏幕上
	mov	bh ,0x00	;页码
	mov	bl,7		; 文字属性
.1	int	10h
	lodsb
	test	al,al
	jnz	.1
	ret
	
errmsg	db	10,"Error Executing bootsector",13
	db	10,"Press any key to reboot",13,10,0
loadmsg	db	10,"Loading kernel...",13,10,0
sucmsg	db	10,"Ready to jump to kernel",13,10,0
size	equ	$ - entry
%if size+11+2 > 512
  %error "code is too large for boot sector"
%endif
	times	(512 - size  - 2) db 0

	db	0x55, 0xAA		;2  byte boot signature
