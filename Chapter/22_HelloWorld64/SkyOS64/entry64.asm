[BITS 64]
extern kmain64
SECTION .text
global main64, gdt64
;��Ŀ ��ũ���Ϳ� ���� 2MB ������ �ε�Ǵ� �ڵ�
main64:
	
	mov ax, 10h
	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax
	mov ss,  ax
	
	;���� �ʱ�ȭ	
	mov rbp, 0x1000f0
	mov rsp, 0x1000f0
	
	call kmain64
	
	jmp $

gdt64:
	lgdt[0x142000]
	ret
SECTION .DATA
ads: dd 0x12345678