#pragma once
#include "windef.h"
#include "SkyStruct.h"
#include "Hal.h"
#include "PIT.h"

#define ASSERT(a, b) if(a == false) SkyConsole::Print("Kernel Panic : %s\n", b); _asm hlt
#define kprintf SkyConsole::Print

/////////////////////////////////////////////////////////////////////////////
//����ȭ
/////////////////////////////////////////////////////////////////////////////
typedef struct _CRITICAL_SECTION {
	
	LONG LockRecursionCount;
	HANDLE OwningThread;        // from the thread's ClientId->UniqueThread
	      
} CRITICAL_SECTION, *LPCRITICAL_SECTION;;

extern CRITICAL_SECTION g_criticalSection;

void SKYAPI kEnterCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void SKYAPI kInitializeCriticalSection(LPCRITICAL_SECTION lpCriticalSection);
void SKYAPI kLeaveCriticalSection(LPCRITICAL_SECTION lpCriticalSection);

/////////////////////////////////////////////////////////////////////////////
//������
/////////////////////////////////////////////////////////////////////////////
DWORD SKYAPI kGetCurrentThreadId();

void GetLocalTime(LPSYSTEMTIME lpSystemTime);
BYTE SetLocalTime(LPSYSTEMTIME lpSystemTime);

void printf(const char* str, ...);
void ksleep(int millisecond);
HANDLE CreateThread(SIZE_T dwStackSize, LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, DWORD dwCreateionFlags, LPDWORD lpThreadId);
void PauseSystem(const char* msg);