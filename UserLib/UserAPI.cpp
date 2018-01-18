#include "UserAPI.h"

//������ ƽ�� ���´�.
uint32_t GetTickCount()
{
	int address = 0;
	__asm
	{
		mov eax, 5
		int 0x80
		mov address, eax
	}
	return address;
}

void free(void *p)
{
	__asm {

		mov ebx, p
			mov eax, 3
			int 0x80
	}
}

u32int malloc(u32int sz)
{
	int address = 0;
	__asm {
		mov ebx, sz
			mov eax, 2
			int 0x80
			mov address, eax
	}

	return address;
}

//���� ����
void CreateHeap()
{
	__asm {

		mov eax, 4
		int 0x80
	}
}

//���μ��� ����
void TerminateProcess()
{
	__asm 
	{	
		mov eax, 1
		int 0x80
	}
}

//Ŀ�� �ܼ� ȭ�鿡 ���ڿ� ���
int printf(const char* szMsg)
{
	__asm 
	{		
		mov ebx, szMsg
		mov eax, 0
		int 0x80
	}
}

void *operator new(size_t size)
{
	return (void *)malloc(size);
}

void *operator new[](size_t size)
{
	return (void *)malloc(size);
}

void operator delete(void *p)
{
	free(p);
}

void operator delete(void *p, size_t size)
{
	free(p);
}

//���� ���� �Լ� ȣ�� ������ ���ؼ��� �ƹ��� ó���� ���� �ʴ´�.
int __cdecl _purecall()
{	
	return 0;
}

void operator delete[](void *p)
{
	free(p);
}