[BITS 64]
extern kmain64
SECTION .text
global main64
;��Ŀ ��ũ���Ϳ� ���� 2MB ������ �ε�Ǵ� �ڵ�
main64:
	
	mov ax, 0x10
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